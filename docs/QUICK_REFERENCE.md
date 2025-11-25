# R-Type Documentation - Quick Reference

## 🚀 Quick Commands

### Start Development Server
```bash
cd docs/website && npm start
```
Opens: http://localhost:3000

### Generate All Documentation
```bash
cd docs && ./generate-docs.sh
```

### Generate Only Doxygen
```bash
cd docs && doxygen Doxyfile
```

### Build for Production
```bash
cd docs/website && npm run build
```

## 📂 Key Files

| File/Directory | Purpose |
|---------------|---------|
| `docs/website/docs/` | Markdown documentation pages |
| `docs/Doxyfile` | Doxygen configuration |
| `docs/generate-docs.sh` | Build script for all docs |
| `docs/website/docusaurus.config.ts` | Docusaurus settings |
| `docs/DOCUMENTATION_GUIDE.md` | Complete usage guide |

## ✍️ Adding Documentation

### User Guide Page
Create `docs/website/docs/my-page.md`:
```markdown
---
sidebar_position: 5
---

# My Page Title

Content here...
```

### API Documentation
Add to C++ header:
```cpp
/**
 * @brief Brief description
 * @param x Parameter description
 * @return Return value
 */
int myFunction(int x);
```

Then run: `cd docs && doxygen Doxyfile`

## 🔗 Important Links

- **Local Dev Server**: http://localhost:3000
- **API Reference**: http://localhost:3000/api
- **GitHub Repo**: https://github.com/My-Epitech-Organisation/Rtype

## 🛠️ Troubleshooting

**Port in use?**
```bash
cd docs/website && npm start -- --port 3001
```

**Clean build?**
```bash
cd docs/website
rm -rf build node_modules .docusaurus
npm install && npm run build
```

**Doxygen missing?**
```bash
sudo apt-get install doxygen graphviz  # Linux
brew install doxygen graphviz          # macOS
```

## 📚 Documentation Structure

```
Documentation Home
├── Introduction (intro.md)
├── Getting Started (getting-started.md)
├── Architecture
│   ├── Overview (architecture/overview.md)
│   └── ECS Guide (architecture/ecs-guide.md)
├── API Reference (/api) → Doxygen
└── Blog
```

## 🎯 npm Scripts (in docs/website/)

```bash
npm start              # Dev server
npm run build          # Production build
npm run serve          # Serve production build
npm run docs:doxygen   # Generate Doxygen
npm run docs:copy-api  # Copy API to static folder
npm run docs:full      # Full documentation build
```

## 📖 More Help

See `docs/DOCUMENTATION_GUIDE.md` for complete instructions.
