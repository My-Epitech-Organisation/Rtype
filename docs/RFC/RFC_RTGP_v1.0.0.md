# **RFC-001: R-Type Game Protocol (RTGP)**

| Metadata | Details |
| :---- | :---- |
| **Version** | 1.0.0 |
| **Status** | Draft / Experimental |
| **Date** | 2025-11-25 |
| **Authors** | R-Type Project Team |
| **Abstract** | This document specifies the binary application-layer protocol used for real-time communication between the R-Type Client and Server. |

## **1\. Introduction**

The R-Type Game Protocol (RTGP) is a lightweight, binary, datagram-oriented protocol designed to facilitate real-time multiplayer gameplay. It prioritizes low latency and bandwidth efficiency.
This specification provides sufficient detail for a third-party developer to implement a compatible Client or Server without access to the reference implementation source code.

## **2\. Terminology & Conventions**

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](https://tools.ietf.org/html/rfc2119).

### **2.1. Data Types**

* **Byte:** 8-bit unsigned integer.
* **uint16:** 16-bit unsigned integer.
* **uint32:** 32-bit unsigned integer.
* **float:** 32-bit IEEE 754 floating point.
* **String:** NOT SUPPORTED in standard packets to avoid allocation overhead, unless specified in the payload.

### **2.2. Byte Order**

All multi-byte numeric fields **MUST** be transmitted in **Network Byte Order (Big-Endian)**. Implementations on Little-Endian architectures (x86/x64) MUST convert data before transmission (htons, htonl) and after reception (ntohs, ntohl).

## **3\. Transport Layer**

* **Protocol:** UDP (User Datagram Protocol).
* **Default Port:** 4242\.
* **MTU Safety:** The total packet size (Header \+ Payload) **SHOULD NOT** exceed 1400 bytes to avoid IP fragmentation on standard networks.

## **4\. Packet Structure**

Every RTGP packet consists of a fixed **8-byte Header** followed by a variable-length **Data Payload**.

### **4.1. Header Format**

```text
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |  Magic Byte   |  Packet Type  |          Packet Size          |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                            User ID                            |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                                                               |
 |                       Data Payload ...                        |
 |                                                               |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**Field Definitions:**

| Field | Type | Description |
| :---- | :---- | :---- |
| **Magic Byte** | uint8 | **MUST** be 0xA1. Used to filter spurious traffic. |
| **Packet Type** | uint8 | The Operation Code (OpCode) defined in Section 5\. |
| **Packet Size** | uint16 | Length of the **Data Payload** in bytes. Excludes Header size. |
| **User ID** | uint32 | The sender's unique identifier. See Section 4.2. |

### **4.2. User ID Conventions**

The User ID field validates the source of the packet at the application layer.

* **Server Authority:** 0xFFFFFFFF (Integer \-1).
  * Only the Server is allowed to send packets with this ID.
  * Clients receiving a packet with this ID know it is an Authoritative State Update.
* **Unassigned Client:** 0x00000000.
  * Used by Clients during the initial handshake before an ID is assigned.
* **Assigned Client:** 0x00000001 to 0xFFFFFFFE.

## **5\. Protocol Operations (OpCodes)**

### **5.1. Session Management**

#### **0x01 \- C\_CONNECT**

* **Sender:** Client
* **Description:** Request to establish a connection.
* **Payload:** Empty.

#### **0x02 \- S\_ACCEPT**

* **Sender:** Server
* **Description:** Connection accepted. Assigns the User ID to the client.
* **Payload:**
  * New User ID (uint32)

#### **0x03 \- DISCONNECT**

* **Sender:** Client OR Server
* **Description:** Graceful termination of the session.
* **Payload:** Empty.

#### **0x04 \- C\_GET\_USERS**

* **Sender:** Client
* **Description:** Request a list of currently connected players (Lobby).
* **Payload:** Empty.

#### **0x05 \- S\_UPDATE\_STATE**

* **Sender:** Server
* **Description:** Notifies clients of a global game state change (e.g., Lobby \-\> GameStart).
* **Payload:**
  * StateID (uint8): 0=Lobby, 1=Running, 2=Paused, 3=GameOver.

### **5.2. Gameplay & Entity Management**

#### **0x10 \- S\_ENTITY\_SPAWN**

* **Sender:** Server
* **Description:** Instructs clients to instantiate a new game object.
* **Payload:**
  * Entity ID (uint32): Unique ID of the object.
  * Type (uint8): 0=Player, 1=Bydos, 2=Missile.
  * PosX (float): Initial X coordinate.
  * PosY (float): Initial Y coordinate.

#### **0x11 \- S\_ENTITY\_MOVE**

* **Sender:** Server
* **Description:** Regular state update for remote entities (Interpolation target).
* **Payload:**
  * Entity ID (uint32)
  * PosX (float)
  * PosY (float)
  * VelX (float)
  * VelY (float)

#### **0x12 \- S\_ENTITY\_DESTROY**

* **Sender:** Server
* **Description:** Instructs clients to remove an entity.
* **Payload:**
  * Entity ID (uint32)

### **5.3. Input & Reconciliation**

#### **0x20 \- C\_INPUT**

* **Sender:** Client
* **Description:** The client sends its current input state to the server.
* **Payload:**
  * Input Mask (uint8): Bitmask of keys.
    * 0x01: UP
    * 0x02: DOWN
    * 0x04: LEFT
    * 0x08: RIGHT
    * 0x10: SHOOT

#### **0x21 \- S\_UPDATE\_POS (Reconciliation)**

* **Sender:** Server
* **Description:** **High Priority Correction.** Sent when the Server detects a Client's local position has drifted significantly from the authoritative state (Packet Loss / Lag).
* **Action:** Client **MUST** override its local player coordinates with these values immediately.
* **Payload:**
  * Authoritative X (float)
  * Authoritative Y (float)

## **6\. Security Considerations**

1. **Header Validation:** Any packet where Header\[0\] \!= 0xA1 **MUST** be silently dropped.
2. **Spoofing Protection:** The Server **MUST** verify that the User ID in the header matches the internal ID map associated with the sender's IP/Port.
3. **Authority Check:** Clients **MUST** ignore packets claiming to be 0xFFFFFFFF (Server) if they do not originate from the known Server IP endpoint.

## **7\. Future Extensions**

* Reserved OpCodes 0xF0 to 0xFF are set aside for debugging and ping measurements.