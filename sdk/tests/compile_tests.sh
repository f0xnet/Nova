#!/bin/bash
# Compile and run NovaEngine unit tests (no SFML, no game assets required)
set -e

SDK_DIR="$(cd "$(dirname "$0")/.." && pwd)/include"
TEST_DIR="$(cd "$(dirname "$0")" && pwd)"
CLIENT_SRC="$(cd "$(dirname "$0")/../../client/src" && pwd)"

echo "Building tests..."
g++ -std=c++17 -O0 -g \
    -I"$SDK_DIR" \
    -I"$TEST_DIR" \
    "$TEST_DIR/main.cpp" \
    "$TEST_DIR/test_entity_registry.cpp" \
    "$TEST_DIR/test_render_texture_pool.cpp" \
    "$CLIENT_SRC/Core/Logger.cpp" \
    -o "$TEST_DIR/nova_tests"

echo "Running tests..."
"$TEST_DIR/nova_tests"
