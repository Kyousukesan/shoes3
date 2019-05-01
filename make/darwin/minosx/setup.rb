# This is a big gulp of copying.
require 'fileutils'
module Make
  include FileUtils
 
  def static_setup (so_list)
    $stderr.puts "setup: dir=#{`pwd`} for #{TGT_DIR}"
=begin		
		#rm_rf "#{TGT_DIR} "
		mkdir_p "#{TGT_DIR}"
		# copy Ruby, dylib, includes - have them in place before
		# we build things 
		puts "Ruby at #{EXT_RUBY} : #{SHOES_GEM_ARCH}"
		rbvt = RUBY_V
		rbvm = RUBY_V[/^\d+\.\d+/]
		rm_rf "#{TGT_DIR}/lib"
		mkdir_p "#{TGT_DIR}/lib"
		# clean out leftovers from last build
		#rm_f "#{TGT_DIR}/libruby.dylib" if File.exist? "#{TGT_DIR}/libruby.dylib"
		#rm_f "#{TGT_DIR}/libruby.#{rbvm}.dylib" if File.exist? "#{TGT_DIR}/libruby.#{rbvm}.dylib"
		#rm_f "#{TGT_DIR}/libruby.#{rbvt}.dylib" if File.exist? "#{TGT_DIR}/libruby.#{rbvt}.dylib"
		#mkdir_p "#{TGT_DIR}/lib/ruby/#{rbvm}.0/#{RUBY_PLATFORM}"
		cp_r "#{EXT_RUBY}/lib/ruby", "#{TGT_DIR}/lib"
		# copy and link libruby.dylib
		cp "#{EXT_RUBY}/lib/libruby.#{rbvt}.dylib", "#{TGT_DIR}"
		# copy include files - it might help build gems
		mkdir_p "#{TGT_DIR}/lib/ruby/include/ruby-#{rbvt}"
		cp_r "#{EXT_RUBY}/include/ruby-#{rbvt}/", "#{TGT_DIR}/lib/ruby/include"
		
		if APP['LIBPATHS']
		  dep_find_and_copy( APP['LIBPATHS'], SOLOCS)
		else
		  SOLOCS.each do |k, v| 
		    cp v, TGT_DIR
		  end
		end
		# copy ssl engines
		if File.exist?("#{ShoesDeps}/lib/engines")
			cp_r "#{ShoesDeps}/lib/engines", "#{TGT_DIR}"
		end
=end		
    srcloc= `pwd`.strip
		ln_s "#{srcloc}/lib", TGT_DIR
		ln_s "#{srcloc}/samples", TGT_DIR
		ln_s "#{srcloc}/static",  TGT_DIR
		ln_s "#{srcloc}/fonts", TGT_DIR
		ln_s "#{srcloc}/themes", TGT_DIR
		cp    "README.md", TGT_DIR
		cp    "CHANGELOG", TGT_DIR
		cp    "COPYING", TGT_DIR
=begin    
    # copy some static stuff
    cp_r  "fonts", "#{TGT_DIR}/fonts"
    cp_r  "lib", "#{TGT_DIR}"
    cp_r  "samples", "#{TGT_DIR}/samples"
    cp_r  "static", "#{TGT_DIR}/static"
    cp    "README.md", "#{TGT_DIR}/README.txt"
    cp    "CHANGELOG", "#{TGT_DIR}/CHANGELOG.txt"
    cp    "COPYING", "#{TGT_DIR}/COPYING.txt"
=end
	end
end

