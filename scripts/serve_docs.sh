#!/usr/bin/env bash
set -euo pipefail

# Get the project root directory (parent of scripts/)
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

echo "🚀 Starting Docusaurus development server..."
echo "Project root: $PROJECT_ROOT"
echo ""

cd "$PROJECT_ROOT/docs/website"

# Check if node_modules exists, if not, install dependencies
if [ ! -d "node_modules" ]; then
    echo "📦 Installing npm dependencies (first time setup)..."
    npm install
    echo ""
fi

npm start
