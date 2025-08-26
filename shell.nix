{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    # Build tools
    cmake
    gnumake
    gcc
    pkg-config
    
    # Libraries
    ncurses
    pipewire
    libsndfile
    taglib
    zlib          # Added this line
    
    # Runtime dependencies
    yt-dlp
    ffmpeg
    
    # Optional: Development tools
    gdb
    valgrind
    ccls  # For LSP support in editors
  ];
  
  shellHook = ''
    echo "╔══════════════════════════════════════╗"
    echo "║      UwU Music Player Dev Env        ║"
    echo "╚══════════════════════════════════════╝"
    echo ""
    echo "Build with:"
    echo "  mkdir -p build && cd build"
    echo "  cmake .."
    echo "  make -j$(nproc)"
    echo ""
    echo "Run with:"
    echo "  ./uwu"
    echo ""
    echo "ヾ(•ω•`)o"
  '';
  
  # Set up environment variables for PipeWire
  PKG_CONFIG_PATH = "${pkgs.pipewire.dev}/lib/pkgconfig:${pkgs.pipewire.dev}/share/pkgconfig";
  
  # Ensure we can find PipeWire headers
  NIX_CFLAGS_COMPILE = "-I${pkgs.pipewire.dev}/include/pipewire-0.3 -I${pkgs.pipewire.dev}/include/spa-0.2";
}
