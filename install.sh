#!/bin/bash
#
# Xenia Installation Script for Linux
#
# Usage:
#   ./install.sh              # Install to ~/.local (user install, no sudo required)
#   sudo ./install.sh system  # Install to /usr/local (system-wide)
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build/bin/Linux"

# Detect build type (prefer Release, fall back to Debug)
if [ -d "${BUILD_DIR}/Release" ] && [ -f "${BUILD_DIR}/Release/xenia" ]; then
    BUILD_TYPE="Release"
elif [ -d "${BUILD_DIR}/Debug" ] && [ -f "${BUILD_DIR}/Debug/xenia" ]; then
    BUILD_TYPE="Debug"
else
    echo "Error: No xenia binary found. Please build first with:"
    echo "  ./xenia-build build"
    exit 1
fi

BIN_DIR="${BUILD_DIR}/${BUILD_TYPE}"

# Determine install prefix
if [ "$1" = "system" ]; then
    PREFIX="/usr/local"
    DESKTOP_DIR="/usr/share/applications"
    ICON_DIR="/usr/share/icons/hicolor"
    echo "Installing system-wide to ${PREFIX}..."
else
    PREFIX="${HOME}/.local"
    DESKTOP_DIR="${HOME}/.local/share/applications"
    ICON_DIR="${HOME}/.local/share/icons/hicolor"
    echo "Installing to ${PREFIX} (user install)..."
fi

# Create directories
mkdir -p "${PREFIX}/bin"
mkdir -p "${DESKTOP_DIR}"
mkdir -p "${ICON_DIR}/16x16/apps"
mkdir -p "${ICON_DIR}/32x32/apps"
mkdir -p "${ICON_DIR}/48x48/apps"
mkdir -p "${ICON_DIR}/64x64/apps"
mkdir -p "${ICON_DIR}/128x128/apps"
mkdir -p "${ICON_DIR}/256x256/apps"
mkdir -p "${ICON_DIR}/512x512/apps"

# Install main binary
echo "Installing xenia binary..."
install -m 755 "${BIN_DIR}/xenia" "${PREFIX}/bin/xenia"

# Install optional tools
OPTIONAL_TOOLS=(
    "xenia-vfs-dump"
    "xenia-gpu-shader-compiler"
    "xenia-gpu-vulkan-trace-viewer"
    "xenia-gpu-vulkan-trace-dump"
)

for tool in "${OPTIONAL_TOOLS[@]}"; do
    if [ -f "${BIN_DIR}/${tool}" ]; then
        echo "Installing ${tool}..."
        install -m 755 "${BIN_DIR}/${tool}" "${PREFIX}/bin/${tool}"
    fi
done

# Install icons
echo "Installing icons..."
ICON_SIZES=(16 32 48 64 128 256 512)
for size in "${ICON_SIZES[@]}"; do
    if [ -f "${SCRIPT_DIR}/assets/icon/${size}.png" ]; then
        install -m 644 "${SCRIPT_DIR}/assets/icon/${size}.png" \
            "${ICON_DIR}/${size}x${size}/apps/xenia.png"
    fi
done

# Create desktop entry
echo "Creating desktop entry..."
cat > "${DESKTOP_DIR}/xenia.desktop" << EOF
[Desktop Entry]
Type=Application
Name=Xenia
GenericName=Xbox 360 Emulator
Comment=Xbox 360 video game console emulator
Exec=${PREFIX}/bin/xenia %f
Icon=xenia
Terminal=false
Categories=Game;Emulator;
MimeType=application/x-iso9660-image;application/x-xbox360-xex;
Keywords=xbox;360;emulator;game;
EOF

# Update icon cache if available
if command -v gtk-update-icon-cache &> /dev/null; then
    echo "Updating icon cache..."
    gtk-update-icon-cache -f -t "${ICON_DIR}" 2>/dev/null || true
fi

# Update desktop database if available
if command -v update-desktop-database &> /dev/null; then
    echo "Updating desktop database..."
    update-desktop-database "${DESKTOP_DIR}" 2>/dev/null || true
fi

echo ""
echo "Installation complete!"
echo ""
echo "Installed:"
echo "  - xenia binary: ${PREFIX}/bin/xenia"
echo "  - Desktop entry: ${DESKTOP_DIR}/xenia.desktop"
echo "  - Icons: ${ICON_DIR}/*/apps/xenia.png"
echo ""

if [ "$1" != "system" ]; then
    # Check if ~/.local/bin is in PATH
    if [[ ":$PATH:" != *":${HOME}/.local/bin:"* ]]; then
        echo "NOTE: ${HOME}/.local/bin is not in your PATH."
        echo "Add it to your shell configuration:"
        echo "  export PATH=\"\${HOME}/.local/bin:\${PATH}\""
        echo ""
    fi
fi

echo "Run 'xenia' to start the emulator, or find it in your application menu."
