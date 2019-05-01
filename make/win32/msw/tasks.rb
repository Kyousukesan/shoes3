include FileUtils
module Make
  include FileUtils
  
  def cc(t)
    sh "#{CC} -I. -c -o#{t.name} #{LINUX_CFLAGS} #{t.source}"
  end

  # Subs in special variables
  def rewrite before, after, reg = /\#\{(\w+)\}/, reg2 = '\1'
    File.open(after, 'w') do |a|
      File.open(before) do |b|
        b.each do |line|
          a << line.gsub(reg) do
            if reg2.include? '\1'
              reg2.gsub(%r!\\1!, Object.const_get($1))
            else
              reg2
            end
          end
        end
      end
    end
  end
 
end

class MakeMinGW
  extend Make

  class << self

    def setup_system_resources
      cp APP['icons']['gtk'], "#{TGT_DIR}/static/app-icon.png"
    end
 
    def make_so(name)
      if OBJ.empty?
        puts "Called w/o need"
        return
      end
      puts "make_so dir=#{pwd} arg=#{name}"
      sh "#{CC} -o #{name} #{OBJ.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
    end
    
    def new_link(name)
=begin
      tgts = name.split('/')
      tgtd = tgts[0]
      bin = "#{tgtd}/shoes.exe"
      binc = "#{tgtd}/cshoes.exe"
      #puts "binc  = #{binc}"
      #rm_f name
      rm_f bin
      rm_f binc
      sh "#{WINDRES} -I. shoes/appwin32.rc shoes/appwin32.o"
      missing = "-lgtk-3 -lgdk-3 -lfontconfig-1 -lpangocairo-1.0" # TODO: This is a bug in env.rb ?
      sh "#{CC} -o #{bin} shoes/main.o shoes/appwin32.o -L#{TGT_DIR} -lshoes -mwindows  #{LINUX_LIBS} #{missing}"
      sh "#{STRIP} #{bin}" unless APP['GDB']
      sh "#{CC} -o #{binc} shoes/main.o shoes/appwin32.o -L#{TGT_DIR} -lshoes #{LINUX_LIBS}  #{missing}"
      sh "#{STRIP} #{binc}" unless APP['GDB']
=end
      tgts = name.split('/')
      tgtd = tgts[0]
      bin = "#{tgtd}/shoes.exe"
      binc = "#{tgtd}/cshoes.exe"
      #puts "binc  = #{binc}"
      #rm_f name
      rm_f bin
      rm_f binc
      tp = "#{TGT_DIR}/#{APP['Bld_Tmp']}"
      sh "#{WINDRES} -I. shoes/appwin32.rc #{tp}/appwin32.o"
      missing = "-lgtk-3 -lgdk-3 -lfontconfig-1 -lpangocairo-1.0" # TODO: This is a bug in env.rb ?
      sh "#{CC} -o #{bin} #{tp}/main.o #{tp}/appwin32.o -L#{TGT_DIR} -lshoes -mwindows  #{LINUX_LIBS} #{missing}"
      sh "#{STRIP} #{bin}" unless APP['GDB']
      sh "#{CC} -o #{binc} #{tp}/main.o #{tp}/appwin32.o -L#{TGT_DIR} -lshoes #{LINUX_LIBS}  #{missing}"
      sh "#{STRIP} #{binc}" unless APP['GDB']      
    end
    
    # this is called from the file task based new_build
    def new_so (name) 
      tgts = name.split('/')
      tgtd = tgts[0]
      $stderr.puts "new_so: #{tgtd}"
      objs = []
      SubDirs.each do |f|
        d = File.dirname(f)
        #$stderr.puts "collecting .o from #{d}"
        objs = objs + FileList["#{d}/*.o"]      
      end
      # TODO  fix: gtk - needs to dig deeper vs osx
      objs = objs + FileList["shoes/native/gtk/*.o"]
      main_o = 'shoes/main.o'
      objs = objs - [main_o]
      sh "#{CC} -o #{tgtd}/libshoes.#{DLEXT} #{objs.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}"
    end
   
    # does nothing
    def make_userinstall
    end
 
    def make_resource(t)
      puts "make resource"
    end

    
    # don't have a qtifw windows install creator on Linux
    def make_installer_gtifw exe_path
    end
   
    def make_installer_nsis exe_path
      # assumes you have NSIS installed on your box in the system PATH
      # def sh(*args); super; end
      $stderr.puts "make_installer #{`pwd`} moving tmp/"
      tp = "#{TGT_DIR}/#{APP['Bld_Tmp']}"
      mp = "#{TGT_DIR}-#{APP['Bld_Tmp']}"
      mv tp, mp
      mkdir_p "pkg"
      cp_r "VERSION.txt", "#{TGT_DIR}/VERSION.txt"
      rm_rf "#{TGT_DIR}/nsis"
      cp_r  "platform/msw", "#{TGT_DIR}/nsis"
      cp APP['icons']['win32'], "#{TGT_DIR}/nsis/setup.ico"
      rewrite "#{TGT_DIR}/nsis/base.nsi", "#{TGT_DIR}/nsis/#{WINFNAME}.nsi"
      Dir.chdir("#{TGT_DIR}/nsis") do
        sh "#{exe_path} -V1 #{WINFNAME}.nsi"
      end
      mv "#{TGT_DIR}/nsis/#{WINFNAME}.exe", "pkg/"
      Dir.chdir('pkg/') do
        Dir.glob("Shoes*.exe").each do |f|
          mv f, "#{f.downcase}"
        end
      end
      $stderr.puts "restore tmp/"
      mv mp, tp
    end
    
    #Allow diffrent installers
    def make_installer
      if APP['INSTALLER'] == 'qtifw'
        installer = "qtifw"
        installer = APP['INSTALLER_LOC'] if APP['INSTALLER_LOC']
        make_installer_gtifw installer
      elsif APP['INSTALLER'] == 'nsis'
        installer = "makensis"
        installer = APP['INSTALLER_LOC'] if APP['INSTALLER_LOC'] 
        make_installer_nsis installer
      else 
        puts "No Installer defined"
      end
    end
  end
end
