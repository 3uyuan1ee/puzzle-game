# Changelog

All notable changes to Puzzle Game will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Enhanced multiplayer features
- Puzzle editor functionality
- Achievement system
- Cloud save support

### Changed
- Improved UI responsiveness
- Optimized performance for large puzzles
- Enhanced network protocol

### Fixed
- Memory leaks in puzzle piece management
- UI scaling issues on high-DPI displays
- Network connection stability

## [1.0.0] - 2025-09-12

### Added
- ✅ Core puzzle game functionality
- ✅ Multiple game modes (Standard, Irregular, Jigsaw)
- ✅ Drag & drop interface with piece rotation
- ✅ Custom image support
- ✅ Timer and move counter
- ✅ Sound effects system (MP3 support)
- ✅ Network functionality with user accounts
- ✅ Online leaderboards
- ✅ Cross-platform support (macOS, Windows, Linux)
- ✅ Qt 6.6+ compatibility
- ✅ Complete user interface
- ✅ Comprehensive documentation

### Features
- **Game Modes**:
  - Standard Puzzle: 3x3, 4x4, 8x8 grid sizes
  - Custom grids: 2x2 to 10x10
  - Irregular Puzzle: 8 different shapes
  - Jigsaw Puzzle: Classic tab-and-blank pieces

- **Interactive Controls**:
  - Left-click drag and drop
  - Right-click rotation (90° increments)
  - Smart piece snapping
  - Real-time progress tracking

- **Audio System**:
  - Background music (MP3)
  - Sound effects for actions
  - Volume control

- **Network Features**:
  - User registration/login
  - Time, move, and level leaderboards
  - Client-server architecture

- **User Interface**:
  - Main menu
  - Game settings
  - Help system
  - Reference image display

### Technical Details
- **Framework**: Qt 6.6+ with C++17
- **Build System**: qmake + Make
- **Architecture**: MVC pattern with separated concerns
- **Platform Support**: macOS, Windows, Linux
- **License**: MIT License

### Dependencies
- Qt Core, GUI, Widgets, Multimedia, Network modules
- C++17 compatible compiler
- System-level audio support

### Known Issues
- No known issues at initial release

## [0.9.0] - 2025-09-10 (Beta)

### Added
- Basic puzzle game mechanics
- Standard grid sizes (3x3, 4x4)
- Drag and drop functionality
- Basic UI implementation
- Sound system foundation

### Changed
- Initial Qt 6 compatibility updates
- Code structure optimization

### Fixed
- Build configuration issues
- Memory management improvements

---

### Version History Philosophy

This project follows [Semantic Versioning](https://semver.org/):
- **MAJOR** version when you make incompatible API changes
- **MINOR** version when you add functionality in a backward compatible manner
- **PATCH** version when you make backward compatible bug fixes

Additional labels for pre-release and build metadata are available as extensions to the MAJOR.MINOR.PATCH format.