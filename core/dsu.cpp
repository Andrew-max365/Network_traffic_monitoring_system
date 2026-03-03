#include "../include/dsu.h"

std::string DSU::find(std::string i) {
    if (parent.find(i) == parent.end() || parent[i] == i)
        return parent[i] = i;
    return parent[i] = find(parent[i]);
}

void DSU::unite(std::string i, std::string j) {
    std::string root_i = find(i);
    std::string root_j = find(j);
    if (root_i != root_j) parent[root_i] = root_j;
}

// 逻辑建议：在 load_data 时调用 unite(src, dst)
// 在可视化时，遍历所有边，如果 edge.src 的 root == target_ip 的 root，则导出该边