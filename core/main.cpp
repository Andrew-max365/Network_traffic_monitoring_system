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
    // 1. 【核心】全局重定向：把所有 printf 的内容都写进这个日志文件
    // "w" 模式表示每次启动都会清空旧内容
    if (freopen("data/ui_bridge.log", "w", stdout) == NULL) {
        // 如果打开失败（比如 data 文件夹不存在），则至少在控制台报错
        fprintf(stderr, "Error: Cannot open log file.\n");
    }

    // 2. 禁用缓冲区：只要有输出，立刻写进磁盘，不许攒着
    setvbuf(stdout, NULL, _IONBF, 0);

    // 3. 设置编码：保证中文不乱码
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    Graph myGraph;
    init_graph(&myGraph);
    int choice;
    char filename[] = "data/network_data.csv";
    char target_ip[50];

    while (1) {
        // 这里的 printf 现在会直接跑进 data/ui_bridge.log
        // printf("\n========= 网络流量分析与异常检测系统 =========\n");
        // printf("1. 加载流量数据 (CSV)\n");
        // printf("2. 节点总流量排序\n");
        // printf("3. 筛选HTTPS流量排序\n");
        // printf("4. 扫描行为检测 (单向流量>80%%)\n");
        // printf("5. 最小跳数与最小拥塞路径查找\n");
        // printf("6. 检测星型拓扑结构\n");
        // printf("7. 安全规则检查\n");
        // printf("---------------- 子系统功能 ------------------\n");
        // printf("8. 从 PCAP 文件提取数据\n");
        // printf("9. 子图可视化展示\n");
        // printf("0. 退出系统\n");
        // printf("==============================================\n");
        // printf("请选择功能: \n");

        printf("\n等待指令输入......\n");

        fflush(stdout); // 每一轮结束都强制冲刷一次

        if (scanf("%d", &choice) != 1) {
            if (feof(stdin)) break;
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        switch (choice) {
            case 1:
                if (load_data_from_csv(filename, &myGraph)) {
                    printf("[成功] 已从 %s 加载并构建图结构。\n", filename);
                } else {
                    printf("[错误] 找不到或无法打开文件: %s\n", filename);
                }
                break;
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
                if (scanf("%s", target_ip) == 1) {
                    char cmd[512];
                    // 关键：指定使用项目目录下的虚拟环境 Python
                    // 注意 Windows 路径要用双反斜杠
                    sprintf(cmd, ".venv\\Scripts\\python.exe scripts/visualizer.py %s", target_ip);

                    printf("[系统] 正在调用可视化引擎，请查看浏览器弹窗...\n");
                    fflush(stdout); // 必须 flush，否则 GUI 看不到这一行

                    system(cmd); // 执行脚本
                }
                break;
            case 0: return 0;
            default: printf("无效选择。\n");
        }
        printf("\n--- 操作完成 ---\n");
        fflush(stdout);
    }
    return 0;
}
