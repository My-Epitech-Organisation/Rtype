# Game Shared API

This directory is reserved for **public shared interfaces** between client and server.

## Current Status

Currently, all shared components are implementation details located in `src/games/rtype/shared/`.

If you need to expose shared game interfaces (e.g., `IGameState`, `IPlayerController`), 
create them here following the same pattern as the engine and network interfaces.

## Implementation Details

All game component implementations are in `src/games/rtype/shared/`:
- Components (Transform, Velocity, NetworkId)
- Systems (Movement, Collision)
