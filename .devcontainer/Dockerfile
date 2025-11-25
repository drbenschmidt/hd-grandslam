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
    && rm -rf /var/lib/apt/lists/*

# Add LLVM official repo (Clang 19 example)
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 19 && \
    rm llvm.sh

# Install build tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake \
    ninja-build \
    git \
    && rm -rf /var/lib/apt/lists/*

# Create user
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=1000

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd -s /bin/bash --uid $USER_UID --gid $USER_GID -m $USERNAME

USER $USERNAME

# Install Conan AS THE NON-ROOT USER
RUN pipx install conan && pipx ensurepath
ENV PATH="/home/${USERNAME}/.local/bin:${PATH}"

WORKDIR /workspaces
