#!/usr/bin/env python3
"""
Simple UDP Packet Inspector
Listens on a UDP port and shows exactly what's being sent/received
Useful for debugging protocol issues
"""

import socket
import sys
import struct
from datetime import datetime

# RTGP Protocol Constants (RFC-001)
RTGP_MAGIC_BYTE = 0xA1
RTGP_HEADER_SIZE = 16
RTGP_FLAG_RELIABLE = 0x01
RTGP_FLAG_IS_ACK = 0x02

PACKET_TYPES = {
    0x01: 'C_CONNECT',
    0x02: 'S_ACCEPT',
    0x03: 'DISCONNECT',
    0x04: 'C_GET_USERS',
    0x05: 'R_GET_USERS',
    0x06: 'S_UPDATE_STATE',
    0x10: 'S_ENTITY_SPAWN',
    0x11: 'S_ENTITY_MOVE',
    0x12: 'S_ENTITY_DESTROY',
    0x13: 'S_ENTITY_HEALTH',
    0x20: 'C_INPUT',
    0x21: 'S_UPDATE_POS',
    0xF0: 'PING',
    0xF1: 'PONG',
}


def build_rtgp_packet(packet_type, payload=b'', user_id=0, sequence_id=1, ack_id=0, flags=RTGP_FLAG_RELIABLE):
    """Build a proper RTGP packet according to RFC-001"""
    header = struct.pack(
        '!BBHIHHBxxx',
        RTGP_MAGIC_BYTE,
        packet_type,
        len(payload),
        user_id,
        sequence_id,
        ack_id,
        flags
    )
    return header + payload


def parse_rtgp_header(data):
    """Parse RTGP header"""
    if len(data) < RTGP_HEADER_SIZE:
        return None
    
    magic, pkt_type, payload_size, user_id, seq_id, ack_id, flags = struct.unpack(
        '!BBHIHHBxxx', data[:RTGP_HEADER_SIZE]
    )
    
    return {
        'magic': magic,
        'type': pkt_type,
        'type_name': PACKET_TYPES.get(pkt_type, f'UNKNOWN(0x{pkt_type:02x})'),
        'payload_size': payload_size,
        'user_id': user_id,
        'sequence_id': seq_id,
        'ack_id': ack_id,
        'flags': flags,
        'is_reliable': bool(flags & RTGP_FLAG_RELIABLE),
        'is_ack': bool(flags & RTGP_FLAG_IS_ACK),
    }


def format_bytes(data: bytes, max_len: int = 64) -> str:
    """Format bytes as hex string"""
    hex_str = data.hex()
    if len(hex_str) > max_len:
        hex_str = hex_str[:max_len] + "..."
    
    # Add spaces every 2 chars for readability
    return ' '.join(hex_str[i:i+2] for i in range(0, len(hex_str), 2))


def format_ascii(data: bytes, max_len: int = 32) -> str:
    """Format bytes as ASCII (replace non-printable with .)"""
    ascii_str = ''.join(chr(b) if 32 <= b < 127 else '.' for b in data[:max_len])
    if len(data) > max_len:
        ascii_str += "..."
    return ascii_str


def main():
    host = '127.0.0.1'
    port = 4242
    
    if len(sys.argv) > 1:
        host = sys.argv[1]
    if len(sys.argv) > 2:
        port = int(sys.argv[2])
    
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    print("=" * 80)
    print("UDP PACKET INSPECTOR")
    print("=" * 80)
    print(f"Connecting to: {host}:{port}")
    print("Sending test packet and listening for responses...")
    print("=" * 80)
    print()
    
    # Send a proper RTGP C_CONNECT packet
    test_packet = build_rtgp_packet(
        packet_type=0x01,  # C_CONNECT
        payload=b'',
        user_id=0x00000000,  # Unassigned
        sequence_id=1,
        ack_id=0,
        flags=RTGP_FLAG_RELIABLE
    )
    
    print(f"[{datetime.now().strftime('%H:%M:%S.%f')[:-3]}] SENDING RTGP C_CONNECT:")
    print(f"  To: {host}:{port}")
    print(f"  Size: {len(test_packet)} bytes (16-byte header + 0 payload)")
    print(f"  Hex: {format_bytes(test_packet)}")
    print(f"  Header breakdown:")
    print(f"    Magic: 0xA1")
    print(f"    Type: 0x01 (C_CONNECT)")
    print(f"    Payload Size: 0")
    print(f"    User ID: 0x00000000 (unassigned)")
    print(f"    Sequence ID: 1")
    print(f"    Ack ID: 0")
    print(f"    Flags: 0x01 (RELIABLE)")
    print()
    
    sock.sendto(test_packet, (host, port))
    
    # Set timeout
    sock.settimeout(2.0)
    
    print("Listening for responses (2 second timeout)...")
    print()
    
    packet_count = 0
    try:
        while True:
            try:
                data, addr = sock.recvfrom(65535)
                packet_count += 1
                
                print("-" * 80)
                print(f"[{datetime.now().strftime('%H:%M:%S.%f')[:-3]}] RECEIVED PACKET #{packet_count}:")
                print(f"  From: {addr[0]}:{addr[1]}")
                print(f"  Size: {len(data)} bytes")
                print(f"  Hex: {format_bytes(data)}")
                print(f"  ASCII: [{format_ascii(data)}]")
                
                # Parse RTGP header
                header = parse_rtgp_header(data)
                if header:
                    print(f"  RTGP Header:")
                    print(f"    Magic: 0x{header['magic']:02x} {'✓' if header['magic'] == RTGP_MAGIC_BYTE else '✗ INVALID!'}")
                    print(f"    Type: 0x{header['type']:02x} ({header['type_name']})")
                    print(f"    Payload Size: {header['payload_size']} bytes")
                    print(f"    User ID: 0x{header['user_id']:08x}")
                    print(f"    Sequence ID: {header['sequence_id']}")
                    print(f"    Ack ID: {header['ack_id']}")
                    print(f"    Flags: 0x{header['flags']:02x} (Reliable={header['is_reliable']}, Ack={header['is_ack']})")
                    
                    # Parse payload based on packet type
                    if header['type'] == 0x02 and len(data) >= RTGP_HEADER_SIZE + 4:  # S_ACCEPT
                        user_id = struct.unpack('!I', data[RTGP_HEADER_SIZE:RTGP_HEADER_SIZE+4])[0]
                        print(f"  S_ACCEPT Payload:")
                        print(f"    Assigned User ID: 0x{user_id:08x} ({user_id})")
                else:
                    print(f"  ⚠️  Not a valid RTGP packet (too small or invalid format)")
                
                # Legacy parsing
                if len(data) >= 1:
                    print(f"  Raw first byte: 0x{data[0]:02x} ({data[0]})")
                
                if len(data) >= 4:
                    try:
                        val = struct.unpack('!I', data[:4])[0]
                        print(f"  First 4 bytes as uint32: {val}")
                    except:
                        pass
                
                print()
                
                # Echo back
                if packet_count <= 3:
                    print(f"Sending echo response...")
                    sock.sendto(data, addr)
                    print()
                    
            except socket.timeout:
                if packet_count == 0:
                    print("⚠️  No response received from server!")
                    print("   Server might not be running or not responding to this packet type")
                else:
                    print(f"✓ Received {packet_count} packet(s), timeout reached")
                break
                
    except KeyboardInterrupt:
        print("\nStopped by user")
    
    sock.close()
    print()
    print("=" * 80)
    print(f"Total packets received: {packet_count}")
    print("=" * 80)


if __name__ == '__main__':
    main()
