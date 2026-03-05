#include "../include/traffic.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static int compare_traffic(const void *a, const void *b) {
    const TrafficRank *r1 = (const TrafficRank *)a;
    const TrafficRank *r2 = (const TrafficRank *)b;
    if (r2->traffic > r1->traffic) return 1;
    if (r2->traffic < r1->traffic) return -1;
    return 0;
}

static void print_rankings(Graph *g, TrafficRank *ranks, int count, const char *title) {
    printf("--- %s TOP 10 ---\n", title);
    if (count == 0) {
        printf("无可展示数据。\n");
        return;
    }
    qsort(ranks, count, sizeof(TrafficRank), compare_traffic);
    for (int i = 0; i < 10 && i < count; ++i) {
        int idx = ranks[i].node_idx;
        printf("%2d. %s | 流量: %ld\n", i + 1, g->nodes[idx].ip, ranks[i].traffic);
    }
}

void rank_all_nodes(Graph *g) {
    TrafficRank ranks[MAX_NODES];
    int r_count = 0;
    for (int i = 0; i < g->count; i++) {
        ranks[r_count].node_idx = i;
        ranks[r_count].traffic = g->nodes[i].in_total + g->nodes[i].out_total;
        r_count++;
    }
    print_rankings(g, ranks, r_count, "节点总流量排序");
}

void rank_https_nodes(Graph *g) {
    TrafficRank ranks[MAX_NODES];
    int r_count = 0;

    for (int i = 0; i < g->count; i++) {
        long total_https_for_node = 0;
        for (EdgeNode *e = g->nodes[i].first_edge; e; e = e->next) {
            total_https_for_node += e->stats.https_bytes;
        }
        if (total_https_for_node > 0) {
            ranks[r_count].node_idx = i;
            ranks[r_count].traffic = total_https_for_node;
            r_count++;
        }
    }
    print_rankings(g, ranks, r_count, "HTTPS (TCP:443) 流量排行榜");
}

void detect_scanning(Graph *g) {
    printf("--- 扫描行为检测 (发出流量占比 > 80%%) ---\n");
    TrafficRank ranks[MAX_NODES];
    int c_count = 0;

    for (int i = 0; i < g->count; i++) {
        long in_total = g->nodes[i].in_total;
        long out_total = g->nodes[i].out_total;
        long total = in_total + out_total;
        if (total == 0) continue;

        double out_ratio = (double)out_total / (double)total;
        if (out_ratio > 0.8) {
            ranks[c_count].node_idx = i;
            ranks[c_count].traffic = total;
            c_count++;
        }
    }

    if (c_count == 0) {
        printf("未发现明显扫描行为。\n");
        return;
    }

    qsort(ranks, c_count, sizeof(TrafficRank), compare_traffic);

    for (int i = 0; i < 10 && i < c_count; i++) {
        int idx = ranks[i].node_idx;
        long out_total = g->nodes[idx].out_total;
        long total = ranks[i].traffic;
        double out_ratio = (double)out_total / (double)total;

        printf("%2d. 可疑扫描源: %-15s | 总流量: %10ld | 发出流量占比: %.2f%%\n",
               i + 1, g->nodes[idx].ip, total, out_ratio * 100.0);
    }
    printf("总共可疑源共有：%d 个\n", c_count);
}


static bool has_edge(Graph *g, int from, int to) {
    for (EdgeNode *e = g->nodes[from].first_edge; e; e = e->next) {
        if (e->dest_idx == to) return true;
    }
    return false;
}

// 灵活星型结构:卫星结点只能与中心节点一个相连（不能有不是叶子节点的相邻节点），但可以自己设置阈值k
void find_star_topology(Graph *g) {
    int k;
    if (scanf("%d", &k) != 1) {     // 仅读取数字
        k = 0;    // 默认值
        while (getchar() != '\n'); // 清理缓冲区
    }
    printf("--- 灵活星型拓扑检测 (允许 %d 个非叶子节点) ---\n", k);
    bool found = false;

    for (int center = 0; center < g->count; center++) {
        bool neighbor[MAX_NODES] = {false};
        int neighbor_count = 0;

        // 1. 识别中心节点的所有邻居 (方向不敏感)
        for (EdgeNode *e = g->nodes[center].first_edge; e; e = e->next) {
            if (!neighbor[e->dest_idx] && e->dest_idx != center) {
                neighbor[e->dest_idx] = true;
                neighbor_count++;
            }
        }
        for (int x = 0; x < g->count; x++) {
            if (x != center && has_edge(g, x, center) && !neighbor[x]) {
                neighbor[x] = true;
                neighbor_count++;
            }
        }

        // 基本规模过滤：邻居数需达到任务书要求的 20 个
        if (neighbor_count < 20) continue;

        // 2. 统计不满足“仅与中心节点相连”条件的邻居数量
        int non_leaf_count = 0;
        for (int i = 0; i < g->count; i++) {
            if (!neighbor[i]) continue;

            bool is_strict_leaf = true;
            // 检查出边：是否连向了除中心外的其他节点
            for (EdgeNode *e = g->nodes[i].first_edge; e; e = e->next) {
                if (e->dest_idx != center) {
                    is_strict_leaf = false;
                    break;
                }
            }
            // 检查入边：是否有除中心外的其他节点连向它
            if (is_strict_leaf) {
                for (int y = 0; y < g->count; y++) {
                    if (y != center && y != i && has_edge(g, y, i)) {
                        is_strict_leaf = false;
                        break;
                    }
                }
            }

            if (!is_strict_leaf) {
                non_leaf_count++;
            }
        }

        // 3. 灵活阈值判断
        if (non_leaf_count <= k) {
            found = true;
            printf("[命中] 中心节点: %s | 总邻居数: %d | 违规邻居数: %d\n",
                   g->nodes[center].ip, neighbor_count, non_leaf_count);

            // 打印结果格式微调，符合任务书输出要求
            printf("               ");
            int printed = 0;
            for (int i = 0; i < g->count; i++) {
                if (neighbor[i]) {
                    printf("%s", g->nodes[i].ip);
                    if (++printed < neighbor_count) printf(", ");
                    if (printed % 5 == 0 && printed < neighbor_count) printf("\n               ");
                }
            }
            printf("\n\n");
        }
    }

    if (!found) printf("未检测到符合条件的星型拓扑。\n");
}

// 宽松版本的判定逻辑只依赖于中心节点的出度
// void find_star_topology(Graph *g) {
//     printf("--- 检测到的星型拓扑 ---\n");
//     bool found = false;
//
//     for (int i = 0; i < g->count; i++) {
//         if (g->nodes[i].out_degree >= 20) {
//             found = true;
//             printf("中心节点: %s | 连接节点数: %d :\n", g->nodes[i].ip, g->nodes[i].out_degree);
//             // 遍历该节点的所有出边
//             EdgeNode *e = g->nodes[i].first_edge;
//             int print_count = 0;
//             printf("               ");
//             while (e != NULL) {
//                 // e->dest_idx 是目标节点在 nodes 数组中的索引
//                 printf("%s", g->nodes[e->dest_idx].ip);
//
//                 e = e->next;
//                 print_count++;
//
//                 if (e != NULL) {
//                     printf(", ");
//                     // 每打印 5 个 IP 换行一次，保持界面整洁
//                     if (print_count % 5 == 0) {
//                         printf("\n               ");
//                     }
//                 }
//             }
//             printf("\n\n"); // 这个中心节点打印完毕，留出空行
//         }
//     }
//
//     if (!found) {
//         printf("未检测到星型拓扑中心节点。\n");
//     }
// }


static uint32_t ip_to_uint32(const char *ip_str) {
    uint32_t ip[4] = {0};
    // 解析 A.B.C.D 格式的 IP 地址
    sscanf(ip_str, "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]);
    // 移位拼接成 32 位整数
    return (ip[0] << 24) | (ip[1] << 16) | (ip[2] << 8) | ip[3];
}

// --- 安全规则检测函数 ---
// target_ip: 被管控的地址1
// range_start: 范围起始地址 (地址2)
// range_end: 范围结束地址 (地址3)
void check_security_rule(Graph *g) {
    // 声明字符数组
    char target_ip[MAX_IP_LEN];
    char range_start[MAX_IP_LEN];
    char range_end[MAX_IP_LEN];

    scanf("%s", target_ip);
    scanf("%s", range_start);
    scanf("%s", range_end);

    // 将输入的字符串转换成数字形式
    uint32_t target_ip_num = ip_to_uint32(target_ip);
    uint32_t start_num = ip_to_uint32(range_start);
    uint32_t end_num = ip_to_uint32(range_end);

    // 容错处理：确保 start_num <= end_num
    if (start_num > end_num) {
        uint32_t temp = start_num;
        start_num = end_num;
        end_num = temp;

        char temp_str[MAX_IP_LEN];
        strcpy(temp_str, range_start);
        strcpy(range_start, range_end);
        strcpy(range_end, temp_str);
    }

    printf("--- 安全规则违规检测 ---\n");
    printf("规则限制: 节点 [%s] 禁止与区间 [%s ~ %s] 发生通信\n", target_ip, range_start, range_end);

    bool found_violation = false;

    // 2. 遍历全图的所有会话（边）
    for (int i = 0; i < g->count; i++) {
        uint32_t src_ip_num = ip_to_uint32(g->nodes[i].ip);

        for (EdgeNode *e = g->nodes[i].first_edge; e; e = e->next) {
            uint32_t dst_ip_num = ip_to_uint32(g->nodes[e->dest_idx].ip);

            bool is_violation = false;

            // 目标IP是源节点，且它向被禁范围发送了数据（主动违规外联）
            if (src_ip_num == target_ip_num && (dst_ip_num >= start_num && dst_ip_num <= end_num)) {
                is_violation = true;
            }

            // 如果发现违规，打印会话详情
            if (is_violation) {
                found_violation = true;
                printf("[!] 违规会话拦截: 源 %-15s -> 目的 %-15s | 流量: %8ld 字节 | 时长: %.2f秒\n",
                       g->nodes[i].ip,
                       g->nodes[e->dest_idx].ip,
                       e->total_bytes,
                       e->duration);
            }
        }
    }

    if (!found_violation) {
        printf("合规检查通过: 未发现违反该安全规则的会话。\n");
    }
    printf("\n");
}

void profile_node_roles(Graph *g) {
    printf("--- 节点类型分析 ---\n");
    if (g->count == 0) {
        printf("[-] 图数据为空，请先加载流量数据。\n");
        return;
    }

    // 1. 动态统计每个节点的入度 (in_degree)
    int in_degree[MAX_NODES] = {0};
    for (int i = 0; i < g->count; i++) {
        for (EdgeNode *e = g->nodes[i].first_edge; e; e = e->next) {
            in_degree[e->dest_idx]++;
        }
    }

    int server_count = 0;
    int scanner_count = 0;
    int p2p_count = 0;
    int client_count = 0;

    printf("%-18s | %-20s | %s\n", "IP 地址", "推断角色", "行为特征说明");
    printf("-------------------+----------------------+----------------------------------------\n");

    for (int i = 0; i < g->count; i++) {
        Vertex *v = &g->nodes[i];
        int out_deg = v->out_degree;
        int in_deg = in_degree[i];
        long total_bytes = v->in_total + v->out_total;

        if (total_bytes == 0) continue;

        char role[50] = "普通终端 (Client)";
        char reason[100] = "行为特征符合标准端点设备";
        bool is_special = false;

        // 遍历该节点发出的所有边，检查是否存在“单点针对性扫描”
        bool is_targeted_scanner = false;
        int target_ports = 0;
        double target_avg_bytes = 0;

        for (EdgeNode *e = v->first_edge; e; e = e->next) {
            // 核心判定：对单一目标IP发起了超过 15 个不同端口/会话的连接，且平均每次交互数据量极小 (< 500B)
            if (e->session_count >= 15 && (e->total_bytes / e->session_count) < 500) {
                is_targeted_scanner = true;
                target_ports = e->session_count;
                target_avg_bytes = (double)e->total_bytes / e->session_count;
                break; // 只要发现针对某一个IP的爆破，就定性为扫描器
            }
        }

        // 规则1：核心业务服务器 (Server)
        // 判定逻辑：被大量独立主机连接 (in_deg >= 10)，且产生了实质性的业务数据交互（总流量 > 100KB）
        // 这样就保护了正常的业务服务器不被误判。
        if (in_deg >= 10 && total_bytes > 102400) {
            strcpy(role, "[*] 核心服务器 (Server)");
            sprintf(reason, "高频被连(%d次), 承担高吞吐业务(%.2f MB)", in_deg, total_bytes / 1048576.0);
            server_count++;
            is_special = true;
        }
        // 【新增规则】：针对性端口扫描器 (Port Scanner) —— 你的 Nmap 行为将被这里完美捕获
        else if (is_targeted_scanner) {
            strcpy(role, "[!] 针对性扫描器 (Port Scanner)");
            sprintf(reason, "单点爆破扫描 (针对单IP探测 %d 次, 均载 %.1f B)", target_ports, target_avg_bytes);
            scanner_count++;
            is_special = true;
        }
        // 规则2：疑似恶意扫描器 (Scanner)
        // 判定逻辑：广撒网 (出度 >= 15)，单点发送极小 (<5KB，属于探针包)，
        // 【核心区分点】：且它几乎没有收到真实的业务回包 (入向总流量 < 50KB)。说明所有的连接都未能建立真实的业务传输！
        else if (out_deg >= 15 && (v->out_total / out_deg) < 5000 && v->in_total < 51200) {
            strcpy(role, "[!] 疑似扫描器 (Scanner)");
            sprintf(reason, "高频探测(出度%d), 无真实负载(入向仅%.1f KB)", out_deg, v->in_total / 1024.0);
            scanner_count++;
            is_special = true;
        }

        // 规则3：P2P节点 / 核心路由网关 (Hub)
        // 判定逻辑：进出方向的连接数都比较高，属于网络结构中的交通枢纽
        else if (in_deg >= 5 && out_deg >= 5) {
            strcpy(role, "[+] 核心枢纽节点 (Hub)");
            sprintf(reason, "双向连接活跃 (入:%d, 出:%d)", in_deg, out_deg);
            p2p_count++;
            is_special = true;
        }
        // 规则4：普通终端 (Client)
        else {
            client_count++;
        }

        // 为了防止控制台被普通终端刷屏，如果全图节点较多，我们只打印特殊角色
        if (is_special || g->count <= 30) {
            printf("%-18s | %-20s | %s\n", v->ip, role, reason);
        }
    }

    printf("-------------------------------------------------------------------------------\n");
    printf("[汇总] 发现 核心服务器: %d 个 | 扫描器: %d 个 | 枢纽节点: %d 个 | 隐藏普通终端: %d 个\n\n",
           server_count, scanner_count, p2p_count, client_count);
}