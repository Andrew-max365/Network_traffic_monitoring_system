#include "../include/utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
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

// 辅助函数：安全地从当前位置提取下一个逗号分隔的字段
// 它会修改传入的字符串，并返回字段的起始指针
char* shift_field(char **line_ptr) {
    char *start = *line_ptr;
    if (start == NULL) return ""; // 防止指针越界

    char *comma = strchr(start, ',');
    if (comma) {
        *comma = '\0';          // 将逗号替换为结束符
        *line_ptr = comma + 1;  // 指针移向下一个字段
    } else {
        // 最后一个字段，处理可能存在的换行符
        char *newline = strpbrk(start, "\r\n");
        if (newline) *newline = '\0';
        *line_ptr = NULL;       // 标记已到行尾
    }
    return start;
}

bool load_data_from_csv(const char *filename, Graph *g) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("无法打开数据文件");
        return false;
    }

    char line[1024]; // 缓冲区
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return false;
    }

    int print_count = 0;
    printf("--- 开始读取数据，前10行预览：---\n");

    while (fgets(line, sizeof(line), fp)) {
        char *ptr = line;

        // 1. 依次按顺序“切开”每一个字段
        char *src_str   = shift_field(&ptr);
        char *dst_str   = shift_field(&ptr);
        char *proto_str = shift_field(&ptr);
        char *sp_str    = shift_field(&ptr);
        char *dp_str    = shift_field(&ptr);
        char *byte_str  = shift_field(&ptr);
        char *dur_str   = shift_field(&ptr);

        // 2. 修剪空格
        trim_whitespace(src_str);
        trim_whitespace(dst_str);

        // 3. 转换类型 (atoi/atof 遇到空字符串会安全地返回 0)
        int proto      = atoi(proto_str);
        int src_port   = atoi(sp_str);
        int dst_port   = atoi(dp_str);
        long bytes     = atol(byte_str);
        double duration = atof(dur_str);

        // 4. 预览打印（前10行）
        if (print_count < 10) {
            printf("[%d] 源:%-15s -> 目的:%-15s | 协议:%-2d | 端口:%d->%d | 大小:%ld bytes | 时长:%.3lf ms\n",
                    print_count + 1,
                    src_str[0] ? src_str : "[空]",
                    dst_str[0] ? dst_str : "[空]",
                    proto, src_port, dst_port, bytes, duration);
        }

        // 5. 加入图中（只有当 IP 不为空时才加入，增加健壮性）
        if (src_str[0] != '\0' && dst_str[0] != '\0') {
            add_session(g, src_str, dst_str, proto, src_port, dst_port, bytes, duration);
        }

        print_count++;
    }

    printf("--- 共处理 %d 条记录，其中端口为0者为数据缺失项 ---\n", print_count);
    fclose(fp);
    return true;
}
