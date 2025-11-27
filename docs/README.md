# R-Type Documentation

This folder contains the complete documentation setup for the R-Type project, including both Docusaurus (web documentation) and Doxygen (API reference).

## Documentation Structure

```
docs/
├── website/          # Docusaurus documentation site
│   ├── docs/         # Markdown documentation files
│   ├── blog/         # Blog posts
│   ├── src/          # React components and pages
│   └── static/       # Static assets (includes Doxygen output)
├── Doxyfile          # Doxygen configuration
├── generate-docs.sh  # Script to build all documentation
└── doxygen/          # Generated Doxygen output (git-ignored)
```

## Quick Start

### View Documentation Locally

#### Start Docusaurus Development Server

```bash
cd website
npm start
```

Open your browser to `http://localhost:3000`

### Generate Complete Documentation

Run the documentation generation script to build both Doxygen and Docusaurus:

```bash
./generate-docs.sh
```

This will:
1. Generate Doxygen API documentation from C++ source code
2. Copy Doxygen HTML output to Docusaurus static folder
3. Build the complete Docusaurus site

### Generate Only Doxygen Documentation

```bash
doxygen Doxyfile
```

Output will be in `doxygen/html/index.html`

## Documentation Technologies

### Docusaurus

- **Purpose**: User-facing documentation, guides, and tutorials
- **Technology**: React-based static site generator
- **Location**: `website/`
- **Content**: Markdown files in `website/docs/`

### Doxygen

- **Purpose**: API reference generated from C++ source code
- **Technology**: C++ documentation generator
- **Configuration**: `Doxyfile`
- **Output**: `doxygen/html/` (accessible via Docusaurus at `/api`)

## Integration

The two documentation systems are integrated:
- Doxygen generates API documentation from C++ comments
- Docusaurus serves as the main documentation portal
- The `generate-docs.sh` script copies Doxygen output to `website/static/api/`
- Docusaurus navigation includes a link to `/api` for the API reference

## Writing Documentation

### Adding User Documentation

1. Create a new Markdown file in `website/docs/`
2. Add frontmatter with `sidebar_position`
3. Write content using Markdown and MDX
4. The page will automatically appear in the sidebar

Example:

```markdown
---
sidebar_position: 3
---

# My New Page

Content goes here...
```

### Adding API Documentation

Add Doxygen comments to your C++ code:

```cpp
/**
 * @brief Brief description of the class
 *
 * Detailed description of what this class does.
 */
class MyClass {
public:
    /**
     * @brief Brief description of the method
     * @param param1 Description of parameter
     * @return Description of return value
     */
    int myMethod(int param1);
};
```

Then regenerate the documentation:

```bash
./generate-docs.sh
```

## Deployment

### Build for Production

```bash
cd website
npm run build
```

Output will be in `website/build/`

### Deploy to GitHub Pages

Configure in `website/docusaurus.config.ts` and run:

```bash
cd website
npm run deploy
```

## Configuration

### Docusaurus Config

Edit `website/docusaurus.config.ts` to customize:
- Site title and tagline
- Navigation menu
- Footer links
- Theme settings
- Plugins

### Doxygen Config

Edit `Doxyfile` to customize:
- Input directories
- Output format
- Included file patterns
- Graph generation options

## Maintenance

### Update Dependencies

```bash
cd website
npm update
```

### Clean Build

```bash
cd website
rm -rf build node_modules
npm install
npm run build
```

## Additional Resources

- [Docusaurus Documentation](https://docusaurus.io/)
- [Doxygen Manual](https://www.doxygen.nl/manual/)
- [Markdown Guide](https://www.markdownguide.org/)

## See Also

- `architecture/` - Architecture diagrams and design documents
- Project root `README.md` - Project overview and build instructions
