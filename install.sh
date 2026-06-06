#!/usr/bin/env bash
set -euo pipefail

BOLD='\033[1m'
GREEN='\033[0;32m'
RESET='\033[0m'

echo -e "${BOLD}==> Chawa – WhatsApp Web Client${RESET}"
echo

echo -e "${BOLD}[1/3] Installing dependencies...${RESET}"
sudo pacman -S --needed --noconfirm \
    gtk4 \
    webkitgtk-6.0 \
    libnotify \
    libva \
    libva-mesa-driver \
    mesa \
    gstreamer \
    gst-plugins-bad \
    gst-plugin-va \
    meson \
    ninja \
    base-devel

echo -e "\n${BOLD}[2/3] Building...${RESET}"
rm -rf build
meson setup build \
    --buildtype=release \
    --optimization=3 \
    -Dstrip=true

cd build
ninja -j"$(nproc)"

echo -e "\n${BOLD}[3/3] Installing...${RESET}"
sudo ninja install
sudo update-desktop-database /usr/local/share/applications/ 2>/dev/null || true
sudo gtk-update-icon-cache -f /usr/local/share/icons/hicolor/ 2>/dev/null || true

echo
echo -e "${GREEN}✓ Done! Jalankan: chawa${RESET}"