# Stackwell Documentation

Документация Stackwell / Stackwell documentation.

## Guides

| Document | Description |
|---|---|
| [User guide](user-guide.md) | Установка из релизного архива, первый запуск, интерфейс, плагины, настройки и устранение проблем. |
| [Developer guide](developer-guide.md) | Сборка исходников, структура проекта, Plugin API v1, создание и проверка DLL-плагинов. |
| [Release 0.1.0](releases/0.1.0.md) | Состав первой версии, ограничения и известные отсутствующие функции. |

## Project status / Состояние проекта

Stackwell 0.1.x is the first usable host foundation, not a feature-complete administration suite.

Implemented:
- C++20 host core, configuration and logging.
- Native plugin discovery and API v1 lifecycle.
- CLI commands `info` and `plugins`.
- Windows desktop shell with Overview, Plugins and Settings pages.
- Build configuration using CMake and vcpkg.

Not implemented in this release:
- In-app plugin catalog browsing, downloading, installation or removal.
- Plugin-contributed GUI pages, commands or inter-plugin messaging.
- Sandboxing or permission isolation for native plugins.
- A graphical settings editor or a standalone setup installer.
- Prebuilt Linux or macOS desktop applications.

The Plugins page shows modules discovered at startup. Adding or removing a plugin file requires restarting Stackwell. The Settings page currently displays configuration information; it does not edit the configuration.

## Язык / Language

The guides are primarily in Russian with English terminology and code identifiers. The project intends to keep user-facing and developer documentation available in both Russian and English as the product grows.
