#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>

#define MAX_IP_LEN 16
#define MAX_NODES 2000

// 协议统计结构
typedef struct {
    long tcp_bytes;
    long udp_bytes;
    long https_bytes;
    double tcp_duration;   // TCP 会话总时长
    double udp_duration;   // UDP 会话总时长
    long icmp_bytes;       // 可选增强：ICMP 统计
    double icmp_duration;
} TrafficStats;

// 邻接表边节点
typedef struct EdgeNode {
    int dest_idx;          // 目的 IP 的编号
    long total_bytes;      // 边的总流量（所有协议之和）
    double duration;
    TrafficStats stats;
    struct EdgeNode *next;
} EdgeNode;

// 顶点定义
typedef struct {
    char ip[MAX_IP_LEN];
    EdgeNode *first_edge;
    long in_total;  // 入向总流量
    long out_total; // 出向总流量
    int out_degree; // 出度，用于拓扑分析
} Vertex;

typedef struct {
    Vertex nodes[MAX_NODES];
    int count;
} Graph;

void init_graph(Graph *g);
void free_graph(Graph *g);  // 释放邻接表内存
int get_or_create_node(Graph *g, const char *ip);
void add_session(Graph *g, char *src, char *dst, int proto,int src_port,int dst_port, long bytes, double duration);

#endif