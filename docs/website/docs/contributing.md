---
sidebar_label: Contributing
sidebar_position: 10
---

# Contributing to R-Type

Thanks for contributing! We welcome improvements — contributions that follow our style and CI will get reviewed faster. This page summarizes our contribution workflow, how to set up the project locally, and useful tips for extending the engine, adding systems, and working with the network protocol.

## Table of contents

- Getting started
- Branching & commit conventions
- How to build & run the project locally
- Tests & quality checks
- How to contribute code
  - Add a new ECS system
  - Add a new network packet/opcode
- Documentation & website
- Reporting issues
- Code review & PR checklist
- Accessibility & testing guidelines

---

## Getting started

1. Fork the repo and create a branch from `main` or `dev` depending on the type of change.
1. Always keep your branch up to date with the base branch:

```bash
# fetch and rebase on the target branch
# Rebase against main or dev depending on your target
git fetch origin
git rebase origin/main
```

1. Create a feature branch with a descriptive name, using dashes for readability:

```bash
# examples:
feature/add-spawner-limit
fix/network/retry-logic
chore/docs/update-protocol
```

---

## Branch & commit conventions

- Branches: `feature/*`, `fix/*`, `chore/*`, `hotfix/*`.
- Use small and focused PRs: 1 feature/bugfix per PR.

Commit message format (recommended):

```text
<type>(<scope>): <short description>

<detailed description (if needed)>
```

Types:

- `feat`: New feature
- `fix`: Bug fix
- `chore`: Non-functional change (scripts, config)
- `docs`: Documentation
- `test`: Add/modify tests
- `refactor`: Refactor code (no functionality change)

Example:

```text
feat(network): add reliable channel statistics counters
```

---

## How to build & run the project locally

We use CMake + vcpkg. There are cross-platform CMake presets available.

1. Clone with submodules:

```bash
git clone --recursive https://github.com/My-Epitech-Organisation/Rtype.git
cd Rtype
```

2. Install dependencies (first time only):

```bash
./scripts/setup-vcpkg.sh
```

3. Configure & build:

```bash
# Linux Debug
cmake --preset linux-debug
# or Linux Release
cmake --preset linux-release

# Windows Debug (MSVC)
cmake --preset windows-debug
# or Windows Release
cmake --preset windows-release

# Build
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure
```

4. Run the server and client (in separate terminals):

```bash
# Start server in one terminal
./scripts/run_server.sh
# Start client in another terminal
./scripts/run_client.sh
```

5. Running specific binaries manually (e.g., debugging):

```bash
# Server binary path depends on your build configuration
./build/bin/r-type_server
# Client binary
./build/bin/r-type_client
```

---

## Running tests & quality checks

- Run unit & integration tests locally:

```bash
ctest --test-dir build --output-on-failure
```

- Run the project's quality check script (lint, cpplint, clang-format checks, etc.):

```bash
./scripts/run_quality_checks.sh
```

- Format code with `clang-format` (recommended):

```bash
clang-format -i $(git ls-files '*.cpp' '*.hpp' '*.c' '*.h')
```

- Run cpplint (optional):

```bash
python3 cpplint/cpplint.py --recursive src include lib
```

**Note**: CI will run tests and quality checks automatically on PRs.

---

## How to contribute code

Make sure your changes are small and test-covered where possible — tests speed up reviews and spot regressions.

### Add a new ECS System (server or client)

1) Create new system header and implementation in appropriate folder:

- `src/games/rtype/server/Systems/` for server systems, or
- `src/games/rtype/client/Systems/` for client systems.

1. Derive from `rtype::engine::ASystem` and implement the `update` method.
1. Use the `Registry` and `View` constructs from `lib/ecs` to find entities/components.
1. Register the system with the scheduler or add it to `GameEngine::initialize()` (server) or scene creation (client).
1. Add unit tests in `tests/**` to validate system logic.
1. Add README or documentation in `docs/website/docs/Architecture` describing the system purpose and interfaces.

### Add a new network message (opcode/payload)

When adding a new payload or opcode, keep in mind that the protocol is binary and validated carefully.

1) Add the opcode to `lib/network/src/protocol/OpCode.hpp`.
2) Add the payload struct to `lib/network/src/protocol/Payloads.hpp` and add size assertions.
3) If needed, add serialization/deserialization logic to `lib/network/src/Serializer.cpp`.
4) Update `lib/network/src/protocol/Validator.hpp` if this opcode requires custom payload validation.
5) Add handlers on the server (`src/server/network/NetworkServer.cpp`) and client (`src/client/network/NetworkClient.cpp`) to process or generate this message.
6) Update the RFC & docs: `docs/RFC/RFC_RTGP_v1.2.0.md` and `docs/website/docs/protocol` (create the page if needed).
7) Add unit tests for serialization and integration tests to ensure both sides interpret the payload correctly.

---

## Documentation & website

- Keep the Docusaurus website docs updated in `docs/website/docs/`.
- For API-level docs, update Doxygen comments and run `docs` CI job.
- Short tasks that require docs update should include a small documentation change in the PR (how to use new API or new system).

How to update the documentation locally:

```bash
# build docs (if BUILD_DOCS preset is supported)
cmake --preset linux-debug -DBUILD_DOCS=ON
cmake --build build --target docs
# run the docs-serve target if necessary
cmake --build build --target docs-serve
```

---

## Reporting issues

- Use the issue tracker in GitHub. Provide the following in an issue:
  - Summary & expected behavior
  - Steps to reproduce
  - Logs, environment details, OS, steps to reproduce locally
  - Minimal reproducible example or test
  - For security concerns, do not open a public issue; contact the maintainers privately.

---

## Pull-request & code review checklist

Make your PR easy to review:

- [ ] Describe the change clearly in the PR (what, why, impact)
- [ ] Add tests for new/changed features (unit/integration as needed)
- [ ] Keep changes focused and small
- [ ] Update or add docs for new public APIs
- [ ] Run `./scripts/run_quality_checks.sh` locally (lint & formatting)
- [ ] Ensure CI passes (tests, lint, docs)

**Reviewers** should consider:

- Architectural fit and system boundaries
- API stability and lifetime
- Thread safety and concurrency issues for server code
- Memory or security concerns for network handling

---

## Accessibility & UX considerations

We aim to make the game accessible. When working on features that affect UI or input:

- Add keyboard-only controls or configurable keys in `config/client/controls.json`
- Include audio cues for essential events and UI toggles to reduce motion
- Add colorblind-friendly palettes and ensure critical gameplay elements are not only color-based
- Add a brief accessibility note in the PR if it touches UI or UX

---

## CI & repository automation

- The repo uses GitHub Actions (see `.github/workflows`):
  - `ci.yml` runs build, tests, lint, and style checks
  - `docs.yml` builds website & Doxygen documentation
  - `release.yml` makes releases and artifacts
- Enforce CI checks on PRs using repo branch protection rules when merging.

---

## Debugging & Integration tests (Network-specific)

- Integration tests and network tests are in the `tests/` hierarchy; run `ctest --output-on-failure`.
- For network testing, try the `tests/integration` suite that verifies client-server message flows.
- Use network tools (tc/netem) to simulate packet loss and latency locally when testing the reliability layer.

---

## Need help?

If you’re not sure how to implement a change, open an issue with a proposal or discuss your approach in a PR draft. We are happy to offer suggestions.

---

Thanks for helping R-Type grow — we appreciate your contributions!
