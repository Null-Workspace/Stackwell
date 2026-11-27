
# Stackwell

https://discord.gg/8hnYveje82

**Stackwell** is an extensible system administration platform written in C++20.

The project follows a modular monolith architecture and supports dynamically loaded plugins through a C ABI.

## Features

- Native Windows desktop application (Stackwell Desktop / SUF foundation)
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

## Desktop Application (Windows)

The Windows build produces two frontends over the same `stackwell_core` library:

- `stackwell.exe` - the existing CLI frontend
- `stackwell-desktop.exe` - the native desktop frontend built with Win32, Direct2D and DirectWrite

The initial desktop shell includes Overview, Plugin Manager, Access Recovery, Diagnostics and Settings pages. Access Recovery actions are read-only and use the existing Core module.

Build both frontends with:

```powershell
cmake --preset "Visual Studio Community 2026 Release - amd64"
cmake --build --preset "Visual Studio Community 2026 Release - amd64-debug"
```

Launch the desktop application:

```powershell
& ".\out\build\Visual Studio Community 2026 Release - amd64\dist\bin\stackwell-desktop.exe"
```

## Current Commands

| Command | Description |
|---|---|
| `stackwell --help` | Display available commands |
| `stackwell info` | Display application information |
| `stackwell plugins` | List loaded plugins and their metadata |
| `stackwell access-recovery scan --path <path>` | Inspect a file or directory security descriptor (read-only) |
| `stackwell access-recovery policies` | Inspect selected Windows policy registry values (read-only) |

## Access Recovery (initial read-only implementation)

The Access Recovery core module provides diagnostic inspection without modifying
system permissions or policy settings.

- NTFS security descriptor inspection for a selected file or directory
- Owner SID, DACL presence, inheritance protection and ACE count reporting
- Read-only inspection of a small set of Windows policy registry values
- Windows-specific inspection is guarded at compile time; non-Windows builds
  report the platform limitation

This first implementation intentionally does not change ownership, ACLs, registry
values, or administrative policy. A later recovery workflow should use explicit
plans, backups, authorization checks and user confirmation before any changes.

## UI Architecture

`src/ui/suf` contains the first native SUF desktop shell. It is intentionally kept separate from the CLI frontend and links against the same `stackwell_core` library. The UI currently uses Win32 for the window and input loop, Direct2D for rendering, and DirectWrite for text.

## Architecture

The project is organized into several components:

- **Core** - application lifecycle, configuration and logging
- **Commands** - command registration and CLI interface
- **Plugin System** - dynamic module loading and lifecycle management
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

Stackwell is under active development. Its architecture, APIs and features may change.

## License

Stackwell is licensed under the Apache License 2.0.

You may use, modify, and distribute Stackwell under the terms
of this license.

See [LICENSE](LICENSE) and [NOTICE](NOTICE) for details.
