#!/usr/bin/env node

class Component {
    constructor() {
        this.name = this.constructor.name;
    }
}

class Position extends Component {
    constructor(x = 0, y = 0) {
        super();
        this.x = x;
        this.y = y;
    }
}

class Velocity extends Component {
    constructor(dx = 0, dy = 0) {
        super();
        this.dx = dx;
        this.dy = dy;
    }
}

class Registry {
    constructor() {
        this.entities = new Map();
        this.nextEntityId = 0;
    }

    createEntity() {
        const entityId = this.nextEntityId++;
        this.entities.set(entityId, new Map());
        return entityId;
    }

    addComponent(entityId, component) {
        if (!this.entities.has(entityId)) {
            throw new Error(`Entity ${entityId} does not exist`);
        }
        this.entities.get(entityId).set(component.constructor.name, component);
    }

    getComponent(entityId, componentType) {
        if (!this.entities.has(entityId)) {
            return null;
        }
        return this.entities.get(entityId).get(componentType.name) || null;
    }

    hasComponent(entityId, componentType) {
        return this.getComponent(entityId, componentType) !== null;
    }

    getEntitiesWith(...componentTypes) {
        const entities = [];
        for (const [entityId, components] of this.entities) {
            const hasAllComponents = componentTypes.every(compType =>
                components.has(compType.name)
            );
            if (hasAllComponents) {
                entities.push(entityId);
            }
        }
        return entities;
    }

    updateMovementSystem() {
        const entities = this.getEntitiesWith(Position, Velocity);
        for (const entityId of entities) {
            const position = this.getComponent(entityId, Position);
            const velocity = this.getComponent(entityId, Velocity);

            if (position && velocity) {
                position.x += velocity.dx;
                position.y += velocity.dy;
                console.log(`Entity ${entityId} moved to (${position.x.toFixed(1)}, ${position.y.toFixed(1)})`);
            }
        }
    }
}

class UDPSocket {
    constructor(port = 12345) {
        this.dgram = require('dgram');
        this.server = this.dgram.createSocket('udp4');
        this.port = port;
        this.bound = false;
    }

    async bind() {
        return new Promise((resolve, reject) => {
            this.server.bind(this.port, '127.0.0.1', () => {
                console.log(`UDP Socket bound to port ${this.port}`);
                this.bound = true;
                resolve();
            });

            this.server.on('error', (err) => {
                reject(err);
            });
        });
    }

    send(message, address = '127.0.0.1', port = 12345) {
        if (!this.bound) {
            throw new Error('Socket not bound');
        }
        const buffer = Buffer.from(message, 'utf8');
        this.server.send(buffer, 0, buffer.length, port, address);
    }

    async receive(timeout = 1000) {
        return new Promise((resolve) => {
            const timeoutId = setTimeout(() => {
                resolve(null);
            }, timeout);

            const messageHandler = (msg, rinfo) => {
                clearTimeout(timeoutId);
                this.server.removeListener('message', messageHandler);
                resolve({
                    message: msg.toString('utf8'),
                    address: rinfo.address,
                    port: rinfo.port
                });
            };

            this.server.on('message', messageHandler);
        });
    }

    close() {
        if (this.server) {
            this.server.close();
        }
    }
}

async function demoECS() {
    console.log("R-Type JavaScript PoC: ECS and Networking Demo");

    const registry = new Registry();

    const entity0 = registry.createEntity();
    const entity1 = registry.createEntity();

    registry.addComponent(entity0, new Position(0.0, 0.0));
    registry.addComponent(entity0, new Velocity(1.0, 0.5));

    registry.addComponent(entity1, new Position(10.0, 5.0));
    registry.addComponent(entity1, new Velocity(-0.5, 0.0));

    for (let i = 0; i < 10; i++) {
        registry.updateMovementSystem();
        await new Promise(resolve => setTimeout(resolve, 100));
    }
}

async function demoNetworking() {
    const udpSocket = new UDPSocket();

    try {
        await udpSocket.bind();

        udpSocket.send("Hello from R-Type!");

        const result = await udpSocket.receive();
        if (result) {
            console.log(`Received: ${result.message} from ${result.address}:${result.port}`);
        }

    } finally {
        udpSocket.close();
    }
}

async function main() {
    try {
        await demoECS();

        await demoNetworking();

        console.log("JavaScript PoC completed successfully!");
    } catch (error) {
        console.error("Error:", error.message);
        process.exit(1);
    }
}

if (require.main === module) {
    main();
}

module.exports = { Component, Position, Velocity, Registry, UDPSocket };