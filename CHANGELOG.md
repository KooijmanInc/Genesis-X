# Changelog

All notable changes to this project will be documented in this file.

The format loosely follows [Keep a Changelog](https://keepachangelog.com/)  
and this project adheres to [Semantic Versioning](https://semver.org/).

## Pre-Release Versions (alpha, beta, rc)
This project follows Semantic Versioning and uses optional pre-release identifiers to communicate the stability of versions during development. These tags appear after the version number:
```
1.0.0-alpha1
1.0.0-beta1
1.0.0-rc1
```
### Alpha (`-alpha`)
- Experimental, unstable versions
- Features may be incomplete or change rapidly
- Breaking changes are expected
- Intended for internal development and early testers only
#### Example use:
Early development of new modules (e.g., physics system, new wizards, or build tooling).

---

### Beta (`-beta`)

- Feature-complete but not yet stable
- APIs should not change drastically, but bugs are expected
- Suitable for testers who want to try upcoming releases
- Not recommended for production use

#### Example use:
Before a milestone release when all features are included but need validation.

---

### Release Candidate (`-rc`)

- Potential final version
- No new features planned — only bugfixes
- If no critical issues are found, this version becomes the stable release
- Focus is on polish and cross-platform verification

#### Example use:
Stabilizing installers, Qt Creator wizards, templates, and cross-platform builds before tagging `1.0.0`.

## [Unreleased]

### Added
- Master install script/entrypoint to install all Genesis-X Qt Creator features and helpers in one go.
- (Planned) Additional platform verification for Linux, macOS, Android and iOS.

### Fixed
- (Planned) Qt Creator wizard template: generated projects no longer contain broken configuration when using the Genesis-X template.
- (Planned) General cleanup of wizard files and installation paths.

### Changed
- Internal build and tooling structure to use a `build/master-install` workflow.

---

## [1.0.0-rc1]

### Added
- Initial public **Genesis-X** release candidate:
  - Core library integration for Qt 6.
  - Base Qt Creator project wizard(s).
  - Initial helpers and tooling for getting started.
