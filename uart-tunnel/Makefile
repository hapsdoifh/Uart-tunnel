#
# Top level makefile for gwsman  application
#
#
# Copyright (C) 2010-2015 6Harmonics Inc.
#
# 
#
include $(TOPDIR)/rules.mk
PKG_NAME:=uart-tunnel
PKG_VERSION:=1.0
PKG_RELEASE:=1

#PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=6H-GWS radio uart-gps tunnel
  DEPENDS:=+libpthread +libgws_sk
  MAINTAINER:=6Harmonics
endef

define Package/$(PKG_NAME)/description
  6Harmonics GWS uart-gps tunnel
endef

define Build/Prepare
	$(CP) -r ./$(PKG_NAME)/* $(PKG_BUILD_DIR)/
#	$(CP) $(PKG_BUILD_DIR)/../libgws/libgws_nl.a $(PKG_BUILD_DIR)/libs/
#	$(CP) $(PKG_BUILD_DIR)/../libgws/libgws_sk.a $(PKG_BUILD_DIR)/libs/
#	rm -f libgws
#	ln -s ../libgws libgws
endef

define Build/Configure
endef
	
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/usr/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dist/Release/GNU_GWS5000_4_4_14-Linux/$(PKG_NAME) $(1)/usr/sbin/$(PKG_NAME)
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

