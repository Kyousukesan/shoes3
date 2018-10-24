require 'rubygems'
require 'rake'
#require 'rake/clean'
require 'fileutils'
require 'find'
require 'yaml'
$stderr.puts "rake ruby: #{RbConfig::CONFIG['prefix']}"
require 'rbconfig'
include FileUtils
build_os = case RUBY_PLATFORM 
  when /darwin/
    :osx
  when /mingw/
    :win32
  when /linux/
    :linux
  when /bsd/
    :bsd
end

APP = YAML.load_file(File.join(ENV['APP'] || ".", "app.yaml"))
# APP['version'] = APP['major'] # for historical reasons
# populate APP[] with uppercase names and string values
APP['VERSION'] = "#{APP['major']}.#{APP['minor']}.#{APP['tiny']}"
APP['MAJOR'] = APP['major'].to_s
APP['MINOR'] = APP['minor'].to_s
APP['TINY'] = APP['tiny'].to_s
APP['NAME'] = APP['release']
APP['DATE'] = Time.now.to_s
APP['PLATFORM'] = RbConfig::CONFIG['arch'] # not correct for cross compile
case APP['revision']
  when 'git'
    gitp = ENV['GIT'] || "git"
    APP['REVISION'] = (`#{gitp} rev-list HEAD`.split.length).to_s
  when 'file'
    File.open('VERSION.txt', 'r') do |f|
      ln = f.read
      rev = ln[/r\(\d+\)/]
      APP['REVISION'] = "#{rev[/\d+/].to_i + 1}"
    end
  else
    if APP['revision'].kind_of? Fixnum
      APP['REVISION'] = APP['revision'].to_s
    else
      APP['REVISION'] = '9' # make it up
    end
end


NAME = APP['shortname'] || APP['name'].downcase.gsub(/\W+/, '')
APPNAME = APP['name'] # OSX needs this
SONAME = 'shoes'
APPARGS = APP['run']

# TODO: Shadow these until replaced with APP[] in env/setup/tasks files
RUBY_SO = RbConfig::CONFIG['RUBY_SO_NAME']
APP['RUBY_SO'] = RbConfig::CONFIG['RUBY_SO_NAME']
RUBY_V = RbConfig::CONFIG['ruby_version']
APP['RUBY_V'] = RbConfig::CONFIG['ruby_version']
SHOES_RUBY_ARCH = RbConfig::CONFIG['arch']
APP['SHOES_RUBY_ARCH'] = RbConfig::CONFIG['arch']

# default exts, gems & locations to build and include - replace with custom.yaml
APP['GEMLOC'] = ""
APP['EXTLOC'] = ""
APP['EXTLIST'] = []
APP['GEMLIST'] = []
APP['Bld_Tmp'] = 'tmp'
APP['Bld_Pre'] = ENV['NFS_ALTP'] if ENV['NFS_ALTP']

if File.exists? "build_target"
  CROSS = true
  File.open('build_target','r') do |f|
    str = f.readline
    TGT_ARCH = str.split('=')[1].strip
    #if RUBY_PLATFORM  =~ /darwin/
    if TGT_ARCH =~ /yosemite|mavericks|minosx|darwin14/
      # osx is just different. It needs build performance optimizations 
	    # is the build output directory outside the shoes3 dir?    
	    if APP['Bld_Pre']
        TOP_DIR = APP['Bld_Pre'] + TGT_ARCH
	      TGT_DIR = TOP_DIR + "/#{APPNAME}.app/Contents/MacOS"
	    else
	      TOP_DIR = TGT_ARCH
        TGT_DIR = TGT_ARCH+"/#{APPNAME}.app/Contents/MacOS"
	    end
    else 
	    # is the build output directory outside the shoes3 dir?    
	    if APP['Bld_Pre']
	      TGT_DIR = APP['Bld_Pre']+TGT_ARCH
	    else
	      TGT_DIR = TGT_ARCH
	    end
      TOP_DIR = TGT_DIR
    end
    mkdir_p "#{TGT_DIR}"
    BLD_ARGS = {}
    # This allows short circuiting the need to load setup and env (and pkg-config overhead)
    # Exprimental - no visible performance gain. see make/linux/minlin/env.rb
    if File.exists? "#{TGT_DIR}/build.yaml"
      $stderr.puts "loading building args"
      thsh = YAML.load_file("#{TGT_DIR}/build.yaml")
      thsh.each {|k,v| BLD_ARGS[k] = v} 
      HAVE_BLD = true
    else 
      HAVE_BLD = false
    end
  end
else
  CROSS = false
  if ARGV.length == 0 # bare rake w/o a setup called earlier
    $stderr.puts "Please Select a #{build_os}:setup: target from the"
    $stderr.puts "  list shown by 'rake -T'"
  end
  TGT_DIR = 'unknown'
end

=begin
BIN = "*.{bundle,jar,o,so,obj,pdb,pch,res,lib,def,exp,exe,ilk}"
#CLEAN.include ["{bin,shoes}/#{BIN}", "req/**/#{BIN}", "#{TGT_DIR}", "*.app"]
#CLEAN.include ["req/**/#{BIN}", "#{TGT_DIR}", "*.app"]
if RUBY_PLATFORM !~ /darwin|mingw|linux/
  CLEAN.include ["#{TGT_DIR}/libshoes.dll", "#{TGT_DIR}/*shoes.exe", 
    "#{TGT_DIR}/libshoes.so","#{TGT_DIR}/shoes", "#{TGT_DIR}/shoes-bin",
    "#{TGT_DIR}/*.app", "#{TGT_DIR}/#{APP['Bld_tmp']}/**/*.o"]
  CLOBBER.include ["#{TGT_DIR}/.DS_Store", "#{TGT_DIR}", "build_target", "cshoes", "shoes/**/*.o"]
end
=end
# for Host building for target
case build_os
when :win32
  if CROSS
    require File.expand_path("make/win32/#{TGT_ARCH}/env")
    require File.expand_path("make/win32/#{TGT_ARCH}/tasks")
    require File.expand_path("make/win32/#{TGT_ARCH}/stubs")
    require File.expand_path("make/win32/#{TGT_ARCH}/setup")
    require File.expand_path("make/gems")
    require File.expand_path('make/subsys')
  else
    # just enough to do a rake w/o target 
    require File.expand_path('make/win32/none/env.rb')
    require File.expand_path('make/win32/none/tasks.rb')
  end
  builder = MakeMinGW
  NAMESPACE = :win32

when :osx
  if CROSS
    case TGT_ARCH
    when /yosemite/, /mxe_osx/
      # Building tight shoes on OSX for OSX
      require File.expand_path("make/darwin/#{TGT_ARCH}/env")
      require File.expand_path("make/darwin/#{TGT_ARCH}/tasks")
      #require File.expand_path("make/darwin/#{TGT_ARCH}/stubs")
      require File.expand_path("make/darwin/#{TGT_ARCH}/setup")
      require File.expand_path("make/gems")
      require File.expand_path("make/subsys")
    when /minosx/
      require File.expand_path('make/darwin/minosx/env')
      require File.expand_path('make/darwin/minosx/tasks')
      require File.expand_path('make/darwin/minosx/setup')
      require File.expand_path('make/gems')
      require File.expand_path('make/subsys')
    else
      require File.expand_path('make/darwin/none/env')
      require File.expand_path('make/darwin/none/tasks')
      $stderr.puts "Please pick a osx:setup: target - see rake -T"
    end
  else
    # just enough to do a rake w/o target 
    require File.expand_path('make/darwin/none/env')
    require File.expand_path('make/darwin/none/tasks')
  end
  builder = MakeDarwin
  NAMESPACE = :osx
  
when :bsd
  #$stderr.puts "running BSD"
  if CROSS
    case TGT_ARCH
    when /freebsd/
      require File.expand_path('make/bsd/freebsd/env')
      require File.expand_path('make/bsd/freebsd/tasks')
      require File.expand_path('make/bsd/freebsd/setup')
      require File.expand_path("make/gems")
      require File.expand_path('make/subsys')
    when /minbsd/
      require File.expand_path('make/bsd/minbsd/env')
      require File.expand_path('make/bsd/minbsd/tasks')
      require File.expand_path('make/bsd/minbsd/setup')
      require File.expand_path('make/subsys')
    else
      require File.expand_path('make/bsd/none/env')
      require File.expand_path('make/bsd/none/tasks')
    end
  else 
    require File.expand_path('make/bsd/none/env')
    require File.expand_path('make/bsd/none/tasks')
    $stderr.puts "Please pick a bsd:setup: target - see rake -T"
  end
  builder = MakeBSD
  NAMESPACE = :bsd
  
when :linux
  if CROSS
    case TGT_ARCH
    when /xlin64/
      require File.expand_path('make/linux/xlin64/env')
      require File.expand_path('make/linux/xlin64/tasks')
      require File.expand_path('make/linux/xlin64/setup')
      require File.expand_path("make/gems")
      require File.expand_path('make/subsys')
    when /lin64/
      require File.expand_path('make/linux/lin64/env')
      require File.expand_path('make/linux/lin64/tasks')
      require File.expand_path('make/linux/lin64/setup')
      require File.expand_path("make/gems")
      require File.expand_path('make/subsys')
=begin
    when /i686-linux/
      require File.expand_path('make/linux/i686-linux/env')
      require File.expand_path('make/linux/i686-linux/tasks')
      require File.expand_path('make/linux/i686-linux/setup')
      require File.expand_path("make/gems")
      require File.expand_path('make/subsys')
=end
    when /xrpi/
      require File.expand_path('make/linux/xrpi/env')
      require File.expand_path('make/linux/xrpi/tasks')
      require File.expand_path('make/linux/xrpi/setup')
      require File.expand_path("make/gems")
      require File.expand_path('make/subsys')
    when /rpi/
      require File.expand_path('make/linux/rpi/env')
      require File.expand_path('make/linux/rpi/tasks')
      require File.expand_path('make/linux/rpi/setup')
      require File.expand_path("make/gems")
      require File.expand_path('make/subsys')
=begin
    when /xarmv6hf/
      require File.expand_path('make/linux/xarm6hf/env')
      require File.expand_path('make/linux/xarm6hf/tasks')
      require File.expand_path('make/gems')
=end
    when /xwin7/
      require File.expand_path('make/linux/xwin7/env')
      require File.expand_path('make/linux/xwin7/tasks')
      require File.expand_path('make/linux/xwin7/stubs')
      require File.expand_path('make/linux/xwin7/setup')
      require File.expand_path('make/linux/xwin7/packdeps')
      require File.expand_path('make/gems')
      require File.expand_path('make/subsys')
   when /xmsys2/
      require File.expand_path('make/linux/xmsys2/env')
      require File.expand_path('make/linux/xmsys2/tasks')
      require File.expand_path('make/linux/xmsys2/stubs')
      require File.expand_path('make/linux/xmsys2/packdeps')
      require File.expand_path('make/linux/xmsys2/setup')
      require File.expand_path('make/gems')
      require File.expand_path('make/subsys')
   when /mxe/
      require File.expand_path('make/linux/mxe/env')
      require File.expand_path('make/linux/mxe/tasks')
      require File.expand_path('make/linux/mxe/stubs')
      require File.expand_path('make/linux/mxe/packdeps')
      require File.expand_path('make/linux/mxe/setup')
      require File.expand_path('make/gems')
      require File.expand_path('make/subsys')
   when /darwin14/
      require File.expand_path('make/linux/darwin14/env')
      # we can use the OSX builder for tasks
      require File.expand_path('make/darwin/yosemite/tasks')
      # require File.expand_path('make/linux/darwin14/stubs')
      require File.expand_path('make/linux/darwin14/setup')
      require File.expand_path('make/gems')
      require File.expand_path('make/subsys')
      builder = MakeDarwin
   when /minlin/ 
      # This is Loose Shoes setup now known as minlin
      if CROSS && HAVE_BLD  # shortcut
        puts "skipping #{TGT_ARCH} env.rb, setup.rb"
        require File.expand_path('make/linux/minlin/tasks')
        require File.expand_path('make/subsys')
        DLEXT = BLD_ARGS['DLEXT']
        CC = BLD_ARGS['CC']
        SOLOCS = BLD_ARGS['SOLOCS']
        LINUX_CFLAGS = BLD_ARGS['LINUX_CFLAGS']
        LINUX_LDFLAGS = BLD_ARGS['LINUX_LDFLAGS']
        LINUX_LIBS = BLD_ARGS['LINUX_LIBS']
      else 
        #puts "Require All minlin:"
        require File.expand_path('make/linux/minlin/env')
        require File.expand_path('make/linux/minlin/tasks')
        require File.expand_path('make/linux/minlin/setup')
        require File.expand_path('make/subsys')
     end
   else
      $stderr.puts "Unknown builder for #{TGT_ARCH}, removing setting"
      rm_rf "build_target" if File.exists? "build_target"
    end
  else
     # just enough to do a rake w/o target - will fail with a decent enough
     # error message
     require File.expand_path('make/linux/none/env')
     require File.expand_path('make/linux/none/tasks')
  end
  builder = MakeLinux unless builder
  NAMESPACE = :linux
else
  puts "Sorry, your platform [#{RUBY_PLATFORM}] is not supported..."
end

# --------------------------
# common platform tasks

desc "Same as `rake build'"
task :default => [:build]

desc "Package Shoes for distribution"
task :package => [:version, :installer]

task :build_os => ["#{TGT_DIR}/#{NAME}"]

task "shoes/version.h" do |t|
  File.open(t.name, 'w') do |f|
    f << "#ifndef SHOES_VERSION_H\n"
    f << "#define SHOES_VERSION_H\n\n"
    f << "// compatatibily pre 3.2.22\n"
    f << "#define SHOES_RELEASE_ID #{APP['MAJOR']}\n"
    f << "#define SHOES_REVISION #{APP['REVISION']}\n"
    f << "#define SHOES_RELEASE_NAME \"#{APP['NAME']}\"\n"
    f << "#define SHOES_BUILD_DATE \"#{APP['DATE']}\"\n"
    f << "#define SHOES_PLATFORM \"#{SHOES_RUBY_ARCH}\"\n"
    f << "// post 3.2.22\n"
    f << "#define SHOES_VERSION_NUMBER \"#{APP['VERSION']}\"\n"
    f << "#define SHOES_VERSION_MAJOR #{APP['MAJOR']}\n"
    f << "#define SHOES_VERSION_MINOR #{APP['MINOR']}\n"
    f << "#define SHOES_VERSION_TINY #{APP['TINY']}\n"
    f << "#define SHOES_VERSION_NAME \"#{APP['NAME']}\"\n"
    f << "#define SHOES_VERSION_REVISION #{APP['REVISION']}\n"
    f << "#define SHOES_VERSION_DATE \"#{APP['DATE']}\"\n"
    f << "#define SHOES_VERSION_PLATFORM \"#{APP['PLATFORM']}\"\n"
    if CROSS && (!TGT_DIR[/minlin/] &&  !TGT_DIR[/minbsd/] )
      f << "#define SHOES_STYLE \"TIGHT_SHOES\"\n\n"
    else
      f << "#define SHOES_STYLE \"LOOSE_SHOES\"\n\n"
    end
    f << "extern VALUE cTypes;\n"
    f << "\nvoid shoes_version_init();\n\n"
    f << "#endif\n"
  end
end

# TODO: Left for historical reasons (aka OSX)
task "#{TGT_DIR}/VERSION.txt" do |t|
  File.open(t.name, 'w') do |f|
    f << %{shoes #{RELEASE_NAME.downcase} (0.r#{REVISION}) [#{SHOES_RUBY_ARCH} Ruby#{RUBY_V}]}
    %w[DEBUG].each { |x| f << " +#{x.downcase}" if ENV[x] }
    f << "\n"
  end
end

#TODO: should the following be a task or file? 
file "shoes/types/types.h" do |t|
   puts "Processing #{t.name}..."
   
   rm_rf "shoes/types/types.h" if File.exists? "shoes/types/types.h"
   
   headers =  Dir["shoes/types/*.h"] - ["shoes/types/types.h"]
   content = headers.collect { |file|
      File.read(file).scan(/shoes_[[:alnum:]_]+_init\(\);/)
   }.flatten

   File.open(t.name, 'w') do |f|
      headers.sort.each { |header|
         f << "#include \"#{header}\"\n"
      }
      f << "\n#define SHOES_TYPES_INIT \\\n#{content.sort.collect { |n| "\t#{n}" }.join(" \\\n") }\n"
   end
end

def create_version_file file_path
  File.open(file_path, 'w') do |f|
    f << "shoes #{APP['NAME'].downcase} #{APP['VERSION']} r(#{APP['REVISION']}) #{APP['PLATFORM']} #{APP['DATE']}"
    f << "\n"
  end
end

# TODO: called from osx(s) copy_files_to_dist in task.rb
def osx_version_txt t
  create_version_file t
end

# desc "create VERSION.txt"
task :version do
 create_version_file 'VERSION.txt'
end


# --------------------------
# tasks depending on Builder = MakeLinux|MakeDarwin|MakeMinGW

desc "Build using your OS setup"
task :build => ["#{NAMESPACE}:build"]


# ------  new build   --------
rtp = "#{TGT_DIR}/#{APP['Bld_Tmp']}"
file  "#{rtp}/zzsetup.done" do
  builder.static_setup SOLOCS
  builder.copy_gems #used to be common_build, located in make/gems.rb
  builder.setup_system_resources
  touch "#{rtp}/zzsetup.done"
  $stderr.puts "Build Products in #{rtp}"
end

#These tasks create object files:
SubDirs = ["#{rtp}/zzbase.done",  "#{rtp}/http/zzdownload.done",
   "#{rtp}/plot/zzplot.done", "#{rtp}/console/zzconsole.done", 
   "#{rtp}/types/zzwidgets.done", "#{rtp}/layout/zzlayout.done",
   "#{rtp}/native/zznative.done"]
    
# Windows doesn't use console - don't try to build it. Delete from dependcies
case TGT_DIR
  when /win7/, /xwin7/, /msys2/, /xmsys2/, /mxe/, /mxe_osx/
    SubDirs.delete("#{rtp}/console/zzconsole.done")
end

# These tasks copy updated Shoes lib/*/*.rb files and samples and the manual
StaticDirs = ["#{rtp}/copyonly/zzmanual.done", "#{rtp}/copyonly/zzshoesrb.done",
    "#{rtp}/copyonly/zzshoesrblib.done", "#{rtp}/copyonly/zzsimple.done",
     "#{rtp}/copyonly/zzgood.done", "#{rtp}/copyonly/zzexpert.done",
      "#{rtp}/copyonly/zzpackrblib.done"]

file "#{TGT_DIR}/libshoes.#{DLEXT}" => ["#{rtp}/zzsetup.done", "shoes/types/types.h",
    "shoes/version.h"] + SubDirs + StaticDirs do
  # need to compile version.c -> .o (and create the verion.h for every build)
  sh "#{CC} -o #{rtp}/version.o -I. -c #{LINUX_CFLAGS} shoes/version.c"
  builder.new_so "#{TGT_DIR}/libshoes.#{DLEXT}"
end

# rake build (or just rake) will get here. 
task :new_build => "#{TGT_DIR}/libshoes.#{DLEXT}"  do
  # We can link shoes now - this will be done via a builder call
  # because it's platform specific.
  builder.new_link "#{TGT_DIR}/shoes"
end

=begin
desc "Not Recommended! Install min Shoes in your ~/.shoes Directory"
task  :install do
  if ! (TGT_DIR[/minlin/] || TGT_DIR[/minbsd/] || TGT_DIR[/minosx/])
     puts "Sorry. You can't do an install of your source built Tight Shoes"
     puts "  Install the 'rake package' distro just like a user does."
  else
    create_version_file "#{TGT_DIR}/VERSION.txt"
    #builder.copy_files_to_dist
    builder.make_userinstall
  end
end
=end

desc "remove objects and libshoes.dylib"
task :clean do
  rm_rf "#{TGT_DIR}/libshoes.#{DLEXT}"
  Dir.chdir("#{TGT_DIR}/tmp") do
    Dir.glob('*') do |f|
      rm_rf f unless f=='zzsetup.done'
    end
  end
end

desc "remove all build products"
task :clobber do
  rm_rf TOP_DIR
  rm_f "build_target"
end

directory "#{TGT_DIR}"	# was 'dist'

def cc(t)
  $stderr.puts "compiling #{t}"
  sh "#{CC} -I. -c -o #{t.name} #{LINUX_CFLAGS} #{t.source}"
end

rule ".o" => ".m" do |t|
  cc t
end

rule ".o" => ".c" do |t|
  cc t
end

task :installer => ["#{NAMESPACE}:installer"]

namespace :setup do
  if build_os == :osx
	desc "Setup for osx 10.10+ from OSX"
	task :osx do
	  puts "Selected 'yosemite'"
      sh "echo 'TGT_ARCH=yosemite' >build_target"
	end
	   
	desc "Setup for Win7+ from OSX using MXE"
	task :win32 do
	   puts "Selected 'mxe_osx'"
       sh "echo 'TGT_ARCH=mxe_osx' >build_target"
	 end
	 
	 desc "Setup for OSX - not distributable"
	 task :minosx do
       sh "echo 'TGT_ARCH=minosx' >build_target"
	 end
  end
  
  if build_os == :win32 
    desc "Build for Windows 7+ using DevKit"
    task :win7 do
      sh "echo TGT_ARCH=win7 >build_target"
	  end
	
	  desc "Setup for Window7+ using MSYS2 (recommended)"
	  task :msys2 do
      sh "echo TGT_ARCH=msys2 >build_target"
	  end
  end
  
  if build_os == :bsd
    desc "Minimal Shoes for linux (default)"
    task :minbsd do
      sh "echo 'TGT_ARCH=minbsd' >build_target"
    end
    
    desc "Setup for FreeBSD 11 from bsd"
    task :freebsd do
      sh "echo 'TGT_ARCH=freebsd' >build_target"
    end
  end
  
  if build_os == :linux 
    if RUBY_PLATFORM =~ /x86_64/
      desc "Minimal Shoes for linux (default)"
      task :minlin do
        sh "echo 'TGT_ARCH=minlin' >build_target"
      end
        
	    desc "Setup for Linux x86_64"
	    task :xlin64 do
	      sh "echo 'TGT_ARCH=xlin64' >build_target"
	    end
	    
	    desc "Setup for Linux X86_64 (chroot)"
	    task :lin64 do
	      sh "echo 'TGT_ARCH=lin64' >build_target"
		  end
		
	    desc "Setup for Linux armeabihf (Pi) using X86_64-linux" 
	    task :xrpi do
	      sh "echo 'TGT_ARCH=xrpi' >build_target"
	    end
	    
	    desc "Setup for Windows 7+ (MXE tools - recommended)"
	    task :mxe do
	      sh "echo 'TGT_ARCH=mxe' >build_target"
	    end 
	
	    desc "Setup for Win7+ using Linux (debian tools)"
	    task :xwin7 do
	      sh "echo 'TGT_ARCH=xwin7' >build_target"
	    end
	    
	    desc "Setup for OSX 10.10+ (osxcross tools)"
	    task :darwin14 do
	      sh "echo 'TGT_ARCH=darwin14' >build_target"
	    end
	  else   
	    desc "Setup for PI using PI - slooowww"
	    task :rpi do
	      sh "echo 'TGT_ARCH=rpi' >build_target"
	    end
	  end
    
    task :build => [:new_build]

    task :installer do
      builder.make_installer
    end

  end
  
end  #namespace :setup

# default for 'rake setup'
case build_os
  when :osx
    task setup: 'setup:minosx'
  when :linux
    task setup: 'setup:minlin'
  when :bsd
    task setup: 'setup:minbsd'
end

namespace :linux do
  task :build => [:new_build]

  task :installer do
    builder.make_installer
  end
end
namespace :osx do
  task :build => [:new_build]

  task :installer do
    builder.make_installer
  end
end
namespace :win32 do
  task :build => ["build_target", :new_build]

  task :installer do
    builder.make_installer
  end
end
namespace :bsd do
  task :build => ["build_target", :new_build]

  task :installer do
    builder.make_installer
  end  
end

# misc task stubs for Windows build on Linux
if build_os == :linux
    require File.expand_path('make/linux/mxe/stubs')
end


# TODO: not all targets call this from setup.rb. Requires env.rb mods.
# Function to find the lib(s), copy and symlink. locs is probably APP['LIBPATHS']
# and shlibs is the SOLOCS hash - only the hash key is used so it needs to be 
# correct
def dep_find_and_copy(locs, shlibs)
  # load all the file names in locs into some hashes
  loc = {}
  locs.each do |lib| 
    loc[lib] = {}
    Dir.glob("#{lib}/*").each do |fp|
      fn = File.basename(fp)
      short = fn[/(\w|\d|_|\-)+/]   # up to first period
      hsh = loc[lib]
      ary = hsh[short]
      ary ? ary << fn : ary=[fn]
      hsh[short] = ary  # needed? reference vs copy
    end
  end
  shlibs.each_key do |libname|
    hit = nil
    p = ""
    loc.each_pair do |k,hsh|
      if hsh[libname]
        hit = hsh[libname]
        p = k
        break
      end
    end
    if !hit
      puts "Failed to find #{libname} in hashes"
      abort
    else
      hits = []
      hit.each do |ent| 
        if ent.include? "#{libname}.#{DLEXT}"
          hits << ent
        end
      end
      #puts "Deal with #{hits.inspect} in #{p}"
      if RUBY_PLATFORM =~ /mingw/
        cp "#{p}/#{hits[0]}", TGT_DIR
      else
        syml = {}
        cph = ""
        hits.each do |fn|
          fp ="#{p}/#{fn}"
          if File.symlink? fp
            syml[fp] = File.readlink(fp)
          else
            cph = fp
          end
        end
        if cph.length > 0
          cp cph, TGT_DIR
          Dir.chdir(TGT_DIR) do
            cph = File.basename(cph)
            syml.each_pair { |k,v| File.symlink(cph, File.basename(k)) }
          end
        else
          # We're Special! A symlink between dirs (libpcre.so?) - chase it
          syml.each_pair do |k,v|
            pos = "#{ShoesDeps}#{v}"
            while File.symlink?(pos) do
               #puts "chase #{pos}"
               pos = "#{ShoesDeps}/#{File.readlink(pos)}"
            end
            dp = File.dirname(v)
            fn = File.basename(pos)
            #puts "chased #{k}, #{v} to here: #{pos}"
            cp "#{ShoesDeps}#{dp}/#{fn}", TGT_DIR
          end
        end
      end
    end
  end
end

def win_dep_find_and_copy(locs, shlibs)
  shlibs.each_pair do |lib, xxx|
    hit = nil
    locs.each do |dir| 
      pos = Dir.glob("#{dir}/#{lib}*.dll")
      if pos && pos.length == 1
        hit = pos[0]
        cp hit, TGT_DIR
        break;
      end
    end
    if hit 
      next
    else
      puts "Can't find #{lib}"
      abort
    end
  end
end


