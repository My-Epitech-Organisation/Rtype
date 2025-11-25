#!/bin/bash

# Script to generate both Doxygen and Docusaurus documentation

echo "Generating Doxygen documentation..."
cd "$(dirname "$0")"
doxygen Doxyfile

echo "Copying Doxygen HTML output to Docusaurus static folder..."
mkdir -p website/static/api
cp -r doxygen/html/* website/static/api/

echo "Building Docusaurus site..."
cd website
npm run build

echo "Documentation generated successfully!"
echo "- Doxygen output: docs/doxygen/html/index.html"
echo "- Docusaurus output: docs/website/build/index.html"
echo ""
echo "To preview Docusaurus locally, run: cd docs/website && npm start"
