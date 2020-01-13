# Build a 32 bit Linux Tight Shoes on a pi2
cf =(ENV['ENV_CUSTOM'] || "#{TGT_ARCH}-custom.yaml")
ignore_deprecations = true
if File.exists? cf
  custmz = YAML.load_file(cf)
  ShoesDeps = custmz['Deps']
  EXT_RUBY = custmz['Ruby']
  APP['GDB'] = 'basic' if custmz['Debug'] == true
  APP['GEMLOC'] = custmz['Gemloc'] if custmz['Gemloc']
  APP['EXTLOC'] = custmz['Extloc'] if custmz['Extloc']
  APP['EXTLIST'] = custmz['Exts'] if custmz['Exts']
  APP['GEMLIST'] = custmz['Gems'] if custmz['Gems']
  APP['INCLGEMS'] = custmz['InclGems'] if custmz['InclGems']
  ignore_deprecations = (!custmz['Deprecations']) if custmz['Deprecations']
else
  abort "missing custom.yaml"
end

APP['GTK'] = 'gtk+-3.0' # installer needs this to name the output
#SHOES_TGT_ARCH = 'armv7l-linux-eabihf'
SHOES_TGT_ARCH = 'arm-linux-gnueabihf'
SHOES_GEM_ARCH = "#{Gem::Platform.local}"
# Setup some shortcuts for the library locations
arch = 'arm-linux-gnueabihf'
uldir = "#{ShoesDeps}/usr/lib"
ularch = "#{ShoesDeps}/usr/lib/#{arch}"
larch = "#{ShoesDeps}/lib/#{arch}"
# Set appropriately
CC = "gcc"
pkgruby ="#{EXT_RUBY}/lib/pkgconfig/ruby-2.5.pc"
pkggtk ="#{ularch}/pkgconfig/gtk+-3.0.pc" 
# Use Ruby or curl for downloads
RUBY_HTTP = true

ADD_DLL = []

# Target environment
#CAIRO_CFLAGS = `pkg-config --cflags cairo`.strip
CAIRO_LIB = `pkg-config --libs cairo`.strip
#PANGO_CFLAGS = `pkg-config --cflags pango`.strip
PANGO_LIB = `pkg-config --libs pango`.strip

png_lib = 'png'

if APP['GDB']
  LINUX_CFLAGS = " -g -O0"
else
  LINUX_CFLAGS = " -O -Wall"
end
LINUX_CFLAGS << " -DRUBY_HTTP -DGNOTE" 
LINUX_CFLAGS << " -DSHOES_GTK -fPIC -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function"
LINUX_CFLAGS << " -I#{ShoesDeps}/usr/include "
LINUX_CFLAGS << `pkg-config --cflags "#{pkgruby}"`.strip+" "
LINUX_CFLAGS << `pkg-config --cflags "#{pkggtk}"`.strip+" "
LINUX_CFLAGS << " -I#{ShoesDeps}/usr/include/ " 
LINUX_CFLAGS << "-I/usr/include/librsvg-2.0/librsvg "
if ignore_deprecations
  LINUX_CFLAGS << " -Wno-deprecated-declarations"
end
MISC_LIB = " #{ularch}/librsvg-2.so"

justgif = File.exist? "#{ularch}/libgif.so.7"
if justgif
  LINUX_LIB_NAMES = %W[gif jpeg yaml]
else
  LINUX_LIB_NAMES = %W[ungif jpeg yaml]
end
DLEXT = "so"
LINUX_LDFLAGS = "-fPIC -shared -L#{ularch} "
LINUX_LDFLAGS << `pkg-config --libs "#{pkggtk}"`.strip+" "
# use the ruby link info
RUBY_LDFLAGS = "-rdynamic -Wl,-export-dynamic "
RUBY_LDFLAGS << "-L#{EXT_RUBY}/lib -lruby "
RUBY_LDFLAGS << "-L#{ularch} -lrt -ldl -lcrypt -lm "

LINUX_LIBS = LINUX_LIB_NAMES.map { |x| "-l#{x}" }.join(' ')

LINUX_LIBS << " #{CURL_LDFLAGS if !RUBY_HTTP} #{RUBY_LDFLAGS} #{CAIRO_LIB} #{PANGO_LIB} #{MISC_LIB}"

SOLOCS = {}
SOLOCS['ungif'] = "#{uldir}/libgif.so.4.1.6" if !justgif
SOLOCS['gif'] = "#{ularch}/libgif.so.7.0.0"  if justgif
SOLOCS['jpeg'] = "#{ularch}/libjpeg.so.62.2.0"
SOLOCS['libyaml'] = "#{ularch}/libyaml-0.so.2.0.5"
SOLOCS['pcre'] = "#{larch}/libpcre.so.3.13.3"  # needed? 
SOLOCS['crypto'] = "#{ularch}/libcrypto.so.1.0.2"
SOLOCS['ssl'] = "#{ularch}/libssl.so.1.0.2"
SOLOCS['sqlite'] = "#{ularch}/libsqlite3.so.0.8.6"
SOLOCS['ffi'] = "#{ularch}/libffi.so.6.0.4" 
#SOLOCS['rsvg2'] = "#{ularch}/librsvg-2.so.2.40.16"
SOLOCS['rsvg2'] = "#{ularch}/librsvg-2.so.2.44.10"
#SOLOCS['curl'] = "#{EXT_RUBY}/lib/libcurl.so.4.4.0"
#SOLOCS['curl'] = "#{ularch}/libcurl.so.4.4.0"
SOLOCS['curl'] = "#{ularch}/libcurl.so.4.5.0"

# sigh, curl and tyhpoeus - processed in setup.rb

SYMLNK = {}
SYMLNK['libcurl.so.4.4.0'] = ['libcurl.so', 'libcurl.so.4']
SYMLNK['libgif.so.7.0.0'] = ['libgif.so', 'libgif.so.7']
SYMLNK['libjpeg.so.62.2.0'] = ['libjpeg.so', 'libjpeg.so.62']
SYMLNK['libyaml-0.so.2.0.5'] = ['libyaml.so', 'libyaml-0.so.2']
SYMLNK['libcrypto.so.1.0.2'] = ['libcrypto.so', 'libcrypto.so.1']
SYMLNK['libssl.so.1.0.2'] = ['libssl.so']
SYMLNK['libsqlite3.so.0.8.6'] = ['libsqlite3.so', 'libsqlite3.so.0']
SYMLNK['libffi.so.6.0.4'] = ['libffi.so', 'libffi.so.6']
#SYMLNK['librsvg-2.so.2.40.16'] = ['librsvg-2.so', 'librsvg-2.so.2']
SYMLNK['librsvg-2.so.2.44.10'] = ['librsvg-2.so', 'librsvg-2.so.2']
