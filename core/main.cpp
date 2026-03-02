#include <stdio.h>
#include <stdlib.h>
#include "../include/graph.h"
#include "../include/traffic.h"
#include "../include/utils.h"  // 这里正式引入辅助工具模块 [cite: 77]

void print_menu() {
    printf("\n========= 网络流量分析与异常检测系统 =========\n");
    printf("1. 加载流量数据 (CSV)\n");
    printf("2. 节点总流量排序 [cite: 81]\n");
    printf("3. 筛选HTTPS流量排序 [cite: 87]\n");
    printf("4. 扫描行为检测 (单向流量>80%%) [cite: 88]\n");
    printf("5. 最小跳数与最小拥塞路径查找 [cite: 89]\n");
    printf("6. 检测星型拓扑结构 [cite: 94]\n");
    printf("0. 退出系统\n");
    printf("==============================================\n");
    printf("请选择功能: ");
}

int main() {
    Graph myGraph;
    init_graph(&myGraph); // 初始化图结构 [cite: 78]
    int choice;
    char filename[] = "data/network_data.csv";

    while (1) {
        print_menu();
        if (scanf("%d", &choice) != 1) break;

        switch (choice) {
            case 1:
                // 调用 utils.c 中的函数实现数据提取 [cite: 65, 167]
                if (load_data_from_csv(filename, &myGraph)) {
                    printf("[成功] 已从 %s 加载并构建图结构。\n", filename);
                }
                break;
            case 2:
                // 任务 3.1: 节点流量排序 [cite: 82, 167]
                rank_all_nodes(&myGraph);
                break;
            case 3:
                // 任务 3.2: 筛选HTTPS节点流量排序 [cite: 87, 167]
                rank_https_nodes(&myGraph);
                break;
            case 4:
                // 任务 3.2: 筛选单向流量节点排序 [cite: 88, 167]
                detect_scanning(&myGraph);
                break;
            case 5:
                // 任务 4: 路径查找
                // 提示用户输入源IP和目的IP，然后调用 path.c 的算法
                printf("功能开发中：将对比最小跳数与最小拥塞路径...\n");
                break;
            case 6:
                // 扩展功能：查找星型拓扑结构 [cite: 94, 167]
                find_star_topology(&myGraph);
                break;
            case 0:
                printf("感谢使用，系统退出。\n");
                return 0;
            default:
                printf("无效选择，请重新输入。\n");
        }
    }
    return 0;
}