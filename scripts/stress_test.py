#!/usr/bin/env python3
"""
R-Type Server Stress Test
=========================
A comprehensive stress testing tool that will push your server to its absolute limits.

Features:
- Spawns hundreds of concurrent client connections
- Simulates realistic game traffic patterns
- Generates random movement inputs at high frequency
- Measures server response times, packet loss, and throughput
- Gradually ramps up load to find breaking points
- Monitors server health and performance metrics

Usage:
    python3 stress_test.py [options]

Examples:
    # Basic stress test with 100 clients
    python3 stress_test.py --host 127.0.0.1 --port 4242 --clients 100

    # Extreme stress test ramping up to 500 clients
    python3 stress_test.py --host 127.0.0.1 --port 4242 --clients 500 --ramp-up 30

    # Sustained load test for 5 minutes
    python3 stress_test.py --clients 200 --duration 300 --input-rate 60
"""

import argparse
import asyncio
import random
import socket
import struct
import sys
import time
from collections import defaultdict
from dataclasses import dataclass, field
from datetime import datetime
from enum import IntEnum
from typing import Dict, List, Optional, Set
import signal

# RTGP Protocol Constants (RFC-001)
RTGP_MAGIC_BYTE = 0xA1
RTGP_HEADER_SIZE = 16
RTGP_SERVER_USER_ID = 0xFFFFFFFF
RTGP_UNASSIGNED_USER_ID = 0x00000000

# RTGP Flags
RTGP_FLAG_RELIABLE = 0x01
RTGP_FLAG_IS_ACK = 0x02


def build_rtgp_header(packet_type: int, payload_size: int, user_id: int, 
                     sequence_id: int, ack_id: int, flags: int) -> bytes:
    """Build a 16-byte RTGP header according to RFC-001"""
    return struct.pack(
        '!BBHIHHBxxx',  # Network byte order (big-endian)
        RTGP_MAGIC_BYTE,    # Magic byte (0xA1)
        packet_type,        # Packet type (OpCode)
        payload_size,       # Payload size (uint16)
        user_id,            # User ID (uint32)
        sequence_id,        # Sequence ID (uint16)
        ack_id,             # Ack ID (uint16)
        flags,              # Flags (uint8)
        # xxx = 3 bytes reserved (padding)
    )


def build_rtgp_packet(packet_type: int, payload: bytes, user_id: int,
                     sequence_id: int, ack_id: int, flags: int) -> bytes:
    """Build a complete RTGP packet (header + payload)"""
    header = build_rtgp_header(packet_type, len(payload), user_id, sequence_id, ack_id, flags)
    return header + payload


class PacketType(IntEnum):
    """RTGP Protocol OpCodes (RFC-001)"""
    # Session Management
    C_CONNECT = 0x01
    S_ACCEPT = 0x02
    DISCONNECT = 0x03
    C_GET_USERS = 0x04
    R_GET_USERS = 0x05
    S_UPDATE_STATE = 0x06
    
    # Gameplay & Entity Management
    S_ENTITY_SPAWN = 0x10
    S_ENTITY_MOVE = 0x11
    S_ENTITY_DESTROY = 0x12
    S_ENTITY_HEALTH = 0x13
    
    # Input & Reconciliation
    C_INPUT = 0x20
    S_UPDATE_POS = 0x21
    
    # Ping/Pong (future)
    PING = 0xF0
    PONG = 0xF1


@dataclass
class ClientMetrics:
    """Tracks metrics for a single client"""
    client_id: int
    connected: bool = False
    user_id: Optional[int] = None
    packets_sent: int = 0
    packets_received: int = 0
    bytes_sent: int = 0
    bytes_received: int = 0
    connection_time: Optional[float] = None
    last_response_time: Optional[float] = None
    response_times: List[float] = field(default_factory=list)
    errors: int = 0
    timeouts: int = 0
    sequence_id: int = 0  # RTGP sequence tracking
    ack_id: int = 0       # Last received sequence ID
    # Separate tracking for reliable packets
    reliable_sent: int = 0
    reliable_received: int = 0
    unreliable_sent: int = 0
    unreliable_received: int = 0


@dataclass
class GlobalMetrics:
    """Global stress test metrics"""
    start_time: float = 0.0
    total_clients: int = 0
    active_clients: int = 0
    total_packets_sent: int = 0
    total_packets_received: int = 0
    total_bytes_sent: int = 0
    total_bytes_received: int = 0
    total_errors: int = 0
    total_timeouts: int = 0
    connection_failures: int = 0
    avg_response_time: float = 0.0
    min_response_time: float = float('inf')
    max_response_time: float = 0.0
    packets_per_second: float = 0.0
    bytes_per_second: float = 0.0
    # Reliable packet tracking
    reliable_sent: int = 0
    reliable_received: int = 0
    unreliable_sent: int = 0
    unreliable_received: int = 0


class StressTestClient:
    def __init__(self, client_id: int, host: str, port: int, input_rate: int = 30,
                 chaos_mode: bool = False, verbose: bool = False, debug: bool = False,
                 aggressive: bool = False, burst_size: int = 0, burst_interval: float = 1.0,
                 force_game_loop: bool = False):
        # Existing constructor fields
        self.client_id = client_id
        self.host = host
        self.port = port
        self.input_rate = input_rate
        self.chaos_mode = chaos_mode
        self.verbose = verbose
        self.debug = debug

        # New aggressive parameters
        self.aggressive = aggressive
        self.burst_size = burst_size
        self.burst_interval = burst_interval
        self.force_game_loop = force_game_loop

        # Rest of initialization remains as before
        self.socket = None
        self.running = False
        self.tasks: Set[asyncio.Task] = set()
        self.metrics = ClientMetrics(client_id)
        self.assigned_user_id = RTGP_UNASSIGNED_USER_ID
        self._recv_buffer = bytearray()

        # Default sequence id start
        self.metrics.sequence_id = 1
        self.metrics.ack_id = 0
    """Individual UDP client that hammers the server"""
        
    async def connect(self) -> bool:
        """Establish connection to server"""
        try:
            # Create UDP socket
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.socket.setblocking(False)
            
            # Build RTGP C_CONNECT packet (RFC-001)
            # Empty payload, RELIABLE flag, unassigned user ID
            self.metrics.sequence_id += 1
            connect_packet = build_rtgp_packet(
                packet_type=PacketType.C_CONNECT,
                payload=b'',
                user_id=RTGP_UNASSIGNED_USER_ID,
                sequence_id=self.metrics.sequence_id,
                ack_id=self.metrics.ack_id,
                flags=RTGP_FLAG_RELIABLE
            )
            
            await self._send_packet(connect_packet, is_reliable=True)
            
            if self.debug:
                print(f"[Client {self.client_id}] Sent RTGP C_CONNECT ({len(connect_packet)} bytes)")
            
            self.metrics.connected = True
            self.metrics.connection_time = time.time()
            return True
            
        except Exception as e:
            if self.debug:
                print(f"[Client {self.client_id}] Connect error: {e}")
            self.metrics.errors += 1
            return False
    
    async def disconnect(self):
        """Gracefully disconnect"""
        if self.socket:
            try:
                self.metrics.sequence_id += 1
                disconnect_packet = build_rtgp_packet(
                    packet_type=PacketType.DISCONNECT,
                    payload=b'',
                    user_id=self.assigned_user_id,
                    sequence_id=self.metrics.sequence_id,
                    ack_id=self.metrics.ack_id,
                    flags=RTGP_FLAG_RELIABLE
                )
                await self._send_packet(disconnect_packet, is_reliable=True)
            except:
                pass
            finally:
                self.socket.close()
                self.socket = None
        self.metrics.connected = False
    
    async def _send_packet(self, data: bytes, is_reliable: bool = False):
        """Send packet to server"""
        if not self.socket:
            return
        
        try:
            loop = asyncio.get_event_loop()
            await loop.sock_sendto(self.socket, data, (self.host, self.port))
            self.metrics.packets_sent += 1
            self.metrics.bytes_sent += len(data)
            
            # Track reliable vs unreliable
            if is_reliable:
                self.metrics.reliable_sent += 1
            else:
                self.metrics.unreliable_sent += 1
            
            if self.debug and self.metrics.packets_sent <= 5:
                print(f"[Client {self.client_id}] SEND {len(data)} bytes: {data.hex()}")
        except Exception as e:
            if self.debug:
                print(f"[Client {self.client_id}] Send error: {e}")
            self.metrics.errors += 1
    
    async def _receive_packets(self):
        """Continuously receive packets from server"""
        if not self.socket:
            return
        
        loop = asyncio.get_event_loop()
        buffer_size = 65535
        
        while self.running:
            try:
                # Try to receive data with timeout
                data, addr = await asyncio.wait_for(
                    loop.sock_recvfrom(self.socket, buffer_size),
                    timeout=0.1
                )
                
                self.metrics.packets_received += 1
                self.metrics.bytes_received += len(data)
                self.metrics.last_response_time = time.time()
                
                # Parse RTGP header
                if len(data) >= RTGP_HEADER_SIZE:
                    magic, pkt_type, payload_size, user_id, seq_id, ack_id, flags = struct.unpack(
                        '!BBHIHHBxxx', data[:RTGP_HEADER_SIZE]
                    )
                    
                    # Track reliable vs unreliable
                    is_reliable = bool(flags & RTGP_FLAG_RELIABLE)
                    if is_reliable:
                        self.metrics.reliable_received += 1
                    else:
                        self.metrics.unreliable_received += 1
                    
                    # Update ack_id for next packet
                    self.metrics.ack_id = seq_id
                    
                    # IMPORTANT: Send ACK for reliable packets to prevent server disconnect
                    if is_reliable:
                        ack_packet = build_rtgp_packet(
                            packet_type=pkt_type,  # Echo back the packet type
                            payload=b'',
                            user_id=self.assigned_user_id if self.assigned_user_id != RTGP_UNASSIGNED_USER_ID else user_id,
                            sequence_id=self.metrics.sequence_id,
                            ack_id=seq_id,  # ACK this sequence ID
                            flags=RTGP_FLAG_IS_ACK  # Mark as ACK
                        )
                        await self._send_packet(ack_packet, is_reliable=False)
                        self.metrics.sequence_id += 1
                    
                    if self.debug:
                        print(f"[Client {self.client_id}] RECV Type=0x{pkt_type:02x} SeqID={seq_id} from {addr}")
                    elif self.verbose and self.metrics.packets_received == 1:
                        print(f"[Client {self.client_id}] First packet! Type=0x{pkt_type:02x}")
                    
                    # Handle S_ACCEPT - server assigned us a user ID
                    if pkt_type == PacketType.S_ACCEPT and len(data) >= RTGP_HEADER_SIZE + 4:
                        assigned_id = struct.unpack('!I', data[RTGP_HEADER_SIZE:RTGP_HEADER_SIZE+4])[0]
                        self.assigned_user_id = assigned_id
                        self.metrics.user_id = assigned_id
                        if self.verbose or self.debug:
                            print(f"[Client {self.client_id}] ✓ Assigned User ID: {assigned_id}")
                else:
                    if self.debug:
                        print(f"[Client {self.client_id}] RECV {len(data)} bytes (too small): {data.hex()}")
                
                # Record response time
                if self.metrics.connection_time:
                    response_time = time.time() - self.metrics.connection_time
                    self.metrics.response_times.append(response_time)
                    
            except asyncio.TimeoutError:
                # No data available, continue
                continue
            except BlockingIOError:
                # Would block, continue
                await asyncio.sleep(0.01)
                continue
            except Exception as e:
                if self.debug:
                    print(f"[Client {self.client_id}] Receive error: {e}")
                self.metrics.errors += 1
                await asyncio.sleep(0.01)
    
    async def _spam_inputs(self):
        """When enabled by --force-game-loop, spam C_INPUT packets to trigger ready state."""
        interval = 1.0 / self.input_rate if self.input_rate > 0 else 1.0
        while self.running:
            try:
                # Normal input spam (this will trigger the game ready behavior)
                input_mask = random.randint(0, 31)
                payload = struct.pack('!B', input_mask)
                self.metrics.sequence_id += 1
                packet = build_rtgp_packet(
                    packet_type=PacketType.C_INPUT,
                    payload=payload,
                    user_id=self.assigned_user_id,
                    sequence_id=self.metrics.sequence_id,
                    ack_id=self.metrics.ack_id,
                    flags=0
                )
                await self._send_packet(packet, is_reliable=False)
                await asyncio.sleep(interval)
            except Exception:
                self.metrics.errors += 1

    async def _send_keepalive_ping(self):
        """Send periodic PING packets so server doesn't timeout idle clients."""
        while self.running:
            try:
                await asyncio.sleep(4.0)  # less than server's 10s timeout
                if self.assigned_user_id != RTGP_UNASSIGNED_USER_ID:
                    # include a timestamp for diagnostics
                    payload = struct.pack('!d', time.time())
                    self.metrics.sequence_id += 1
                    packet = build_rtgp_packet(
                        packet_type=PacketType.PING,
                        payload=payload,
                        user_id=self.assigned_user_id,
                        sequence_id=self.metrics.sequence_id,
                        ack_id=self.metrics.ack_id,
                        flags=0  # UNRELIABLE
                    )
                    await self._send_packet(packet, is_reliable=False)

                    # If aggressive, send an extra burst of small unreliable pings
                    if self.aggressive and self.burst_size > 0:
                        for _ in range(max(1, int(self.burst_size / 4))):
                            self.metrics.sequence_id += 1
                            bpayload = struct.pack('!d', time.time())
                            bpacket = build_rtgp_packet(
                                packet_type=PacketType.PING,
                                payload=bpayload,
                                user_id=self.assigned_user_id,
                                sequence_id=self.metrics.sequence_id,
                                ack_id=self.metrics.ack_id,
                                flags=0
                            )
                            await self._send_packet(bpacket, is_reliable=False)
            except Exception:
                self.metrics.errors += 1

    async def _reliable_burst(self):
        """Send bursts of reliable C_GET_USERS packets to stress reliable channel."""
        while self.running:
            try:
                await asyncio.sleep(self.burst_interval)
                if self.assigned_user_id == RTGP_UNASSIGNED_USER_ID:
                    continue
                # Send burst
                for _ in range(self.burst_size):
                    self.metrics.sequence_id += 1
                    pkt = build_rtgp_packet(
                        packet_type=PacketType.C_GET_USERS,
                        payload=b'',
                        user_id=self.assigned_user_id,
                        sequence_id=self.metrics.sequence_id,
                        ack_id=self.metrics.ack_id,
                        flags=RTGP_FLAG_RELIABLE
                    )
                    await self._send_packet(pkt, is_reliable=True)
                    # small backoff to avoid local scheduler starvation
                    await asyncio.sleep(0.001)
            except Exception:
                self.metrics.errors += 1

    # NOTE: Keepalive removed - server interprets ANY C_INPUT as "player ready"
    # which triggers game start/stop loops. Let server handle timeouts.

    async def run(self):
        """Run the stress test client"""
        if not await self.connect():
            return
        
        self.running = True
        
        # Start concurrent tasks
        tasks = [
            asyncio.create_task(self._receive_packets()),
            asyncio.create_task(self._send_keepalive_ping()),
        ]
        if self.force_game_loop:
            tasks.append(asyncio.create_task(self._spam_inputs()))
        if self.burst_size > 0:
            tasks.append(asyncio.create_task(self._reliable_burst()))
        
        self.tasks.update(tasks)
        
        try:
            await asyncio.gather(*tasks, return_exceptions=True)
        finally:
            await self.disconnect()
    
    async def stop(self):
        """Stop the client"""
        self.running = False
        for task in self.tasks:
            task.cancel()
        await self.disconnect()


class StressTestOrchestrator:
    """Orchestrates the entire stress test"""
    
    def __init__(self, args):
        self.args = args
        self.clients: List[StressTestClient] = []
        self.metrics = GlobalMetrics()
        self.running = False
        self.start_time = 0.0
        
        # Churn bookkeeping
        self._next_client_id = 0
        
        # Setup signal handlers
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)
    
    def _signal_handler(self, signum, frame):
        """Handle Ctrl+C gracefully"""
        print("\n\n🛑 Received interrupt signal, shutting down gracefully...")
        self.running = False

    async def _churn_loop(self):
        """Periodically disconnect/reconnect a fraction of clients to stress connection handling."""
        percent = max(0.0, min(100.0, self.args.churn_percent))
        interval = max(0.1, self.args.churn_interval)
        total = self.args.clients
        while self.running:
            try:
                await asyncio.sleep(interval)
                count = max(1, int(total * percent / 100.0))
                # pick random clients to restart
                picks = random.sample(self.clients, min(count, len(self.clients)))
                for c in picks:
                    try:
                        await c.disconnect()
                        # short pause
                        await asyncio.sleep(0.01)
                        # spawn replacement client
                        await self.spawn_client(self._next_client_id)
                    except Exception:
                        pass
            except Exception:
                pass
    
    async def spawn_client(self, client_id: int):
        """Spawn a single client"""
        client = StressTestClient(
            client_id=client_id,
            host=self.args.host,
            port=self.args.port,
            input_rate=self.args.input_rate,
            chaos_mode=self.args.chaos,
            verbose=self.args.verbose,
            debug=self.args.debug,
            aggressive=self.args.aggressive,
            burst_size=self.args.burst_size,
            burst_interval=self.args.burst_interval,
            force_game_loop=self.args.force_game_loop
        )
        self.clients.append(client)
        self._next_client_id = max(self._next_client_id, client_id + 1)
        
        # Run client in background
        asyncio.create_task(client.run())
        await asyncio.sleep(0.001)  # Small delay to prevent socket exhaustion
    
    async def spawn_clients_gradual(self):
        """Gradually spawn clients (ramp-up)"""
        total_clients = self.args.clients
        ramp_up_time = self.args.ramp_up
        
        if ramp_up_time > 0:
            interval = ramp_up_time / total_clients
            print(f"🚀 Ramping up {total_clients} clients over {ramp_up_time}s...")
            
            for i in range(total_clients):
                if not self.running:
                    break
                await self.spawn_client(i)
                await asyncio.sleep(interval)
        else:
            print(f"💥 SPAWNING {total_clients} CLIENTS INSTANTLY!")
            tasks = [self.spawn_client(i) for i in range(total_clients)]
            await asyncio.gather(*tasks)
    
    def calculate_metrics(self):
        # Ensure attributes exist for compatibility
        setattr(self.metrics, 'reliable_sent', getattr(self.metrics, 'reliable_sent', 0))
        setattr(self.metrics, 'reliable_received', getattr(self.metrics, 'reliable_received', 0))
        setattr(self.metrics, 'unreliable_sent', getattr(self.metrics, 'unreliable_sent', 0))
        setattr(self.metrics, 'unreliable_received', getattr(self.metrics, 'unreliable_received', 0))

        """Calculate aggregate metrics"""
        self.metrics.total_clients = len(self.clients)
        self.metrics.active_clients = sum(1 for c in self.clients if c.metrics.connected)
        self.metrics.total_packets_sent = sum(c.metrics.packets_sent for c in self.clients)
        self.metrics.total_packets_received = sum(c.metrics.packets_received for c in self.clients)
        self.metrics.total_bytes_sent = sum(c.metrics.bytes_sent for c in self.clients)
        self.metrics.total_bytes_received = sum(c.metrics.bytes_received for c in self.clients)
        self.metrics.total_errors = sum(c.metrics.errors for c in self.clients)
        self.metrics.total_timeouts = sum(c.metrics.timeouts for c in self.clients)
        self.metrics.reliable_sent = sum(c.metrics.reliable_sent for c in self.clients)
        self.metrics.reliable_received = sum(c.metrics.reliable_received for c in self.clients)
        self.metrics.unreliable_sent = sum(c.metrics.unreliable_sent for c in self.clients)
        self.metrics.unreliable_received = sum(c.metrics.unreliable_received for c in self.clients)
        
        # Calculate response times
        all_response_times = []
        for client in self.clients:
            all_response_times.extend(client.metrics.response_times)
        
        if all_response_times:
            self.metrics.avg_response_time = sum(all_response_times) / len(all_response_times)
            self.metrics.min_response_time = min(all_response_times)
            self.metrics.max_response_time = max(all_response_times)
        
        # Calculate rates
        elapsed = time.time() - self.start_time
        if elapsed > 0:
            self.metrics.packets_per_second = self.metrics.total_packets_sent / elapsed
            self.metrics.bytes_per_second = self.metrics.total_bytes_sent / elapsed
    
    def print_status(self):
        """Print live status"""
        self.calculate_metrics()
        m = self.metrics
        
        # Clear screen and move to top (skip in verbose/debug mode)
        if not self.args.verbose and not self.args.debug:
            print("\033[2J\033[H", end="")
        
        print("=" * 80)
        print("🔥 R-TYPE SERVER STRESS TEST 🔥".center(80))
        print("=" * 80)
        print(f"Target: {self.args.host}:{self.args.port}")
        print(f"Time Elapsed: {time.time() - self.start_time:.1f}s")
        print("-" * 80)
        
        print(f"\n📊 CLIENT STATISTICS:")
        print(f"  Total Clients:        {m.total_clients}")
        print(f"  Active Clients:       {m.active_clients} ({m.active_clients/m.total_clients*100 if m.total_clients > 0 else 0:.1f}%)")
        print(f"  Connection Failures:  {m.connection_failures}")
        
        print(f"\n📦 PACKET STATISTICS:")
        print(f"  Total Sent:           {m.total_packets_sent:,}")
        print(f"  Total Received:       {m.total_packets_received:,}")
        print(f"  Overall Loss:         {(1 - m.total_packets_received/max(m.total_packets_sent, 1))*100:.2f}%")
        print(f"  Packets/sec:          {m.packets_per_second:.1f}")
        print(f"\n  📡 RELIABLE (Critical):")
        reliable_loss = (1 - m.reliable_received/max(m.reliable_sent, 1))*100
        print(f"    Sent:               {m.reliable_sent:,}")
        print(f"    Received:           {m.reliable_received:,}")
        print(f"    Loss:               {reliable_loss:.2f}% {'✓ Good' if reliable_loss < 5 else '⚠️ HIGH!'}")
        print(f"\n  📤 UNRELIABLE (Inputs/Moves):")
        unreliable_loss = (1 - m.unreliable_received/max(m.unreliable_sent, 1))*100
        print(f"    Sent:               {m.unreliable_sent:,}")
        print(f"    Received:           {m.unreliable_received:,}")
        print(f"    Loss:               {unreliable_loss:.2f}% (expected for UDP)")
        
        print(f"\n💾 BANDWIDTH:")
        print(f"  Bytes Sent:           {m.total_bytes_sent:,} ({m.total_bytes_sent/1024/1024:.2f} MB)")
        print(f"  Bytes Received:       {m.total_bytes_received:,} ({m.total_bytes_received/1024/1024:.2f} MB)")
        print(f"  Throughput:           {m.bytes_per_second/1024:.1f} KB/s")
        
        print(f"\n⚡ PERFORMANCE:")
        if m.avg_response_time > 0:
            print(f"  Avg Response Time:    {m.avg_response_time*1000:.2f} ms")
            print(f"  Min Response Time:    {m.min_response_time*1000:.2f} ms")
            print(f"  Max Response Time:    {m.max_response_time*1000:.2f} ms")
        else:
            print(f"  Response Times:       No data yet...")
        
        print(f"\n❌ ERRORS:")
        print(f"  Total Errors:         {m.total_errors:,}")
        print(f"  Timeouts:             {m.total_timeouts:,}")
        
        print("\n" + "=" * 80)
        print("Press Ctrl+C to stop".center(80))
        print("=" * 80)
    
    async def monitor_loop(self):
        """Continuously monitor and display stats"""
        while self.running:
            self.print_status()
            await asyncio.sleep(1.0)
    
    async def run(self):
        """Run the stress test"""
        print("🔥" * 40)
        print("R-TYPE SERVER STRESS TEST - PREPARING TO DESTROY YOUR SERVER")
        print("🔥" * 40)
        print(f"\nConfiguration:")
        print(f"  Server: {self.args.host}:{self.args.port}")
        print(f"  Clients: {self.args.clients}")
        print(f"  Input Rate: {self.args.input_rate} packets/sec per client")
        print(f"  Ramp-up Time: {self.args.ramp_up}s")
        print(f"  Duration: {self.args.duration}s" if self.args.duration else "  Duration: Unlimited")
        print(f"  Chaos Mode: {'ENABLED ☠️' if self.args.chaos else 'Disabled'}")
        print(f"  Aggressive Mode: {'ENABLED' if self.args.aggressive else 'Disabled'}")
        if self.args.aggressive:
            print(f"  Burst Size: {self.args.burst_size} every {self.args.burst_interval}s")
            if self.args.force_game_loop:
                print("  Force Game Loop: ENABLED (this will intentionally trigger game start/stop)")
        print()
        
        if self.args.chaos:
            print("⚠️  WARNING: CHAOS MODE ENABLED - SENDING RANDOM GARBAGE ⚠️")
            print()
        
        input("Press Enter to begin the carnage... ")
        
        self.running = True
        self.start_time = time.time()
        self.metrics.start_time = self.start_time
        
        # Start spawning clients
        spawn_task = asyncio.create_task(self.spawn_clients_gradual())
        
        # Optional churn task (reconnects)
        churn_task = None
        if getattr(self.args, 'churn_percent', 0) > 0 and getattr(self.args, 'churn_interval', 0) > 0:
            churn_task = asyncio.create_task(self._churn_loop())
        
        # Start monitoring
        monitor_task = asyncio.create_task(self.monitor_loop())
        
        # Run for specified duration or until interrupted
        try:
            if self.args.duration:
                await asyncio.sleep(self.args.duration)
                self.running = False
            else:
                while self.running:
                    await asyncio.sleep(1)
        except KeyboardInterrupt:
            pass
        finally:
            self.running = False
            
            # Stop all clients
            print("\n\n🛑 Stopping all clients...")
            stop_tasks = [client.stop() for client in self.clients]
            await asyncio.gather(*stop_tasks, return_exceptions=True)
            
            # Cancel tasks
            spawn_task.cancel()
            monitor_task.cancel()
            if churn_task:
                churn_task.cancel()
            
            # Print final report
            self.print_final_report()
    
    def print_final_report(self):
        """Print final test report"""
        self.calculate_metrics()
        m = self.metrics
        
        print("\n\n")
        print("=" * 80)
        print("📋 FINAL STRESS TEST REPORT".center(80))
        print("=" * 80)
        
        duration = time.time() - self.start_time
        
        print(f"\n⏱️  Test Duration: {duration:.1f}s")
        print(f"\n📊 Client Statistics:")
        print(f"  • Spawned: {m.total_clients}")
        print(f"  • Peak Active: {m.active_clients}")
        print(f"  • Failed Connections: {m.connection_failures}")
        
        print(f"\n📦 Traffic Statistics:")
        print(f"  • Total Packets Sent: {m.total_packets_sent:,}")
        print(f"  • Total Packets Received: {m.total_packets_received:,}")
        print(f"  • Overall Loss Rate: {(1 - m.total_packets_received/max(m.total_packets_sent, 1))*100:.2f}%")
        print(f"  • Average Throughput: {m.packets_per_second:.1f} packets/sec")
        print(f"\n  RELIABLE Packets (Connection, Spawns, etc):")
        reliable_loss = (1 - m.reliable_received/max(m.reliable_sent, 1))*100
        print(f"    - Sent: {m.reliable_sent:,}")
        print(f"    - Received: {m.reliable_received:,}")
        print(f"    - Loss: {reliable_loss:.2f}%")
        print(f"\n  UNRELIABLE Packets (Inputs, Movement):")
        unreliable_loss = (1 - m.unreliable_received/max(m.unreliable_sent, 1))*100
        print(f"    - Sent: {m.unreliable_sent:,}")
        print(f"    - Received: {m.unreliable_received:,}")
        print(f"    - Loss: {unreliable_loss:.2f}% (expected for UDP)")
        
        print(f"\n💾 Bandwidth:")
        print(f"  • Data Sent: {m.total_bytes_sent/1024/1024:.2f} MB")
        print(f"  • Data Received: {m.total_bytes_received/1024/1024:.2f} MB")
        print(f"  • Average Bandwidth: {m.bytes_per_second/1024:.1f} KB/s")
        
        print(f"\n⚡ Performance:")
        if m.avg_response_time > 0:
            print(f"  • Avg Response Time: {m.avg_response_time*1000:.2f} ms")
            print(f"  • Min Response Time: {m.min_response_time*1000:.2f} ms")
            print(f"  • Max Response Time: {m.max_response_time*1000:.2f} ms")
        
        print(f"\n❌ Errors:")
        print(f"  • Total Errors: {m.total_errors:,}")
        print(f"  • Total Timeouts: {m.total_timeouts:,}")
        
        # Server verdict (based on RELIABLE packet loss, not overall)
        print(f"\n" + "=" * 80)
        reliable_success = (m.reliable_received / max(m.reliable_sent, 1)) * 100
        
        if reliable_success > 98 and m.avg_response_time < 0.1:
            print("🏆 VERDICT: Your server is SOLID! Reliable packets delivered flawlessly!")
        elif reliable_success > 95:
            print("👍 VERDICT: Server held up well! Minor reliable packet loss is acceptable.")
        elif reliable_success > 85:
            print("⚠️  VERDICT: Server struggled with reliable packets. Check network/load.")
        else:
            print("💀 VERDICT: Critical packet loss! Server can't handle the load!")
        
        print("=" * 80)
        print()


def main():
    parser = argparse.ArgumentParser(
        description="Stress test the R-Type server to its absolute limits",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Basic stress test
  %(prog)s --clients 100

  # Extreme load test
  %(prog)s --clients 500 --input-rate 100 --chaos

  # Gradual ramp-up test
  %(prog)s --clients 300 --ramp-up 60 --duration 300
        """
    )
    
    parser.add_argument('--host', default='127.0.0.1',
                        help='Server hostname or IP (default: 127.0.0.1)')
    parser.add_argument('--port', type=int, default=4242,
                        help='Server port (default: 4242)')
    parser.add_argument('--clients', type=int, default=100,
                        help='Number of concurrent clients (default: 100)')
    parser.add_argument('--input-rate', type=int, default=30,
                        help='Input packets per second per client (default: 30)')
    parser.add_argument('--ramp-up', type=int, default=10,
                        help='Ramp-up time in seconds (default: 10, 0 for instant)')
    parser.add_argument('--duration', type=int, default=0,
                        help='Test duration in seconds (default: unlimited)')
    parser.add_argument('--chaos', action='store_true',
                        help='Enable chaos mode: send random garbage packets')
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='Enable verbose output (show packet details)')
    parser.add_argument('--debug', action='store_true',
                        help='Enable debug mode (extremely verbose)')
    parser.add_argument('--aggressive', action='store_true',
                        help='Enable aggressive mode: burst reliable packets and extra pings')
    parser.add_argument('--burst-size', type=int, default=0,
                        help='Burst size for reliable packets (0 disables)')
    parser.add_argument('--burst-interval', type=float, default=1.0,
                        help='Interval between bursts in seconds')
    parser.add_argument('--churn-percent', type=float, default=0.0,
                        help='Percent of clients to disconnect/reconnect each churn interval')
    parser.add_argument('--churn-interval', type=float, default=0.0,
                        help='Interval in seconds between churn events')
    parser.add_argument('--force-game-loop', action='store_true',
                        help='Force C_INPUT spam to trigger the server game loop (dangerous)')
    
    args = parser.parse_args()
    
    # Validate arguments
    if args.clients <= 0:
        print("Error: --clients must be positive")
        sys.exit(1)
    
    if args.clients > 10000:
        print("⚠️  WARNING: More than 10,000 clients may exhaust system resources!")
        response = input("Continue? (yes/no): ")
        if response.lower() not in ['yes', 'y']:
            sys.exit(0)

    if args.force_game_loop:
        print("⚠️  WARNING: --force-game-loop will intentionally trigger the server's game loop and can destabilize it.")
        resp = input("Do you want to proceed? (yes/no): ")
        if resp.lower() not in ['yes', 'y']:
            sys.exit(0)

    if args.churn_percent > 0 and args.churn_interval <= 0:
        print("Error: --churn-interval must be > 0 when --churn-percent is set")
        sys.exit(1)
    
    # Run the stress test
    orchestrator = StressTestOrchestrator(args)
    
    try:
        asyncio.run(orchestrator.run())
    except KeyboardInterrupt:
        print("\n\nTest interrupted by user.")
    except Exception as e:
        print(f"\n\nFATAL ERROR: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
