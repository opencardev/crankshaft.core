# Changelog

This project follows a changelog generated via CI to keep track of notable changes. Latest entries are also published as a CI artifact (`changelog-ci.md`) from the **Changelog** workflow.

## Unreleased
- Add About page acknowledgements, SBOM link, and changelog link
- Add link and header setting components for richer settings UI
- Introduce multi-architecture build pipeline (amd64, arm64, armhf) with DEB packaging
- Generate CycloneDX SBOM as part of CI/CD
- Create dedicated changelog workflow and documentation entry
- Update maintainer contact to opencardevuk@gmail.com

## 0.1.0
- Initial MVP: Core (C++/Qt6) and UI (QML/Qt6) with WebSocket bridge and EventBus
- Theme system (light/dark), i18n (en-GB, de-DE), Material Design Icons
- Unit tests and base CI pipeline
