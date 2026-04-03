# CHANGELOG.md

All notable changes to firmware.

## [Unreleased]

### Added

### Changed
- Route the STM32U575 scratch hello world through `firmware/app` and `firmware/port` while keeping Cube as the hardware-init target.

### Fixed

### Removed

---

## Rules

- One line per change.
- Focus on technical impact.
- Skip process narration.
- Update when behavior, interfaces, or safety-relevant handling changes.

Example:

```md
### Fixed
- Prevent repeated EXTI event counting caused by unstable input edge.
```
