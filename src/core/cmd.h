#pragma once



namespace Hawk::cmd{
	struct Style{
		inline static const char* log     = "\x1b[90m";
		inline static const char* success = "\x1b[32m";
		inline static const char* info    = "\x1b[36m";
		inline static const char* warning = "\x1b[33m";
		inline static const char* error   = "\x1b[31m";
		inline static const char* fatal   = "\x1b[41m";
		// inline static const char* init    = "\x1b[34m";
	};


	void out(const char* str);


	template<typename... Args>
	void print(const char* str, Args&&... args){
		out((Hawk::fmt::format(str, args...)+"\x1b[0m\n").c_str());
	};


	template<typename Arg>
	void print(Arg&& arg){
		print("{}\x1b[0m", arg);
	}
	template<typename Arg>
	void print(const Arg& arg){
		print("{}\x1b[0m", arg);
	}


	template<>
	void print<std::string>(std::string&& arg){
		out((arg + "\x1b[0m\n").c_str());
	}
	template<>
	void print<std::string>(const std::string& arg){
		out((arg + "\x1b[0m\n").c_str());
	}


	void use_no_color();
	bool using_color();



	template<typename... Args>
	void log(const char* str, Args&&... args){
		if(Hawk::cmd::using_color()){
			print("{}{}", Hawk::cmd::Style::log, Hawk::fmt::format(str, args...));
		}else{
			print(str, args...);
		}
	};

	template<typename... Args>
	void success(const char* str, Args&&... args){
		if(Hawk::cmd::using_color()){
			print("{}{}", Hawk::cmd::Style::success, Hawk::fmt::format(str, args...));
		}else{
			print(str, args...);
		}
	};

	template<typename... Args>
	void info(const char* str, Args&&... args){
		if(Hawk::cmd::using_color()){
			print("{}{}", Hawk::cmd::Style::info, Hawk::fmt::format(str, args...));
		}else{
			print(str, args...);
		}
	};

	template<typename... Args>
	void warning(const char* str, Args&&... args){
		if(Hawk::cmd::using_color()){
			print("{}{}", Hawk::cmd::Style::warning, Hawk::fmt::format(str, args...));
		}else{
			print(str, args...);
		}
	};

	template<typename... Args>
	void error(const char* str, Args&&... args){
		if(Hawk::cmd::using_color()){
			print("{}{}", Hawk::cmd::Style::error, Hawk::fmt::format(str, args...));
		}else{
			print(str, args...);
		}
	};

	template<typename... Args>
	void fatal(const char* str, Args&&... args){
		if(Hawk::cmd::using_color()){
			print("{}{}", Hawk::cmd::Style::fatal, Hawk::fmt::format(str, args...));
		}else{
			print(str, args...);
		}
	};



}



template <>
struct std::formatter<fs::path> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const fs::path& path, std::format_context& ctx) {
        return std::format_to(ctx.out(), "{}", path.string());
    }
};
