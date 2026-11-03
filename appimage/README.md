# Linux AppImage build

This directory contains Linux AppImage build configuration and generated
artifacts. Build and packaging outputs here are ignored by the local
`.gitignore`.

Install the project's build dependencies, including a C++ compiler, CMake, and
the Qt 6 development modules used by the project. Linux builds also require
the Qt 6 Keychain development package. Configure and build with limited
parallelism if memory is constrained:

```sh
cmake -S . -B appimage/build \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build appimage/build \
  --target appskywalker --parallel
```

Download the x86_64 `linuxdeploy` and `linuxdeploy-plugin-qt` AppImages into
this directory and make them executable. Stage the application and its project
libraries in the AppDir:

```sh
APPIMAGE_DIR="$PWD/appimage"
APPDIR="$APPIMAGE_DIR/AppDir"
BUILD_DIR="$APPIMAGE_DIR/build"

mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/lib"
cp "$BUILD_DIR/appskywalker" "$APPDIR/usr/bin/"
cp "$BUILD_DIR/skywalker/liblibskywalker.so" "$APPDIR/usr/lib/"
cp "$BUILD_DIR/atproto/lib/liblibatproto.so" "$APPDIR/usr/lib/"
```

Deploy Qt, QML, and system-library dependencies and create the image:

```sh
cd "$APPIMAGE_DIR"
NO_STRIP=1 QMAKE=/path/to/Qt/6/bin/qmake \
  QML_SOURCES_PATHS="$PWD/../skywalker/qml" \
  QML_MODULES_PATHS="$PWD/build" \
  ./linuxdeploy-x86_64.AppImage \
    --appdir AppDir \
    --executable AppDir/usr/bin/appskywalker \
    --custom-apprun AppRun \
    --desktop-file skywalker.desktop \
    --icon-file ../images/skywalker.png \
    --plugin qt \
    --output appimage
```

Set `QMAKE` to the Qt 6 `qmake` executable installed on your system; this is
important if multiple Qt versions are installed. `NO_STRIP=1` disables binary
stripping, which can avoid incompatibilities with some toolchains.
`AppRun` sets the bundled Qt plugin and QML import paths and defaults to XCB.
