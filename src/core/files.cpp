#include "pch.h"


namespace Hawk::files{

	std::string read(fs::path&& path){
		// PH_ASSERT(fs::exists(path), "Attempted to read file that doesn't exist\n\tfile: {}", path.string());

		constexpr auto read_size = std::size_t(4096);
	    auto stream = std::ifstream(path);
	    stream.exceptions(std::ios_base::badbit);
	    
	    auto out = std::string();
	    auto buf = std::string(read_size, '\0');
	    while (stream.read(& buf[0], read_size)) {
	        out.append(buf, 0, stream.gcount());
	    }
	    out.append(buf, 0, stream.gcount());
	    return out;
	}

	std::string read(const fs::path& path){
		// PH_ASSERT(fs::exists(path), "Attempted to read file that doesn't exist\n\tfile: {}", path.string());

		constexpr auto read_size = std::size_t(4096);
	    auto stream = std::ifstream(path);
	    stream.exceptions(std::ios_base::badbit);
	    
	    auto out = std::string();
	    auto buf = std::string(read_size, '\0');
	    while (stream.read(& buf[0], read_size)) {
	        out.append(buf, 0, stream.gcount());
	    }
	    out.append(buf, 0, stream.gcount());
	    return out;
	}
	
}
