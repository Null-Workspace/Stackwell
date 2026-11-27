# Stackwell

Stackwell is a minimal, plugin-first application host written in C++20.

The host provides infrastructure only: application lifecycle, configuration, logging,
plugin discovery and plugin lifecycle. Product functionality belongs in plugins.
A plugin is distributed as one native module file (`.dll` on Windows, `.so` on Linux,
`.dylib` on macOS) placed directly in the configured plugin directory. The host scans
that directory only; it does not require a folder or sidecar manifest for each plugin.

## Components

- **Core** — startup/shutdown, configuration and logging.
- **Plugin runtime** — native module loading, ABI/version validation and lifecycle.
- **CLI** — a thin frontend for host information and plugin inventory.
- **Desktop** — a native Windows frontend using Win32, Direct2D and DirectWrite.

The host contains no bundled utility modules or example plugins. With an empty plugin
directory, Stackwell remains an empty host and plugin manager.

The community plugin catalog, public SDK header and plugin examples live in the separate
[Stackwell Plugins repository](https://github.com/Null-Workspace/Stackwell-plugins).
The catalog data is available at
[`catalog.json`](https://raw.githubusercontent.com/Null-Workspace/Stackwell-plugins/main/catalog.json).
The desktop application does not yet install catalog entries automatically; catalog
browsing and installation require a later host-side integration.

## Build

Requirements: C++20 compiler, CMake 3.25+, vcpkg.

```powershell
cmake --preset "Visual Studio Community 2026 Release - amd64"
cmake --build --preset "Visual Studio Community 2026 Release - amd64-debug"
```

Executables are placed in `out/build/<preset>/dist/bin`. Plugins are loaded from
`out/build/<preset>/dist/plugins`.

## Plugin ABI

The C-compatible contract is declared in
`include/stackwell/plugin/plugin_api.h`. A plugin exports
`stackwell_plugin_query`, returns a descriptor, and implements initialize/shutdown.
The host validates the descriptor and API version before initialization.

Plugins are native code and execute in-process with the host's permissions. Only install
plugins from sources you trust.

## Configuration

Runtime configuration is copied to `dist/resources/config/config.json`. Relative
paths are resolved from the distribution directory. The plugin directory defaults to
`plugins`; logs and storage default to `logs` and `data`.

## Desktop

The desktop shell contains only Overview, Plugins and Settings. It does not expose
hardcoded pages for optional functionality. Plugin-specific UI contributions require
an explicit extension API and are not represented as available until that API exists.

## License

Apache License 2.0. See [LICENSE](LICENSE) and [NOTICE](NOTICE).
