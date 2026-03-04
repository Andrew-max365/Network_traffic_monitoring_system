'''import pandas as pd
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
        print("用法: python scripts/visualizer.py <target_ip>")'''

import pandas as pd
import networkx as nx
from pyvis.network import Network
import sys
import os
import webbrowser
import math
import json

def generate_visual(target_ip):
    # 1. 路径处理
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    csv_path = os.path.join(project_root, "data", "subgraph_edges.csv")
    output_html = os.path.join(project_root, "data", "subgraph.html")

    print(f"[*] 前端渲染引擎启动: 正在对目标 {target_ip} 的连通子图进行安全画像")

    if not os.path.exists(csv_path):
        print(f"[-] 错误: 找不到子图数据文件 {csv_path}，请确保已执行数据提取")
        return

    try:
        # 2. 读取子图数据 (包含风险评分字段)
        df = pd.read_csv(csv_path)

        if df.empty:
            print(f"[-] 警告: {target_ip} 的连通子图为空")
            return

        G = nx.DiGraph()

        # 3. 统计节点信息：聚合流量并提取风险分
        # 我们假设同一个 IP 在 CSV 中的风险分是一致的
        node_profile = {}
        for _, r in df.iterrows():
            src, dst = str(r['Source']), str(r['Destination'])
            b = float(r['TotalBytes'])
            # 提取风险分 (对应 C++ 修改后的 CSV 结构)
            s_risk = int(r['SourceRisk'])
            d_risk = int(r['DestRisk'])

            if src not in node_profile:
                node_profile[src] = {"risk": s_risk, "traffic": 0.0}
            node_profile[src]["traffic"] += b

            if dst not in node_profile:
                node_profile[dst] = {"risk": d_risk, "traffic": 0.0}
            node_profile[dst]["traffic"] += b

        # 4. 将节点加入网络图，并进行安全画像上色
        for n, info in node_profile.items():
            risk = info["risk"]
            total_b = info["traffic"]

            # 安全画像颜色映射逻辑
            if risk >= 61:
                color = "#ff3333"  # 高危：红色
                status_text = "🔴 高危 (High Risk)"
            elif risk >= 31:
                color = "#ffcc00"  # 警告：黄色
                status_text = "🟡 警告 (Warning)"
            else:
                color = "#4da6ff"  # 正常：蓝色
                status_text = "🟢 正常 (Safe)"

            # 计算节点大小
            base_size = 32 if n == target_ip else 15
            size = base_size + min(20, math.log10(total_b + 1) * 3)

            # 组装鼠标悬停提示 (HTML 格式)
            title = f"""
            <div style='font-family:Consolas; color:white; background-color:rgba(0,0,0,0.7); padding:10px; border-radius:5px;'>
                <b style='font-size:14px;'>节点: {n}</b><br/>
                <hr style='margin:5px 0;'>
                画像状态: {status_text}<br/>
                <b>安全风险评分: {risk} / 100</b><br/>
                相关总流量: {int(total_b)} bytes
            </div>
            """

            # 突出显示核心查询节点 (金边)
            border_width = 4 if n == target_ip else 1
            border_color = "#ffd166" if n == target_ip else "#ffffff"

            G.add_node(n,
                       color=color,
                       size=size,
                       title=title,
                       borderWidth=border_width,
                       color_border=border_color)

        # 5. 将边加入网络图
        for _, r in df.iterrows():
            src, dst = str(r['Source']), str(r['Destination'])
            total_b = float(r['TotalBytes'])
            https_b = float(r['HttpsBytes'])
            duration = float(r['Duration'])

            # 如果包含 HTTPS 流量，边显示为暖色调
            edge_color = "#34A853" if https_b > 0 else "#4da6ff"

            edge_title = (
                f"会话: {src} -> {dst}<br/>"
                f"流量: {int(total_b)} bytes<br/>"
                f"时长: {duration:.3f} s"
            )

            G.add_edge(
                src, dst,
                color=edge_color,
                value=math.log10(total_b + 1),  # 边宽根据流量对数缩放
                title=edge_title,
                arrows="to",
                smooth={"type": "curvedCW", "roundness": 0.1}
            )

        # 6. 配置物理引擎 (采用您喜欢的温和、动感且不爆炸的配置)
        net = Network(height="860px", width="100%", bgcolor="#111111", font_color="white", directed=True)
        net.from_nx(G)

        visual_options = {
            "nodes": {
                "shape": "dot",
                "font": {"size": 16, "color": "#ffffff"},
                "borderWidth": 1.5
            },
            "edges": {
                "scaling": {"min": 1, "max": 8, "label": False},
                "smooth": {"enabled": True, "type": "dynamic"},
                "color": {"inherit": False},
                "selectionWidth": 2
            },
            "physics": {
                "enabled": True,
                "solver": "barnesHut",
                "barnesHut": {
                    "gravitationalConstant": -25000,  # 负值越大排斥越强，这个值能让节点撑开但不至于飞走
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

        print(f"[+] 安全画像渲染完毕！已自动打开浏览器...")
        webbrowser.open("file://" + abs_html_path)

    except Exception as e:
        print(f"[-] 脚本运行异常: {str(e)}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        generate_visual(sys.argv[1])
    else:
        print("用法: python scripts/visualizer.py <target_ip>")