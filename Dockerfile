# GeantCAD Dockerfile
# Build environment for GeantCAD with Qt6, VTK, and Geant4 support
#
# Build: docker build -t geantcad:latest .
# Run:   docker run -it --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix geantcad:latest

FROM ubuntu:22.04

LABEL maintainer="GeantCAD Team"
LABEL description="GeantCAD - CAD-like editor for Geant4 geometries"
LABEL version="0.2.0"

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Rome

# Install system dependencies
RUN apt-get update && apt-get install -y \
    # Build tools
    build-essential \
    cmake \
    ninja-build \
    git \
    wget \
    curl \
    pkg-config \
    # Qt6 dependencies
    qt6-base-dev \
    qt6-base-dev-tools \
    libqt6opengl6-dev \
    libqt6openglwidgets6 \
    qt6-tools-dev \
    # OpenGL and graphics
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libxrender-dev \
    libxcursor-dev \
    libxft-dev \
    libxinerama-dev \
    libxrandr-dev \
    libxi-dev \
    libxss-dev \
    libxt-dev \
    libxv-dev \
    libxxf86vm-dev \
    # VTK dependencies
    libvtk9-dev \
    libvtk9-qt-dev \
    # JSON library
    nlohmann-json3-dev \
    # Python (for bindings)
    python3 \
    python3-dev \
    python3-pip \
    python3-venv \
    # X11 for GUI
    x11-apps \
    libx11-xcb1 \
    # Additional utilities
    nano \
    && rm -rf /var/lib/apt/lists/*

# Install pybind11
RUN pip3 install --no-cache-dir pybind11

# Set Qt6 as default
ENV QT_QPA_PLATFORM=xcb
ENV QT_SELECT=qt6

# ===== Build VTK with Qt6 support (if not available in repos) =====
# Uncomment the following section if you need a custom VTK build with Qt6

# ARG VTK_VERSION=9.3.0
# RUN cd /opt && \
#     wget -q https://www.vtk.org/files/release/9.3/VTK-${VTK_VERSION}.tar.gz && \
#     tar -xzf VTK-${VTK_VERSION}.tar.gz && \
#     rm VTK-${VTK_VERSION}.tar.gz && \
#     mkdir VTK-${VTK_VERSION}/build && \
#     cd VTK-${VTK_VERSION}/build && \
#     cmake .. \
#         -GNinja \
#         -DCMAKE_BUILD_TYPE=Release \
#         -DCMAKE_INSTALL_PREFIX=/usr/local \
#         -DVTK_QT_VERSION=6 \
#         -DVTK_GROUP_ENABLE_Qt=YES \
#         -DVTK_MODULE_ENABLE_VTK_GUISupportQt=YES \
#         -DVTK_BUILD_TESTING=OFF \
#         -DVTK_BUILD_EXAMPLES=OFF \
#         && \
#     ninja && \
#     ninja install && \
#     cd /opt && rm -rf VTK-${VTK_VERSION}

# ===== Optional: Install Geant4 =====
# Note: This adds ~2GB to the image. Enable if needed for testing generated projects.

# ARG GEANT4_VERSION=11.2.0
# RUN cd /opt && \
#     wget -q https://geant4-data.web.cern.ch/releases/geant4-v${GEANT4_VERSION}.tar.gz && \
#     tar -xzf geant4-v${GEANT4_VERSION}.tar.gz && \
#     rm geant4-v${GEANT4_VERSION}.tar.gz && \
#     mkdir geant4-v${GEANT4_VERSION}/build && \
#     cd geant4-v${GEANT4_VERSION}/build && \
#     cmake .. \
#         -GNinja \
#         -DCMAKE_BUILD_TYPE=Release \
#         -DCMAKE_INSTALL_PREFIX=/opt/geant4 \
#         -DGEANT4_USE_GDML=ON \
#         -DGEANT4_USE_QT=ON \
#         -DGEANT4_USE_OPENGL_X11=ON \
#         -DGEANT4_INSTALL_DATA=ON \
#         && \
#     ninja && \
#     ninja install && \
#     cd /opt && rm -rf geant4-v${GEANT4_VERSION}
# 
# ENV GEANT4_INSTALL_DIR=/opt/geant4
# ENV PATH="${GEANT4_INSTALL_DIR}/bin:${PATH}"
# ENV LD_LIBRARY_PATH="${GEANT4_INSTALL_DIR}/lib:${LD_LIBRARY_PATH}"

# ===== Optional: Install ROOT =====
# Note: This adds ~1GB to the image. Enable if needed for ROOT output support.

# RUN apt-get update && apt-get install -y root-system && rm -rf /var/lib/apt/lists/*

# Create working directory
WORKDIR /workspace

# Copy source code
COPY . /workspace/geantcad

# Build GeantCAD
RUN cd /workspace/geantcad && \
    mkdir -p build && \
    cd build && \
    cmake .. \
        -GNinja \
        -DCMAKE_BUILD_TYPE=Release \
        -DGEANTCAD_PREFER_QT6=ON \
        -DGEANTCAD_BUILD_PYTHON_BINDINGS=ON \
        -DGEANTCAD_USE_ROOT=OFF \
        && \
    ninja

# Set up environment
ENV LD_LIBRARY_PATH="/workspace/geantcad/build:${LD_LIBRARY_PATH}"
ENV PATH="/workspace/geantcad/build:${PATH}"
ENV PYTHONPATH="/workspace/geantcad/build:${PYTHONPATH}"

# Create entrypoint script
RUN echo '#!/bin/bash\n\
echo "GeantCAD Docker Container"\n\
echo "========================"\n\
echo ""\n\
echo "Available commands:"\n\
echo "  geantcad          - Launch the GeantCAD GUI"\n\
echo "  python3           - Python with geantcad_python module"\n\
echo "  cmake --build .   - Rebuild GeantCAD"\n\
echo ""\n\
if [ "$1" ]; then\n\
    exec "$@"\n\
else\n\
    exec /bin/bash\n\
fi\n' > /entrypoint.sh && chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]

# Default command
CMD ["geantcad"]

