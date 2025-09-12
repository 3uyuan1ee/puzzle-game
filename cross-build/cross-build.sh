#!/bin/bash

# Qt Puzzle Game Cross-Compilation Build Script
# This script builds the game for multiple target platforms

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_DIR/cross-build"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}===========================================${NC}"
echo -e "${BLUE}  Qt Puzzle Game Cross-Compilation Build   ${NC}"
echo -e "${BLUE}===========================================${NC}"

# Function to build for a specific platform
build_platform() {
    local platform=$1
    local config_file=$2
    local target_dir="$BUILD_DIR/$platform"
    
    echo -e "\n${YELLOW}Building for $platform...${NC}"
    
    # Create target directory
    mkdir -p "$target_dir"
    
    # Change to target directory
    cd "$target_dir"
    
    # Run qmake with platform-specific config
    if [ -f "$config_file" ]; then
        qmake "$config_file" -spec linux-g++ "$PROJECT_DIR/PuzzleGame.pro"
    else
        echo -e "${RED}Error: Configuration file not found: $config_file${NC}"
        return 1
    fi
    
    # Build with make
    make -j$(nproc)
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ Build successful for $platform${NC}"
        return 0
    else
        echo -e "${RED}✗ Build failed for $platform${NC}"
        return 1
    fi
}

# Build for each platform
echo -e "\n${BLUE}Starting cross-compilation builds...${NC}"

# Windows x86_64
build_platform "windows-x86_64" "$BUILD_DIR/windows-x86_64.conf"

# Linux x86_64 (native build for testing)
build_platform "linux-x86_64" "$BUILD_DIR/linux-x86_64.conf"

# macOS arm64 (if on ARM64 Mac)
if [ "$(uname -m)" = "arm64" ]; then
    echo -e "\n${YELLOW}Building native macOS arm64 version...${NC}"
    cd "$PROJECT_DIR"
    qmake PuzzleGame.pro
    make clean
    make release
    mkdir -p "$BUILD_DIR/macos-arm64"
    cp "../release/PuzzleGame.app" "$BUILD_DIR/macos-arm64/" 2>/dev/null || cp "../release/PuzzleGame" "$BUILD_DIR/macos-arm64/" 2>/dev/null || true
fi

# Linux arm64 (if cross-compiler available)
if command -v aarch64-linux-gnu-g++ >/dev/null 2>&1; then
    build_platform "linux-arm64" "$BUILD_DIR/linux-arm64.conf"
else
    echo -e "${YELLOW}Warning: aarch64-linux-gnu-g++ not found, skipping Linux arm64 build${NC}"
fi

echo -e "\n${BLUE}===========================================${NC}"
echo -e "${GREEN}Cross-compilation build completed!${NC}"
echo -e "${BLUE}===========================================${NC}"

# Show build results
echo -e "\n${YELLOW}Build artifacts:${NC}"
for platform in windows-x86_64 linux-x86_64 macos-arm64 linux-arm64; do
    if [ -d "$BUILD_DIR/$platform" ]; then
        echo -e "  ${GREEN}✓ $platform:${NC} $BUILD_DIR/$platform"
        ls -la "$BUILD_DIR/$platform" 2>/dev/null | grep -E "(PuzzleGame|\.exe|\.app)$" || true
    fi
done

echo -e "\n${BLUE}To deploy the built executables:${NC}"
echo -e "  - Windows: Copy the .exe file and required Qt libraries"
echo -e "  - Linux: Copy the binary and required Qt libraries"
echo -e "  - macOS: Copy the .app bundle"