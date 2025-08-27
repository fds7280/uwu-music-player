#!/usr/bin/env bash  

echo "🎵 Installing UwU Music Player dependencies..."

# Detect distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
    DISTRO=$ID
fi

# Install based on distribution
if [ "$DISTRO" = "arch" ] || [ "$DISTRO" = "manjaro" ]; then
    echo "Detected Arch-based system..."
    
    # Install everything for Arch
    sudo pacman -S --needed --noconfirm base-devel cmake git ncurses libsndfile taglib zlib pipewire pipewire-pulse ffmpeg yt-dlp imagemagick curl
    
elif [ "$DISTRO" = "ubuntu" ] || [ "$DISTRO" = "debian" ] || [ "$DISTRO" = "linuxmint" ] || [ "$DISTRO" = "pop" ]; then
    echo "Detected Debian-based system..."
    
    # Update package lists
    sudo apt update
    
    # Install build tools
    sudo apt install -y build-essential cmake pkg-config git curl
    
    # Install libraries
    sudo apt install -y libncurses-dev libsndfile1-dev libtag1-dev zlib1g-dev
    
    # Install PipeWire
    sudo apt install -y libpipewire-0.3-dev pipewire
    
    # Install runtime dependencies
    sudo apt install -y ffmpeg imagemagick
    
    # Install yt-dlp
    sudo curl -L https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp -o /usr/local/bin/yt-dlp
    sudo chmod a+rx /usr/local/bin/yt-dlp
    
elif [ "$DISTRO" = "fedora" ] || [ "$DISTRO" = "rhel" ] || [ "$DISTRO" = "centos" ]; then
    echo "Detected Red Hat-based system..."
    
    # Install development tools
    sudo dnf groupinstall -y "Development Tools"
    sudo dnf install -y cmake pkg-config git curl
    
    # Install libraries
    sudo dnf install -y ncurses-devel libsndfile-devel taglib-devel zlib-devel
    
    # Install PipeWire
    sudo dnf install -y pipewire-devel pipewire
    
    # Install runtime dependencies
    sudo dnf install -y ffmpeg imagemagick
    
    # Install yt-dlp
    sudo curl -L https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp -o /usr/local/bin/yt-dlp
    sudo chmod a+rx /usr/local/bin/yt-dlp
    
else
    echo "Unsupported distribution: $DISTRO"
    echo "Please install dependencies manually:"
    echo "  - Build tools: cmake, make, gcc/g++"
    echo "  - Libraries: ncurses, libsndfile, taglib, pipewire"
    echo "  - Runtime: ffmpeg, yt-dlp, imagemagick, curl"
    exit 1
fi

# WSL-specific: Install PulseAudio as fallback
if grep -qi microsoft /proc/version; then
    echo "WSL detected, installing PulseAudio..."
    if [ "$DISTRO" = "arch" ] || [ "$DISTRO" = "manjaro" ]; then
        sudo pacman -S --needed --noconfirm pulseaudio
    elif [ "$DISTRO" = "fedora" ] || [ "$DISTRO" = "rhel" ] || [ "$DISTRO" = "centos" ]; then
        sudo dnf install -y pulseaudio
    else
        sudo apt install -y pulseaudio
    fi
    pulseaudio --start 2>/dev/null || true
fi

echo "✅ Dependencies installed!"
echo ""
echo "🎵 UwU Music Player Features:"
echo "  • Local music playback with metadata"
echo "  • YouTube search and streaming"
echo "  • YouTube playlist import and management"
echo "  • ASCII art thumbnails"
echo "  • Progress bars and controls"
echo ""
echo "Build with:"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  make -j$(nproc)"
echo "  ./uwu"
echo ""
echo "🎶 Enjoy your music!"
