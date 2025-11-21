# Server Public API

This directory is reserved for **server-specific public interfaces**.

## Current Status

Currently, all server code is in `src/games/rtype/server/` and `src/server/`.

If you need to expose server-specific interfaces (e.g., `IGameSession`, `IPlayerManager`),
create them here following the same pattern as the engine and network interfaces.

## Implementation Details

Server implementations are located in:
- `src/games/rtype/server/` - Game-specific server logic
- `src/server/` - Server executable entry point
