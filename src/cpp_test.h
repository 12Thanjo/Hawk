#pragma once


extern "C" {
	std::string* cpp_string(const char* str);
	const char* cpp_string_to_c_str(const std::string* str);
}

