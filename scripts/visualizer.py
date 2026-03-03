import pandas as pd
import networkx as nx
from pyvis.network import Network
import sys
import os
import webbrowser

def generate_visual(target_ip):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    csv_path = os.path.join(project_root, "data", "network_data.csv")
    output_html = os.path.join(project_root, "data", "subgraph.html")

    print(f"[*] 正在分析节点: {target_ip}")

    if not os.path.exists(csv_path):
        print(f"[-] 错误: 找不到数据文件 {csv_path}")
        return

    try:
        # 读取 CSV
        df = pd.read_csv(csv_path)

        # --- 调试代码：在 GUI 打印列名 ---
        print(f"[*] 检测到 CSV 列名: {list(df.columns)}")

        src_col = 'Source'
        dst_col = 'Destination'
        prot_col = 'Protocol'

        if src_col not in df.columns or dst_col not in df.columns:
            print(f"[-] 错误: CSV 中找不到列 '{src_col}' 或 '{dst_col}'。请检查表头！")
            return

        # 筛选数据
        sub_df = df[(df[src_col] == target_ip) | (df[dst_col] == target_ip)]

        if sub_df.empty:
            print(f"[-] 警告: 流量库中没有关于 {target_ip} 的记录")
            return

        # 构建图
        G = nx.Graph()
        for _, row in sub_df.iterrows():
            # 这里的协议名也要对应
            color = "#ff4d4d" if row[prot_col] == "HTTPS" else "#4da6ff"
            G.add_edge(row[src_col], row[dst_col], color=color)

        # 生成可视化
        net = Network(height="800px", width="100%", bgcolor="#1a1a1a", font_color="white")
        net.from_nx(G)
        net.toggle_physics(True)

        abs_html_path = os.path.abspath(output_html)
        net.write_html(abs_html_path)

        print(f"[+] 成功生成网页并尝试打开浏览器")
        webbrowser.open('file://' + abs_html_path)

    except Exception as e:
        print(f"[-] 脚本运行异常: {str(e)}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        generate_visual(sys.argv[1])