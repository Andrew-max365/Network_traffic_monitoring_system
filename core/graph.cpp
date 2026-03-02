#include "../include/graph.h"
#include <string.h>
#include <stdlib.h>

void init_graph(Graph *g) { g->count = 0; }

int get_or_create_node(Graph *g, const char *ip) {
    for (int i = 0; i < g->count; i++) {
        if (strcmp(g->nodes[i].ip, ip) == 0) return i;
    }
    strcpy(g->nodes[g->count].ip, ip);
    g->nodes[g->count].first_edge = NULL;
    g->nodes[g->count].in_total = 0;
    g->nodes[g->count].out_total = 0;
    g->nodes[g->count].out_degree = 0;
    return g->count++;
}

void add_session(Graph *g, char *src, char *dst, int proto, long bytes, double duration) {
    int u = get_or_create_node(g, src);
    int v = get_or_create_node(g, dst);

    g->nodes[u].out_total += bytes;
    g->nodes[v].in_total += bytes;

    // 查找是否存在已有边（会话合并逻辑）[cite: 61]
    EdgeNode *p = g->nodes[u].first_edge;
    while (p) {
        if (p->dest_idx == v) {
            p->total_bytes += bytes;
            p->duration += duration;
            if (proto == 6) p->p_stats.tcp_bytes += bytes;
            return;
        }
        p = p->next;
    }

    // 新建边 [cite: 80]
    EdgeNode *new_e = (EdgeNode*)malloc(sizeof(EdgeNode));
    new_e->dest_idx = v;
    new_e->total_bytes = bytes;
    new_e->duration = duration;
    new_e->next = g->nodes[u].first_edge;
    g->nodes[u].first_edge = new_e;
    g->nodes[u].out_degree++;
}