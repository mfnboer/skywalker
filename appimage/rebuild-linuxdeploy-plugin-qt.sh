#!/bin/sh
set -eu

APPIMAGE_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
LINUXDEPLOY_IMAGE="$APPIMAGE_DIR/linuxdeploy-x86_64.AppImage"
QT_PLUGIN_IMAGE="$APPIMAGE_DIR/linuxdeploy-plugin-qt-x86_64.AppImage"

for image in "$LINUXDEPLOY_IMAGE" "$QT_PLUGIN_IMAGE"; do
  if [ ! -x "$image" ]; then
    echo "Required executable not found: $image" >&2
    exit 1
  fi
done

WORK_DIR=$(mktemp -d "${TMPDIR:-/tmp}/skywalker-qt-plugin.XXXXXX")
trap 'rm -rf -- "$WORK_DIR"' EXIT

mkdir -p "$WORK_DIR/linuxdeploy" "$WORK_DIR/qt-plugin"
(
  cd "$WORK_DIR/linuxdeploy"
  "$LINUXDEPLOY_IMAGE" --appimage-extract >/dev/null
)
(
  cd "$WORK_DIR/qt-plugin"
  "$QT_PLUGIN_IMAGE" --appimage-extract >/dev/null
)

LINUXDEPLOY_ROOT="$WORK_DIR/linuxdeploy/squashfs-root"
QT_PLUGIN_ROOT="$WORK_DIR/qt-plugin/squashfs-root"
PATCHER="$LINUXDEPLOY_ROOT/usr/bin/patchelf"

if [ "$("$PATCHER" --version)" != "patchelf 0.19.1" ]; then
  echo "Expected linuxdeploy's patchelf 0.19.1" >&2
  exit 1
fi

cp "$PATCHER" "$QT_PLUGIN_ROOT/usr/bin/patchelf"
OUTPUT="$WORK_DIR/linuxdeploy-plugin-qt-x86_64.AppImage"
LDAI_OUTPUT="$OUTPUT" NO_STRIP=1 \
  "$LINUXDEPLOY_ROOT/usr/bin/linuxdeploy" \
    --appdir "$QT_PLUGIN_ROOT" \
    --output appimage

if [ ! -x "$OUTPUT" ]; then
  echo "Replacement Qt plugin was not created: $OUTPUT" >&2
  exit 1
fi

mv -f "$OUTPUT" "$QT_PLUGIN_IMAGE"
echo "Updated $QT_PLUGIN_IMAGE with patchelf 0.19.1"
