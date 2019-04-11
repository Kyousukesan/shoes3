# This file is part of MXE. See LICENSE.md for licensing information.

PKG             := ruby
$(PKG)_WEBSITE  := https://ruby-lang.org
$(PKG)_IGNORE   :=
$(PKG)_VERSION  := 2.4.5
$(PKG)_CHECKSUM := 6737741ae6ffa61174c8a3dcdd8ba92bc38827827ab1d7ea1ec78bc3cefc5198
$(PKG)_SUBDIR   := ruby-$($(PKG)_VERSION)
$(PKG)_FILE     := ruby-$($(PKG)_VERSION).tar.gz
$(PKG)_URL      := https://cache.ruby-lang.org/pub/ruby/2.4/$($(PKG)_FILE)
$(PKG)_URL_2    := https://cache.ruby-lang.org/pub/ruby/2.4/ruby-2.4.5.tar.gz
$(PKG)_DEPS     := cc yaml libffi openssl  gdbm 

$(PKG)_DEPS_$(BUILD) := autotools 

define $(PKG)_UPDATE
    echo 'TODO: write update script for $(PKG).' >&2;
    echo $($(PKG)_VERSION)
endef

define $(PKG)_BUILD
    # remove previous install - ruby has permissions that don't
    # allow overwriting. 
    rm -rfv '$(PREFIX)/$(TARGET)/include/ruby*'
    rm -rf '$(PREFIX)/$(TARGET)/lib/ruby' 
    rm -rfv '$(PREFIX)/$(TARGET)/lib/pkgconfig/ruby*.pc'
    rm -rf '$(PREFIX)/$(TARGET)/bin/rake.bat'
    rm -rf '$(PREFIX)/$(TARGET)/bin/rake'
    # build and install the library
    export LDFLAGS="-L $(PREFIX)/$(TARGET)/bin -lyaml"
    cd '$(BUILD_DIR)' && $(SOURCE_DIR)/configure \
        $(MXE_CONFIGURE_OPTS) \
        --enable-shared \
        --enable-load-relative \
        --disable-install-doc \
        --without-tk --without-tcllib --without-tcltk 
    $(MAKE) -C '$(BUILD_DIR)' -j '$(JOBS)'
    $(MAKE) -C '$(BUILD_DIR)' -j 1 install
endef

