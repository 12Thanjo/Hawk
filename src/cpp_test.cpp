#include "pch.h"
#include "cpp_test.h"



std::string* cpp_string(const char* str){
	return new std::string{str};
}

const char* cpp_string_to_c_str(const std::string* str){
	return str->c_str();
};


