---
sidebar_position: 1
---

# RTGP (R-Type Game Protocol) — Cheatsheet

This quick cheatsheet summarises the binary protocol used by the R-Type client & server (RTGP). For a formal specification, see `docs/RFC/RFC_RTGP_v1.2.0.md`.

## Packet Overview

- **Transport:** UDP
- **Header Size:** 16 bytes (fixed)
- **Payload:** variable length; must match header's payloadSize
- **MTU:** Keep payload small to avoid fragmentation; `kMaxPacketSize` is used in code.

## Header (16 bytes)

- magic (1 byte) — `0xA1` (used by validator)
- opcode (1 byte) — see `OpCode` values below
- payloadSize (2 bytes) — network byte order (big endian)
- userId (4 bytes) — network byte order
- seqId (2 bytes) — sequence id
- ackId (2 bytes) — ack of last received seq
- flags (1 byte) — reliability flag, ack flag, etc.
- reserved (3 bytes) — padding (must be zero)

> Note: Use `ByteOrderSpec` helpers for conversions and `Validator` for safe parsing.

## Reliability

- Some opcodes are marked as *reliable* and will be retransmitted if not acked.
- Reliable opcodes include spawn/destroy/accept/update state.
- Unreliable opcodes include movement updates and client inputs (latest state wins).

## Common OpCodes (summary)

- `C_CONNECT` (0x01) — Client connect, empty payload
- `S_ACCEPT` (0x02) — Server accepts connection: AcceptPayload (newUserId)
- `DISCONNECT` (0x03) — Disconnect notification
- `C_GET_USERS` / `R_GET_USERS` — Request/response for user lists
- `S_UPDATE_STATE` — Server state change payload (1 byte)
- `S_ENTITY_SPAWN` — Payload: { entityId(uint32), type(uint8), posX(float), posY(float) }
- `S_ENTITY_MOVE` — Payload: { entityId(uint32), posX(float), posY(float), velX(float), velY(float) }
- `S_ENTITY_DESTROY` — Payload: { entityId(uint32) }
- `C_INPUT` — Payload: input mask (uint8)
- `S_UPDATE_POS` — Payload: posX(float), posY(float) for corrections
- `PING` / `PONG` — Keepalive; empty payloads; uses seq/ack for RTT

For the full opcode list and payload structures, see:

- `lib/rtype_network/src/protocol/OpCode.hpp`
- `lib/rtype_network/src/protocol/Payloads.hpp`

## Best Practices

- Always validate packet size before parsing; use `Validator::validatePacket`.
- Use `ByteOrderSpec::toNetwork`/`fromNetwork` for integer fields to handle endianness.
- For unreliable packets, avoid relying on order — treat them as `latest state`.
- For critical events (entity creation/destroy/state), use reliable opcodes.
- For multiplayer, keep authoritative server/higher trust on server’s sequence.

## Server/Client Tips

- Server assigns `userId` on connect; clients send `userId` in every packet.
- Client input updates (`C_INPUT`) are sent via UDP unreliably; server will process and update game world and broadcast state.
- Position reconciliation: `S_UPDATE_POS` is used to correct client's predicted state.

---

🔗 See the RFC for full details and security considerations: `docs/RFC/RFC_RTGP_v1.2.0.md`.
