#include "../include/path.h"
#include <stdio.h>
#include <float.h>

#define INF DBL_MAX

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

double get_edge_congestion(EdgeNode *e) {
    if (!e || e->duration <= 0) return INF;
    return (double)e->total_bytes / e->duration;
}

void find_min_hop_path(Graph *g, int start, int end) {
    if (!is_valid_node(g, start) || !is_valid_node(g, end)) {
        printf("节点索引无效。\n");
        return;
    }

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
        print_path(g, parent, end);
    }
}

void find_min_congestion_path(Graph *g, int start, int end) {
    if (!is_valid_node(g, start) || !is_valid_node(g, end)) {
        printf("节点索引无效。\n");
        return;
    }

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
            double congestion = get_edge_congestion(e);
            if (congestion == INF) continue;
            if (dist[u] + congestion < dist[e->dest_idx]) {
                dist[e->dest_idx] = dist[u] + congestion;
                parent[e->dest_idx] = u;
            }
        }
    }

    if (dist[end] == INF) {
        printf("未找到路径。\n");
    } else {
        printf("最小拥塞路径 (代价: %.2f): ", dist[end]);
        print_path(g, parent, end);
    }
}
