# ReCore

**ReCore** is a command-line system administration toolkit written in C++20.

The project uses a modular monolith architecture and supports dynamically loaded plugins through a C ABI.

## Features

- Command-line interface powered by CLI11
- Structured logging with spdlog
- JSON-based application configuration
- Dynamic plugin loading
- Plugin API version validation
- Host and plugin version compatibility checks
- Plugin metadata support
- SQLite integration
- CMake-based build system
- vcpkg dependency management

## Current Commands

| Command | Description |
|---|---|
| `recore --help` | Display available commands |
| `recore info` | Display application information |
| `recore plugins` | List loaded plugins and their metadata |

## Architecture

The project is organized into several components:

- **Core** - application lifecycle, configuration and logging
- **Commands** - command registration and CLI interface
- **Plugin System** - dynamic module loading and lifecycle management
- **Storage** - persistent data infrastructure
- **Plugins** - independently built modules

## Technology Stack

- C++20
- CMake
- MSVC, GCC and Clang
- CLI11
- spdlog
- nlohmann/json
- SQLite
- vcpkg

## Building on Windows

### Requirements

- Visual Studio 2026 with C++ development tools
- CMake
- Git
- vcpkg

### Configure vcpkg

Set the `VCPKG_ROOT` environment variable to your vcpkg installation directory.

Example:

```powershell
$env:VCPKG_ROOT = "C:\vcpkg"
```

### Configure the project

For x64:

```powershell
cmake --preset "Visual Studio Community 2026 Release - amd64"
```

### Build

```powershell
cmake --build --preset "Visual Studio Community 2026 Release - amd64-debug"
```

The executable and plugin modules are generated in the build distribution directory.

## Project Status

ReCore is under active development. Its architecture, APIs and features may change.

## License

No license has been selected yet. No permission to reuse, modify or redistribute this code is granted by this repository.
