#include "../include/graph.h"
#include <string.h>
#include <stdlib.h>

void init_graph(Graph *g) { g->count = 0; }

int get_or_create_node(Graph *g, const char *ip) {
    for (int i = 0; i < g->count; i++) {
        if (strcmp(g->nodes[i].ip, ip) == 0) return i;
    }

    if (g->count >= MAX_NODES) {
        return -1;
    }

    strcpy(g->nodes[g->count].ip, ip);
    g->nodes[g->count].first_edge = NULL;
    g->nodes[g->count].in_total = 0;
    g->nodes[g->count].out_total = 0;
    g->nodes[g->count].out_degree = 0;
    return g->count++;
}

void add_session(Graph *g, char *src, char *dst, int proto, int src_port, int dst_port, long bytes, double duration) {
    int u = get_or_create_node(g, src);
    int v = get_or_create_node(g, dst);
    if (u < 0 || v < 0) return;

    g->nodes[u].out_total += bytes; // 源 IP 发出流量
    g->nodes[v].in_total += bytes;  // 目的 IP 接收流量

    // 1. 查找是否存在 u -> v 的边
    EdgeNode *p = g->nodes[u].first_edge;
    while (p) {
        if (p->dest_idx == v) {
            // 找到已存在的边，累加基础数据
            p->total_bytes += bytes;
            p->duration += duration;

            // 分类投递
            if (proto == 6) { // TCP
                p->stats.tcp_bytes += bytes;
                p->stats.tcp_duration += duration;
                // 只要目的端口是 443，即视为 HTTPS 相关流量
                if (dst_port == 443) {
                    p->stats.https_bytes += bytes;
                }
            } else if (proto == 17) { // UDP
                p->stats.udp_bytes += bytes;
                p->stats.udp_duration += duration;
            }
            return; // 命中现有边，处理完毕直接返回
        }
        p = p->next;
    }

    // 2. 如果没找到边，创建新边
    EdgeNode *new_e = (EdgeNode*)malloc(sizeof(EdgeNode));
    if (!new_e) return;

    // 最安全的做法：将整块内存置零，包括内部的 stats 结构体和 next 指针
    memset(new_e, 0, sizeof(EdgeNode));

    // 初始化基础字段
    new_e->dest_idx = v;
    new_e->total_bytes = bytes;
    new_e->duration = duration;

    // 第一次“分类投递”
    if (proto == 6) {
        new_e->stats.tcp_bytes = bytes;
        new_e->stats.tcp_duration = duration;
        if (dst_port == 443) {
            new_e->stats.https_bytes = bytes;
        }
    } else if (proto == 17) {
        new_e->stats.udp_bytes = bytes;
        new_e->stats.udp_duration = duration;
    }

    // 插入到邻接表头部（头插法）
    new_e->next = g->nodes[u].first_edge;
    g->nodes[u].first_edge = new_e;

    // 【修复点】：增加源节点的出度
    g->nodes[u].out_degree++;
}
