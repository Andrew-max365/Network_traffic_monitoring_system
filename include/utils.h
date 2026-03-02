#ifndef UTILS_H
#define UTILS_H

#include "graph.h"

/**
 * 功能：从指定的 CSV 文件读取网络流量数据并构建图
 * 输入：filename - 文件路径；g - 指向图结构的指针
 * 返回：成功返回 true，失败返回 false
 */
bool load_data_from_csv(const char *filename, Graph *g);

/**
 * 功能：辅助函数，去除字符串首尾空格（处理 CSV 时常用）
 */
void trim_whitespace(char *str);

#endif