#ifndef TRAFFIC_H
#define TRAFFIC_H
#include "graph.h"

// 排序结构体
typedef struct {
    int node_idx;
    long traffic;
} TrafficRank;

void rank_all_nodes(Graph *g);              // 任务3.1: 节点流量排序
void rank_https_nodes(Graph *g);            // 任务3.2: HTTPS筛选排序
void detect_scanning(Graph *g);             // 任务3.2: 单向流量检测
void find_star_topology(Graph *g);          // 扩展: 星型结构检测
void check_security_rule(Graph *g);         // 拓展: 自定义的安全规则
void profile_node_roles(Graph *g);          // 创新: 节点类型判断

#endif