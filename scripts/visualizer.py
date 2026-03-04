import pandas as pd
import networkx as nx
from pyvis.network import Network
import sys
import os
import webbrowser
import math
import json

def generate_visual(target_ip):
    # 1. 路径处理，直接读取 C 程序刚吐出来的子图文件
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    csv_path = os.path.join(project_root, "data", "subgraph_edges.csv")
    output_html = os.path.join(project_root, "data", "subgraph.html")

    print(f"[*] 前端渲染引擎启动: 渲染目标 {target_ip} 的连通子图")

    if not os.path.exists(csv_path):
        print(f"[-] 错误: 找不到 C 程序生成的子图文件 {csv_path}")
        return

    try:
        # 2. 读取提纯好的子图数据
        df = pd.read_csv(csv_path)

        if df.empty:
            print(f"[-] 警告: {target_ip} 的连通子图为空")
            return

        G = nx.DiGraph()

        # 3. 统计该子图中各节点的总流量，用于计算气泡大小
        node_total = {}
        for _, r in df.iterrows():
            src, dst = str(r['Source']), str(r['Destination'])
            b = float(r['TotalBytes'])
            node_total[src] = node_total.get(src, 0.0) + b
            node_total[dst] = node_total.get(dst, 0.0) + b

        # 4. 将节点加入网络图
        for n, total_b in node_total.items():
            if n == target_ip:
                color = "#ffd166"
                size = 32
                title = f"<b>{n}</b><br/>核心查询节点<br/>相关总流量: {int(total_b)} bytes"
            else:
                color = "#97c2fc"
                size = 12 + min(18, math.log10(total_b + 1) * 4)
                title = f"{n}<br/>相关总流量: {int(total_b)} bytes"
            G.add_node(n, color=color, size=size, title=title)

        # 5. 将边加入网络图
        for _, r in df.iterrows():
            src, dst = str(r['Source']), str(r['Destination'])
            total_b = float(r['TotalBytes'])
            https_b = float(r['HttpsBytes'])
            duration = float(r['Duration'])

            is_https_heavy = https_b > 0
            color = "#ff4d4d" if is_https_heavy else "#4da6ff"

            title = (
                f"{src} -> {dst}<br/>"
                f"会话总流量: {int(total_b)} bytes<br/>"
                f"包含 HTTPS 字节: {int(https_b)}<br/>"
                f"总时长: {duration:.3f} 秒"
            )

            G.add_edge(
                src, dst,
                color=color,
                value=total_b,  # 用于边宽自动缩放
                title=title,
                arrows="to",
                smooth={"type": "curvedCW", "roundness": 0.08}
            )

        # 6. 配置物理引擎选项 (继承您原来的优秀配置)
        net = Network(height="860px", width="100%", bgcolor="#111111", font_color="white", directed=True)
        net.from_nx(G)

        visual_options = {
            "nodes": {
                "shape": "dot",
                "font": {"size": 16, "color": "#ffffff"},
                "borderWidth": 1.5
            },
            "edges": {
                "scaling": {"min": 1, "max": 10, "label": False},
                "smooth": {"enabled": True, "type": "dynamic"},
                "color": {"inherit": False},
                "selectionWidth": 2
            },
            "physics": {
                "enabled": True,
                "solver": "barnesHut",
                "barnesHut": {
                    "gravitationalConstant": -20000,  # 负值越大排斥越强，这个值能让节点撑开但不至于飞走
                    "centralGravity": 0.05,           # 适当加强向心力，把节点束缚在中心附近，产生凝聚感
                    "springLength": 125,              # 增加弹簧长度，给节点游走留出空间
                    "springConstant": 0.1,            # 极软的弹簧，让动作像在水里一样慢条斯理
                    "damping": 0.5,                   # 适中的阻尼，能有效吸收“爆炸”动能，防止震荡
                    "avoidOverlap": 0.5
                },
                "maxVelocity": 3,                    # 限制最大速度，杜绝任何瞬时的剧烈跳动
                "minVelocity": 0.01,                 # 极低的停止阈值，确保即使在很慢时引擎也不休眠
                "stabilization": {
                    "enabled": True,
                    "iterations": 600,               # 【核心修改】3 秒左右的预计算
                    "updateInterval": 10
                }
            }
        }

        net.set_options(json.dumps(visual_options))
        abs_html_path = os.path.abspath(output_html)
        net.write_html(abs_html_path)

        print(f"[+] 渲染完毕！已自动打开浏览器...")
        webbrowser.open("file://" + abs_html_path)

    except Exception as e:
        print(f"[-] 脚本运行异常: {str(e)}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        generate_visual(sys.argv[1])
    else:
        print("用法: python scripts/visualizer.py <target_ip>")