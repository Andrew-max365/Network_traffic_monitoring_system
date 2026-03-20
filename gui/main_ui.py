import os
import sys
import subprocess
import customtkinter as ctk
from tkinter import messagebox

# 全局配置
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

# ==========================================
# 1. 通用输入弹窗组件
# ==========================================
class ModernInputDialog(ctk.CTkToplevel):
    def __init__(self, title, message, callback):
        super().__init__()
        self.title(title)
        self.geometry("500x320")
        self.callback = callback
        self.attributes("-topmost", True)
        self.resizable(False, False)

        self.label = ctk.CTkLabel(self, text=message, font=ctk.CTkFont(size=18, weight="bold"))
        self.label.pack(pady=(40, 20))
        self.entry = ctk.CTkEntry(self, width=380, height=55, font=ctk.CTkFont(size=22))
        self.entry.pack(pady=10)
        self.entry.focus_set()

        self.btn = ctk.CTkButton(self, text="确 定", height=45, width=200, command=self.on_confirm)
        self.btn.pack(pady=30)
        self.bind("<Return>", lambda e: self.on_confirm())

    def on_confirm(self):
        val = self.entry.get().strip()
        if val: self.callback(val); self.destroy()

class MultiInputDialog(ctk.CTkToplevel):
    def __init__(self, title, labels, callback):
        super().__init__()
        self.title(title)
        self.geometry(f"450x{150 + len(labels)*70}")
        self.callback = callback
        self.entries = []
        self.attributes("-topmost", True)

        for label_text in labels:
            ctk.CTkLabel(self, text=label_text, font=ctk.CTkFont(size=14, weight="bold")).pack(pady=(15, 5))
            entry = ctk.CTkEntry(self, width=350, height=35)
            entry.pack()
            self.entries.append(entry)

        ctk.CTkButton(self, text="确认提交", height=40, command=self.on_confirm).pack(pady=25)
        self.bind("<Return>", lambda e: self.on_confirm())

    def on_confirm(self):
        values = [e.get().strip() for e in self.entries]
        if all(values): self.callback(values); self.destroy()


class FileSelectDialog(ctk.CTkToplevel):
    def __init__(self, title, files, callback):
        super().__init__()
        self.title(title)
        self.geometry("400x200")
        self.callback = callback
        self.attributes("-topmost", True)
        self.resizable(False, False)

        ctk.CTkLabel(self, text="请选择要加载的 CSV 数据集:", font=ctk.CTkFont(size=16, weight="bold")).pack(pady=(30, 10))

        # 下拉选择框
        self.combo = ctk.CTkComboBox(self, values=files, width=300, font=ctk.CTkFont(size=14))
        self.combo.pack(pady=10)
        if files:
            self.combo.set(files[0]) # 默认选中第一个

        ctk.CTkButton(self, text="确 定 加 载", height=40, command=self.on_confirm).pack(pady=20)

    def on_confirm(self):
        val = self.combo.get()
        if val:
            self.callback(val)
        self.destroy()


# ==========================================
# 2. Sentinel Dashboard 主界面
# ==========================================

class SentinelDashboard(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("网络流量监控系统 - Sentinel Dashboard")
        self.geometry("1200x850")

        # 核心路径环境
        gui_dir = os.path.dirname(os.path.abspath(__file__))
        self.project_root = os.path.dirname(gui_dir)
        exe_path = os.path.join(self.project_root, "cmake-build-debug", "Network_traffic_monitoring_system.exe")
        self.log_path = os.path.join(self.project_root, "data", "ui_bridge.log")

        # 启动后端进程
        try:
            self.process = subprocess.Popen(
                [exe_path], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                text=True, bufsize=1, cwd=self.project_root,
                creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0
            )
        except Exception as e:
            messagebox.showerror("启动失败", f"无法连接后端核心: {e}")
            sys.exit(1)

        self.create_layout()
        self.last_pos = os.path.getsize(self.log_path) if os.path.exists(self.log_path) else 0
        self.poll_log_file()
        self.protocol("WM_DELETE_WINDOW", self.on_closing)

    def create_layout(self):
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)

        # --- 左侧侧边栏 ---
        self.sidebar = ctk.CTkFrame(self, width=250, corner_radius=0)
        self.sidebar.grid(row=0, column=0, sticky="nsew")

        self.logo = ctk.CTkLabel(self.sidebar, text="SENTINEL\nSYSTEM", font=ctk.CTkFont(size=24, weight="bold"))
        self.logo.pack(pady=40)

        # 1. 基础功能菜单 (1-8号保持原样)
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

        # 映射逻辑，确保 1,5,6,7 依然能触发对应的输入框
        special_handlers = {
            "1": self.handle_data_load,
            "5": self.handle_path_query,
            "6": self.handle_star_topology,
            "7": self.handle_security_policy
        }

        for text, cmd in menu_items:
            handler = special_handlers.get(cmd, lambda c=cmd, t=text: self.execute_command(c, t))
            btn = ctk.CTkButton(self.sidebar, text=text, height=35, anchor="w",
                                command=handler, fg_color="transparent", hover_color="#333333")
            btn.pack(fill="x", padx=20, pady=2)

        # 2. 核心分析区 (像可视化一样独立出来的高级功能)
        ctk.CTkLabel(self.sidebar, text="核心安全分析", text_color="gray70", font=ctk.CTkFont(size=12)).pack(anchor="w", padx=25, pady=(30, 5))

        # 创新点按钮：多维风险画像评估 (采用醒目的红色)
        self.risk_btn = ctk.CTkButton(self.sidebar, text="🛡 节点风险评估", height=45,
                                      fg_color="#C0392B", hover_color="#A93226",
                                      font=ctk.CTkFont(size=15, weight="bold"),
                                      command=self.handle_risk_assessment)
        self.risk_btn.pack(fill="x", padx=20, pady=5)

        # 核心按钮：交互式图渲染 (采用深蓝色)
        self.viz_btn = ctk.CTkButton(self.sidebar, text="🕸 交互式图可视化", height=45,
                                     fg_color="#1F538D", hover_color="#14375E",
                                     font=ctk.CTkFont(size=15, weight="bold"),
                                     command=self.ask_ip_visualize)
        self.viz_btn.pack(fill="x", padx=20, pady=5)

        # 创新点按钮：节点类型画像评估 (采用深绿色系)
        self.profile_btn = ctk.CTkButton(self.sidebar, text="👤 节点类型分析", height=45,
                                         fg_color="#27AE60", hover_color="#1E8449",
                                         font=ctk.CTkFont(size=15, weight="bold"),
                                         command=lambda: self.execute_command("11", "NODE_PROFILING"))
        self.profile_btn.pack(fill="x", padx=20, pady=5)


        # 3. 底部退出
        self.exit_btn = ctk.CTkButton(self.sidebar, text="退出系统", fg_color="#444444", command=self.on_closing)
        self.exit_btn.pack(side="bottom", fill="x", padx=20, pady=30)

        # --- 右侧显示区 ---
        self.textbox = ctk.CTkTextbox(self, font=('Consolas', 15), text_color="#FFFFFF", fg_color="#000000")
        self.textbox.configure(state="disabled") # 只读模式
        self.textbox.grid(row=0, column=1, padx=20, pady=20, sticky="nsew")

    # ------------------ 功能处理逻辑 ------------------

    def execute_command(self, cmd, name):
        """通用指令执行逻辑"""
        self.textbox.configure(state="normal")
        self.textbox.delete("1.0", "end")
        self.textbox.insert("end", f">>> 正在执行: {name}\n")
        self.textbox.insert("end", "="*60 + "\n")
        self.textbox.configure(state="disabled")
        self.send_cmd(cmd)

    def handle_data_load(self):
        """扫描 data 目录并弹出文件选择框"""
        data_dir = os.path.join(self.project_root, "data")
        # 扫描 data 目录下所有的 .csv 文件
        csv_files = [f for f in os.listdir(data_dir) if f.endswith('.csv')]

        if not csv_files:
            messagebox.showwarning("无数据", "在 data/ 目录下没有找到任何 CSV 文件！")
            return

        def on_selected(filename):
            # 将相对路径传给 C++ 后端
            rel_path = f"data/{filename}"
            self.execute_command("1", f"加载数据集: {filename}")
            self.send_cmd(rel_path)

        # 弹出选择框
        FileSelectDialog("选择数据集", csv_files, on_selected)


    def handle_risk_assessment(self):
        """新按钮逻辑：触发 C++ 端的 A 指令"""
        self.execute_command("10", "多维节点安全风险画像评估")

    def handle_path_query(self):
        def callback(ips):
            self.execute_command("5", f"路径查询: {ips[0]} -> {ips[1]}")
            self.send_cmd(f"{ips[0]} {ips[1]}")
        MultiInputDialog("路径参数配置", ["源 IP:", "目的 IP:"], callback)

    def handle_star_topology(self):
        def callback(k):
            self.execute_command("6", f"星型拓扑探测 [k={k}]")
            self.send_cmd(k)
        ModernInputDialog("星型拓扑配置", "请输入容忍阈值 k:", callback)

    def handle_security_policy(self):
        def callback(ips):
            self.execute_command("7", "安全规则实时匹配")
            self.send_cmd(ips[0]); self.send_cmd(ips[1]); self.send_cmd(ips[2])
        MultiInputDialog("规则配置", ["管控 IP:", "范围起始 IP:", "范围结束 IP:"], callback)

    def ask_ip_visualize(self):
        def on_ip_received(ip):
            self.execute_command("9", f"生成节点子图: {ip}")
            self.send_cmd(ip)
        ModernInputDialog("可视化中心", "请输入要透视的节点 IP:", on_ip_received)

    # ------------------ 底层控制 ------------------

    def poll_log_file(self):
        if os.path.exists(self.log_path):
            try:
                with open(self.log_path, "r", encoding='utf-8', errors='ignore') as f:
                    f.seek(self.last_pos)
                    data = f.read()
                    if data:
                        self.textbox.configure(state="normal")
                        self.textbox.insert("end", data)
                        self.textbox.see("end")
                        self.textbox.configure(state="disabled")
                        self.last_pos = f.tell()
            except: pass
        if self.process.poll() is None: self.after(100, self.poll_log_file)

    def send_cmd(self, cmd):
        if self.process.poll() is None:
            self.process.stdin.write(f"{cmd}\n")
            self.process.stdin.flush()

    def on_closing(self):
        if self.process.poll() is None: self.process.terminate()
        self.destroy()

if __name__ == "__main__":
    app = SentinelDashboard()
    app.mainloop()