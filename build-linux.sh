#!/bin/bash
#
# Xenia-XT Build Script for Linux
#
# Usage:
#   ./build-linux.sh              # Build Release configuration
#   ./build-linux.sh --config=Debug      # Build Debug configuration
#   ./build-linux.sh setup        # Setup submodules and run premake
#   ./build-linux.sh clean        # Clean build outputs
#   ./build-linux.sh dist         # Build and create distribution package
#   ./build-linux.sh help         # Show help message
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Print colored messages
print_error() {
    echo -e "${RED}ERROR: $1${NC}" >&2
}

print_success() {
    echo -e "${GREEN}$1${NC}"
}

print_info() {
    echo -e "${YELLOW}$1${NC}"
}

# Check for Python 3
check_python() {
    if ! command -v python3 &> /dev/null; then
        print_error "Python 3 is required but not found"
        echo "Install Python 3.6+ and ensure it's in PATH"
        exit 1
    fi

    # Check Python version
    PYTHON_VERSION=$(python3 -c 'import sys; print(".".join(map(str, sys.version_info[:2])))')
    REQUIRED_VERSION="3.6"

    if ! python3 -c "import sys; sys.exit(0 if sys.version_info >= (3, 6) else 1)"; then
        print_error "Python version ${PYTHON_VERSION} is too old"
        echo "Python ${REQUIRED_VERSION}+ is required"
        exit 1
    fi
}

# Check for required build tools
check_build_tools() {
    print_info "Checking build dependencies..."

    MISSING_TOOLS=()

    # Check for essential build tools
    if ! command -v clang++ &> /dev/null && ! command -v g++ &> /dev/null; then
        MISSING_TOOLS+=("clang++ or g++")
    fi

    if ! command -v make &> /dev/null && ! command -v ninja &> /dev/null; then
        MISSING_TOOLS+=("make or ninja")
    fi

    if [ ${#MISSING_TOOLS[@]} -ne 0 ]; then
        print_error "Missing required build tools:"
        for tool in "${MISSING_TOOLS[@]}"; do
            echo "  - $tool"
        done
        echo ""
        echo "On Debian/Ubuntu, install with:"
        echo "  sudo apt-get install build-essential clang llvm ninja-build"
        echo ""
        echo "On Arch/Manjaro, install with:"
        echo "  sudo pacman -S base-devel clang llvm ninja"
        echo ""
        echo "On Fedora, install with:"
        echo "  sudo dnf install gcc-c++ clang llvm ninja-build"
        exit 1
    fi
}

# Check for required development libraries
check_dev_libraries() {
    print_info "Note: Some development libraries may be required for building."
    print_info "If the build fails, you may need to install additional packages."
    echo ""
    echo "On Debian/Ubuntu:"
    echo "  sudo apt-get install libgtk-3-dev libpthread-stubs0-dev liblz4-dev \\"
    echo "    libx11-dev libx11-xcb-dev libvulkan-dev libsdl2-dev libiberty-dev \\"
    echo "    libunwind-dev libc++-dev libc++abi-dev"
    echo ""
    echo "On Arch/Manjaro:"
    echo "  sudo pacman -S gtk3 libx11 vulkan-headers vulkan-icd-loader sdl2 libunwind"
    echo ""
    echo "On Fedora:"
    echo "  sudo dnf install gtk3-devel libX11-devel vulkan-headers vulkan-loader-devel \\"
    echo "    SDL2-devel libunwind-devel"
    echo ""
}

# Show help message
show_help() {
    cat << EOF
Xenia-XT Build Script for Linux

Usage: ./build-linux.sh [COMMAND] [OPTIONS]

Commands:
  (none)          Build Release configuration (default)
  setup           Setup submodules and run premake
  clean           Clean build outputs
  dist            Build and create distribution package
  help            Show this help message

Options:
  --config=TYPE   Build configuration: Debug, Release (default: Release)
  --target=NAME   Build specific target (e.g., xenia-app, xenia-vfs-dump)
  -j N            Number of parallel build jobs (default: number of CPU cores)
  --force         Force rebuild (clean before building)

Examples:
  ./build-linux.sh                    # Build Release
  ./build-linux.sh --config=Debug     # Build Debug
  ./build-linux.sh setup              # First-time setup
  ./build-linux.sh clean              # Clean outputs
  ./build-linux.sh dist               # Create distribution
  ./build-linux.sh --target=xenia-app # Build only xenia-app
  ./build-linux.sh -j 8               # Build with 8 parallel jobs

After building:
  - Binary location: build/bin/Linux/Release/xenia (or Debug/xenia)
  - Install system-wide: sudo ./install.sh system
  - Install for user: ./install.sh

For more information, see docs/building.md
EOF
}

# Parse command line arguments
COMMAND=""
CONFIG="release"
BUILD_ARGS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        setup|clean|dist|help)
            COMMAND="$1"
            shift
            ;;
        --config=*)
            CONFIG="${1#*=}"
            CONFIG=$(echo "$CONFIG" | tr '[:upper:]' '[:lower:]')
            BUILD_ARGS+=("--config=$CONFIG")
            shift
            ;;
        --target=*)
            BUILD_ARGS+=("$1")
            shift
            ;;
        -j)
            BUILD_ARGS+=("$1" "$2")
            shift 2
            ;;
        --force)
            BUILD_ARGS+=("$1")
            shift
            ;;
        *)
            print_error "Unknown argument: $1"
            echo "Run './build-linux.sh help' for usage information"
            exit 1
            ;;
    esac
done

# Handle commands
case "$COMMAND" in
    help)
        show_help
        exit 0
        ;;
    setup)
        print_info "Setting up Xenia build environment..."
        check_python
        check_build_tools
        check_dev_libraries

        print_info "Updating submodules..."
        git submodule update --init --recursive

        print_info "Running setup..."
        ./xenia-build setup

        print_success "Setup complete!"
        print_info "You can now build with: ./build-linux.sh"
        exit 0
        ;;
    clean)
        print_info "Cleaning build outputs..."
        ./xenia-build clean
        print_success "Clean complete!"
        exit 0
        ;;
    dist)
        print_info "Creating distribution package..."

        # Build release first
        CONFIG="release"
        BUILD_ARGS=("--config=release")

        check_python
        ./xenia-build build "${BUILD_ARGS[@]}"

        # Create dist directory
        DIST_DIR="${SCRIPT_DIR}/dist"
        rm -rf "${DIST_DIR}"
        mkdir -p "${DIST_DIR}"

        # Detect build type and copy binaries
        BUILD_DIR="${SCRIPT_DIR}/build/bin/Linux"
        if [ -f "${BUILD_DIR}/Release/xenia" ]; then
            BUILD_TYPE="Release"
        elif [ -f "${BUILD_DIR}/Debug/xenia" ]; then
            BUILD_TYPE="Debug"
        else
            print_error "No xenia binary found. Build may have failed."
            exit 1
        fi

        BIN_DIR="${BUILD_DIR}/${BUILD_TYPE}"

        # Copy main binary
        cp "${BIN_DIR}/xenia" "${DIST_DIR}/"

        # Copy optional tools if they exist
        OPTIONAL_TOOLS=(
            "xenia-vfs-dump"
            "xenia-gpu-shader-compiler"
            "xenia-gpu-vulkan-trace-viewer"
            "xenia-gpu-vulkan-trace-dump"
        )

        for tool in "${OPTIONAL_TOOLS[@]}"; do
            if [ -f "${BIN_DIR}/${tool}" ]; then
                cp "${BIN_DIR}/${tool}" "${DIST_DIR}/"
            fi
        done

        # Copy README and LICENSE
        if [ -f "README.md" ]; then
            cp README.md "${DIST_DIR}/"
        fi
        if [ -f "LICENSE" ]; then
            cp LICENSE "${DIST_DIR}/"
        fi

        # Create archive
        ARCHIVE_NAME="xenia-linux-$(date +%Y%m%d).tar.gz"
        tar -czf "${ARCHIVE_NAME}" -C dist .

        print_success "Distribution created!"
        echo "  Directory: ${DIST_DIR}"
        echo "  Archive: ${ARCHIVE_NAME}"
        exit 0
        ;;
    "")
        # Default: build
        check_python

        print_info "Building Xenia (${CONFIG} configuration)..."

        # Add default config if not specified
        if [[ ! " ${BUILD_ARGS[@]} " =~ " --config=" ]]; then
            BUILD_ARGS+=("--config=${CONFIG}")
        fi

        # Build
        ./xenia-build build "${BUILD_ARGS[@]}"

        # Show success message and binary location
        BUILD_DIR="${SCRIPT_DIR}/build/bin/Linux"
        CONFIG_CAPITALIZED="$(echo ${CONFIG:0:1} | tr '[:lower:]' '[:upper:]')${CONFIG:1}"

        if [ -f "${BUILD_DIR}/${CONFIG_CAPITALIZED}/xenia-xt" ]; then
            print_success "Build complete!"
            echo ""
            echo "Binary location: build/bin/Linux/${CONFIG_CAPITALIZED}/xenia-xt"
            echo ""
            echo "To run: ./build/bin/Linux/${CONFIG_CAPITALIZED}/xenia-xt"
            echo "To install: ./install.sh (user) or sudo ./install.sh system (system-wide)"
        else
            print_error "Build may have completed but binary not found at expected location"
            exit 1
        fi
        ;;
esac
