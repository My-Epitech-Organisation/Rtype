#!/usr/bin/env python3
"""
R-Type Bandwidth Monitor
Monitors UDP traffic on the game port and calculates bandwidth usage.

Usage:
    sudo python3 scripts/bandwidth_monitor.py [--port 4242] [--duration 60]
"""

import argparse
import socket
import time
import struct
from collections import defaultdict
from dataclasses import dataclass, field
from typing import Dict

# RTGP Protocol constants
MAGIC_BYTE = 0xA1
HEADER_SIZE = 16

# OpCode names for reporting
OPCODE_NAMES = {
    0x01: "C_CONNECT",
    0x02: "S_ACCEPT",
    0x03: "DISCONNECT",
    0x04: "C_GET_USERS",
    0x05: "R_GET_USERS",
    0x06: "S_UPDATE_STATE",
    0x07: "S_GAME_OVER",
    0x10: "S_ENTITY_SPAWN",
    0x11: "S_ENTITY_MOVE",
    0x12: "S_ENTITY_DESTROY",
    0x13: "S_ENTITY_HEALTH",
    0x14: "S_POWERUP_EVENT",
    0x15: "S_ENTITY_MOVE_BATCH",
    0x20: "C_INPUT",
    0x21: "S_UPDATE_POS",
    0xF0: "PING",
    0xF1: "PONG",
}


@dataclass
class PacketStats:
    """Statistics for a specific packet type."""
    count: int = 0
    total_bytes: int = 0
    min_size: int = 9999
    max_size: int = 0

    def add(self, size: int):
        self.count += 1
        self.total_bytes += size
        self.min_size = min(self.min_size, size)
        self.max_size = max(self.max_size, size)

    @property
    def avg_size(self) -> float:
        return self.total_bytes / self.count if self.count > 0 else 0


@dataclass
class BandwidthStats:
    """Overall bandwidth statistics."""
    start_time: float = field(default_factory=time.time)
    packets_received: int = 0
    packets_sent: int = 0
    bytes_received: int = 0
    bytes_sent: int = 0
    packet_stats: Dict[int, PacketStats] = field(default_factory=lambda: defaultdict(PacketStats))
    invalid_packets: int = 0

    def add_packet(self, data: bytes, is_incoming: bool):
        size = len(data)

        if is_incoming:
            self.packets_received += 1
            self.bytes_received += size
        else:
            self.packets_sent += 1
            self.bytes_sent += size

        # Parse RTGP header
        if size >= HEADER_SIZE and data[0] == MAGIC_BYTE:
            opcode = data[1]
            self.packet_stats[opcode].add(size)
        else:
            self.invalid_packets += 1

    @property
    def duration(self) -> float:
        return time.time() - self.start_time

    @property
    def rx_bandwidth_kbps(self) -> float:
        return (self.bytes_received * 8) / (self.duration * 1000) if self.duration > 0 else 0

    @property
    def tx_bandwidth_kbps(self) -> float:
        return (self.bytes_sent * 8) / (self.duration * 1000) if self.duration > 0 else 0

    @property
    def total_bandwidth_kbps(self) -> float:
        return self.rx_bandwidth_kbps + self.tx_bandwidth_kbps

    @property
    def packets_per_second(self) -> float:
        return (self.packets_received + self.packets_sent) / self.duration if self.duration > 0 else 0


def parse_header(data: bytes) -> dict:
    """Parse RTGP header from raw bytes."""
    if len(data) < HEADER_SIZE:
        return None

    magic, opcode, payload_size, user_id, seq_id, ack_id, flags = struct.unpack(
        "!BBHIHH B", data[:13]
    )

    if magic != MAGIC_BYTE:
        return None

    return {
        "opcode": opcode,
        "payload_size": payload_size,
        "user_id": user_id,
        "seq_id": seq_id,
        "ack_id": ack_id,
        "flags": flags,
    }


def format_bytes(num_bytes: int) -> str:
    """Format bytes to human readable."""
    if num_bytes < 1024:
        return f"{num_bytes} B"
    elif num_bytes < 1024 * 1024:
        return f"{num_bytes / 1024:.2f} KB"
    else:
        return f"{num_bytes / (1024 * 1024):.2f} MB"


def print_stats(stats: BandwidthStats):
    """Print formatted statistics."""
    print("\n" + "=" * 60)
    print(f"{'R-Type Bandwidth Report':^60}")
    print("=" * 60)

    print(f"\n📊 Duration: {stats.duration:.1f} seconds")
    print(f"\n📥 Received: {stats.packets_received} packets ({format_bytes(stats.bytes_received)})")
    print(f"📤 Sent:     {stats.packets_sent} packets ({format_bytes(stats.bytes_sent)})")
    print(f"❌ Invalid:  {stats.invalid_packets} packets")

    print(f"\n📈 Bandwidth:")
    print(f"   RX: {stats.rx_bandwidth_kbps:.2f} Kbps")
    print(f"   TX: {stats.tx_bandwidth_kbps:.2f} Kbps")
    print(f"   Total: {stats.total_bandwidth_kbps:.2f} Kbps")
    print(f"   Packets/sec: {stats.packets_per_second:.1f}")

    if stats.packet_stats:
        print(f"\n📦 Packet Breakdown:")
        print(f"{'OpCode':<20} {'Count':>8} {'Bytes':>10} {'Avg Size':>10} {'%Traffic':>10}")
        print("-" * 60)

        total_bytes = stats.bytes_received + stats.bytes_sent
        sorted_stats = sorted(stats.packet_stats.items(), key=lambda x: x[1].total_bytes, reverse=True)

        for opcode, pstats in sorted_stats:
            name = OPCODE_NAMES.get(opcode, f"0x{opcode:02X}")
            pct = (pstats.total_bytes / total_bytes * 100) if total_bytes > 0 else 0
            print(f"{name:<20} {pstats.count:>8} {format_bytes(pstats.total_bytes):>10} "
                  f"{pstats.avg_size:>10.1f} {pct:>9.1f}%")

    print("\n" + "=" * 60)


def monitor_with_tcpdump(port: int, duration: int):
    """Monitor using tcpdump (requires sudo)."""
    import subprocess
    import tempfile
    import os

    print(f"🔍 Monitoring UDP port {port} for {duration} seconds...")
    print("   (Using tcpdump - requires root privileges)")

    stats = BandwidthStats()

    # Use tcpdump to capture packets
    with tempfile.NamedTemporaryFile(suffix='.pcap', delete=False) as f:
        pcap_file = f.name

    try:
        # Start tcpdump
        proc = subprocess.Popen(
            ['tcpdump', '-i', 'any', '-w', pcap_file, f'udp port {port}'],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )

        # Show progress
        for i in range(duration):
            time.sleep(1)
            print(f"\r⏱️  Capturing... {i+1}/{duration}s", end="", flush=True)
        print()

        proc.terminate()
        proc.wait()

        # Try multiple methods to parse pcap
        parsed = False

        # Method 1: Try tshark (comes with wireshark)
        if not parsed:
            parsed = parse_with_tshark(pcap_file, port, stats)

        # Method 2: Try dpkt
        if not parsed:
            parsed = parse_with_dpkt(pcap_file, port, stats)

        # Method 3: Try scapy
        if not parsed:
            parsed = parse_with_scapy(pcap_file, port, stats)

        # Method 4: Manual pcap parsing (no dependencies)
        if not parsed:
            parsed = parse_pcap_manual(pcap_file, port, stats)

        if not parsed:
            print("❌ Could not parse pcap file. Install one of:")
            print("   - tshark: sudo apt install tshark")
            print("   - scapy: pip install scapy")
            print("   - dpkt: pip install dpkt")

    finally:
        if os.path.exists(pcap_file):
            os.unlink(pcap_file)

    return stats


def parse_with_tshark(pcap_file: str, port: int, stats: BandwidthStats) -> bool:
    """Parse pcap using tshark (part of Wireshark)."""
    import subprocess

    try:
        # Check if tshark is available
        subprocess.run(['tshark', '--version'], capture_output=True, check=True)
    except (subprocess.CalledProcessError, FileNotFoundError):
        return False

    print("📊 Parsing with tshark...")

    try:
        # Get packet details: frame length, udp.dstport, and raw data
        result = subprocess.run(
            ['tshark', '-r', pcap_file, '-T', 'fields',
             '-e', 'frame.len',
             '-e', 'udp.dstport',
             '-e', 'udp.srcport',
             '-e', 'data.data'],
            capture_output=True,
            text=True,
            check=True
        )

        for line in result.stdout.strip().split('\n'):
            if not line:
                continue

            parts = line.split('\t')
            if len(parts) >= 3:
                try:
                    frame_len = int(parts[0])
                    dst_port = int(parts[1]) if parts[1] else 0
                    src_port = int(parts[2]) if parts[2] else 0
                    hex_data = parts[3] if len(parts) > 3 else ""

                    # Convert hex to bytes for RTGP parsing
                    if hex_data:
                        # Remove colons if present
                        hex_data = hex_data.replace(':', '')
                        data = bytes.fromhex(hex_data)
                    else:
                        data = b'\xa1\x00' + b'\x00' * 14  # Fake header for size counting

                    is_incoming = dst_port == port
                    stats.add_packet(data, is_incoming)
                except (ValueError, IndexError):
                    continue

        return True

    except subprocess.CalledProcessError as e:
        print(f"⚠️  tshark error: {e}")
        return False


def parse_with_dpkt(pcap_file: str, port: int, stats: BandwidthStats) -> bool:
    """Parse pcap using dpkt library."""
    try:
        import dpkt
    except ImportError:
        return False

    print("📊 Parsing with dpkt...")

    try:
        with open(pcap_file, 'rb') as f:
            try:
                pcap = dpkt.pcap.Reader(f)
            except ValueError:
                # Try pcapng format
                f.seek(0)
                pcap = dpkt.pcapng.Reader(f)

            for ts, buf in pcap:
                try:
                    # Handle different link types
                    if len(buf) < 20:
                        continue

                    # Try to find UDP payload
                    # Skip link layer (varies: ethernet=14, linux cooked=16, loopback=4)
                    for offset in [14, 16, 4, 0]:
                        if offset + 20 > len(buf):
                            continue
                        # Check for IPv4
                        if (buf[offset] >> 4) == 4:
                            ip_header_len = (buf[offset] & 0x0f) * 4
                            protocol = buf[offset + 9]
                            if protocol == 17:  # UDP
                                udp_start = offset + ip_header_len
                                if udp_start + 8 <= len(buf):
                                    src_port = (buf[udp_start] << 8) | buf[udp_start + 1]
                                    dst_port = (buf[udp_start + 2] << 8) | buf[udp_start + 3]
                                    udp_len = (buf[udp_start + 4] << 8) | buf[udp_start + 5]
                                    payload = buf[udp_start + 8:udp_start + udp_len]
                                    is_incoming = dst_port == port
                                    stats.add_packet(bytes(payload), is_incoming)
                                    break
                except Exception:
                    continue

        return stats.packets_received > 0 or stats.packets_sent > 0

    except Exception as e:
        print(f"⚠️  dpkt error: {e}")
        return False


def parse_with_scapy(pcap_file: str, port: int, stats: BandwidthStats) -> bool:
    """Parse pcap using scapy library."""
    try:
        from scapy.all import rdpcap, UDP
    except ImportError:
        return False

    print("📊 Parsing with scapy...")

    try:
        packets = rdpcap(pcap_file)
        for pkt in packets:
            if UDP in pkt:
                udp = pkt[UDP]
                payload = bytes(udp.payload)
                is_incoming = udp.dport == port
                stats.add_packet(payload, is_incoming)

        return True

    except Exception as e:
        print(f"⚠️  scapy error: {e}")
        return False


def parse_pcap_manual(pcap_file: str, port: int, stats: BandwidthStats) -> bool:
    """Parse pcap file manually without external dependencies."""
    print("📊 Parsing pcap manually...")

    try:
        with open(pcap_file, 'rb') as f:
            # Read pcap global header (24 bytes)
            global_header = f.read(24)
            if len(global_header) < 24:
                return False

            magic = struct.unpack('I', global_header[0:4])[0]

            # Determine byte order
            if magic == 0xa1b2c3d4:
                byte_order = '<'  # Little endian
            elif magic == 0xd4c3b2a1:
                byte_order = '>'  # Big endian
            elif magic == 0x0a0d0d0a:
                print("   pcapng format detected, use tshark")
                return False
            else:
                return False

            # Read packets
            while True:
                # Packet header (16 bytes)
                pkt_header = f.read(16)
                if len(pkt_header) < 16:
                    break

                ts_sec, ts_usec, incl_len, orig_len = struct.unpack(
                    f'{byte_order}IIII', pkt_header
                )

                # Read packet data
                pkt_data = f.read(incl_len)
                if len(pkt_data) < incl_len:
                    break

                # Parse link layer + IP + UDP
                # Try different link layer offsets
                for link_offset in [16, 14, 4, 0]:  # Linux cooked, Ethernet, Loopback, None
                    if link_offset + 28 > len(pkt_data):
                        continue

                    ip_start = link_offset

                    # Check IPv4
                    if (pkt_data[ip_start] >> 4) != 4:
                        continue

                    ip_header_len = (pkt_data[ip_start] & 0x0f) * 4
                    protocol = pkt_data[ip_start + 9]

                    if protocol != 17:  # Not UDP
                        continue

                    udp_start = ip_start + ip_header_len
                    if udp_start + 8 > len(pkt_data):
                        continue

                    src_port = (pkt_data[udp_start] << 8) | pkt_data[udp_start + 1]
                    dst_port = (pkt_data[udp_start + 2] << 8) | pkt_data[udp_start + 3]
                    udp_len = (pkt_data[udp_start + 4] << 8) | pkt_data[udp_start + 5]

                    payload_start = udp_start + 8
                    payload_end = udp_start + udp_len
                    payload = pkt_data[payload_start:payload_end]

                    is_incoming = dst_port == port
                    stats.add_packet(bytes(payload), is_incoming)
                    break

        return stats.packets_received > 0 or stats.packets_sent > 0

    except Exception as e:
        print(f"⚠️  Manual parsing error: {e}")
        return False


def monitor_with_socket(port: int, duration: int, is_server: bool = True):
    """Monitor by sniffing on a raw socket (simplified approach)."""
    print(f"🔍 Monitoring UDP port {port} for {duration} seconds...")

    stats = BandwidthStats()

    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.settimeout(0.1)

    # For testing, we'll just count what we can see
    print("⚠️  Note: Direct socket monitoring requires the game to run separately.")
    print("   For full capture, use: sudo tcpdump -i any udp port 4242 -w capture.pcap")

    end_time = time.time() + duration

    try:
        while time.time() < end_time:
            try:
                data, addr = sock.recvfrom(1500)
                stats.add_packet(data, True)
            except socket.timeout:
                continue
            except Exception as e:
                pass

            # Print live stats every 5 seconds
            if int(stats.duration) % 5 == 0 and stats.duration > 0:
                print(f"\r⏱️  {stats.duration:.0f}s | "
                      f"RX: {stats.rx_bandwidth_kbps:.1f} Kbps | "
                      f"Packets: {stats.packets_received}", end="", flush=True)

    except KeyboardInterrupt:
        print("\n\n⏹️  Monitoring stopped by user")

    return stats


def calculate_theoretical_bandwidth():
    """Calculate theoretical bandwidth requirements."""
    print("\n" + "=" * 60)
    print(f"{'Theoretical Bandwidth Calculator':^60}")
    print("=" * 60)

    # Packet sizes
    packets = {
        "S_ENTITY_MOVE": 36,   # Header(16) + EntityId(4) + x,y(8) + vx,vy(8)
        "S_ENTITY_SPAWN": 41,  # Header(16) + EntityId(4) + Type(1) + x,y(8) + vx,vy(8) + extra(4)
        "C_INPUT": 17,         # Header(16) + InputMask(1)
        "S_ENTITY_DESTROY": 20, # Header(16) + EntityId(4)
        "PING/PONG": 16,       # Header only
    }

    print("\n📦 Packet Sizes:")
    for name, size in packets.items():
        print(f"   {name}: {size} bytes")

    # Scenarios
    print("\n📊 Bandwidth per scenario (Server → Client):")
    print(f"{'Entities':>10} {'Tick Rate':>10} {'Move Packets':>15} {'Bandwidth':>12}")
    print("-" * 50)

    for entities in [5, 10, 20, 50]:
        for tickrate in [20, 30, 60]:
            bytes_per_sec = packets["S_ENTITY_MOVE"] * tickrate * entities
            kbps = bytes_per_sec * 8 / 1000
            print(f"{entities:>10} {tickrate:>10} Hz {entities * tickrate:>15}/s {kbps:>11.1f} Kbps")

    print("\n💡 Recommendations:")
    print("   - Target: < 100 Kbps per client for good experience")
    print("   - 10 entities @ 60 Hz = 172.8 Kbps (consider 30 Hz updates)")
    print("   - 10 entities @ 30 Hz = 86.4 Kbps (good target)")


def main():
    parser = argparse.ArgumentParser(description="R-Type Bandwidth Monitor")
    parser.add_argument("--port", type=int, default=4242, help="UDP port to monitor")
    parser.add_argument("--duration", type=int, default=30, help="Duration in seconds")
    parser.add_argument("--calculate", action="store_true", help="Show theoretical calculations only")
    parser.add_argument("--tcpdump", action="store_true", help="Use tcpdump for capture (requires sudo)")

    args = parser.parse_args()

    if args.calculate:
        calculate_theoretical_bandwidth()
        return

    print("🎮 R-Type Bandwidth Monitor")
    print(f"   Port: {args.port}")
    print(f"   Duration: {args.duration}s")
    print()

    if args.tcpdump:
        stats = monitor_with_tcpdump(args.port, args.duration)
    else:
        stats = monitor_with_socket(args.port, args.duration)

    print_stats(stats)
    calculate_theoretical_bandwidth()


if __name__ == "__main__":
    main()
