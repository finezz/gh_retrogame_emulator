# $Id: app.mk 9862 2009-05-23 18:55:54Z mthuurne $
#
# Create an application directory for Darwin.

APP_SUPPORT_DIR:=build/package-darwin
APP_DIR:=$(BINDIST_DIR)/openMSX.app
APP_EXE_DIR:=$(APP_DIR)/Contents/MacOS
APP_PLIST:=$(APP_DIR)/Contents/Info.plist
APP_RES:=$(APP_DIR)/Contents/Resources
APP_ICON:=$(APP_RES)/openmsx-logo.icns

# Override install locations.
INSTALL_BINARY_DIR:=$(APP_EXE_DIR)
INSTALL_SHARE_DIR:=$(APP_DIR)/share
INSTALL_DOC_DIR:=$(BINDIST_DIR)/Documentation

PACKAGE_FULL:=$(shell PYTHONPATH=build $(PYTHON) -c \
  "import version; print version.getVersionedPackageName()" \
  )
BINDIST_PACKAGE:=$(BUILD_PATH)/$(PACKAGE_FULL)-mac-$(OPENMSX_TARGET_CPU)-bin.dmg
BINDIST_README:=$(BINDIST_DIR)/README.html
BINDIST_LICENSE:=$(INSTALL_DOC_DIR)/GPL.txt

# TODO: What is needed for an app folder?
app: install $(APP_PLIST) $(APP_ICON)

bindist: app $(BINDIST_README) $(BINDIST_LICENSE)
	@echo "Creating disk image:"
	@hdiutil create -srcfolder $(BINDIST_DIR) \
		-volname openMSX \
		-imagekey zlib-level=9 \
		-ov $(BINDIST_PACKAGE)
	@hdiutil internet-enable -yes $(BINDIST_PACKAGE)

$(APP_PLIST): $(APP_DIR)/Contents/%: $(APP_SUPPORT_DIR)/% bindistclean
	@echo "  Writing meta-info..."
	@mkdir -p $(@D)
	@sed -e 's/%ICON%/$(notdir $(APP_ICON))/' \
		-e 's/%VERSION%/$(PACKAGE_DETAILED_VERSION)/' < $< > $@
	@echo "APPLoMSX" > $(@D)/PkgInfo

$(APP_ICON): $(APP_RES)/%: $(APP_SUPPORT_DIR)/% bindistclean
	@echo "  Copying resources..."
	@mkdir -p $(@D)
	@cp $< $@

$(BINDIST_README): $(APP_SUPPORT_DIR)/README.html
	@echo "  Copying README..."
	@mkdir -p $(@D)
	@cp $< $@

$(BINDIST_LICENSE): doc/GPL.txt app
	@echo "  Copying license..."
	@mkdir -p $(@D)
# Remove form feeds from the GPL document, so Safari will treat it as text.
	@awk '!/\f/ ; /\f/ { print "" }' $< > $@
