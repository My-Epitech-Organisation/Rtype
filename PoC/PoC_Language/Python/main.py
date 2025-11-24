#!/usr/bin/env python3
"""
R-Type Python Proof of Concept
Demonstrates Entity-Component-System (ECS) and UDP networking for real-time games
"""

import socket
import time
from dataclasses import dataclass
from typing import Dict, List, Optional, Any
from abc import ABC


class Component(ABC):
    """Base class for all components"""
    pass


@dataclass
class Position(Component):
    """Position component for entities"""
    x: float
    y: float


@dataclass
class Velocity(Component):
    """Velocity component for movement"""
    dx: float
    dy: float


class Registry:
    """Entity Component System Registry"""

    def __init__(self):
        self._entities: Dict[int, Dict[type, Component]] = {}
        self._next_entity_id = 0

    def create_entity(self) -> int:
        """Create a new entity and return its ID"""
        entity_id = self._next_entity_id
        self._next_entity_id += 1
        self._entities[entity_id] = {}
        return entity_id

    def add_component(self, entity_id: int, component: Component) -> None:
        """Add a component to an entity"""
        if entity_id not in self._entities:
            raise ValueError(f"Entity {entity_id} does not exist")
        self._entities[entity_id][type(component)] = component

    def get_component(self, entity_id: int, component_type: type) -> Optional[Component]:
        """Get a component from an entity"""
        if entity_id not in self._entities:
            return None
        return self._entities[entity_id].get(component_type)

    def has_component(self, entity_id: int, component_type: type) -> bool:
        """Check if entity has a specific component"""
        return self.get_component(entity_id, component_type) is not None

    def get_entities_with(self, *component_types: type) -> List[int]:
        """Get all entities that have all specified components"""
        entities = []
        for entity_id, components in self._entities.items():
            if all(comp_type in components for comp_type in component_types):
                entities.append(entity_id)
        return entities

    def update_movement_system(self) -> None:
        """Update system for movement (position + velocity)"""
        entities = self.get_entities_with(Position, Velocity)
        for entity_id in entities:
            position = self.get_component(entity_id, Position)
            velocity = self.get_component(entity_id, Velocity)

            if position and velocity:
                position.x += velocity.dx
                position.y += velocity.dy
                print(f"Entity {entity_id} moved to ({position.x:.1f}, {position.y:.1f})")


class UDPSocket:
    """Simple UDP socket wrapper"""

    def __init__(self, port: int = 12345):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(('127.0.0.1', port))
        self.port = port
        print(f"UDP Socket bound to port {port}")

    def send(self, message: str, address: tuple = ('127.0.0.1', 12345)) -> None:
        """Send a message to the specified address"""
        self.sock.sendto(message.encode('utf-8'), address)

    def receive(self, timeout: float = 1.0) -> Optional[tuple]:
        """Receive a message with timeout"""
        self.sock.settimeout(timeout)
        try:
            data, addr = self.sock.recvfrom(1024)
            return data.decode('utf-8'), addr
        except socket.timeout:
            return None
        except Exception as e:
            print(f"Receive error: {e}")
            return None

    def close(self) -> None:
        """Close the socket"""
        self.sock.close()


def demo_ecs() -> None:
    """Demonstrate ECS functionality"""
    print("R-Type Python PoC: ECS and Networking Demo")

    registry = Registry()

    entity0 = registry.create_entity()
    entity1 = registry.create_entity()

    registry.add_component(entity0, Position(0.0, 0.0))
    registry.add_component(entity0, Velocity(1.0, 0.5))

    registry.add_component(entity1, Position(10.0, 5.0))
    registry.add_component(entity1, Velocity(-0.5, 0.0))

    for _ in range(10):
        registry.update_movement_system()
        time.sleep(0.1)


def demo_networking() -> None:
    """Demonstrate UDP networking"""
    udp_sock = UDPSocket()

    udp_sock.send("Hello from R-Type!")

    result = udp_sock.receive()
    if result:
        message, addr = result
        print(f"Received: {message} from {addr[0]}:{addr[1]}")

    udp_sock.close()


def main() -> None:
    """Main demonstration function"""
    demo_ecs()

    demo_networking()

    print("Python PoC completed successfully!")


if __name__ == "__main__":
    main()