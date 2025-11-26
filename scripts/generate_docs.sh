#!/usr/bin/env bash
set -euo pipefail

# Get the project root directory (parent of scripts/)
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

echo "📚 Generating R-Type Documentation..."
echo "Project root: $PROJECT_ROOT"
echo ""

# Navigate to docs directory
cd "$PROJECT_ROOT/docs"

echo "🔨 Generating Doxygen documentation..."
doxygen Doxyfile

echo "📦 Copying Doxygen HTML output to Docusaurus static folder..."
mkdir -p website/static/api
cp -r doxygen/html/* website/static/api/

echo "🏗️  Building Docusaurus site..."
cd website

# Check if node_modules exists, if not, install dependencies
if [ ! -d "node_modules" ]; then
    echo "📦 Installing npm dependencies (first time setup)..."
    npm install
fi

npm run build

echo ""
echo "✅ Documentation generated successfully!"
echo "📂 Doxygen output: docs/doxygen/html/index.html"
echo "📂 Docusaurus output: docs/website/build/index.html"
echo ""
echo "💡 To preview Docusaurus locally, run:"
echo "   ./scripts/serve_docs.sh"
echo "   or: make docs-serve"
