#!/bin/bash
#
# Xenia Uninstallation Script for Linux
#
# Usage:
#   ./uninstall.sh              # Uninstall from ~/.local
#   sudo ./uninstall.sh system  # Uninstall from /usr/local
#

set -e

# Determine install prefix
if [ "$1" = "system" ]; then
    PREFIX="/usr/local"
    DESKTOP_DIR="/usr/share/applications"
    ICON_DIR="/usr/share/icons/hicolor"
    echo "Uninstalling from ${PREFIX} (system-wide)..."
else
    PREFIX="${HOME}/.local"
    DESKTOP_DIR="${HOME}/.local/share/applications"
    ICON_DIR="${HOME}/.local/share/icons/hicolor"
    echo "Uninstalling from ${PREFIX} (user install)..."
fi

# Remove binaries
BINARIES=(
    "xenia"
    "xenia-vfs-dump"
    "xenia-gpu-shader-compiler"
    "xenia-gpu-vulkan-trace-viewer"
    "xenia-gpu-vulkan-trace-dump"
)

echo "Removing binaries..."
for bin in "${BINARIES[@]}"; do
    if [ -f "${PREFIX}/bin/${bin}" ]; then
        rm -f "${PREFIX}/bin/${bin}"
        echo "  Removed ${PREFIX}/bin/${bin}"
    fi
done

# Remove desktop entry
echo "Removing desktop entry..."
if [ -f "${DESKTOP_DIR}/xenia.desktop" ]; then
    rm -f "${DESKTOP_DIR}/xenia.desktop"
    echo "  Removed ${DESKTOP_DIR}/xenia.desktop"
fi

# Remove icons
echo "Removing icons..."
ICON_SIZES=(16 32 48 64 128 256 512)
for size in "${ICON_SIZES[@]}"; do
    icon_path="${ICON_DIR}/${size}x${size}/apps/xenia.png"
    if [ -f "${icon_path}" ]; then
        rm -f "${icon_path}"
        echo "  Removed ${icon_path}"
    fi
done

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
echo "Uninstallation complete!"
echo ""
echo "Note: User configuration files in ~/.config/xenia/ were NOT removed."
echo "Remove them manually if desired:"
echo "  rm -rf ~/.config/xenia/"
