APP_TITLE =  Fizeau
APP_VERSION = v2.8.1
ifeq ($(RELEASE),)
	APP_VERSION	:=	$(APP_VERSION)-$(shell git describe --always)
endif
export APP_TITLE
export APP_VERSION
APP_TITID := $(shell grep -oP '"tid"\s*:\s*"\K(\w+)' $(CURDIR)/sysmodule/toolbox.json)
OUT		:=	SdOut

# -----------------------------------------------
all:
	@rm -rf $(OUT)
	@mkdir -p $(OUT)

	@$(MAKE) -C application $(MAKECMDGOALS) --no-print-directory
	@$(MAKE) -C sysmodule $(MAKECMDGOALS) --no-print-directory
	@$(MAKE) -C overlay $(MAKECMDGOALS) --no-print-directory

	@wget $(shell curl -s https://api.github.com/repos/averne/Fizeau/releases/latest|grep 'browser_'|cut -d\" -f4|head -1) -O $(OUT)/Fizeau-latest.zip >/dev/null 2>&1
	@unzip $(OUT)/Fizeau-latest.zip -d $(OUT)/ >/dev/null 2>&1

	@rm -rf $(OUT)/config
	@rm -rf $(OUT)/switch
	@rm -rf $(OUT)/atmosphere/contents/$(APP_TITID)/

	@mkdir -p $(OUT)/config/$(APP_TITLE)
	@mkdir -p $(OUT)/switch/$(APP_TITLE)
	@mkdir -p $(OUT)/switch/.overlays/lang/$(APP_TITLE)
	@mkdir -p $(OUT)/atmosphere/contents/$(APP_TITID)/flags
	@mkdir -p $(OUT)/atmosphere/exefs_patches/nvnflinger_cmu

	@cp -f misc/default.ini $(OUT)/config/$(APP_TITLE)/config.ini
	@cp -f application/out/$(APP_TITLE).nro $(OUT)/switch/$(APP_TITLE)/$(APP_TITLE).nro
	@cp -f overlay/out/$(APP_TITLE).ovl $(OUT)/switch/.overlays/$(APP_TITLE).ovl
	@cp -f overlay/lang/* $(OUT)/switch/.overlays/lang/$(APP_TITLE)/
	@cp -f sysmodule/out/$(APP_TITLE).nsp $(OUT)/atmosphere/contents/$(APP_TITID)/exefs.nsp
	@cp -f sysmodule/toolbox.json $(OUT)/atmosphere/contents/$(APP_TITID)/toolbox.json
	@touch $(OUT)/atmosphere/contents/$(APP_TITID)/flags/boot2.flag
	@cp -f misc/patches/*.ips $(OUT)/atmosphere/exefs_patches/nvnflinger_cmu >/dev/null 2>&1 || :

	@7z a $(OUT)/$(APP_TITLE).zip ./$(OUT)/atmosphere ./$(OUT)/config ./$(OUT)/switch >/dev/null 2>&1

clean:
	@$(MAKE) -C application clean
	@$(MAKE) -C sysmodule clean
	@$(MAKE) -C overlay clean
	@rm -rf $(OUT)
