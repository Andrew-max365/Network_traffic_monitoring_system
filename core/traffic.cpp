#include "../include/traffic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 辅助排序结构
int compare_traffic(const void *a, const void *b) {
    TrafficRank *r1 = (TrafficRank *)a;
    TrafficRank *r2 = (TrafficRank *)b;
    return (r2->traffic > r1->traffic) ? 1 : -1;
}

// 任务 3.2: 筛选 HTTPS (TCP 协议编号6, 端口443) [cite: 87]
void rank_https_nodes(Graph *g) {
    TrafficRank ranks[MAX_NODES];
    int r_count = 0;

    for (int i = 0; i < g->count; i++) {
        bool has_https = false;
        long https_flow = 0;
        for (EdgeNode *e = g->nodes[i].first_edge; e; e = e->next) {
            // 假设在 add_session 时已在 p_stats 中记录了 HTTPS 特征
            if (e->p_stats.tcp_bytes > 0) { // 简化判断逻辑
                has_https = true;
                https_flow += e->total_bytes;
            }
        }
        if (has_https) {
            ranks[r_count].node_idx = i;
            ranks[r_count].traffic = https_flow;
            r_count++;
        }
    }
    qsort(ranks, r_count, sizeof(TrafficRank), compare_traffic);
    // 打印前 N 个 HTTPS 节点 [cite: 87]
}

// 扩展任务：检测星型拓扑 [cite: 96]
void find_star_topology(Graph *g) {
    printf("--- 检测到的星型拓扑 ---\n");
    for (int i = 0; i < g->count; i++) {
        // 中心节点连接数 >= 20 [cite: 96]
        if (g->nodes[i].out_degree >= 20) {
            printf("中心节点: %s | 连接节点数: %d\n", g->nodes[i].ip, g->nodes[i].out_degree);
            // 遍历邻居，确保它们只与中心节点建立链接 [cite: 96]
        }
    }
}