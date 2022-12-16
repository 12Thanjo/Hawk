

#include "pch.h"

// dll test


// gcc -c -o dll_test.o dll_test.cpp
// gcc -o dll_test.dll -s -shared dll_test.o





// const char* get_name(){
// 	return "Hi, I am dll";
// }


extern "C" {
	__declspec(dllexport) int add_one(int num){
		return num + 1;
	};
};





