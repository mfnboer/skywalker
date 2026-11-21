# Build and package Skywalker as a Linux AppImage.
# Run from anywhere with: make -C appimage <target>

HERE := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
ROOT := $(abspath $(HERE)/..)
APPDIR := $(HERE)/AppDir

JOBS ?= 2
QMAKE ?= /usr/bin/qmake6
NO_STRIP ?= 1

DEBUG_BUILD_DIR := $(HERE)/build
RELEASE_BUILD_DIR := $(HERE)/build-release
DEBUG_IMAGE := Skywalker-x86_64-debug.AppImage
RELEASE_IMAGE := Skywalker-x86_64.AppImage

METAINFO := eu.thereforeiam.skywalker.metainfo.xml
LINUXDEPLOY := linuxdeploy-x86_64.AppImage
QT_PLUGIN := linuxdeploy-plugin-qt-x86_64.AppImage
LINUXDEPLOY_URL := https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/$(LINUXDEPLOY)
QT_PLUGIN_URL := https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/$(QT_PLUGIN)

# The linuxdeploy AppImages cannot be mounted without libfuse2
export APPIMAGE_EXTRACT_AND_RUN ?= 1

# $(1): build dir, $(2): build type
define build
	test -f $(1)/build.ninja || cmake -S $(ROOT) -B $(1) -G Ninja -DCMAKE_BUILD_TYPE=$(2)
	cmake --build $(1) --parallel $(JOBS)
endef

# $(1): build dir, $(2): output file name
define package
	rm -rf $(APPDIR)
	mkdir -p $(APPDIR)/usr/bin $(APPDIR)/usr/lib
	cp $(1)/appskywalker $(APPDIR)/usr/bin/
	cp $(1)/skywalker/liblibskywalker.so $(APPDIR)/usr/lib/
	cp $(1)/atproto/lib/liblibatproto.so $(APPDIR)/usr/lib/
	mkdir -p $(APPDIR)/usr/share/metainfo
	cp $(HERE)/$(METAINFO) $(APPDIR)/usr/share/metainfo/
	cd $(HERE) && NO_STRIP=$(NO_STRIP) QMAKE=$(QMAKE) \
		QML_SOURCES_PATHS=$(ROOT)/skywalker/qml LDAI_OUTPUT=$(2) \
		./$(LINUXDEPLOY) \
			--appdir AppDir \
			--executable AppDir/usr/bin/appskywalker \
			--custom-apprun AppRun \
			--desktop-file skywalker.desktop \
			--icon-file ../images/skywalker.png \
			--plugin qt \
			--output appimage
endef

.PHONY: help debug release build-debug build-release package-debug package-release tools clean

help:
	@echo "Targets:"
	@echo "  debug            build and package $(DEBUG_IMAGE)"
	@echo "  release          build and package $(RELEASE_IMAGE)"
	@echo "  build-debug      only build (build/)"
	@echo "  build-release    only build (build-release/)"
	@echo "  package-debug    only package the existing debug build"
	@echo "  package-release  only package the existing release build"
	@echo "  tools            download linuxdeploy and its Qt plugin"
	@echo "  clean            remove AppDir and built AppImages"
	@echo "Variables: JOBS=$(JOBS) QMAKE=$(QMAKE) NO_STRIP=$(NO_STRIP)"

debug: build-debug package-debug

release: build-release package-release

build-debug:
	$(call build,$(DEBUG_BUILD_DIR),Debug)

build-release:
	$(call build,$(RELEASE_BUILD_DIR),Release)

package-debug: $(HERE)/$(LINUXDEPLOY) $(HERE)/$(QT_PLUGIN)
	$(call package,$(DEBUG_BUILD_DIR),$(DEBUG_IMAGE))

package-release: $(HERE)/$(LINUXDEPLOY) $(HERE)/$(QT_PLUGIN)
	$(call package,$(RELEASE_BUILD_DIR),$(RELEASE_IMAGE))

tools: $(HERE)/$(LINUXDEPLOY) $(HERE)/$(QT_PLUGIN)

$(HERE)/$(LINUXDEPLOY):
	curl -fL -o $@ $(LINUXDEPLOY_URL)
	chmod +x $@

# The upstream plugin bundles a patchelf that corrupts some libraries, so it is
# rebuilt with the patchelf from linuxdeploy.
$(HERE)/$(QT_PLUGIN): | $(HERE)/$(LINUXDEPLOY)
	curl -fL -o $@ $(QT_PLUGIN_URL)
	chmod +x $@
	$(HERE)/rebuild-linuxdeploy-plugin-qt.sh

clean:
	rm -rf $(APPDIR) $(HERE)/$(DEBUG_IMAGE) $(HERE)/$(RELEASE_IMAGE)
