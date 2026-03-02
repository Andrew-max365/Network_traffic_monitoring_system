#include <stdio.h>
#include <stdlib.h>
#include "include/graph.h"
#include "include/path.h"
#include "include/traffic.h"
#include "include/utils.h"

static void print_menu(void) {
    printf("\n========= 网络流量分析与异常检测系统 =========\n");
    printf("1. 加载流量数据 (CSV)\n");
    printf("2. 节点总流量排序\n");
    printf("3. 筛选HTTPS流量排序\n");
    printf("4. 扫描行为检测 (单向流量>80%%)\n");
    printf("5. 最小跳数与最小拥塞路径查找\n");
    printf("6. 检测星型拓扑结构\n");
    printf("0. 退出系统\n");
    printf("==============================================\n");
    printf("请选择功能: ");
}

int main(void) {
    Graph myGraph;
    init_graph(&myGraph);
    int choice;
    const char filename[] = "data/network_data.csv";

    while (1) {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            break;
        }

        switch (choice) {
            case 1:
                if (load_data_from_csv(filename, &myGraph)) {
                    printf("[成功] 已从 %s 加载并构建图结构。\n", filename);
                }
                break;
            case 2:
                rank_all_nodes(&myGraph);
                break;
            case 3:
                rank_https_nodes(&myGraph);
                break;
            case 4:
                detect_scanning(&myGraph);
                break;
            case 5: {
                int start, end;
                printf("请输入源节点索引和目的节点索引(空格分隔): ");
                if (scanf("%d %d", &start, &end) == 2) {
                    find_min_hop_path(&myGraph, start, end);
                    find_min_congestion_path(&myGraph, start, end);
                } else {
                    printf("输入无效。\n");
                }
                break;
            }
            case 6:
                find_star_topology(&myGraph);
                break;
            case 0:
                printf("感谢使用，系统退出。\n");
                return 0;
            default:
                printf("无效选择，请重新输入。\n");
                break;
        }
    }

    return 0;
}
