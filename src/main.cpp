#include "pch.h"


#include "CharacterStream.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "SemanticAnalyzer.h"
#include "Compiler.h"



namespace Hawk{



	void print_version(){
		cmd::info("Hawk version: 0.12.1");
	};
	void print_help();


	int main(std::vector<std::string>&& argv){
		fs::path program_path = argv[0];

		fs::path path;
		// fs::path path = program_path.parent_path() / "../../../tests/test.hawk";

		bool print_feedback = false;
		bool print_tokens = false;
		bool print_ast = false;
		bool print_ir = false;

		enum class OutputMode{
			assembly,
			exe,
			interpret,
			llvm,
			object,
		};

		OutputMode output_mode = OutputMode::exe;



		if(argv.size() > 1){
			for(int i = 1; i < argv.size(); i++){
				auto arg = argv[i];


					  if(arg == "-c=asm"){  output_mode = OutputMode::assembly;
				}else if(arg == "-c=exe"){	output_mode = OutputMode::exe; //default
				}else if(arg == "-c=int"){	output_mode = OutputMode::interpret;
				}else if(arg == "-c=llvm"){	output_mode = OutputMode::llvm;
				}else if(arg == "-c=o"){	output_mode = OutputMode::object;

				}else if(arg == "-h"){		print_help(); return 0;
				}else if(arg == "-v"){		print_version(); return 0;

				}else if(arg == "-f"){		print_feedback = true;
				}else if(arg == "-nc"){	 	cmd::use_no_color();

				}else if(arg == "-ast"){	print_ast = true;
				}else if(arg == "-ir"){     print_ir = true;
				}else if(arg == "-tokens"){ print_tokens = true;

				}else if(i != 1){
					cmd::error("Unknown arg: {}", arg);
					return -1;
				}

			}

			path = argv[1];
		}else{
			cmd::error("No target file given");
			return -1;
		}

		if(!fs::exists(path)){
			cmd::error("file \"{}\" does not exist", path);
			return -1;	
		}


		std::string file = files::read(path);
		bool error_free = true;

		auto char_stream = CharacterStream(file);

		auto tokenizer = Tokenizer(char_stream);
		tokenizer.start();
		error_free = tokenizer.success();


		auto parser = Parser(tokenizer.tokens);
		if(error_free){

			if(print_tokens){
				cmd::info("\nTokens:");
				cmd::log("-------------------------------");
				for(auto& token : tokenizer.tokens){
					cmd::print("token: {}", token.value);
				}
				cmd::log("-------------------------------\n");
			}


			parser.start();
			error_free = parser.success();
		}



		auto semantic_analyzer = SemanticAnalyzer(parser.statements);
		if(error_free){

			if(print_ast){
				cmd::info("\nAST:");
				cmd::log("-------------------------------");
				for(auto* stmt : parser.statements){
					stmt->print(0);
				}
				cmd::log("-------------------------------\n");
			}

			semantic_analyzer.begin();
			error_free = semantic_analyzer.get_error_count() == 0;
			bool warning_free = semantic_analyzer.get_warning_count() == 0;

			if(!error_free){
				cmd::error("\n[Errors: {}, Warnings: {}]\n", semantic_analyzer.get_error_count(), semantic_analyzer.get_warning_count());
			}else if(!warning_free){
				cmd::warning("\n[Errors: 0, Warnings: {}]\n", semantic_analyzer.get_warning_count());
			}
		}


		if(error_free){
			auto compiler = Compiler(semantic_analyzer.global_vars, semantic_analyzer.functions, "hawk_module");
			compiler.build_ir();
			compiler.save_ir_to_file();


			if(print_ir){
				cmd::info("\nLLVM IR:");
				cmd::log("-------------------------------");
				compiler.print_ir();
				cmd::log("-------------------------------\n");
			}



			if(output_mode == OutputMode::assembly){
				compiler.compile_to_asm();

			}else if(output_mode == OutputMode::exe){
				compiler.compile_to_obj();
				compiler.compile_exe(program_path);

			}else if(output_mode == OutputMode::interpret){
				cmd::info("\nInterpreter:");
				cmd::log("-------------------------------");
				compiler.run_interpreter();
				cmd::log("-------------------------------\n");

			}else if(output_mode == OutputMode::llvm){
				// do nothing

			}else if(output_mode == OutputMode::object){
				compiler.compile_to_obj();
			}



		}


		if(print_feedback){
			if(error_free){
				cmd::success("Hawk Compiled Successfully");
			}else{
				cmd::error("Hawk Compilation Failed");
			}
		}

		return error_free ? 0 : -1;
	};




	void print_help(){
		cmd::print("\n");

		cmd::info("Hawk help:");

		cmd::info("\n\tgeneral usage:");
		cmd::print("\t\thawk [path/to/file.hawk] [-flags...]");

		cmd::info("\n\n\tfunctions:");

		cmd::print("\t\t-c=[mode]:   set the compiler output mode");
		cmd::print("\t\t\tasm:     set output mode to assembly (also runs -llvm)");
		cmd::print("\t\t\texe:     [default] set output mode to a .exe (also runs -llvm and -o)");
		cmd::print("\t\t\tint:     run the interpreter (also runs -llvm)");
		cmd::print("\t\t\tllvm:    set output mode to llvm IR (Intermediate Representation");
		cmd::print("\t\t\to:       set output mode an object file (also runs -llvm)");

		cmd::print("");
		cmd::print("\t\t-h:      help (you're here now)");
		cmd::print("\t\t-v:      get the version");


		cmd::info("\n\n\tflags:");
		  cmd::print("\t\t-f:    print feedback at the end of compilation (success / fail)");
		  cmd::print("\t\t-nc:   logs without color. If \"-nc\" is put before \"-h\", the help menu will also not be in color");

		cmd::info("\n\n\tdebug:");
		cmd::print("\t\t-ast:	 show the AST (Abstract Syntax Tree)");
		cmd::print("\t\t-ir:     show the llvm IR (Intermediate Representation)");
		cmd::print("\t\t-tokens: show the program tokens");

		cmd::print("\n");
	};


}

int main(int argc, const char *args[]){
	return Hawk::main(std::vector<std::string>(args, args + argc));
};