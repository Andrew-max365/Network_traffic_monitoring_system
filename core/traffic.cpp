#include "../include/traffic.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static int compare_traffic(const void *a, const void *b) {
    const TrafficRank *r1 = (const TrafficRank *)a;
    const TrafficRank *r2 = (const TrafficRank *)b;
    if (r2->traffic > r1->traffic) return 1;
    if (r2->traffic < r1->traffic) return -1;
    return 0;
}

static void print_rankings(Graph *g, TrafficRank *ranks, int count, const char *title) {
    printf("--- %s TOP 10 ---\n", title);
    if (count == 0) {
        printf("无可展示数据。\n");
        return;
    }
    qsort(ranks, count, sizeof(TrafficRank), compare_traffic);
    for (int i = 0; i < 10 && i < count; ++i) {
        int idx = ranks[i].node_idx;
        printf("%2d. %s | 流量: %ld\n", i + 1, g->nodes[idx].ip, ranks[i].traffic);
    }
}

void rank_all_nodes(Graph *g) {
    TrafficRank ranks[MAX_NODES];
    int r_count = 0;
    for (int i = 0; i < g->count; i++) {
        ranks[r_count].node_idx = i;
        ranks[r_count].traffic = g->nodes[i].in_total + g->nodes[i].out_total;
        r_count++;
    }
    print_rankings(g, ranks, r_count, "节点总流量排序");
}

void rank_https_nodes(Graph *g) {
    TrafficRank ranks[MAX_NODES];
    int r_count = 0;

    for (int i = 0; i < g->count; i++) {
        long total_https_for_node = 0;
        for (EdgeNode *e = g->nodes[i].first_edge; e; e = e->next) {
            total_https_for_node += e->stats.https_bytes;
        }
        if (total_https_for_node > 0) {
            ranks[r_count].node_idx = i;
            ranks[r_count].traffic = total_https_for_node;
            r_count++;
        }
    }
    print_rankings(g, ranks, r_count, "HTTPS (TCP:443) 流量排行榜");
}

void detect_scanning(Graph *g) {
    printf("--- 扫描行为检测 (发出流量占比 > 80%%) ---\n");
    TrafficRank ranks[MAX_NODES];
    int c_count = 0;

    for (int i = 0; i < g->count; i++) {
        long in_total = g->nodes[i].in_total;
        long out_total = g->nodes[i].out_total;
        long total = in_total + out_total;
        if (total == 0) continue;

        double out_ratio = (double)out_total / (double)total;
        if (out_ratio > 0.8) {
            ranks[c_count].node_idx = i;
            ranks[c_count].traffic = total;
            c_count++;
        }
    }

    if (c_count == 0) {
        printf("未发现明显扫描行为。\n");
        return;
    }

    qsort(ranks, c_count, sizeof(TrafficRank), compare_traffic);

    for (int i = 0; i < 10 && i < c_count; i++) {
        int idx = ranks[i].node_idx;
        long out_total = g->nodes[idx].out_total;
        long total = ranks[i].traffic;
        double out_ratio = (double)out_total / (double)total;

        printf("%2d. 可疑扫描源: %-15s | 总流量: %10ld | 发出流量占比: %.2f%%\n",
               i + 1, g->nodes[idx].ip, total, out_ratio * 100.0);
    }
    printf("总共可疑源共有：%d 个\n", c_count);
}


static bool has_edge(Graph *g, int from, int to) {
    for (EdgeNode *e = g->nodes[from].first_edge; e; e = e->next) {
        if (e->dest_idx == to) return true;
    }
    return false;
}

// 灵活星型结构:卫星结点只能与中心节点一个相连（不能有不是叶子节点的相邻节点），但可以自己设置阈值k
void find_star_topology(Graph *g) {
    int k;
    if (scanf("%d", &k) != 1) {     // 仅读取数字
        k = 0;    // 默认值
        while (getchar() != '\n'); // 清理缓冲区
    }
    printf("--- 灵活星型拓扑检测 (允许 %d 个非叶子节点) ---\n", k);
    bool found = false;

    for (int center = 0; center < g->count; center++) {
        bool neighbor[MAX_NODES] = {false};
        int neighbor_count = 0;

        // 1. 识别中心节点的所有邻居 (方向不敏感)
        for (EdgeNode *e = g->nodes[center].first_edge; e; e = e->next) {
            if (!neighbor[e->dest_idx] && e->dest_idx != center) {
                neighbor[e->dest_idx] = true;
                neighbor_count++;
            }
        }
        for (int x = 0; x < g->count; x++) {
            if (x != center && has_edge(g, x, center) && !neighbor[x]) {
                neighbor[x] = true;
                neighbor_count++;
            }
        }

        // 基本规模过滤：邻居数需达到任务书要求的 20 个
        if (neighbor_count < 20) continue;

        // 2. 统计不满足“仅与中心节点相连”条件的邻居数量
        int non_leaf_count = 0;
        for (int i = 0; i < g->count; i++) {
            if (!neighbor[i]) continue;

            bool is_strict_leaf = true;
            // 检查出边：是否连向了除中心外的其他节点
            for (EdgeNode *e = g->nodes[i].first_edge; e; e = e->next) {
                if (e->dest_idx != center) {
                    is_strict_leaf = false;
                    break;
                }
            }
            // 检查入边：是否有除中心外的其他节点连向它
            if (is_strict_leaf) {
                for (int y = 0; y < g->count; y++) {
                    if (y != center && y != i && has_edge(g, y, i)) {
                        is_strict_leaf = false;
                        break;
                    }
                }
            }

            if (!is_strict_leaf) {
                non_leaf_count++;
            }
        }

        // 3. 灵活阈值判断
        if (non_leaf_count <= k) {
            found = true;
            printf("[命中] 中心节点: %s | 总邻居数: %d | 违规邻居数: %d\n",
                   g->nodes[center].ip, neighbor_count, non_leaf_count);

            // 打印结果格式微调，符合任务书输出要求
            printf("               ");
            int printed = 0;
            for (int i = 0; i < g->count; i++) {
                if (neighbor[i]) {
                    printf("%s", g->nodes[i].ip);
                    if (++printed < neighbor_count) printf(", ");
                    if (printed % 5 == 0 && printed < neighbor_count) printf("\n               ");
                }
            }
            printf("\n\n");
        }
    }

    if (!found) printf("未检测到符合条件的星型拓扑。\n");
}

// 宽松版本的判定逻辑只依赖于中心节点的出度
// void find_star_topology(Graph *g) {
//     printf("--- 检测到的星型拓扑 ---\n");
//     bool found = false;
//
//     for (int i = 0; i < g->count; i++) {
//         if (g->nodes[i].out_degree >= 20) {
//             found = true;
//             printf("中心节点: %s | 连接节点数: %d :\n", g->nodes[i].ip, g->nodes[i].out_degree);
//             // 遍历该节点的所有出边
//             EdgeNode *e = g->nodes[i].first_edge;
//             int print_count = 0;
//             printf("               ");
//             while (e != NULL) {
//                 // e->dest_idx 是目标节点在 nodes 数组中的索引
//                 printf("%s", g->nodes[e->dest_idx].ip);
//
//                 e = e->next;
//                 print_count++;
//
//                 if (e != NULL) {
//                     printf(", ");
//                     // 每打印 5 个 IP 换行一次，保持界面整洁
//                     if (print_count % 5 == 0) {
//                         printf("\n               ");
//                     }
//                 }
//             }
//             printf("\n\n"); // 这个中心节点打印完毕，留出空行
//         }
//     }
//
//     if (!found) {
//         printf("未检测到星型拓扑中心节点。\n");
//     }
// }


static uint32_t ip_to_uint32(const char *ip_str) {
    uint32_t ip[4] = {0};
    // 解析 A.B.C.D 格式的 IP 地址
    sscanf(ip_str, "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]);
    // 移位拼接成 32 位整数
    return (ip[0] << 24) | (ip[1] << 16) | (ip[2] << 8) | ip[3];
}

// --- 安全规则检测函数 ---
// target_ip: 被管控的地址1
// range_start: 范围起始地址 (地址2)
// range_end: 范围结束地址 (地址3)
void check_security_rule(Graph *g) {
    // 声明字符数组
    char target_ip[MAX_IP_LEN];
    char range_start[MAX_IP_LEN];
    char range_end[MAX_IP_LEN];

    scanf("%s", target_ip);
    scanf("%s", range_start);
    scanf("%s", range_end);

    // 将输入的字符串转换成数字形式
    uint32_t target_ip_num = ip_to_uint32(target_ip);
    uint32_t start_num = ip_to_uint32(range_start);
    uint32_t end_num = ip_to_uint32(range_end);

    // 容错处理：确保 start_num <= end_num
    if (start_num > end_num) {
        uint32_t temp = start_num;
        start_num = end_num;
        end_num = temp;

        char temp_str[MAX_IP_LEN];
        strcpy(temp_str, range_start);
        strcpy(range_start, range_end);
        strcpy(range_end, temp_str);
    }

    printf("--- 安全规则违规检测 ---\n");
    printf("规则限制: 节点 [%s] 禁止与区间 [%s ~ %s] 发生通信\n", target_ip, range_start, range_end);

    bool found_violation = false;

    // 2. 遍历全图的所有会话（边）
    for (int i = 0; i < g->count; i++) {
        uint32_t src_ip_num = ip_to_uint32(g->nodes[i].ip);

        for (EdgeNode *e = g->nodes[i].first_edge; e; e = e->next) {
            uint32_t dst_ip_num = ip_to_uint32(g->nodes[e->dest_idx].ip);

            bool is_violation = false;

            // 目标IP是源节点，且它向被禁范围发送了数据（主动违规外联）
            if (src_ip_num == target_ip_num && (dst_ip_num >= start_num && dst_ip_num <= end_num)) {
                is_violation = true;
            }

            // 如果发现违规，打印会话详情
            if (is_violation) {
                found_violation = true;
                printf("[!] 违规会话拦截: 源 %-15s -> 目的 %-15s | 流量: %8ld 字节 | 时长: %.2f秒\n",
                       g->nodes[i].ip,
                       g->nodes[e->dest_idx].ip,
                       e->total_bytes,
                       e->duration);
            }
        }
    }

    if (!found_violation) {
        printf("合规检查通过: 未发现违反该安全规则的会话。\n");
    }
    printf("\n");
}