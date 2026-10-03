#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

: "${PICO_SDK_PATH:=$HOME/pico-sdk}"

if [[ ! -d "$PICO_SDK_PATH" ]]; then
    echo "Error: PICO_SDK_PATH does not exist: $PICO_SDK_PATH" >&2
    exit 1
fi

echo "==> Pico SDK: $PICO_SDK_PATH"
echo "==> Configuring..."
cmake -S . -B build -G Ninja

echo "==> Building..."
cmake --build build -j"$(nproc 2>/dev/null || echo 2)"

UF2="$SCRIPT_DIR/aic_pico.uf2"
BUILD_UF2="$SCRIPT_DIR/build/src/aic_pico.uf2"

if [[ -f "$UF2" ]]; then
    echo
    echo "Build complete: $UF2"
elif [[ -f "$BUILD_UF2" ]]; then
    echo
    echo "Build complete: $BUILD_UF2"
else
    echo
    echo "Build completed, but aic_pico.uf2 was not found." >&2
    exit 1
fi
