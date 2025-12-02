#!/bin/bash
# setup-vcpkg.sh - Initialise vcpkg pour le projet R-Type
#
# Ce script :
#   1. Initialise le submodule vcpkg si nécessaire
#   2. Bootstrap vcpkg (compile l'exécutable)
#   3. Affiche les instructions pour la suite
#
# Usage:
#   ./scripts/setup-vcpkg.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VCPKG_DIR="$PROJECT_ROOT/external/vcpkg"

echo "╔══════════════════════════════════════════════════════════╗"
echo "║           R-Type - vcpkg Setup Script                    ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

# Vérifie si VCPKG_ROOT est défini (vcpkg personnel)
if [ -n "$VCPKG_ROOT" ] && [ -f "$VCPKG_ROOT/vcpkg" ]; then
    echo "✓ Using personal vcpkg from VCPKG_ROOT: $VCPKG_ROOT"
    echo ""
    echo "You can now configure the project with:"
    echo "  cmake --preset linux-debug"
    echo ""
    exit 0
fi

# Initialise le submodule si nécessaire
if [ ! -d "$VCPKG_DIR/.git" ] && [ ! -f "$VCPKG_DIR/.git" ]; then
    echo "→ Initializing vcpkg submodule..."
    cd "$PROJECT_ROOT"
    git submodule update --init --recursive external/vcpkg
fi

# Bootstrap vcpkg si l'exécutable n'existe pas
if [ ! -f "$VCPKG_DIR/vcpkg" ]; then
    echo "→ Bootstrapping vcpkg (this may take a few minutes)..."
    cd "$VCPKG_DIR"
    ./bootstrap-vcpkg.sh -disableMetrics
fi

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║                    Setup Complete!                       ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "vcpkg is ready at: $VCPKG_DIR"
echo ""
echo "Next steps:"
echo "  1. Configure:  cmake --preset linux-debug"
echo "  2. Build:      cmake --build build"
echo "  3. Run:        ./build/bin/r-type_server"
echo ""
echo "Tips:"
echo "  • To use your own vcpkg, set VCPKG_ROOT environment variable"
echo "  • Dependencies are defined in vcpkg.json"
echo ""
