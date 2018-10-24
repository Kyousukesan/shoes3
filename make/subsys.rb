# this is make _like_. very hand crafted. Modify with care and wisdom.
# it creates the file tasks triggered at build time. 
Base_h = FileList["shoes/*.h"] - ["shoes/appwin32.h", "shoes/version.h"]
# touching one of those could/should rebuild everything. 
#
# Shoes shoes/base.lib (canvas, ruby, image....
tp = "#{TGT_DIR}/#{APP['Bld_Tmp']}"
mkdir_p tp, verbose: false
base_src = FileList["shoes/*.c"]
base_obj = []
base_src.each do |c|
  fnm = File.basename(c, ".*")
  o = "#{tp}/#{fnm}.o"
  base_obj  << o
  file o => [c] + Base_h do |t|
    sh "#{CC} -o #{o} -I. -c #{LINUX_CFLAGS} #{c}"
  end
end
file "#{tp}/zzbase.done" => base_obj do
  touch "#{tp}/zzbase.done"
end

# Shoes/widget ruby interface (aka types/)
mkdir_p "#{tp}/types", verbose: false
rbwidget_src = FileList["shoes/types/*.c"]
rbwidget_obj = []
rbwidget_hdr = []
rbwidget_src.each do |c|
  fnm = File.basename(c, ".*")
  o = "#{tp}/types/#{fnm}.o"
  rbwidget_obj << o
  h = c.gsub(/.c$/, '.h')
  rbwidget_hdr << h
  file o => [c] + [h] + Base_h do |t| 
   sh "#{CC} -o #{o} -I. -c #{LINUX_CFLAGS} #{c}"
  end
end
file "#{tp}/types/zzwidgets.done" => rbwidget_obj do
  touch "#{tp}/types/zzwidgets.done"
end

# Shoes Native
nat_src = []
nat_obj = []
mkdir_p "#{tp}/native", verbose: false
#if RUBY_PLATFORM =~ /darwin/
if TGT_ARCH =~ /yosemite|mavericks|minosx|darwin14/
  nat_src = FileList['shoes/native/cocoa/*.m']
  nat_src.each do |c|
    fnm = File.basename(c,".*")
    o = "#{tp}/native/#{fnm}.o"
    nat_obj << o
    h = c.gsub(/.m$/, '.h')
    #$stderr.puts "creating rule for #{o} => #{c}"
    file o => [c] +[h] + ["shoes/native/cocoa.h"] + Base_h do
      sh "#{CC} -o #{o} -I. -c #{LINUX_CFLAGS} #{c}"
    end
  end
  file "#{tp}/native/cocoa.o" => ["shoes/native/cocoa.m", "shoes/native/cocoa.h"] + Base_h do
    sh "#{CC} -o #{tp}/native/cocoa.o -I. -c #{LINUX_CFLAGS} shoes/native/cocoa.m"
  end
  file "#{tp}/native/zznative.done" => ["#{tp}/native/cocoa.o"] + nat_obj do
    touch "#{tp}/native/zznative.done"
  end
else
  nat_src = FileList['shoes/native/gtk/*.c']
  nat_src.each do |c|
    fnm = File.basename(c, ".*")
    o = "#{tp}/native/#{fnm}.o"
    nat_obj << o
    h = c.gsub(/.c$/, '.h')
    file o => [c] + [h] + ['shoes/native/native.h'] + Base_h do
      sh "#{CC} -o #{o} -I. -c #{LINUX_CFLAGS} #{c}"
    end
  end
  file "#{tp}/native/gtk.o" => ["shoes/native/gtk.c", "shoes/native/gtk.h", "shoes/native/native.h"] + Base_h do
    sh "#{CC} -o #{tp}/native/gtk.o -I. -c #{LINUX_CFLAGS} shoes/native/gtk.c"
  end
  file "#{tp}/native/zznative.done" => ["#{tp}/native/gtk.o"] + nat_obj do
    touch "#{tp}/native/zznative.done"
  end
end

# Shoes/http
dnl_src = []
dnl_obj = []
mkdir_p "#{tp}/http", verbose: false
#if RUBY_PLATFORM =~ /darwin/
if TGT_ARCH =~ /yosemite|mavericks|minosx|darwin14/
  dnl_src = ["shoes/http/nsurl.m"]
else
  dnl_src = ["shoes/http/rbload.c"]
end
dnl_src.each do |c|
  fnm = File.basename(c, ".*")
  o = "#{tp}/http/#{fnm}.o"
  dnl_obj << o
  file o => [c] + Base_h do |t|
    cc t
  end
end
file "#{tp}/http/zzdownload.done" => dnl_obj do
  touch "#{tp}/http/zzdownload.done"
end

# Plot
mkdir_p "#{tp}/plot", verbose: false
plot_src = FileList['shoes/plot/*.c']
plot_obj = []
plot_src.each do |c|
  fnm = File.basename(c, ".*")
  o = "#{tp}/plot/#{fnm}.o"
  plot_obj << o
  file o => [c] + ["shoes/plot/plot.h", "shoes/plot/plot_util.c", "shoes/plot/chart_series.c"] do
    sh "#{CC} -o #{o} -I. -c #{LINUX_CFLAGS} #{c}"
  end
end
file "#{tp}/plot/zzplot.done" => plot_obj do 
  touch "#{tp}/plot/zzplot.done"
end

# Layout 
mkdir_p "#{tp}/layout", verbose: false
lay_src = FileList['shoes/layout/*.c']
lay_hdr = FileList['shoes/layout/*.h']
lay_obj = []
lay_src.each do |c|
  fnm = File.basename(c, ".*")
  o = "#{tp}/layout/#{fnm}.o"
  lay_obj << o
  file o => [c] + lay_hdr do
    sh "#{CC} -o #{o} -I. -c #{LINUX_CFLAGS} #{c}"
  end
end
file "#{tp}/layout/zzlayout.done" => lay_obj do 
  touch "#{tp}/layout/zzlayout.done"
end

# Console 
mkdir_p "#{tp}/console", verbose: false
#if RUBY_PLATFORM =~ /darwin/
if TGT_ARCH =~ /yosemite|mavericks|minosx|darwin14/
  src = ["shoes/console/tesi.c", "shoes/console/colortab.c", "shoes/console/cocoa-term.m"]
  obj =[]
  src.each do |c|
    fnm = File.basename(c, ".*")
    o = "#{tp}/console/#{fnm}.o"
    obj << o
    file o => [c] + ["shoes/console/tesi.h"] do
      sh "#{CC} -o #{o} -I. -c #{LINUX_CFLAGS} #{c}"
    end
  end
  file "#{tp}/console/zzconsole.done" =>  obj do
    touch "#{tp}/console/zzconsole.done"
  end
else 
  src = ["shoes/console/tesi.c", "shoes/console/colortab.c", "shoes/console/gtk-terminal.c"]
  obj = []
  src.each do |c|
    fnm = File.basename(c, ".*")
    o = "#{tp}/console/#{fnm}.o"
    obj << o
    file o => [c] + ["shoes/console/tesi.h"] do
        sh "#{CC} -o #{o} -I. -c #{LINUX_CFLAGS} #{c}"
    end
  end
  file "#{tp}/console/zzconsole.done" =>  obj do
    touch "#{tp}/console/zzconsole.done"
  end
end

# To keep the main Rakefile almost legible, create some file tasks here for detecting 
# updates to static/manual-en.txt and lib/shoes.rb, lib/shoes/*.rb after shoes is 'setup'
# but BEFORE new_so/new_link is called - an OSX requirement 
# Also handle sample/*/*.rb changes 
mkdir_p "#{tp}/copyonly", verbose: false
file "#{tp}/copyonly/zzmanual.done" => ["#{tp}/zzsetup.done", "static/manual-en.txt"] do
  if CROSS && (! TGT_DIR[/minlin/]) && (! TGT_DIR[/minbsd/]) && (! TGT_DIR[/minosx/])
    cp "static/manual-en.txt", "#{TGT_DIR}/static/manual-en.txt"
  end
  touch "#{tp}/copyonly/zzmanual.done"
end

file "#{tp}/copyonly/zzshoesrb.done" => ["#{tp}/zzsetup.done", "lib/shoes.rb"] do
  if CROSS && (! TGT_DIR[/minlin/]) && (! TGT_DIR[/minbsd/]) && (! TGT_DIR[/minosx/])
    cp "lib/shoes.rb", "#{TGT_DIR}/lib/shoes.rb"
  end
  touch "#{tp}/copyonly/zzshoesrb.done" 
end

# update lib/shoes/*.rb if changed. And ssl certs file.
if CROSS && TGT_DIR != 'minlin' && TGT_DIR != 'minbsd' && (! TGT_DIR[/minosx/])
  shoesrblib = FileList["lib/shoes/*.rb"] 
  shoesrblib << "lib/shoes/cacert.pem"
  taskl = []
  shoesrblib.each do |fp| 
    taskl << "#{TGT_DIR}/#{fp}"
    file "#{TGT_DIR}/#{fp}" => [fp] do |t|
      cp "#{fp}", "#{TGT_DIR}/#{fp}"
    end
  end
  file "#{tp}/copyonly/zzshoesrblib.done" => taskl do
    touch "#{tp}/copyonly/zzshoesrblib.done", verbose: false
  end
else
  # nothing to do because minlin uses symlinks
  file "#{tp}/copyonly/zzshoesrblib.done" => ["#{tp}/zzsetup.done"] do
    touch "#{tp}/copyonly/zzshoesrblib.done", verbose: false
  end
end

# update lib/package/*.rb if changed
if CROSS && TGT_DIR != 'minlin' && TGT_DIR != 'minbsd' && (! TGT_DIR[/minosx/])
  packrblib = FileList["lib/package/*.rb"] 
  taskl = []
  packrblib.each do |fp| 
    taskl << "#{TGT_DIR}/#{fp}"
    file "#{TGT_DIR}/#{fp}" => [fp] do |t|
      cp "#{fp}", "#{TGT_DIR}/#{fp}"
    end
  end
  file "#{tp}/copyonly/zzpackrblib.done" => taskl do
    touch "#{tp}/copyonly/zzpackrblib.done", verbose: false
  end
else
  # nothing to do because minlin uses symlinks
  file "#{tp}/copyonly/zzpackrblib.done" => ["#{tp}/zzsetup.done"] do
    touch "#{tp}/copyonly/zzpackrblib.done", verbose: false
  end
end

# samples simple/ good/ expert/
['simple', 'good', 'expert'].each do |sub|
  if CROSS && TGT_DIR != 'minlin' && TGT_DIR != 'minbsd' && (! TGT_DIR[/minosx/])
    samples = FileList["samples/#{sub}/*.rb"]
    sampl = []
    samples.each do |fp|
      sampl << "#{TGT_DIR}/#{fp}"
      file "#{TGT_DIR}/#{fp}" => [fp] do
        cp "#{fp}", "#{TGT_DIR}/#{fp}"
      end
    end
    file "#{tp}/copyonly/zz#{sub}.done" => sampl do
      touch "#{tp}/copyonly/zz#{sub}.done", verbose: false
    end
  else
    file "#{tp}/copyonly/zz#{sub}.done" => ["#{tp}/zzsetup.done"] do
      touch "#{tp}/copyonly/zz#{sub}.done", verbose: false
    end
  end
end
