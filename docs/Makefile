.PHONY: help install dev build doxygen clean serve docs-full

help: ## Show this help message
	@echo "R-Type Documentation - Available Commands:"
	@echo ""
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2}'
	@echo ""

install: ## Install Docusaurus dependencies
	cd website && npm install

dev: ## Start Docusaurus development server
	cd website && npm start

build: ## Build Docusaurus site only
	cd website && npm run build

doxygen: ## Generate Doxygen documentation
	doxygen Doxyfile

docs-full: ## Generate complete documentation (Doxygen + Docusaurus)
	./generate-docs.sh

serve: ## Serve production build locally
	cd website && npm run serve

clean: ## Clean all generated documentation
	rm -rf doxygen/
	rm -rf website/build/
	rm -rf website/.docusaurus/
	rm -rf website/static/api/

clean-all: clean ## Clean generated docs and node_modules
	rm -rf website/node_modules/

update: ## Update Docusaurus dependencies
	cd website && npm update

check: ## Check documentation for broken links
	cd website && npm run build

deploy: ## Deploy to GitHub Pages
	cd website && npm run deploy
