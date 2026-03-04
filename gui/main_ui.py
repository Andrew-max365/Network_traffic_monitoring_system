import os
import sys
import subprocess
import customtkinter as ctk
from tkinter import messagebox

# 全局配置
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

# ==========================================
# 1. 单字段大尺寸输入弹窗类 (用于 IP 透视等单项输入)
# ==========================================
class ModernInputDialog(ctk.CTkToplevel):
    def __init__(self, title, message, callback):
        super().__init__()
        self.title(title)
        self.geometry("500x320")
        self.callback = callback

        # 窗口置顶并防止缩放
        self.attributes("-topmost", True)
        self.resizable(False, False)

        # 布局
        self.grid_columnconfigure(0, weight=1)

        # 标题/消息
        self.label = ctk.CTkLabel(self, text=message, font=ctk.CTkFont(size=20, weight="bold"))
        self.label.pack(pady=(40, 20))

        # 醒目的大输入框
        self.entry = ctk.CTkEntry(self, width=380, height=55, font=ctk.CTkFont(size=22),
                                  placeholder_text="请输入...")
        self.entry.pack(pady=10)
        self.entry.focus_set()

        # 确定按钮
        self.btn = ctk.CTkButton(self, text="确 定 并 分 析", height=45, width=200,
                                 font=ctk.CTkFont(size=18, weight="bold"),
                                 command=self.on_confirm)
        self.btn.pack(pady=30)

        # 绑定快捷键
        self.bind("<Return>", lambda e: self.on_confirm())
        self.bind("<Escape>", lambda e: self.destroy())

    def on_confirm(self):
        val = self.entry.get().strip()
        if val:
            self.callback(val)
            self.destroy()
        else:
            messagebox.showwarning("输入错误", "输入不能为空！")


# ==========================================
# 2. 多字段输入弹窗类 (用于 路径查询、安全规则 等多项输入)
# ==========================================
class MultiInputDialog(ctk.CTkToplevel):
    def __init__(self, title, labels, callback):
        super().__init__()
        self.title(title)
        # 根据字段数量动态调整窗口高度
        window_height = 150 + len(labels) * 70
        self.geometry(f"450x{window_height}")
        self.callback = callback
        self.entries = []

        self.attributes("-topmost", True)
        self.resizable(False, False)

        for label_text in labels:
            label = ctk.CTkLabel(self, text=label_text, font=ctk.CTkFont(size=15, weight="bold"))
            label.pack(pady=(15, 5))
            entry = ctk.CTkEntry(self, width=350, height=35)
            entry.pack(pady=0)
            self.entries.append(entry)

        if self.entries:
            self.entries[0].focus_set()

        self.btn = ctk.CTkButton(self, text="确 认 提 交", height=40, width=180,
                                 font=ctk.CTkFont(size=16, weight="bold"),
                                 command=self.on_confirm)
        self.btn.pack(pady=25)

        self.bind("<Return>", lambda e: self.on_confirm())
        self.bind("<Escape>", lambda e: self.destroy())

    def on_confirm(self):
        values = [e.get().strip() for e in self.entries]
        # 确保所有字段都已填写
        if all(values):
            self.callback(values)
            self.destroy()
        else:
            messagebox.showwarning("输入错误", "请填写完整所有相关字段！")


# ==========================================
# 3. 增强型主监控界面
# ==========================================
class SentinelDashboard(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("网络流量监控系统 - Sentinel Dashboard")
        self.geometry("1200x850")

        # 路径环境初始化
        gui_dir = os.path.dirname(os.path.abspath(__file__))
        self.project_root = os.path.dirname(gui_dir)
        exe_path = os.path.join(self.project_root, "cmake-build-debug", "Network_traffic_monitoring_system.exe")
        self.log_path = os.path.join(self.project_root, "data", "ui_bridge.log")

        # 启动后端进程
        try:
            self.process = subprocess.Popen(
                [exe_path],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1,
                cwd=self.project_root,
                creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0
            )
        except Exception as e:
            messagebox.showerror("启动失败", f"无法连接后端核心: {e}")
            sys.exit(1)

        self.create_layout()
        self.last_pos = os.path.getsize(self.log_path) if os.path.exists(self.log_path) else 0
        self.poll_log_file()

        # 窗口关闭协议
        self.protocol("WM_DELETE_WINDOW", self.on_closing)

    def create_layout(self):
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)

        # 左侧边栏
        self.sidebar = ctk.CTkFrame(self, width=250, corner_radius=0)
        self.sidebar.grid(row=0, column=0, sticky="nsew")

        self.logo = ctk.CTkLabel(self.sidebar, text="NETWORK\nMONITOR", font=ctk.CTkFont(size=24, weight="bold"))
        self.logo.pack(pady=50)

        # 动态功能映射表：区分哪些需要直接执行，哪些需要先弹窗收集输入
        dispatch_map = {
            "1": lambda: self.execute_command("1", "DATA_LOAD"),
            "2": lambda: self.execute_command("2", "TRAFFIC_RANK"),
            "3": lambda: self.execute_command("3", "HTTPS_SECURITY"),
            "4": lambda: self.execute_command("4", "SCAN_DETECT"),
            "5": self.handle_path_query,
            "6": self.handle_star_topology,
            "7": self.handle_security_policy,
            "8": lambda: self.execute_command("8", "PCAP_EXTRACT")
        }

        menu_items = [
            ("📊 加载流量数据", "1"),
            ("📈 流量节点排行", "2"),
            ("🔒 HTTPS 协议分析", "3"),
            ("🔍 异常扫描检测", "4"),
            ("🛤 路径拥塞查询", "5"),
            ("⭐ 星型拓扑识别", "6"),
            ("🛡 安全规则检查", "7"),
            ("📦 PCAP 离线提取", "8")
        ]

        for text, cmd in menu_items:
            btn = ctk.CTkButton(self.sidebar, text=text, height=40,
                                command=dispatch_map[cmd],
                                fg_color="transparent", text_color="#D1D1D1",
                                hover_color="#333333", anchor="w")
            btn.pack(fill="x", padx=20, pady=5)

        # 核心可视化按钮 (醒目配色)
        self.viz_btn = ctk.CTkButton(self.sidebar, text="🕸 交互式图可视化", height=40,
                                     fg_color="#1F538D", hover_color="#14375E",
                                     font=ctk.CTkFont(size=16, weight="bold"),
                                     command=self.ask_ip_visualize)
        self.viz_btn.pack(fill="x", padx=20, pady=40)

        # 底部退出
        self.exit_btn = ctk.CTkButton(self.sidebar, text="退出系统", fg_color="#7B241C", hover_color="#641E16", command=self.on_closing)
        self.exit_btn.pack(side="bottom", fill="x", padx=20, pady=30)

        # 右侧显示区 (白色Consolas字体，默认只读防止误删)
        self.textbox = ctk.CTkTextbox(self, font=('Consolas', 15), text_color="white", fg_color="#0A0A0A")
        self.textbox.configure(state="disabled") # 核心修复：禁止用户手动编辑
        self.textbox.grid(row=0, column=1, padx=20, pady=20, sticky="nsew")

    # ------------------ 业务交互处理逻辑 ------------------

    def execute_command(self, cmd, name):
        """清屏并发送基本指令"""
        if os.path.exists(self.log_path):
            self.last_pos = os.path.getsize(self.log_path)

        # 临时开启编辑权限来清空和写入系统提示
        self.textbox.configure(state="normal")
        self.textbox.delete("1.0", "end")
        self.textbox.insert("end", f">>> STATUS: EXECUTING {name}...\n")
        self.textbox.insert("end", "-" * 70 + "\n\n")
        self.textbox.configure(state="disabled")

        self.send_cmd(cmd)

    def handle_path_query(self):
        """处理菜单项 5: 路径查找 (需两个输入)"""
        def callback(ips):
            self.execute_command("5", f"PATH_ANALYSIS: {ips[0]} -> {ips[1]}")
            # C 语言中是 scanf("%s %s", ...)，可以用空格拼起来一起发过去
            self.send_cmd(f"{ips[0]} {ips[1]}")

        MultiInputDialog("路径拥塞查询", ["源节点 IP:", "目的节点 IP:"], callback)

    def handle_star_topology(self):
        """处理菜单项 6: 星型拓扑 (需阈值 k)"""
        def callback(k_str):
            self.execute_command("6", f"STAR_TOPOLOGY_CHECK [k={k_str}]")
            # 紧接着发送数字 k 给后端的 scanf
            self.send_cmd(k_str)

        ModernInputDialog("星型拓扑配置", "请输入允许的非叶子节点数 k:", callback)

    def handle_security_policy(self):
        """处理菜单项 7: 安全规则检查 (需三个输入)"""
        def callback(ips):
            self.execute_command("7", "SECURITY_RULE_CHECK")
            # 依序发给后端三个连续的 scanf("%s")
            self.send_cmd(ips[0])
            self.send_cmd(ips[1])
            self.send_cmd(ips[2])

        MultiInputDialog("安全规则配置", ["管控目标 IP:", "限制区间起始 IP:", "限制区间结束 IP:"], callback)

    def ask_ip_visualize(self):
        """处理菜单项 9: 图可视化 (需目标 IP)"""
        def on_ip_received(ip):
            self.execute_command("9", f"SUBGRAPH_ANALYSIS [{ip}]")
            self.send_cmd(ip)

        ModernInputDialog("分析目标", "请输入要透视的节点 IP 地址:", on_ip_received)


    # ------------------ 核心系统控制 ------------------

    def poll_log_file(self):
        """定时轮询 C 程序输出的日志并更新到只读文本框"""
        if os.path.exists(self.log_path):
            try:
                with open(self.log_path, "r", encoding='utf-8', errors='ignore') as f:
                    f.seek(self.last_pos)
                    data = f.read()
                    if data:
                        # 核心修复：插入日志数据前开启编辑，写完立刻设为只读
                        self.textbox.configure(state="normal")
                        self.textbox.insert("end", data)
                        self.textbox.see("end")
                        self.textbox.configure(state="disabled")
                        self.last_pos = f.tell()
            except: pass

        if self.process.poll() is None:
            self.after(100, self.poll_log_file)

    def send_cmd(self, cmd):
        """将指令和参数发送到 C 程序的标准输入 (stdin)"""
        if self.process.poll() is None:
            self.process.stdin.write(f"{cmd}\n")
            self.process.stdin.flush()

    def on_closing(self):
        if self.process.poll() is None:
            self.process.terminate()
        self.destroy()

if __name__ == "__main__":
    app = SentinelDashboard()
    app.mainloop()