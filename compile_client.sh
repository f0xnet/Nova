#!/bin/bash

# Nova Engine Client Build Script
echo "========================================"
echo "  NOVAENGINE BUILD SYSTEM (Linux)"
echo "========================================"
echo ""

# Configuration
PROJECT_DIR="/home/user/Nova"
SOURCE_DIR="$PROJECT_DIR/client/src"
BIN_DIR="$PROJECT_DIR/client/bin/Release"
OBJ_DIR="$PROJECT_DIR/client/obj/Release"
SDK_DIR="$PROJECT_DIR/sdk/include"
LIB_DIR="$PROJECT_DIR/sdk/libs"

# Create directories
mkdir -p "$BIN_DIR"
mkdir -p "$OBJ_DIR"

echo "[STEP 1/3] Compiling source files..."

# Find all .cpp files
CPP_FILES=$(find "$SOURCE_DIR" -name "*.cpp")

# Compile each file
OBJECT_FILES=""
for file in $CPP_FILES; do
    # Get filename without path and extension
    filename=$(basename "$file" .cpp)
    # Get relative path
    relpath=$(dirname "${file#$SOURCE_DIR/}")

    # Create object directory structure
    mkdir -p "$OBJ_DIR/$relpath"

    obj_file="$OBJ_DIR/${file#$SOURCE_DIR/}"
    obj_file="${obj_file%.cpp}.o"

    echo "  Compiling: $file"

    g++ -c "$file" -o "$obj_file" \
        -I"$SDK_DIR" \
        -DSFML_STATIC \
        -std=c++17 \
        -O2 \
        -Wall

    if [ $? -ne 0 ]; then
        echo "ERROR: Compilation failed for $file"
        exit 1
    fi

    OBJECT_FILES="$OBJECT_FILES $obj_file"
done

# Also compile main.cpp
echo "  Compiling: $PROJECT_DIR/client/main.cpp"
g++ -c "$PROJECT_DIR/client/main.cpp" -o "$OBJ_DIR/main.o" \
    -I"$SDK_DIR" \
    -DSFML_STATIC \
    -std=c++17 \
    -O2 \
    -Wall

if [ $? -ne 0 ]; then
    echo "ERROR: Compilation failed for main.cpp"
    exit 1
fi

OBJECT_FILES="$OBJECT_FILES $OBJ_DIR/main.o"

echo ""
echo "[STEP 2/3] Linking executable..."

g++ $OBJECT_FILES -o "$BIN_DIR/NovaEngine" \
    -L"$LIB_DIR" \
    -DSFML_STATIC \
    -lsfml-graphics-s \
    -lsfml-window-s \
    -lsfml-audio-s \
    -lsfml-system-s \
    -lopengl32 \
    -lwinmm \
    -lgdi32 \
    -lfreetype \
    -lopenal32 \
    -lFLAC \
    -lvorbisenc \
    -lvorbisfile \
    -lvorbis \
    -logg \
    -std=c++17 \
    -static-libgcc \
    -static-libstdc++ \
    -static

if [ $? -ne 0 ]; then
    echo "ERROR: Linking failed"
    exit 1
fi

echo ""
echo "[STEP 3/3] Build complete!"
echo ""
echo "Executable: $BIN_DIR/NovaEngine"
echo ""
echo "========================================"
echo "  BUILD SUCCESSFUL!"
echo "========================================"
