SET(CMAKE_SYSTEM_NAME Darwin)
SET(CMAKE_C_COMPILER clang)
SET(CMAKE_CXX_COMPILER clang++)
# The macOS equivalent of the Linux glibc floor: this is the oldest macOS the
# shipped .xpl will load on. Leave it set. Without it, clang defaults to the
# version of whatever Mac ran the build, so the floor silently follows the
# build machine (that is how boarderline and avitab-browser ended up shipping
# plugins that required macOS 26). arm64 slices clamp to 11.0 regardless, which
# is expected; the value below governs the x86_64 slice.
SET(CMAKE_OSX_DEPLOYMENT_TARGET "12.0")
