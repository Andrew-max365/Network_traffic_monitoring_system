#include "../include/utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void trim_whitespace(char *str) {
    if (!str || *str == '\0') return;

    char *start = str;
    while (isspace((unsigned char)*start)) start++;

    if (*start == '\0') {
        str[0] = '\0';
        return;
    }

    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    end[1] = '\0';

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

bool load_data_from_csv(const char *filename, Graph *g) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("无法打开数据文件");
        return false;
    }

    char line[512];
    // 跳过第一行表头 [cite: 68]
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return false;
    }

    char src[MAX_IP_LEN], dst[MAX_IP_LEN];
    int proto, src_port, dst_port;
    long bytes;
    double duration;

    // 循环读取每一行数据 [cite: 75, 77]
    // 格式: Source, Destination, Protocol, SrcPort, DstPort, DataSize, Duration
    while (fgets(line, sizeof(line), fp)) {
        // 使用 sscanf 解析逗号分隔的行
        int count = sscanf(line, "%[^,],%[^,],%d,%d,%d,%ld,%lf", 
                           src, dst, &proto, &src_port, &dst_port, &bytes, &duration);
        
        if (count == 7) {
            trim_whitespace(src);
            trim_whitespace(dst);
            // 调用之前在 graph.c 中实现的会话合并逻辑
            add_session(g, src, dst, proto, bytes, duration);
        }
    }

    fclose(fp);
    return true;
}
