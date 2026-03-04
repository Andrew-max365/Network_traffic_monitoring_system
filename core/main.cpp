#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/graph.h"
#include "../include/path.h"
#include "../include/traffic.h"
#include "../include/utils.h"
#include "../include/dsu.h"

int main(int argc, char* argv[]) {
    if (freopen("data/ui_bridge.log", "w", stdout) == NULL) {
        fprintf(stderr, "Error: Cannot open log file.\n");
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    Graph myGraph;
    init_graph(&myGraph);

    int choice;
    char filename[] = "data/network_data.csv";
    char target_ip[50];

    while (1) {
        printf("\n等待指令输入......\n");
        fflush(stdout);

        if (scanf("%d", &choice) != 1) {
            if (feof(stdin)) break;
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        switch (choice) {
            case 1: {
                char dynamic_filename[256];
                if (scanf("%255s", dynamic_filename) == 1) {
                    free_graph(&myGraph);
                    init_graph(&myGraph);
                    if (load_data_from_csv(dynamic_filename, &myGraph)) {
                        printf("[成功] 已从 %s 加载并构建图结构。\n", dynamic_filename);
                    } else {
                        printf("[错误] 找不到或无法打开文件: %s\n", dynamic_filename);
                    }
                }
                break;
            }
            case 2: rank_all_nodes(&myGraph); break;
            case 3: rank_https_nodes(&myGraph); break;
            case 4: detect_scanning(&myGraph); break;
            case 5: print_path_result(&myGraph); break;
            case 6: find_star_topology(&myGraph); break;
            case 7: check_security_rule(&myGraph); break;
            case 8:
                printf("[执行] 正在启动外部提取脚本...\n");
                system("python scripts/pcap_extractor.py");
                break;
            case 9:
                if (scanf("%49s", target_ip) == 1) { // 限宽防溢出
                    // 调用并查集函数，生成子图数据
                    extract_subgraph_by_ip(&myGraph, target_ip);
                    // 启动 Python 渲染引擎
                    char cmd[512];
                    sprintf(cmd, ".venv\\Scripts\\python.exe scripts/visualizer.py %s", target_ip);
                    printf("[系统] 正在调用可视化引擎，请查看浏览器弹窗...\n");
                    fflush(stdout);
                    system(cmd);
                }
                break;
            case 10: // 【创新：风险评估】
                printf("\n>>> 正在启动多维节点安全风险评估...\n");
                compute_all_risk_scores(&myGraph); // 调用 graph.cpp 里的算法
                printf("[分析结果] 已完成全网画像统计。请通过“交互式图可视化”查看效果。\n");
                fflush(stdout);
                break;
            case 11:
                profile_node_roles(&myGraph);
                break;
            case 0:
                free_graph(&myGraph);
                return 0;
            default:
                printf("无效选择。\n");
        }

        printf("\n--- 操作完成 ---\n");
        fflush(stdout);
    }

    free_graph(&myGraph);
    return 0;
}
