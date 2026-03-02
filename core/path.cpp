#include "../include/graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#define INF DBL_MAX

// 1. 最小跳数查找 (BFS)
void find_min_hop_path(Graph *g, int start, int end) {
    int parent[MAX_NODES];
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
        printf("未找到路径。\n");
    } else {
        printf("最小跳数路径 (跳数: %d): ", dist[end]);
        // 递归打印路径逻辑省略，实际需通过parent回溯
    }
}

// 2. 最小拥塞路径 (Dijkstra)
void find_min_congestion_path(Graph *g, int start, int end) {
    double dist[MAX_NODES];
    int parent[MAX_NODES];
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
            // 拥塞程度 = 流量 / 持续时间
            double congestion = (e->duration > 0) ? (double)e->total_bytes / e->duration : 0;
            if (dist[u] + congestion < dist[e->dest_idx]) {
                dist[e->dest_idx] = dist[u] + congestion;
                parent[e->dest_idx] = u;
            }
        }
    }
    printf("拥塞最小路径计算完成。\n");
}