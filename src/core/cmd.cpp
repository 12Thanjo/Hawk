#include "pch.h"
#include "cmd.h"


namespace Hawk::cmd{

	void out(const char* str){
		printf("%s%s", str, "\x1b[0m");
	};






	static bool should_use_color = true;

	void use_no_color(){
		should_use_color = false;
	};

	bool using_color(){
		return should_use_color;
	};

	
}

