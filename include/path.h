#ifndef PATH_H
#define PATH_H

#include "graph.h"

/**
 * 功能：寻找并输出两点间拥塞程度最小的路径
 * 算法：Dijkstra 算法，权值为 (流量 / 持续时间)
 * 输入：g - 图结构指针；start - 起点IP；end - 终点IP
 */
void find_min_congestion_path(Graph *g, char *start_ip, char *end_ip);

/**
 * 功能：寻找并输出两点间跳数最小的路径
 * 算法：广度优先搜索 (BFS)
 * 输入：g - 图结构指针；start - 起点IP；end - 终点IP
 */
void find_min_hop_path(Graph *g, char *start_ip, char *end_ip);

/**
 * 辅助功能：计算两点间的拥塞度
 * 公式：Congestion = Total_Bytes / Duration
 */
double get_edge_congestion(EdgeNode *e);

#endif