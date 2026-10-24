#!/bin/sh

# Added the “export” command to initialise the ‘CMAKE_OSX_ARCHITECTURES’ macOS variable by default. 
# Without this initialisation, assigning a value to the variable later does not appear to work, and the
# creation of a universal binary – whilst possible with a compatible version of the SDK – does not occur.
export CMAKE_OSX_ARCHITECTURES="x86_64"

# ---------------------------------------------------------------------------
# Project configuration. This block is the ONLY part that differs between the
# WINCTRL, Boarderline and ToLiss MCDU Plus copies of this script. Everything
# below the separator is shared; change it in one place and copy it across.
# ---------------------------------------------------------------------------

# Leave empty to derive from the .xcodeproj name, lowercased. Set it explicitly
# only when the Xcode project name and the .xpl basename disagree.
PROJECT_NAME=""

# Skunkcrafts updater identity.
MODULE_URL="https://ramonster.nl/winctrl-plugin"
MODULE_NAME="WINCTRL"

# Google Drive folder the release zip is uploaded to.
GDRIVE_FOLDER="1NtjQGUKH9Y8hrfOscwMPC99bVMnh7C0x"

# Folders copied verbatim into the distribution bundle. Space separated, may be empty.
BUNDLE_DIRS="fonts"

# Run after a successful build and upload. May be empty.
POST_BUILD="./update_readme.sh"

# Module folder on ramonster.nl to publish the finished zip to, e.g. "boarderline". Set this for a
# private repository, where the updater server cannot pull the release from GitHub itself. Leave it
# empty to skip the step entirely and not be asked about it.
UPLOAD_FOLDER=""

# ---------------------------------------------------------------------------
# Shared build logic. Keep this section byte-identical across projects; if you
# need to change something per project, promote it into the block above.
# ---------------------------------------------------------------------------

CONFIG_H="src/include/config.h"
AVAILABLE_PLATFORMS="mac win lin"

if [ -z "$PROJECT_NAME" ]; then
    PROJECT_NAME=$(find . -name "*.xcodeproj" | sed 's/\.xcodeproj//g' | sed 's/^\.\///g' | tr '[:upper:]' '[:lower:]')
fi
VERSION=$(grep "#define VERSION " $CONFIG_H | cut -d " " -f 3 | tr -d '"')
# VERSION is reassigned to the release tag (0.0.3-XP12) further down, but the updater server keys
# off the plain number, so keep a copy of it for the upload step.
BASE_VERSION="$VERSION"

# Shared secret for the /upload route, matching upload_token.txt in the server's module folder.
# Sits next to this script and is listed in .gitignore, so it is never committed.
UPLOAD_TOKEN_FILE="build_platforms_upload_token.txt"

echo "Building $PROJECT_NAME.xpl version $VERSION. Is this correct? (y/n) [default: y]:"
read CONFIRM

if [ -z "$CONFIRM" ]; then
    CONFIRM="y"
fi

if [ "$CONFIRM" != "y" ]; then
    echo "Please update the version number in $CONFIG_H and try again."
    exit 1
fi

# Platforms come from the command line when given, e.g. `./build_platforms.sh mac`,
# which is what you want while iterating. With no arguments it asks, defaulting to all.
if [ $# -gt 0 ]; then
    PLATFORMS="$@"
else
    echo "Which platforms would you like to build? ($AVAILABLE_PLATFORMS) [default: all]:"
    read PLATFORMS

    if [ -z "$PLATFORMS" ]; then
        PLATFORMS=$AVAILABLE_PLATFORMS
    fi
fi

for platform in $PLATFORMS; do
    if ! echo $AVAILABLE_PLATFORMS | grep -q $platform; then
        echo "Invalid platform: $platform. Exiting."
        exit 1
    fi
done

echo "Building for platforms: \033[1m$PLATFORMS\033[0m\n"

if [ ! -d "SDK" ]; then
    echo "SDK/ folder not found. Please ensure the SDK is present in the project root."
    exit 1
fi

SDK_VERSION=$(grep "#define kXPLM_Version" SDK/CHeaders/XPLM/XPLMDefs.h | awk '{print $3}' | tr -d '()')
JOBS=$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 4)

if [ $SDK_VERSION -lt 400 ]; then
    XPLANE_VERSION=XP11
else
    XPLANE_VERSION=XP12
fi
RELEASE_TAG=$VERSION-$XPLANE_VERSION
SYMBOLS_DIR="build/symbols/$RELEASE_TAG"

# $BIN (make's link target) is never stripped; stripping happens on $BIN.stripped, which is
# what gets shipped. That keeps debug info intact across repeated no-op rebuilds.
MIN_DEBUG_KB=512
check_debug_size() {
    SIZE_KB=$(du -sk "$1" 2>/dev/null | awk '{print $1}')
    if [ -z "$SIZE_KB" ] || [ "$SIZE_KB" -lt "$MIN_DEBUG_KB" ]; then
        echo "\033[1;31mERROR: debug info at $1 is only ${SIZE_KB:-0}KB - it looks empty.\033[0m"
        echo "If this is the first run after a build tree that stripped \$BIN in place, delete $BIN so make relinks it. Otherwise check that the build still compiles with -g1."
        exit 1
    fi
}

echo "Building with SDK version $SDK_VERSION\n"

echo "Clean build directory? (y/n) [default: n]:"
read CLEAN_BUILD

if [ -z "$CLEAN_BUILD" ]; then
    CLEAN_BUILD="n"
fi

echo "Upload to Google Drive after build? (y/n) [default: y]:"
read UPLOAD_TO_DRIVE

if [ -z "$UPLOAD_TO_DRIVE" ]; then
    UPLOAD_TO_DRIVE="y"
fi

# Asked here rather than at the end so the build runs unattended once the questions are answered.
if [ -n "$UPLOAD_FOLDER" ]; then
    echo "Publish to https://ramonster.nl/$UPLOAD_FOLDER after build? (y/n) [default: y]:"
    read UPLOAD_TO_SERVER

    if [ -z "$UPLOAD_TO_SERVER" ]; then
        UPLOAD_TO_SERVER="y"
    fi

    # Checked up front: finding this out after a three-platform build would waste the whole run.
    if [ "$UPLOAD_TO_SERVER" = "y" ] && [ ! -f "$UPLOAD_TOKEN_FILE" ]; then
        echo "\033[1;31mERROR: manual upload is enabled but $UPLOAD_TOKEN_FILE is missing.\033[0m"
        echo "It must hold the same secret as upload_token.txt in the server's $UPLOAD_FOLDER/ folder."
        echo "Generate a new pair with: openssl rand -hex 32 > $UPLOAD_TOKEN_FILE && chmod 600 $UPLOAD_TOKEN_FILE"
        exit 1
    fi
else
    UPLOAD_TO_SERVER="n"
fi

if [ "$CLEAN_BUILD" = "y" ]; then
    echo "Cleaning build directories..."
    if [ -d "build" ]; then
        rm -rf build
    fi
fi

for platform in $PLATFORMS; do
    echo "Building $platform..."
    BIN="build/$platform/${platform}_x64/${PROJECT_NAME}.xpl"
    if [ $platform = "lin" ]; then
        docker build -t xplane-build:jammy-gcc13 -f ./docker/Dockerfile.linux . && \
        docker run --user $(id -u):$(id -g) --rm -v $(pwd):/src -w /src xplane-build:jammy-gcc13 bash -c "\
        cmake -DCMAKE_CXX_FLAGS='-march=x86-64' -DCMAKE_TOOLCHAIN_FILE=toolchain-$platform.cmake -DSDK_VERSION=$SDK_VERSION -Bbuild/$platform -H. && \
        make -C build/$platform -j\$(nproc) && \
        if objdump -T $BIN | grep -q '__isoc23_'; then \
            echo 'ERROR: this build links post-glibc-2.36 symbols:'; \
            objdump -T $BIN | grep -o '__isoc23_[a-z]*' | sort -u; \
            echo 'The Linux image base is too new; the plugin will not load on older distros. See docker/Dockerfile.linux.'; \
            exit 1; \
        fi && \
        objcopy --only-keep-debug $BIN $BIN.debug && \
        cp $BIN $BIN.stripped && \
        strip --strip-debug --strip-unneeded $BIN.stripped && \
        objcopy --add-gnu-debuglink=$BIN.debug $BIN.stripped"
    else
        # The SDK version test for the macOS target architecture has been moved from ‘toolchain-mac.cmake’
        # to here, allowing CMake to better handle the desired parameters.
        if [ "$SDK_VERSION" -ge 400 ]; then
            CMAKE_OSX_ARCHITECTURES="arm64;x86_64"
        else
            CMAKE_OSX_ARCHITECTURES="x86_64"
        fi
        echo
        echo "============CMAKE PARAMS FOR MAC================="
        echo "TOOLCHAIN FILE CALLED : 'toolchain-$platform.cmake'"
        echo "SDK VERSION DETECTED  : '$SDK_VERSION'"
        echo "MAC ARCHITECTURE CIBLE: '$CMAKE_OSX_ARCHITECTURES'"
        echo

        cmake -DCMAKE_TOOLCHAIN_FILE=toolchain-$platform.cmake -DSDK_VERSION=$SDK_VERSION -Bbuild/$platform -H.
        make -C build/$platform -j$JOBS
    fi

    if [ $? -eq 0 ]; then
        # Archive debug info to build/symbols/ before stripping the shipped .xpl.
        mkdir -p "$SYMBOLS_DIR/${platform}_x64"
        case $platform in
            mac)
                dsymutil "$BIN" -o "$BIN.dSYM"
                check_debug_size "$BIN.dSYM"
                cp "$BIN" "$BIN.stripped"
                strip -x "$BIN.stripped"
                cp -r "$BIN.dSYM" "$SYMBOLS_DIR/${platform}_x64/"
                ;;
            win)
                x86_64-w64-mingw32-objcopy --only-keep-debug "$BIN" "$BIN.debug"
                check_debug_size "$BIN.debug"
                cp "$BIN" "$BIN.stripped"
                x86_64-w64-mingw32-strip -s "$BIN.stripped"
                x86_64-w64-mingw32-objcopy --add-gnu-debuglink="$BIN.debug" "$BIN.stripped"
                cp "$BIN.debug" "$SYMBOLS_DIR/${platform}_x64/"
                ;;
            lin)
                # objcopy/strip already ran in the docker container above.
                check_debug_size "$BIN.debug"
                cp "$BIN.debug" "$SYMBOLS_DIR/${platform}_x64/"
                ;;
        esac

        echo "\n\n"
        echo "\033[1;32m$platform build succeeded.\033[0m\nProduct: $BIN.stripped"
        file $BIN.stripped
        sleep 3
    else
        echo "\033[1;31m$platform build failed.\033[0m"
        exit 1
    fi
done

echo "Building has finished."

echo "Creating distribution bundle..."
if [ -d "build/dist" ]; then
    rm -rf build/dist
fi

for platform in $AVAILABLE_PLATFORMS; do
    mkdir -p build/dist/${platform}_x64
    SRC="build/$platform/${platform}_x64/${PROJECT_NAME}.xpl"
    # Prefer the stripped copy; fall back to the raw binary for platforms built before this change.
    if [ -f "$SRC.stripped" ]; then
        cp "$SRC.stripped" build/dist/${platform}_x64/${PROJECT_NAME}.xpl
    elif [ -f "$SRC" ]; then
        cp "$SRC" build/dist/${platform}_x64/${PROJECT_NAME}.xpl
    fi
done

for dir in $BUNDLE_DIRS; do
    if [ -d "$dir" ]; then
        cp -r "$dir" build/dist
    else
        echo "\033[1;31mERROR: BUNDLE_DIRS names '$dir' but that folder does not exist.\033[0m"
        exit 1
    fi
done

# Only add Skunkcrafts for XP12. The module URL is where the updater endpoint lives, and that
# endpoint derives it from its own folder name, so the two must stay in step.
if [ $SDK_VERSION -ge 400 ]; then
    echo "module|$MODULE_URL\nname|$MODULE_NAME\nversion|$VERSION\nlocked|false\ndisabled|false\nzone|custom" > build/dist/skunkcrafts_updater.cfg
fi

cd build
mv dist $PROJECT_NAME

VERSION=$RELEASE_TAG

rm -f $PROJECT_NAME-$VERSION.zip
zip -rq $PROJECT_NAME-$VERSION.zip $PROJECT_NAME -x "*/.DS_Store" -x "*/__MACOSX/*"

mv $PROJECT_NAME dist
cd ..

echo "Bundle created. Distribution: build/$PROJECT_NAME-$VERSION.zip"
echo "Debug symbols archived: $SYMBOLS_DIR (keep this if you need to symbolicate crashes from this release)"

# Upload to Google Drive if requested and gdrive is available
if [ "$UPLOAD_TO_DRIVE" = "y" ] && command -v gdrive > /dev/null 2>&1; then
    echo "Uploading to Google Drive..."
    FOLDER="$GDRIVE_FOLDER"

    # Delete old file with same name if it exists
    OLD_FILE_ID=$(gdrive files list --parent $FOLDER | grep "$PROJECT_NAME-$VERSION.zip" | awk '{print $1}')
    if [ ! -z "$OLD_FILE_ID" ]; then
        echo "Removing old version..."
        gdrive files delete $OLD_FILE_ID
    fi

    FILE_ID=$(gdrive files upload --parent $FOLDER --print-only-id "build/$PROJECT_NAME-$VERSION.zip")
    if [ ! -z "$FILE_ID" ]; then
        echo "\033[1;36mFile was uploaded to Google Drive:\033[0m"
        echo "\033[1;36mhttps://drive.google.com/file/d/$FILE_ID/view\033[0m"
        echo "\n\033[1;36mFolder link:\033[0m"
        echo "\033[1;36mhttps://drive.google.com/drive/folders/$FOLDER\033[0m"
    fi
fi

# Push the zip straight to the updater server. Needed for private repositories, where the server
# cannot pull the release itself; the module's index.php must have $allow_manual_upload = true.
if [ "$UPLOAD_TO_SERVER" = "y" ]; then
    echo "Publishing to https://ramonster.nl/$UPLOAD_FOLDER ..."
    UPLOAD_RESULT=$(curl -sS --fail-with-body -X POST \
        -H "X-Upload-Token: $(cat "$UPLOAD_TOKEN_FILE")" \
        -F "version=$BASE_VERSION" \
        -F "zip=@build/$PROJECT_NAME-$VERSION.zip" \
        "https://ramonster.nl/$UPLOAD_FOLDER/upload" 2>&1)

    if [ $? -eq 0 ]; then
        echo "\033[1;32mServer updated: $UPLOAD_RESULT\033[0m"
        echo "Verify with: curl https://ramonster.nl/$UPLOAD_FOLDER/current-version"
    else
        echo "\033[1;31mUpload failed: $UPLOAD_RESULT\033[0m"
        exit 1
    fi
fi

if [ -n "$POST_BUILD" ]; then
    $POST_BUILD
fi
