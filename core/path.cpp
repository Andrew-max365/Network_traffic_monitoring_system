#include "../include/path.h"
#include <stdio.h>
#include <float.h>
#include <string.h>

#define INF DBL_MAX

// ==========================================
// 全局静态变量：用于暂存最后一次查询的路径，以便进行对比
// ==========================================
static int g_parent_hop[MAX_NODES];
static int g_parent_cong[MAX_NODES];
static bool g_found_hop = false;
static bool g_found_cong = false;

static bool is_valid_node(Graph *g, int idx) {
    return idx >= 0 && idx < g->count;
}

static void print_path(Graph *g, int *parent, int end) {
    int path[MAX_NODES];
    int len = 0;

    for (int cur = end; cur != -1 && len < MAX_NODES; cur = parent[cur]) {
        path[len++] = cur;
    }

    for (int i = len - 1; i >= 0; --i) {
        printf("%s%s", g->nodes[path[i]].ip, (i == 0) ? "\n" : " -> ");
    }
}

// 内部辅助：将 parent 数组转换为顺序的路径序列以便比对
static int get_path_sequence(int *parent, int end, int *seq) {
    int temp[MAX_NODES];
    int len = 0;
    for (int cur = end; cur != -1 && len < MAX_NODES; cur = parent[cur]) {
        temp[len++] = cur;
    }
    for (int i = 0; i < len; i++) {
        seq[i] = temp[len - 1 - i];
    }
    return len;
}

// 内部辅助：比较两条路径序列是否完全一致
static bool are_paths_equal(int *seq1, int len1, int *seq2, int len2) {
    if (len1 != len2) return false;
    for (int i = 0; i < len1; i++) {
        if (seq1[i] != seq2[i]) return false;
    }
    return true;
}

double get_edge_congestion(EdgeNode *e) {
    if (!e || e->duration <= 0) return INF;
    return (double)e->total_bytes / e->duration;
}

void find_min_hop_path(Graph *g, int start, int end) {
    g_found_hop = false;
    if (!is_valid_node(g, start) || !is_valid_node(g, end)) {
        printf("节点索引无效。\n");
        return;
    }

    int *parent = g_parent_hop; // 使用静态数组暂存结果
    int dist[MAX_NODES];
    int queue[MAX_NODES], head = 0, tail = 0;

    for (int i = 0; i < g->count; i++) {
        dist[i] = -1;
        parent[i] = -1;
    }

    dist[start] = 0;
    queue[tail++] = start;

    while (head < tail) {
        int u = queue[head++];
        if (u == end) break;

        for (EdgeNode *e = g->nodes[u].first_edge; e; e = e->next) {
            if (dist[e->dest_idx] == -1) {
                dist[e->dest_idx] = dist[u] + 1;
                parent[e->dest_idx] = u;
                queue[tail++] = e->dest_idx;
            }
        }
    }

    if (dist[end] == -1) {
        printf("未找到最小跳数路径。\n");
    } else {
        printf("最小跳数路径 (跳数: %d): ", dist[end]);
        print_path(g, parent, end);
        g_found_hop = true;
    }
}

void find_min_congestion_path(Graph *g, int start, int end) {
    g_found_cong = false;
    if (!is_valid_node(g, start) || !is_valid_node(g, end)) {
        printf("节点索引无效。\n");
        return;
    }

    double dist[MAX_NODES];
    int *parent = g_parent_cong; // 使用静态数组暂存结果
    bool visited[MAX_NODES];

    for (int i = 0; i < g->count; i++) {
        dist[i] = INF;
        parent[i] = -1;
        visited[i] = false;
    }

    dist[start] = 0;

    for (int i = 0; i < g->count; i++) {
        int u = -1;
        double min_d = INF;
        for (int j = 0; j < g->count; j++) {
            if (!visited[j] && dist[j] < min_d) {
                min_d = dist[j];
                u = j;
            }
        }

        if (u == -1 || u == end) break;
        visited[u] = true;

        for (EdgeNode *e = g->nodes[u].first_edge; e; e = e->next) {
            double congestion = get_edge_congestion(e);
            if (congestion == INF) continue;
            if (dist[u] + congestion < dist[e->dest_idx]) {
                dist[e->dest_idx] = dist[u] + congestion;
                parent[e->dest_idx] = u;
            }
        }
    }

    if (dist[end] == INF) {
        printf("未找到最小拥塞路径。\n");
    } else {
        printf("最小拥塞路径 (代价: %.2f): ", dist[end]);
        print_path(g, parent, end);
        g_found_cong = true;
    }
}

void print_path_result(Graph *g) {
    char start_ip[MAX_IP_LEN];
    char end_ip[MAX_IP_LEN];
    int start_idx = -1;
    int end_idx = -1;

    // 使用 %s 读取字符串（IP 地址）
    if (scanf("%s %s", start_ip, end_ip) == 2) {

        // 在图中查找这两个 IP 对应的索引
        for (int i = 0; i < g->count; i++) {
            if (strcmp(g->nodes[i].ip, start_ip) == 0) {
                start_idx = i;
            }
            if (strcmp(g->nodes[i].ip, end_ip) == 0) {
                end_idx = i;
            }
        }

        // 检查是否都找到了对应的节点
        if (start_idx != -1 && end_idx != -1) {
            // 1. 分别计算两条路径（它们会自动打印，并将路径存入静态变量）
            find_min_hop_path(g, start_idx, end_idx);
            find_min_congestion_path(g, start_idx, end_idx);

            // 2. 只有两条路径都成功找到时，才进行对比
            if (g_found_hop && g_found_cong) {
                int seq_hop[MAX_NODES], seq_cong[MAX_NODES];
                int len_hop = get_path_sequence(g_parent_hop, end_idx, seq_hop);
                int len_cong = get_path_sequence(g_parent_cong, end_idx, seq_cong);

                printf("\n=== 路径智能对比分析 ===\n");
                if (are_paths_equal(seq_hop, len_hop, seq_cong, len_cong)) {
                    printf("[i] 结论：当前网络状态良好，物理最短路径即为最优拥塞路径。\n");
                } else {
                    printf("[!] 发现差异：物理最短路径上检测到高负载链路，系统已自动重新规划拥塞更小的绕行路线。\n");
                }
            }

        } else {
            printf("错误：未在当前拓扑图中找到您输入的 IP 节点。\n");
            if (start_idx == -1) printf(" -> 找不到源 IP: %s\n", start_ip);
            if (end_idx == -1) printf(" -> 找不到目的 IP: %s\n", end_ip);
        }
    } else {
        printf("输入格式错误。\n");
        // 清空输入缓冲区，防止死循环
        while (getchar() != '\n');
    }
}
