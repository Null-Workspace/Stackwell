# Stackwell

**Stackwell** is a minimal, plugin-first application host. The host provides the runtime; optional functionality is supplied by independently distributed native plugins.

> **Current release line: 0.1.x (initial working foundation).** The Windows desktop shell, CLI, configuration, logging and native plugin lifecycle are implemented. Catalog browsing/installation, plugin-provided GUI pages and an installer are not part of this version.

## Get started

- **[User guide](docs/user-guide.md)** — launch the application, navigate the desktop interface, manage plugin files and troubleshoot common problems.
- **[Developer guide](docs/developer-guide.md)** — build the host, create a native plugin and understand the API v1 contract.
- **[Documentation index](docs/README.md)** — documentation map and project status.
- **[First release notes](docs/releases/0.1.0.md)** — scope and known limitations of the first version.

The first packaged Windows x64 release will be published on the [GitHub Releases page](https://github.com/Null-Workspace/Stackwell/releases) after the release build workflow completes.

## Product model

The host is intentionally small:

- **Core** — startup/shutdown, configuration and logging.
- **Plugin runtime** — native library discovery, ABI validation and plugin lifecycle.
- **CLI** — host information and plugin inventory.
- **Desktop** — native Windows shell built with Win32, Direct2D and DirectWrite.

The host does not bundle optional utility plugins. With an empty plugin directory, Stackwell remains a host and plugin manager.

A plugin is distributed as one native module file: `.dll` on Windows, `.so` on Linux or `.dylib` on macOS. Put the file directly in the configured plugin directory; no per-plugin folder or sidecar manifest is required.

The community SDK, example plugins and catalog are maintained separately in the [Stackwell Plugins repository](https://github.com/Null-Workspace/Stackwell-plugins). The desktop host in 0.1.x does **not** download or install catalog entries automatically.

## Build from source

Requirements:

- CMake 3.25 or newer
- C++20 compiler
- vcpkg
- Visual Studio 2026 C++ workload for the supplied Windows presets

Configure and build the Windows x64 Debug preset:

```powershell
cmake --preset "Visual Studio Community 2026 Release - amd64"
cmake --build --preset "Visual Studio Community 2026 Release - amd64-debug"
```

The desktop and CLI executables are written to `out/build/<preset>/dist/bin`. Runtime resources are copied to `dist/resources/config`; plugins are loaded from `dist/plugins`.

See the [developer guide](docs/developer-guide.md) for the full setup and plugin ABI.

## Security

Plugins are native code loaded into the Stackwell process. They run with the same operating-system permissions as the host and are **not sandboxed**. Install plugins only from sources you trust.

## License

Apache License 2.0. See [LICENSE](LICENSE) and [NOTICE](NOTICE).
