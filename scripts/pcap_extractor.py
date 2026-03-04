from scapy.all import rdpcap
from scapy.layers.inet import IP, TCP, UDP
import csv
import os

def extract_pcap(pcap_file, output_csv):
    try:
        packets = rdpcap(pcap_file)
    except FileNotFoundError:
        print(f"[-] 错误: 找不到文件 {pcap_file}。请确保该 pcap 文件存在。")
        return

    # 用于聚合会话流的字典
    # Key: (src_ip, dst_ip, protocol_num, src_port, dst_port)
    flows = {}

    for pkt in packets:
        if IP in pkt:
            src = pkt[IP].src
            dst = pkt[IP].dst
            proto = pkt[IP].proto  # 直接提取数字协议号 (如 6=TCP, 17=UDP, 1=ICMP)

            sport = ""
            dport = ""

            # 提取端口号
            if TCP in pkt:
                sport = pkt[TCP].sport
                dport = pkt[TCP].dport
            elif UDP in pkt:
                sport = pkt[UDP].sport
                dport = pkt[UDP].dport

            length = len(pkt)
            time = float(pkt.time)

            # 定义五元组作为流的唯一标识
            flow_key = (src, dst, proto, sport, dport)

            if flow_key not in flows:
                # 初始化新的会话流
                flows[flow_key] = {
                    'bytes': 0,
                    'start_time': time,
                    'end_time': time
                }

            # 累加流量并更新会话的结束时间
            flows[flow_key]['bytes'] += length
            flows[flow_key]['end_time'] = max(flows[flow_key]['end_time'], time)
            flows[flow_key]['start_time'] = min(flows[flow_key]['start_time'], time)

    # 导出为 C++ 后端兼容的 CSV 格式
    with open(output_csv, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        # 严格匹配 utils.cpp 需要的 7 个表头字段
        writer.writerow(['Source', 'Destination', 'Protocol', 'SrcPort', 'DstPort', 'DataSize', 'Duration'])

        for key, data in flows.items():
            src, dst, proto, sport, dport = key
            # 计算会话持续时间
            duration = data['end_time'] - data['start_time']

            writer.writerow([
                src,
                dst,
                proto,
                sport,
                dport,
                data['bytes'],
                f"{duration:.3f}"
            ])

if __name__ == "__main__":
    # 确保目录存在
    os.makedirs('data', exist_ok=True)
    print("[*] 正在解析 pcap 文件并聚合会话流信息 (这可能需要一些时间)...")

    # 提取文件
    extract_pcap('data/real_data.pcap', 'data/pcap_data.csv')

    print("[+] 提取与转换完成！")
    print("    输出文件: data/pcap_data.csv")
    print("    该文件现在已完全兼容系统的基础格式，可以在 GUI 界面中直接选择加载了。")