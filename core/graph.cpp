#include "../include/graph.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void init_graph(Graph *g) { g->count = 0; }

void free_graph(Graph *g) {
    if (!g) return;

    for (int i = 0; i < g->count; i++) {
        EdgeNode *cur = g->nodes[i].first_edge;
        while (cur) {
            EdgeNode *next = cur->next;
            free(cur);
            cur = next;
        }
        g->nodes[i].first_edge = NULL;
    }
    g->count = 0;
}

int get_or_create_node(Graph *g, const char *ip) {
    for (int i = 0; i < g->count; i++) {
        if (strcmp(g->nodes[i].ip, ip) == 0) return i;
    }

    if (g->count >= MAX_NODES) return -1;

    strcpy(g->nodes[g->count].ip, ip);
    g->nodes[g->count].first_edge = NULL;
    g->nodes[g->count].in_total = 0;
    g->nodes[g->count].out_total = 0;
    g->nodes[g->count].out_degree = 0;
    return g->count++;
}

void add_session(Graph *g, char *src, char *dst, int proto, int src_port, int dst_port, long bytes, double duration) {
    (void)src_port; // 当前未用
    int u = get_or_create_node(g, src);
    int v = get_or_create_node(g, dst);
    if (u < 0 || v < 0) return;

    g->nodes[u].out_total += bytes;
    g->nodes[v].in_total += bytes;

    EdgeNode *p = g->nodes[u].first_edge;
    while (p) {
        if (p->dest_idx == v) {
            p->total_bytes += bytes;
            p->duration += duration;

            if (proto == 6) {
                p->stats.tcp_bytes += bytes;
                p->stats.tcp_duration += duration;
                if (dst_port == 443) p->stats.https_bytes += bytes;
            } else if (proto == 17) {
                p->stats.udp_bytes += bytes;
                p->stats.udp_duration += duration;
            } else if (proto == 1) {
                p->stats.icmp_bytes += bytes;
                p->stats.icmp_duration += duration;
            }
            return;
        }
        p = p->next;
    }

    EdgeNode *new_e = (EdgeNode *)malloc(sizeof(EdgeNode));
    if (!new_e) return;
    memset(new_e, 0, sizeof(EdgeNode));

    new_e->dest_idx = v;
    new_e->total_bytes = bytes;
    new_e->duration = duration;

    if (proto == 6) {
        new_e->stats.tcp_bytes = bytes;
        new_e->stats.tcp_duration = duration;
        if (dst_port == 443) new_e->stats.https_bytes = bytes;
    } else if (proto == 17) {
        new_e->stats.udp_bytes = bytes;
        new_e->stats.udp_duration = duration;
    } else if (proto == 1) {
        new_e->stats.icmp_bytes = bytes;
        new_e->stats.icmp_duration = duration;
    }

    new_e->next = g->nodes[u].first_edge;
    g->nodes[u].first_edge = new_e;
    g->nodes[u].out_degree++;
}

// ==========================================
// 并查集 (DSU) 与子图提取模块
// ==========================================

// 1. 并查集查找函数（带路径压缩优化）
static int dsu_find(int *parent, int i) {
    if (parent[i] == i)
        return i;
    return parent[i] = dsu_find(parent, parent[i]);
}

// 2. 并查集合并函数
static void dsu_union(int *parent, int i, int j) {
    int root_i = dsu_find(parent, i);
    int root_j = dsu_find(parent, j);
    if (root_i != root_j) {
        parent[root_i] = root_j; // 合并两个连通分量
    }
}

// 3. 提取连通子图并导出为 CSV
void extract_subgraph_by_ip(Graph *g, const char *target_ip) {
    int target_idx = -1;
    // 寻找目标IP在图中的索引
    for (int i = 0; i < g->count; i++) {
        if (strcmp(g->nodes[i].ip, target_ip) == 0) {
            target_idx = i;
            break;
        }
    }

    if (target_idx == -1) {
        printf("[-] 未在图结构中找到节点: %s\n", target_ip);
        return;
    }

    // 初始化并查集：每个节点的父节点最初都是自己
    int parent[MAX_NODES];
    for (int i = 0; i < g->count; i++) {
        parent[i] = i;
    }

    // 遍历图中的所有边，将有通信关系的节点合并到同一个集合（无向连通分量）
    for (int i = 0; i < g->count; i++) {
        for (EdgeNode *e = g->nodes[i].first_edge; e; e = e->next) {
            dsu_union(parent, i, e->dest_idx);
        }
    }

    // 找到目标节点所属的集合根节点
    int target_root = dsu_find(parent, target_idx);

    // 将属于该连通分量的所有边导出到专用的子图文件
    FILE *fp = fopen("data/subgraph_edges.csv", "w");
    if (!fp) {
        printf("[-] 无法创建子图数据文件。\n");
        return;
    }

    // 写入表头，我们直接写入汇总好的数据，减轻 Python 端的压力
    fprintf(fp, "Source,Destination,TotalBytes,HttpsBytes,Duration,SourceRisk,DestRisk\n");

    int edge_count = 0;
    int node_count = 0;

    // 再次遍历全图，如果节点的根节点是 target_root，说明在同一个子图里
    for (int i = 0; i < g->count; i++) {
        if (dsu_find(parent, i) == target_root) {
            node_count++;
            for (EdgeNode *e = g->nodes[i].first_edge; e; e = e->next) {
                fprintf(fp, "%s,%s,%ld,%ld,%.3f,%d,%d\n",
                        g->nodes[i].ip,
                        g->nodes[e->dest_idx].ip,
                        e->total_bytes,
                        e->stats.https_bytes,
                        e->duration,
                        g->nodes[i].risk_score,      // 源节点风险分
                        g->nodes[e->dest_idx].risk_score // 目的节点风险分
                        );
                edge_count++;
            }
        }
    }
    fclose(fp);
    printf("[+] 并查集划分完成！\n");
    printf("    中心节点 [%s] 所在的连通子图共包含 %d 个节点，%d 条边。\n", target_ip, node_count, edge_count);
    printf("    已导出至: data/subgraph_edges.csv\n");
}


void compute_all_risk_scores(Graph *g) {
    if (g->count == 0) return;

    // 1. 初始化分数
    for (int i = 0; i < g->count; i++) {
        g->nodes[i].risk_score = 0;
    }

    // 计算全网平均流量作为基准
    long total_net_traffic = 0;
    for (int i = 0; i < g->count; i++) total_net_traffic += (g->nodes[i].in_total + g->nodes[i].out_total);
    long avg_traffic = total_net_traffic / g->count;

    for (int i = 0; i < g->count; i++) {
        int score = 0;
        double out_ratio = (g->nodes[i].out_total + g->nodes[i].in_total > 0) ?
                           (double)g->nodes[i].out_total / (g->nodes[i].out_total + g->nodes[i].in_total) : 0;

        // --- 维度 1：扫描行为（最严重的安全威胁） ---
        if (g->nodes[i].out_degree > 50 && out_ratio > 0.95) {
            score += 75; // 确定性扫描源，直接变红
        } else if (g->nodes[i].out_degree > 20 && out_ratio > 0.7) {
            score += 40; // 疑似扫描行为
        }

        // --- 维度 2：数据泄露风险（异常外发） ---
        // 外发流量超过 1MB 且 外发比例超过入向 10 倍
        if (g->nodes[i].out_total > 1048576 && out_ratio > 0.9) {
            score += 50;
        }

        // --- 维度 3：拓扑重要性（攻击影响面） ---
        // 复用之前的邻居统计逻辑
        int neighbor_count = 0;
        bool neighbor[MAX_NODES] = {false};
        for (EdgeNode *e = g->nodes[i].first_edge; e; e = e->next) {
            if (!neighbor[e->dest_idx]) { neighbor[e->dest_idx] = true; neighbor_count++; }
        }
        if (neighbor_count > 30) {
            score += 35; // 核心节点，一旦失陷影响极大
        }

        // --- 维度 4：异常流量大户 ---
        if ((g->nodes[i].in_total + g->nodes[i].out_total) > avg_traffic * 10) {
            score += 20;
        }

        // 最终得分封顶 100
        g->nodes[i].risk_score = (score > 100) ? 100 : score;
    }

    printf("[报告] 多维风险建模完成：已识别风险节点并更新评分矩阵。\n");
    fflush(stdout);
}
