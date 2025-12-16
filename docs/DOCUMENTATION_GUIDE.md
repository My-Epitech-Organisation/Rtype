# Documentation Guide

## Overview

This document provides complete instructions for working with the R-Type documentation system.

## Structure

The R-Type project uses a dual documentation system:

1. **Docusaurus**: Web-based documentation for guides, tutorials, and architecture
2. **Doxygen**: Auto-generated API reference from C++ source code comments

Both systems are integrated so users can seamlessly navigate between conceptual documentation and API references.

## Setup

### Prerequisites

```bash
# Install Node.js (v20+)
# Install Doxygen
sudo apt-get install doxygen graphviz  # Linux
brew install doxygen graphviz          # macOS

# Install npm dependencies
cd docs/website
npm install
```

## Usage

### Development Workflow

#### 1. Start Docusaurus Dev Server

From project root:
```bash
cmake --preset linux-debug -DBUILD_DOCS=ON
cmake --build build --target docs-serve
```

This opens `http://localhost:3000` with live reload.

#### 2. Generate Doxygen Documentation

From project root:
```bash
cmake --preset linux-debug -DBUILD_DOCS=ON
cmake --build build --target docs-doxygen
```

Output will be in `build/docs/doxygen/html/index.html`

#### 3. Generate Complete Documentation

From project root (recommended):
```bash
cmake --preset linux-debug -DBUILD_DOCS=ON
cmake --build build --target docs
```

This will:
- Generate Doxygen API documentation
- Install Docusaurus dependencies (if not already installed)
- Copy Doxygen output to Docusaurus static folder

#### 4. Build Production Documentation

```bash
cmake --preset linux-release -DBUILD_DOCS=ON
cmake --build build --target docs-build
```

Output will be in `docs/website/build/`.

### Writing Documentation

#### Adding User Documentation

Create Markdown files in `docs/website/docs/`:

```markdown
---
sidebar_position: 3
---

# Page Title

Your content here...
```

#### Adding API Documentation

Add Doxygen comments to C++ headers:

```cpp
/**
 * @file MyClass.hpp
 * @brief Brief description of the file
 */

/**
 * @class MyClass
 * @brief Brief description of the class
 *
 * Detailed description with multiple paragraphs if needed.
 *
 * Example usage:
 * @code
 * MyClass obj;
 * obj.doSomething();
 * @endcode
 */
class MyClass {
public:
    /**
     * @brief Brief description of method
     * @param param1 Description of first parameter
     * @param param2 Description of second parameter
     * @return Description of return value
     * @throws std::runtime_error Description of when this is thrown
     */
    int doSomething(int param1, std::string param2);

private:
    int m_member; ///< Brief description of member variable
};
```

### Doxygen Comment Tags

Common tags:

- `@file` - File description
- `@class` - Class description
- `@brief` - Brief one-line description
- `@param` - Parameter description
- `@return` - Return value description
- `@throws` - Exception description
- `@code` / `@endcode` - Code example
- `@see` - Cross-reference
- `@note` - Important note
- `@warning` - Warning message
- `@deprecated` - Marks as deprecated

### Customization

#### Docusaurus Config

Edit `docs/website/docusaurus.config.ts`:

```typescript
const config: Config = {
  title: 'Your Title',
  tagline: 'Your tagline',
  url: 'https://your-site.com',
  baseUrl: '/',
  // ... more options
};
```

#### Doxygen Config

Key settings in `docs/Doxyfile`:

```
PROJECT_NAME           = "R-Type"
INPUT                  = ../include ../src
RECURSIVE              = YES
GENERATE_HTML          = YES
GENERATE_XML           = YES  # For potential Docusaurus integration
```

## Deployment

### Build for Production

```bash
cd docs/website
npm run docs:full
```

Output: `docs/website/build/`

### Deploy to GitHub Pages

1. Configure `docusaurus.config.ts`:
```typescript
organizationName: 'My-Epitech-Organisation',
projectName: 'Rtype',
```

2. Deploy:
```bash
cd docs/website
GIT_USER=<your-username> npm run deploy
```

### Deploy to Custom Server

```bash
cd docs/website
npm run build
# Upload build/ directory to your web server
```

## Maintenance

### Update Dependencies

```bash
cd docs/website
npm update
npm audit fix
```

### Rebuild from Scratch

```bash
cd docs/website
rm -rf build node_modules .docusaurus
npm install
npm run docs:full
```

### Clean Generated Files

```bash
cd docs
rm -rf doxygen website/build website/.docusaurus website/static/api
```

## Tips & Best Practices

### Documentation Writing

1. **Be Concise**: Write clear, concise descriptions
2. **Use Examples**: Include code examples for complex features
3. **Cross-Reference**: Link related documentation
4. **Keep Updated**: Update docs when code changes
5. **Test Examples**: Ensure code examples actually compile

### Doxygen Best Practices

1. Document all public APIs
2. Use `@brief` for short summaries
3. Add detailed descriptions after `@brief`
4. Document parameters and return values
5. Include usage examples with `@code`
6. Group related items with `@defgroup`

### Docusaurus Best Practices

1. Use clear sidebar hierarchy
2. Add frontmatter to all pages
3. Use MDX for interactive components
4. Optimize images before adding
5. Test on mobile devices

## Troubleshooting

### Doxygen Issues

**Problem**: "Input file not found"
```bash
# Check paths in Doxyfile INPUT setting
INPUT = ../include ../src
```

**Problem**: "dot: command not found"
```bash
# Install graphviz
sudo apt-get install graphviz
```

### Docusaurus Issues

**Problem**: Port 3000 already in use
```bash
npm start -- --port 3001
```

**Problem**: Build fails
```bash
npm run clear
npm install
npm run build
```

## Resources

- [Docusaurus Docs](https://docusaurus.io/docs)
- [Doxygen Manual](https://www.doxygen.nl/manual/index.html)
- [Markdown Guide](https://www.markdownguide.org/)
- [MDX Documentation](https://mdxjs.com/)

## Support

For issues or questions:
- Check existing GitHub issues
- Create a new issue with [docs] prefix
- Contact the documentation team
