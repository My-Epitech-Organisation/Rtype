package main

import (
	"fmt"
	"net"
	"time"
)

type Component interface{}

type TransformComponent struct {
	X, Y float64
}

type VelocityComponent struct {
	VX, VY float64
}

type Entity int

type Registry struct {
	nextID     Entity
	components map[Entity]map[string]Component
}

func NewRegistry() *Registry {
	return &Registry{
		components: make(map[Entity]map[string]Component),
	}
}

func (r *Registry) CreateEntity() Entity {
	id := r.nextID
	r.nextID++
	r.components[id] = make(map[string]Component)
	return id
}

func (r *Registry) AddComponent(entity Entity, component Component, name string) {
	if comps, exists := r.components[entity]; exists {
		comps[name] = component
	}
}

func (r *Registry) GetComponent(entity Entity, name string) Component {
	if comps, exists := r.components[entity]; exists {
		return comps[name]
	}
	return nil
}

func (r *Registry) HasComponent(entity Entity, name string) bool {
	if comps, exists := r.components[entity]; exists {
		_, ok := comps[name]
		return ok
	}
	return false
}

func (r *Registry) Update(dt float64) {
	for entity, comps := range r.components {
		transform, hasTransform := comps["transform"].(*TransformComponent)
		velocity, hasVelocity := comps["velocity"].(*VelocityComponent)
		if hasTransform && hasVelocity {
			transform.X += velocity.VX * dt
			transform.Y += velocity.VY * dt
			fmt.Printf("Entity %d moved to (%.1f, %.1f)\n", entity, transform.X, transform.Y)
		}
	}
}

type UDPSocket struct {
	conn *net.UDPConn
}

func (s *UDPSocket) Create(port int) error {
	addr := &net.UDPAddr{IP: net.IPv4zero, Port: port}
	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		return err
	}
	s.conn = conn
	return nil
}

func (s *UDPSocket) SendTo(message string, ip string, port int) error {
	addr := &net.UDPAddr{IP: net.ParseIP(ip), Port: port}
	_, err := s.conn.WriteToUDP([]byte(message), addr)
	return err
}

func (s *UDPSocket) ReceiveFrom() (string, string, int, error) {
	buffer := make([]byte, 1024)
	n, addr, err := s.conn.ReadFromUDP(buffer)
	if err != nil {
		return "", "", 0, err
	}
	return string(buffer[:n]), addr.IP.String(), addr.Port, nil
}

func main() {
	fmt.Println("R-Type Go PoC: ECS and Networking Demo")

	registry := NewRegistry()

	player := registry.CreateEntity()
	registry.AddComponent(player, &TransformComponent{X: 0, Y: 0}, "transform")
	registry.AddComponent(player, &VelocityComponent{VX: 1, VY: 0.5}, "velocity")

	enemy := registry.CreateEntity()
	registry.AddComponent(enemy, &TransformComponent{X: 10, Y: 5}, "transform")
	registry.AddComponent(enemy, &VelocityComponent{VX: -0.5, VY: 0}, "velocity")

	for i := 0; i < 5; i++ {
		registry.Update(1.0)
		time.Sleep(500 * time.Millisecond)
	}

	socket := &UDPSocket{}
	if err := socket.Create(12345); err != nil {
		fmt.Println("Failed to create socket:", err)
		return
	}

	fmt.Println("UDP Socket bound to port 12345")

	if err := socket.SendTo("Hello from R-Type!", "127.0.0.1", 12345); err != nil {
		fmt.Println("Failed to send:", err)
	}

	message, ip, port, err := socket.ReceiveFrom()
	if err != nil {
		fmt.Println("Failed to receive:", err)
	} else {
		fmt.Printf("Received: %s from %s:%d\n", message, ip, port)
	}

	fmt.Println("Go PoC completed successfully!")
}
