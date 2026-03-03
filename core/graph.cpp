#include "../include/graph.h"
#include <string.h>
#include <stdlib.h>

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
