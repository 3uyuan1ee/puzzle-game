# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a Qt-based puzzle game application with multiple difficulty levels (3x3, 4x4, and 8x8). The game allows players to drag and drop puzzle pieces to solve the puzzle, with features like image rotation, custom image selection, and timer-based scoring.

## Code Architecture

### Main Components

1. **Main Menu** (`src/DlgMenu.*`) - Entry point with options to start game, select difficulty, choose custom images
2. **Game Modes**:
   - `src/play4x4.*` - Dynamic puzzle game supporting various grid sizes (2x2 to 10x10)
3. **Help System** (`help.*`) - Displays reference images for solved puzzles
4. **Networking Protocol** (`protocol.h`) - Defines network communication structures for potential online features

### Key Features

- Drag and drop puzzle piece manipulation
- Right-click rotation of puzzle pieces (90° increments)
- Custom image selection for puzzles
- Timer and move counter
- Multiple difficulty levels
- Resource management via Qt Resource System

## Build System

This project uses qmake build system. The project file is `PuzzleGame.pro`.

### Build Commands

```bash
# Generate Makefile
qmake PuzzleGame.pro

# Build the project
make

# Or build with qmake directly
qmake && make

# Output directories
# Debug builds: ../debug/
# Release builds: ../release/
```

### Build Directories

Build outputs are organized in `build/` directory with configurations for different Qt versions and compilers:
- `build/Desktop-Debug/` (current macOS builds)
- `build/Desktop_Qt_6_9_2_MinGW_64_bit-Debug/`
- `build/Desktop_Qt_6_8_0_MSVC2022_64bit-Debug/`
- `debug/` and `release/` (final executable locations)

## Development Environment

- **Framework**: Qt (C++)
- **Language**: C++11
- **UI**: Qt Widgets
- **Resource Management**: Qt Resource System (`.qrc` files)
- **Build Tools**: qmake/Make

## Key Project Files

- `PuzzleGame.pro` - Main project configuration
- `src/main.cpp` - Application entry point
- `src/DlgMenu.*` - Main menu implementation
- `src/play4x4.*` - 4x4 puzzle game implementation
- `res/img.qrc` - Resource file for images
- `protocol.h` - Network protocol definitions

## Game Flow

1. Application starts with `main()` in `src/main.cpp`
2. Main menu (`DlgMenu`) is displayed
3. Player selects difficulty level (3x3, 4x4, 8x8)
4. Corresponding game window is initialized and shown
5. Game starts with image selection (random or custom)
6. Image is sliced into puzzle pieces
7. Pieces are randomized and displayed
8. Player solves puzzle via drag/drop and rotation
9. Win condition triggers restart or return to menu

## Resource Structure

Images are stored in `res/img/` and referenced via Qt Resource System:
- Built-in images: `:photo/img/1.jpg` through `:photo/img/8.jpg`
- Additional images: `:/img/` prefix for UI elements (blank.png, imageshow.jpg, etc.)
- Custom images: Stored in application cache directory

## Network Protocol

The project includes a comprehensive networking protocol (`protocol.h`) for potential online features:
- User authentication (login/register)
- Ranking system with difficulty levels
- Structured packet communication with headers
- Support for different game difficulties (easy, medium, hard)