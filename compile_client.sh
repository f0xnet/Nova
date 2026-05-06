#!/bin/bash

echo "========================================"
echo "  NOVAENGINE BUILD SYSTEM (Linux)"
echo "========================================"
echo ""

PROJECT_DIR="/home/user/Nova"
SOURCE_DIR="$PROJECT_DIR/client/src"
BIN_DIR="$PROJECT_DIR/client/bin/Release"
OBJ_DIR="$PROJECT_DIR/client/obj/Release"
SDK_DIR="$PROJECT_DIR/sdk/include"
LIB_DIR="$PROJECT_DIR/sdk/libs"

mkdir -p "$BIN_DIR"
mkdir -p "$OBJ_DIR"
mkdir -p "$OBJ_DIR/nlohmann"

# ------------------------------------
# PCH — nlohmann/json.hpp
# Recompile uniquement si le header est plus récent que le .gch
# ------------------------------------
PCH_SRC="$SDK_DIR/nlohmann/json.hpp"
PCH_OUT="$OBJ_DIR/nlohmann/json.hpp.gch"

if [ ! -f "$PCH_OUT" ] || [ "$PCH_SRC" -nt "$PCH_OUT" ]; then
    echo "[PCH] Compiling precompiled header (json.hpp)..."
    g++ -x c++-header "$PCH_SRC" -o "$PCH_OUT" \
        -I"$SDK_DIR" \
        -DSFML_STATIC \
        -std=c++17 \
        -O2
    if [ $? -ne 0 ]; then
        echo "ERROR: PCH compilation failed"
        exit 1
    fi
    echo "[PCH] Done."
else
    echo "[PCH] Precompiled header up-to-date, skipping."
fi
echo ""

COMPILE_FLAGS="-I\"$SDK_DIR\" -I\"$OBJ_DIR\" -include nlohmann/json.hpp -DSFML_STATIC -std=c++17 -O2 -Wall"

echo "[STEP 1/3] Compiling source files..."

CPP_FILES=$(find "$SOURCE_DIR" -name "*.cpp")

OBJECT_FILES=""
for file in $CPP_FILES; do
    relpath=$(dirname "${file#$SOURCE_DIR/}")
    mkdir -p "$OBJ_DIR/$relpath"

    obj_file="$OBJ_DIR/${file#$SOURCE_DIR/}"
    obj_file="${obj_file%.cpp}.o"

    echo "  Compiling: $file"

    g++ -c "$file" -o "$obj_file" \
        -I"$SDK_DIR" \
        -I"$OBJ_DIR" \
        -include nlohmann/json.hpp \
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

echo "  Compiling: $PROJECT_DIR/client/main.cpp"
g++ -c "$PROJECT_DIR/client/main.cpp" -o "$OBJ_DIR/main.o" \
    -I"$SDK_DIR" \
    -I"$OBJ_DIR" \
    -include nlohmann/json.hpp \
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
