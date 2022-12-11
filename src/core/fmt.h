#pragma once




namespace Hawk::fmt{
	
	template<typename... Args>
	std::string format(const char* str, Args&&... args){
		return std::vformat(str, std::make_format_args(args...));
	}



}

