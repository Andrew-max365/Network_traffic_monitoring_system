import pandas as pd
import networkx as nx
from pyvis.network import Network
import sys
import os
import webbrowser
import math
import json  # 导入 json 模块用于安全转换配置

def generate_visual(target_ip):
    # 路径处理
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    csv_path = os.path.join(project_root, "data", "network_data.csv")
    output_html = os.path.join(project_root, "data", "subgraph.html")

    print(f"[*] 正在分析节点: {target_ip}")

    if not os.path.exists(csv_path):
        print(f"[-] 错误: 找不到数据文件 {csv_path}")
        return

    try:
        df = pd.read_csv(csv_path)

        required = ['Source', 'Destination', 'Protocol', 'DstPort', 'DataSize']
        for c in required:
            if c not in df.columns:
                print(f"[-] 错误: 缺少列 {c}")
                return

        # 类型归一化
        df['Protocol'] = pd.to_numeric(df['Protocol'], errors='coerce')
        df['DstPort'] = pd.to_numeric(df['DstPort'], errors='coerce')
        df['DataSize'] = pd.to_numeric(df['DataSize'], errors='coerce').fillna(0)

        # 只取目标IP相关的一阶子图
        sub_df = df[(df['Source'] == target_ip) | (df['Destination'] == target_ip)].copy()
        if sub_df.empty:
            print(f"[-] 警告: 流量库中没有关于 {target_ip} 的记录")
            return

        # 按方向聚合
        agg = (
            sub_df
            .groupby(['Source', 'Destination'], as_index=False)
            .agg(
                total_bytes=('DataSize', 'sum'),
                pkt_count=('DataSize', 'count'),
                https_bytes=('DataSize', lambda s: 0.0)
            )
        )

        # 重新计算每条有向边的 HTTPS 字节
        https_df = sub_df[(sub_df['Protocol'] == 6) & (sub_df['DstPort'] == 443)]
        https_agg = (
            https_df
            .groupby(['Source', 'Destination'], as_index=False)
            .agg(https_bytes=('DataSize', 'sum'))
        )

        agg = agg.drop(columns=['https_bytes']).merge(
            https_agg, how='left', on=['Source', 'Destination']
        )
        agg['https_bytes'] = agg['https_bytes'].fillna(0.0)

        G = nx.DiGraph()

        # 节点统计
        node_total = {}
        for _, r in agg.iterrows():
            src, dst = str(r['Source']), str(r['Destination'])
            b = float(r['total_bytes'])
            node_total[src] = node_total.get(src, 0.0) + b
            node_total[dst] = node_total.get(dst, 0.0) + b

        # 加节点
        for n, total_b in node_total.items():
            if n == target_ip:
                color = "#ffd166"
                size = 32
                title = f"<b>{n}</b><br/>中心节点<br/>相关总流量: {int(total_b)} bytes"
            else:
                color = "#97c2fc"
                size = 12 + min(18, math.log10(total_b + 1) * 4)
                title = f"{n}<br/>相关总流量: {int(total_b)} bytes"
            G.add_node(n, color=color, size=size, title=title)

        # 加边
        for _, r in agg.iterrows():
            src, dst = str(r['Source']), str(r['Destination'])
            total_b = float(r['total_bytes'])
            pkt_c = int(r['pkt_count'])
            https_b = float(r['https_bytes'])

            is_https_heavy = https_b > 0
            color = "#ff4d4d" if is_https_heavy else "#4da6ff"

            title = (
                f"{src} -> {dst}<br/>"
                f"总流量: {int(total_b)} bytes<br/>"
                f"记录条数: {pkt_c}<br/>"
                f"HTTPS字节(TCP&&DstPort=443): {int(https_b)}"
            )

            # --- 修改：使用 value 属性让 pyvis 自动处理粗细 ---
            G.add_edge(
                src, dst,
                color=color,
                value=total_b,  # 这里的 value 会被 scaling 映射到粗细
                title=title,
                arrows="to",
                smooth={"type": "curvedCW", "roundness": 0.08}
            )

        net = Network(
            height="860px",
            width="100%",
            bgcolor="#111111",
            font_color="white",
            directed=True
        )

        net.from_nx(G)

        # 使用 Python 字典定义配置，避免 JSON 解析错误
        visual_options = {
            "nodes": {
                "shape": "dot",
                "font": {"size": 16, "color": "#ffffff"},
                "borderWidth": 1.5
            },
            "edges": {
                "scaling": {
                    "min": 1,      # 最小流量宽度
                    "max": 10,     # 最大流量宽度 (可以根据需要调大)
                    "label": False
                },
                "smooth": {"enabled": True, "type": "dynamic"},
                "color": {"inherit": False},
                "selectionWidth": 2
            },
            "interaction": {
                "hover": True,
                "tooltipDelay": 120,
                "navigationButtons": True,
                "keyboard": True
            },
            "physics": {
                "enabled": True,
                "solver": "forceAtlas2Based",
                "forceAtlas2Based": {
                    "gravitationalConstant": -55,
                    "centralGravity": 0.01,
                    "springLength": 120,
                    "springConstant": 0.07,
                    "damping": 0.45,
                    "avoidOverlap": 0.7
                },
                "stabilization": {
                    "enabled": True,
                    "iterations": 1200,
                    "updateInterval": 50
                }
            }
        }

        # 将字典转换为 JSON 字符串传入，解决报错问题
        net.set_options(json.dumps(visual_options))

        abs_html_path = os.path.abspath(output_html)
        net.write_html(abs_html_path)

        print(f"[+] 已生成子图: {abs_html_path}")
        webbrowser.open("file://" + abs_html_path)

    except Exception as e:
        print(f"[-] 脚本运行异常: {str(e)}")


if __name__ == "__main__":
    if len(sys.argv) > 1:
        generate_visual(sys.argv[1])
    else:
        print("用法: python scripts/visualizer.py <target_ip>")