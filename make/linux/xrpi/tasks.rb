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


include FileUtils

class MakeLinux
  extend Make

  class << self
    def setup_system_resources
      cp APP['icons']['gtk'], "#{TGT_DIR}/static/app-icon.png"
    end

    def new_so (name) 
      $stderr.puts "new_so: #{name}"
      tgtd = File.dirname(name)
      objs = []
      SubDirs.each do |f|
        d = File.dirname(f)
        objs = objs + FileList["#{d}/*.o"]      
      end
      objs = objs + FileList["shoes/native/gtk/*.o"]
      main_o = 'shoes/main.o'
      objs = objs - [main_o]
      sh "#{CC} -o  #{tgtd}/libshoes.#{DLEXT} #{objs.join(' ')} #{LINUX_LDFLAGS} #{LINUX_LIBS}" 
    end
    
    def new_link(name)
      #name is actually a file path
      $stderr.puts "new_link: arg=#{name}"
      dpath = File.dirname(name)
      fname = File.basename(name)
      bin = "#{fname}-bin"
      sh "#{CC} -o #{dpath}/#{bin} #{dpath}/tmp/main.o  -L#{dpath} -lshoes -L#{TGT_DIR}  #{LINUX_LIBS} --sysroot=#{ShoesDeps}"
      rewrite "platform/nix/shoes.launch", name, %r!/shoes-bin!, "/#{NAME}-bin"
      sh %{echo 'cd "$OLDPWD"\nLD_LIBRARY_PATH=$APPPATH $APPPATH/#{File.basename(bin)} "$@"' >> #{name}}
      chmod 0755, "#{name}" 
      # write a gdb launched shoes
      rewrite "platform/nix/shoes.launch", "#{TGT_DIR}/debug", %r!/shoes-bin!, "/#{NAME}-bin"
      sh %{echo 'cd "$OLDPWD"\nLD_LIBRARY_PATH=$APPPATH gdb $APPPATH/#{File.basename(bin)} "$@"' >> #{TGT_DIR}/debug}
      chmod 0755, "#{TGT_DIR}/debug" 
    end
    
    def make_installer
      if APP['INSTALLER'] == 'appimage'
        make_installer_appimage
      else
        make_installer_makeself
      end
    end
    
    def make_installer_appimage
      puts "Creating AppImage Appdir"
      pkg = ""
      if APP['Bld_Pre']
        pkg = "#{APP['Bld_Pre']}pkg/#{TGT_ARCH}/AppDir"
        mkdir_p pkg
      else
        pkg = "pkg/#{TGT_ARCH}/AppDir"
      end
      rm_rf pkg
      mkdir_p pkg               
      # 'AppRun' link to launch shoes script
      Dir.chdir(pkg) do
        ln_s "usr/lib/#{TGT_ARCH}/shoes", "AppRun"
     end
      #cp "static/app-icon.png", pkg
      mkdir_p "#{pkg}/usr/bin"
      mkdir_p "#{pkg}/usr/lib"
      mkdir_p "#{pkg}/usr/share/applications"
      mkdir_p "#{pkg}/usr/share/icons/hicolor/256x256"
      sh "cp -r #{TGT_DIR} #{pkg}/usr/lib"
      make_desktop_appimg pkg
      cp "#{pkg}/Shoes.desktop", "#{pkg}/usr/share/applications/"
      cp "#{TGT_DIR}/static/app-icon.png", "#{pkg}/Shoes.png"
      cp "#{TGT_DIR}/static/app-icon.png", "#{pkg}/usr/share/icons/hicolor/256x256/Shoes.png"
      dest = "pkg/Shoes-#{APP['VERSION']}-armhf.appimage"
      rm_rf dest
      sh "appimagetool #{pkg} #{dest} --runtime-file bin/runtime.armhf"
      rm_rf pkg
    end

   def make_desktop_appimg(dir)
      File.open("#{dir}/Shoes.desktop",'w') do |f|
        f << "[Desktop Entry]\n"
        f << "Name=Shoes #{APP['NAME'].capitalize}\n"
        f << "Exec=shoes %f\n"
        f << "StartupNotify=true\n"
        f << "Terminal=false\n"
        f << "Type=Application\n"
        f << "Comment=Ruby Graphical Programming\n"
        f << "Icon=Shoes\n"
        f << "Categories=Development;\n"
        f << "X-AppImage-Integrate=false\n"
      end
    end
    

    # make a .install with all the bits and peices. 
    def make_installer_makeself
      gtkv = '3'
      arch = 'armhf'
      appname =  "#{APP['name'].downcase}"
      rlname = "#{appname}-#{APP['VERSION']}-gtk#{gtkv}-#{arch}"
      #puts "Creating Pkg for #{rlname}"
      pkg = ""
      if APP['Bld_Pre']
        pkg = "#{APP['Bld_Pre']}pkg"
        mkdir_p pkg
      else
        pkg = 'pkg'
      end
      rm_r "#{pkg}/#{rlname}" if File.exists? "#{pkg}/#{rlname}"
      cp_r "VERSION.txt", "#{TGT_DIR}"
      mkdir_p "#{pkg}/#{rlname}"
      sh "cp -r #{TGT_DIR}/* #{pkg}/#{rlname}"
      Dir.chdir "#{pkg}/#{rlname}" do
        rm_r "#{APP['Bld_Tmp']}"
        rm_r "pkg" if File.exist? "pkg"
        make_desktop_makeself
        make_uninstall_script
        make_install_script
        make_smaller unless APP['GDB']
      end
      Dir.chdir "#{pkg}" do
        puts `pwd`
        sh "makeself #{rlname} #{rlname}.install #{appname} ./shoes-install.sh "
      end
      if APP['Bld_Pre']
        # copy installer to the shoes3 source pkg/ dir (on an nfs server?)
        cp "#{pkg}/#{rlname}.install", "pkg"
      end
    end
    
    def make_desktop_makeself
      File.open("Shoes.desktop.tmpl",'w') do |f|
        f << "[Desktop Entry]\n"
        f << "Name=Shoes #{APP['NAME'].capitalize}\n"
        f << "Exec={hdir}/.shoes/#{APP['NAME']}/shoes\n"
        f << "StartupNotify=true\n"
        f << "Terminal=false\n"
        f << "Type=Application\n"
        f << "Comment=Ruby Graphical Programming\n"
        f << "Icon={hdir}/.shoes/#{APP['NAME']}/static/app-icon.png\n"
        f << "Categories=Application;Development;Education;\n"
      end
      File.open("Shoes.remove.tmpl",'w') do |f|
        f << "[Desktop Entry]\n"
        f << "Name=Uninstall Shoes #{APP['NAME'].capitalize}\n"
        f << "Exec={hdir}/.shoes/#{APP['NAME']}/shoes-uninstall.sh\n"
        f << "StartupNotify=true\n"
        f << "Terminal=false\n"
        f << "Type=Application\n"
        f << "Comment=Delete Shoes\n"
        f << "Icon={hdir}/.shoes/#{APP['NAME']}/static/app-icon.png\n"
        f << "Categories=Application;Development;Education;\n"
      end
    end
    
    def make_uninstall_script
      File.open("shoes-uninstall.sh", 'w') do |f|
        f << "#!/bin/bash\n"
        f << "#pwd\n"
        f << "cd $HOME/.shoes/#{APP['NAME']}\n"
        f << "xdg-desktop-menu uninstall Shoes.remove.desktop\n"
        f << "xdg-desktop-menu uninstall Shoes.desktop\n"
        f << "cd ../\n"
        f << "rm -rf #{APP['NAME']}\n"
      end
      chmod "+x", "shoes-uninstall.sh"
    end
    
    # the install script that runs on the user's system can be simple. 
    # Copy things from where it's run to ~/.shoes/federales/ and then
    # sed the desktop file and copy it with xdg-desktop-menu
    def make_install_script
      File.open("shoes-install.sh", 'w') do |f|
        f << "#!/bin/bash\n"
        f << "#pwd\n"
        f << "ddir=$HOME/.shoes/#{APP['NAME']}\n"
        f << "#echo $ddir\n"
        f << "mkdir -p $ddir\n"
        f << "cp -r * $ddir/\n"
        f << "sed -e \"s@{hdir}@$HOME@\" <Shoes.desktop.tmpl >Shoes.desktop\n"
        f << "cp Shoes.desktop $ddir/Shoes.desktop\n"
        f << "xdg-desktop-menu install --novendor Shoes.desktop\n"
        f << "sed -e \"s@{hdir}@$HOME@\" <Shoes.remove.tmpl >Shoes.remove.desktop\n"
        f << "cp Shoes.remove.desktop $ddir/Shoes.remove.desktop\n"
        f << "xdg-desktop-menu install --novendor Shoes.remove.desktop\n"
        f << "echo \"Shoes has been copied to $ddir. and menus created\"\n"
        f << "echo \"If you don't see Shoes in the menu, logout and login\"\n"
      end
      chmod "+x", "shoes-install.sh"
    end
    
    # run strip on the libraries, remove unneeded ruby code (tk,
    #  readline and more)
    def make_smaller
      puts "Shrinking #{`pwd`}"
      sh "#{STRIP} *.so"
      sh "#{STRIP} *.so.*"
      Dir.glob("lib/ruby/**/*.so").each {|lib| sh "#{STRIP} #{lib}"}
    end
  end
end
