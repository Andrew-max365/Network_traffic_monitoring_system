#ifndef PATH_H
#define PATH_H

#include "graph.h"

void find_min_congestion_path(Graph *g, int start, int end);
void find_min_hop_path(Graph *g, int start, int end);
double get_edge_congestion(EdgeNode *e);

#endif
