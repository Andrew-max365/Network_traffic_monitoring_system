from scapy.all import rdpcap
from scapy.layers.inet import IP, TCP, UDP
import csv
import os

def extract_pcap(pcap_file, output_csv):
    packets = rdpcap(pcap_file)
    with open(output_csv, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['src', 'dst', 'protocol', 'length']) # 表头

        for pkt in packets:
            if IP in pkt:
                src = pkt[IP].src
                dst = pkt[IP].dst
                proto = "TCP" if TCP in pkt else ("UDP" if UDP in pkt else "OTHER")
                length = len(pkt)
                writer.writerow([src, dst, proto, length])

if __name__ == "__main__":
    # 确保目录存在
    os.makedirs('data', exist_ok=True)
    print("正在解析 pcap 文件...")
    extract_pcap('data/real_data.pcap', 'data/pcap_data.csv')
    print("提取完成，存至 data/pcap_data.csv")