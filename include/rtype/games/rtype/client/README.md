# Client Public API

This directory is reserved for **client-specific public interfaces**.

## Current Status

Currently, all client code is in `src/games/rtype/client/` and `src/client/`.

If you need to expose client-specific interfaces (e.g., `IRenderer`, `IInputManager`),
create them here following the same pattern as the engine and network interfaces.

## Implementation Details

Client implementations are located in:
- `src/games/rtype/client/` - Game-specific client logic
- `src/client/` - Client executable entry point
