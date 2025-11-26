# R-Type Project Makefile
# Global targets for build, run, and documentation

.PHONY: help build-debug build-release clean docs docs-serve docs-build run-server run-client test

# Default target: show help
help: ## Show this help message
	@echo "╔════════════════════════════════════════════════════════════════╗"
	@echo "║              R-Type Project - Available Commands               ║"
	@echo "╚════════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "📦 Build Commands:"
	@grep -E '^build-.*:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@echo "🎮 Run Commands:"
	@grep -E '^run-.*:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@echo "📚 Documentation Commands:"
	@grep -E '^docs.*:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@echo "🧪 Test Commands:"
	@grep -E '^test.*:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@echo "🧹 Utility Commands:"
	@grep -E '^clean.*:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo ""

# Build targets
build-debug: ## Build the project in debug mode
	@./scripts/build_debug.sh

build-release: ## Build the project in release mode
	@./scripts/build_release.sh

# Run targets
run-server: ## Run the R-Type server
	@./scripts/run_server.sh

run-client: ## Run the R-Type client
	@./scripts/run_client.sh

# Documentation targets
docs: ## Generate complete documentation (Doxygen + Docusaurus)
	@./scripts/generate_docs.sh

docs-serve: ## Start Docusaurus development server
	@./scripts/serve_docs.sh

docs-build: ## Build Docusaurus for production only
	@cd docs/website && npm run build

docs-install: ## Install documentation dependencies (npm)
	@echo "📦 Installing documentation dependencies..."
	@cd docs/website && npm install
	@echo "✅ Dependencies installed"

# Test targets
test: ## Run all tests
	@cd build && ctest --output-on-failure

# Clean targets
clean: ## Clean build artifacts
	@echo "🧹 Cleaning build artifacts..."
	@rm -rf build/
	@rm -rf build-debug/
	@rm -rf build-release/
	@echo "✅ Build artifacts cleaned"

clean-docs: ## Clean documentation artifacts
	@echo "🧹 Cleaning documentation artifacts..."
	@rm -rf docs/doxygen/
	@rm -rf docs/website/build/
	@rm -rf docs/website/.docusaurus/
	@rm -rf docs/website/static/api/
	@echo "✅ Documentation artifacts cleaned"

clean-all: clean clean-docs ## Clean all artifacts (build + docs)
	@echo "🧹 Cleaning node_modules..."
	@rm -rf docs/website/node_modules/
	@echo "✅ All artifacts cleaned"

# Quick aliases
.PHONY: debug release server client
debug: build-debug     ## Alias for build-debug
release: build-release ## Alias for build-release
server: run-server     ## Alias for run-server
client: run-client     ## Alias for run-client
