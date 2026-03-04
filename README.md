# 网络流量分析与异常检测系统

## 🚀 核心功能 (Core Features)

### 1. 深度流量审计 (C++ Engine)
**高效图拓扑构建**：基于邻接表结构（Adjacency List）存储网络节点及会话信息 ，支持源/目的 IP、协议类型（TCP/UDP/ICMP）、数据大小及持续时间的完整记录。
**流量多维排序**：实现对全节点总流量的快速排序，并支持针对 HTTPS（TCP:443）协议的专项流量排行榜。
* **智能路径规划**：
    * **最小跳数路径**：通过 BFS 算法计算节点间的最短逻辑链路。
    * **最小拥塞路径**：基于改进的 Dijkstra 算法，以“流量/时长”为代价动态寻找负载最小路径。

### 2. 安全威胁检测
* **异常扫描识别**：自动筛选并标记“单向流量占比 > 80%”的可疑扫描节点，辅助定位潜在攻击源。
* **星型拓扑检测**：智能识别连接数超过 20 且符合星型辐射特征的核心枢纽节点。
* **安全规则审计**：支持自定义目标 IP 与范围区间的黑名单管控，实时拦截违规通信行为。

### 3. 现代化交互与可视化
* **Sentinel Dashboard**：基于 `CustomTkinter` 构建的深色模式 GUI，实现 C++ 后端进程的无缝调度与日志实时回显。
* **交互式图透视**：利用 `NetworkX` 与 `Pyvis` 引擎，将复杂的网络子图转化为可动态拖拽、缩放的交互式 HTML 拓扑图。
* **离线数据提取**：内置基于 `Scapy` 的数据转换脚本，支持从原始 `.pcap` 抓包文件中自动化提取结构化 CSV 数据。

---

## 🛠️ 技术栈 (Tech Stack)

| 领域 | 选型 | 说明 |
| :--- | :--- | :--- |
| **底层核心** | **C++ 17** | 负责高性能图运算与数据处理 |
| **图形界面** | **Python / CustomTkinter** | 提供现代化的用户交互体验 |
| **图算法** | **Dijkstra / BFS / DSU** | 解决路径搜索与连通性问题 |
| **可视化** | **Pyvis / NetworkX** | 实现动态、可交互的拓扑呈现 |
| **数据分析** | **Pandas / Scapy** | 负责流量记录的清洗、转换与提取 |

---

## 📂 项目结构 (Project Structure)

```text
.
├── cmake-build-debug/     # C++ 编译目标文件夹
├── data/                  # 存放 network_data.csv 及生成的 HTML 可视化报告
├── include/               # C++ 头文件目录 (.h)
├── scripts/               # Python 辅助脚本
│   ├── visualizer.py      # Pyvis 拓扑渲染引擎
│   └── pcap_extractor.py  # PCAP 离线数据提取工具
├── src/                   # C++ 源代码目录 (.cpp)
│   ├── main.cpp           # 后端指令调度中心
│   ├── traffic.cpp        # 流量统计与安全规则实现
│   ├── path.cpp           # 路径搜索算法
│   └── utils.cpp          # CSV 解析与数据加载
├── main_ui.py             # Sentinel Dashboard (GUI 主程序)
└── README.md
