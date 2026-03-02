#include "../include/traffic.h"
#include <stdio.h>
#include <stdlib.h>

static int compare_traffic(const void *a, const void *b) {
    const TrafficRank *r1 = (const TrafficRank *)a;
    const TrafficRank *r2 = (const TrafficRank *)b;
    if (r2->traffic > r1->traffic) return 1;
    if (r2->traffic < r1->traffic) return -1;
    return 0;
}

static void print_rankings(Graph *g, TrafficRank *ranks, int count, const char *title) {
    printf("--- %s ---\n", title);
    if (count == 0) {
        printf("无可展示数据。\n");
        return;
    }

    qsort(ranks, count, sizeof(TrafficRank), compare_traffic);
    for (int i = 0; i < count; ++i) {
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
        long https_flow = 0;
        for (EdgeNode *e = g->nodes[i].first_edge; e; e = e->next) {
            https_flow += e->p_stats.tcp_bytes;
        }
        if (https_flow > 0) {
            ranks[r_count].node_idx = i;
            ranks[r_count].traffic = https_flow;
            r_count++;
        }
    }

    print_rankings(g, ranks, r_count, "HTTPS(TCP)流量排序");
}

void detect_scanning(Graph *g) {
    printf("--- 扫描行为检测 (单向流量>80%%) ---\n");
    bool found = false;

    for (int i = 0; i < g->count; i++) {
        long in_total = g->nodes[i].in_total;
        long out_total = g->nodes[i].out_total;
        long total = in_total + out_total;
        if (total == 0) continue;

        long dominant = (in_total > out_total) ? in_total : out_total;
        double ratio = (double)dominant / (double)total;
        if (ratio > 0.8) {
            found = true;
            printf("可疑节点: %s | 入:%ld 出:%ld 占比:%.2f%%\n",
                   g->nodes[i].ip, in_total, out_total, ratio * 100.0);
        }
    }

    if (!found) {
        printf("未发现明显扫描行为。\n");
    }
}

void find_star_topology(Graph *g) {
    printf("--- 检测到的星型拓扑 ---\n");
    bool found = false;

    for (int i = 0; i < g->count; i++) {
        if (g->nodes[i].out_degree >= 20) {
            found = true;
            printf("中心节点: %s | 连接节点数: %d\n", g->nodes[i].ip, g->nodes[i].out_degree);
        }
    }

    if (!found) {
        printf("未检测到星型拓扑中心节点。\n");
    }
}
