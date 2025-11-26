FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies for apt-transport and LLVM keys
RUN apt-get update && apt-get install -y --no-install-recommends \
    wget \
    gnupg \
    apt-transport-https \
    lsb-release \
    sudo \
    pipx \
    python3-venv \
    python3-pip \
    software-properties-common \
    gnupg \
    libva-dev \
    libvdpau-dev

# # install XCB stack
# RUN apt-get update && apt-get install -y \
#     xorg-dev \
#     libxcb-util-dev \
#     libxcb-icccm4-dev \
#     libxcb-image0-dev \
#     libxcb-keysyms1-dev \
#     libxcb-render-util0-dev \
#     libxcb-xkb-dev \
#     uuid-dev

# Add LLVM official repo (Clang 19 example)
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 19 && \
    rm llvm.sh

# Register toolchain so VS Code + CMake can find it
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-19 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-19 100 && \
    update-alternatives --install /usr/bin/lldb lldb /usr/bin/lldb-19 100 && \
    update-alternatives --install /usr/bin/cc cc /usr/bin/clang-19 100 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-19 100

# Install build tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake \
    ninja-build \
    build-essential \
    pkg-config \
    git

# Create user
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=1000

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd -s /bin/bash --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && usermod -aG sudo $USERNAME \
    && echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

USER $USERNAME

# Install Conan AS THE NON-ROOT USER
RUN pipx install conan && pipx ensurepath
ENV PATH="/home/${USERNAME}/.local/bin:${PATH}"

WORKDIR /workspaces
