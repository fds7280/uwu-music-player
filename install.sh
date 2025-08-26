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
    sudo pacman -S --needed --noconfirm base-devel cmake git ncurses libsndfile taglib zlib pipewire pipewire-pulse ffmpeg yt-dlp
    
elif [ "$DISTRO" = "ubuntu" ] || [ "$DISTRO" = "debian" ] || [ "$DISTRO" = "linuxmint" ] || [ "$DISTRO" = "pop" ]; then
    echo "Detected Debian-based system..."
    
    # Update package lists
    sudo apt update
    
    # Install build tools
    sudo apt install -y build-essential cmake pkg-config git
    
    # Install libraries
    sudo apt install -y libncurses-dev libsndfile1-dev libtag1-dev zlib1g-dev
    
    # Install PipeWire
    sudo apt install -y libpipewire-0.3-dev pipewire
    
    # Install runtime dependencies
    sudo apt install -y ffmpeg
    
    # Install yt-dlp
    sudo curl -L https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp -o /usr/local/bin/yt-dlp
    sudo chmod a+rx /usr/local/bin/yt-dlp
    
else
    echo "Unsupported distribution: $DISTRO"
    echo "Please install dependencies manually"
    exit 1
fi

# WSL-specific: Install PulseAudio as fallback
if grep -qi microsoft /proc/version; then
    echo "WSL detected, installing PulseAudio..."
    if [ "$DISTRO" = "arch" ] || [ "$DISTRO" = "manjaro" ]; then
        sudo pacman -S --needed --noconfirm pulseaudio
    else
        sudo apt install -y pulseaudio
    fi
    pulseaudio --start 2>/dev/null || true
fi

echo "✅ Dependencies installed!"
echo ""
echo "Build with:"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  make -j$(nproc)"
echo "  ./uwu"
EOF

