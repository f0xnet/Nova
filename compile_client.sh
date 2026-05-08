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
LUA_SRC="$PROJECT_DIR/deps/lua-5.4.7/src"

mkdir -p "$BIN_DIR" "$OBJ_DIR" "$OBJ_DIR/nlohmann" "$OBJ_DIR/sol"

# ------------------------------------
# Lua 5.4 — compile si liblua54.a absent ou sources plus récentes
# ------------------------------------
LUA_LIB="$LIB_DIR/liblua54.a"
if [ ! -f "$LUA_LIB" ] || [ "$LUA_SRC/lvm.c" -nt "$LUA_LIB" ]; then
    echo "[LUA] Compiling Lua 5.4.7..."
    LUA_OBJ_DIR="$OBJ_DIR/lua"
    mkdir -p "$LUA_OBJ_DIR"
    gcc -O2 -c -DLUA_USE_POSIX \
        "$LUA_SRC/lapi.c"    "$LUA_SRC/lauxlib.c" "$LUA_SRC/lbaselib.c" \
        "$LUA_SRC/lcode.c"   "$LUA_SRC/lcorolib.c" "$LUA_SRC/lctype.c" \
        "$LUA_SRC/ldblib.c"  "$LUA_SRC/ldebug.c"  "$LUA_SRC/ldo.c"    \
        "$LUA_SRC/ldump.c"   "$LUA_SRC/lfunc.c"   "$LUA_SRC/lgc.c"    \
        "$LUA_SRC/linit.c"   "$LUA_SRC/liolib.c"  "$LUA_SRC/llex.c"   \
        "$LUA_SRC/lmathlib.c" "$LUA_SRC/lmem.c"   "$LUA_SRC/loadlib.c"\
        "$LUA_SRC/lobject.c" "$LUA_SRC/lopcodes.c" "$LUA_SRC/loslib.c" \
        "$LUA_SRC/lparser.c" "$LUA_SRC/lstate.c"  "$LUA_SRC/lstring.c"\
        "$LUA_SRC/lstrlib.c" "$LUA_SRC/ltable.c"  "$LUA_SRC/ltablib.c"\
        "$LUA_SRC/ltm.c"     "$LUA_SRC/lundump.c" "$LUA_SRC/lutf8lib.c"\
        "$LUA_SRC/lvm.c"     "$LUA_SRC/lzio.c" 2>&1
    if [ $? -ne 0 ]; then echo "ERROR: Lua compilation failed"; exit 1; fi
    mv *.o "$LUA_OBJ_DIR/"
    ar rcs "$LUA_LIB" "$LUA_OBJ_DIR/"*.o
    echo "[LUA] Done -> $LUA_LIB"
else
    echo "[LUA] liblua54.a up-to-date, skipping."
fi
echo ""

# ------------------------------------
# PCH — nlohmann/json.hpp
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
    if [ $? -ne 0 ]; then echo "ERROR: PCH compilation failed"; exit 1; fi
    echo "[PCH] Done."
else
    echo "[PCH] json.hpp precompiled header up-to-date, skipping."
fi

# ------------------------------------
# PCH — sol/sol.hpp (sol2 — header-only, très lourd en templates)
# ------------------------------------
SOL_PCH_SRC="$SDK_DIR/sol/sol.hpp"
SOL_PCH_OUT="$OBJ_DIR/sol/sol.hpp.gch"

if [ ! -f "$SOL_PCH_OUT" ] || [ "$SOL_PCH_SRC" -nt "$SOL_PCH_OUT" ]; then
    echo "[PCH] Compiling precompiled header (sol/sol.hpp)..."
    g++ -x c++-header "$SOL_PCH_SRC" -o "$SOL_PCH_OUT" \
        -I"$SDK_DIR" \
        -DSFML_STATIC \
        -std=c++17 \
        -O2
    if [ $? -ne 0 ]; then echo "ERROR: sol PCH compilation failed"; exit 1; fi
    echo "[PCH] Done."
else
    echo "[PCH] sol/sol.hpp precompiled header up-to-date, skipping."
fi
echo ""

# ------------------------------------
# Dépendances incrémentielles
# Lit le fichier .d généré lors de la dernière compilation et vérifie si
# l'un des headers listés est plus récent que le .o — si oui, recompile.
# Retourne 0 (vrai) = recompilation nécessaire, 1 (faux) = à jour.
# ------------------------------------
needs_rebuild() {
    local obj="$1"
    local dep="${obj%.o}.d"

    # Pas encore compilé
    [ ! -f "$obj" ] && return 0

    # Pas de .d — impossible de vérifier les dépendances, on recompile par sécurité
    [ ! -f "$dep" ] && return 0

    # Lit le .d, supprime les '\' de continuation, éclate sur les espaces,
    # ignore le token "cible.o:" et vérifie chaque fichier listé
    local deps
    deps=$(sed 's/\\$//' "$dep" | tr '\n' ' ' | sed 's/[^:]*://')
    for f in $deps; do
        [ -f "$f" ] && [ "$f" -nt "$obj" ] && return 0
    done

    return 1
}

# ------------------------------------
# Compilation incrémentielle
# ------------------------------------
echo "[STEP 1/3] Compiling source files..."

# main.cpp est dans client/ (pas dans client/src/)
CPP_FILES="$(find "$SOURCE_DIR" -name "*.cpp") $PROJECT_DIR/client/main.cpp"

OBJECT_FILES=""
COMPILED=0
SKIPPED=0

for file in $CPP_FILES; do
    # Calcul du chemin .o et .d
    if [[ "$file" == "$SOURCE_DIR/"* ]]; then
        rel="${file#$SOURCE_DIR/}"
        obj_file="$OBJ_DIR/${rel%.cpp}.o"
    else
        # main.cpp → obj/Release/main.o
        obj_file="$OBJ_DIR/main.o"
    fi
    dep_file="${obj_file%.o}.d"

    mkdir -p "$(dirname "$obj_file")"

    if needs_rebuild "$obj_file"; then
        echo "  [COMPILE] ${file#$PROJECT_DIR/}"

        g++ -c "$file" -o "$obj_file" \
            -MMD -MF "$dep_file" \
            -I"$SDK_DIR" \
            -I"$OBJ_DIR" \
            -include nlohmann/json.hpp \
            -include sol/sol.hpp \
            -DSFML_STATIC \
            -std=c++17 \
            -O2 \
            -Wall

        if [ $? -ne 0 ]; then
            echo "ERROR: Compilation failed for $file"
            exit 1
        fi
        COMPILED=$((COMPILED + 1))
    else
        echo "  [UP-TO-DATE] ${file#$PROJECT_DIR/}"
        SKIPPED=$((SKIPPED + 1))
    fi

    OBJECT_FILES="$OBJECT_FILES $obj_file"
done

echo ""
echo "  Compiled: $COMPILED file(s) — Skipped: $SKIPPED file(s) (up-to-date)"

# ------------------------------------
# Aucun fichier recompilé et exécutable déjà présent → pas besoin de relinker
# ------------------------------------
if [ $COMPILED -eq 0 ] && [ -f "$BIN_DIR/NovaEngine" ]; then
    echo ""
    echo "Nothing to do — executable is up-to-date."
    echo "========================================"
    echo "  BUILD SUCCESSFUL (no changes)"
    echo "========================================"
    exit 0
fi

echo ""
echo "[STEP 2/3] Linking executable..."

g++ $OBJECT_FILES -o "$BIN_DIR/NovaEngine" \
    -L"$LIB_DIR" \
    -DSFML_STATIC \
    -lsfml-graphics-s \
    -lsfml-window-s \
    -lsfml-audio-s \
    -lsfml-system-s \
    -llua54 \
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

if [ $? -ne 0 ]; then echo "ERROR: Linking failed"; exit 1; fi

echo ""
echo "[STEP 3/3] Build complete!"
echo ""
echo "Executable: $BIN_DIR/NovaEngine"
echo ""
echo "========================================"
echo "  BUILD SUCCESSFUL!"
echo "========================================"
