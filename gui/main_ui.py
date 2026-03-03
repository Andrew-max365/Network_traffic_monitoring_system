import os
import sys
import subprocess
import customtkinter as ctk
from tkinter import messagebox

# 全局配置
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

# ==========================================
# 1. 现代化大尺寸输入弹窗类
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
                                  placeholder_text="请输入 IP (如: 192.168.1.1)")
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
            messagebox.showwarning("输入错误", "IP 地址不能为空！")

# ==========================================
# 2. 增强型主监控界面
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

        menu_config = [
            ("📊 加载流量数据", "1", "DATA_LOAD"),
            ("📈 流量节点排行", "2", "TRAFFIC_RANK"),
            ("🔒 HTTPS 协议分析", "3", "HTTPS_SECURITY"),
            ("🔍 异常扫描检测", "4", "SCAN_DETECT"),
            ("🛤 路径拥塞查询", "5", "PATH_QUERY"),
            ("⭐ 星型拓扑识别", "6", "STAR_TOPOLOGY"),
            ("🛡 安全规则检查", "7", "SECURITY_POLICY"),
            ("📦 PCAP 离线提取", "8", "PCAP_EXTRACT")
        ]

        for text, cmd, name in menu_config:
            btn = ctk.CTkButton(self.sidebar, text=text, height=40,
                                command=lambda c=cmd, n=name: self.execute_command(c, n),
                                fg_color="transparent", text_color="#D1D1D1", hover_color="#333333", anchor="w")
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

        # 右侧显示区 (白色Consolas字体)
        self.textbox = ctk.CTkTextbox(self, font=('Consolas', 15), text_color="white", fg_color="#0A0A0A")
        self.textbox.grid(row=0, column=1, padx=20, pady=20, sticky="nsew")

    def execute_command(self, cmd, name):
        """清屏并发送指令"""
        if os.path.exists(self.log_path):
            self.last_pos = os.path.getsize(self.log_path)

        self.textbox.delete("1.0", "end")
        self.textbox.insert("end", f">>> STATUS: EXECUTING {name}...\n")
        self.textbox.insert("end", "-" * 70 + "\n\n")
        self.send_cmd(cmd)

    def ask_ip_visualize(self):
        """唤起大尺寸输入框"""
        def on_ip_received(ip):
            self.execute_command("9", f"SUBGRAPH_ANALYSIS [{ip}]")
            self.send_cmd(ip)

        ModernInputDialog("分析目标", "请输入要透视的节点 IP 地址:", on_ip_received)

    def poll_log_file(self):
        if os.path.exists(self.log_path):
            try:
                with open(self.log_path, "r", encoding='utf-8', errors='ignore') as f:
                    f.seek(self.last_pos)
                    data = f.read()
                    if data:
                        self.textbox.insert("end", data)
                        self.textbox.see("end")
                        self.last_pos = f.tell()
            except: pass

        if self.process.poll() is None:
            self.after(100, self.poll_log_file)

    def send_cmd(self, cmd):
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