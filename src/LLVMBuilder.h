#pragma once


#include "Parser.h"

#pragma warning (push, 0)
	#include <llvm/IR/IRBuilder.h>
#pragma warning (pop)


namespace Hawk{

	class LLVMBuilder{
		public:
			LLVMBuilder(std::vector<AST::Stmt>& statements, const std::string& module_name);
			~LLVMBuilder();

			void build_ir();

			void save_ir_to_file();
			void print_ir();
			void run_interpreter();
			
			void compile_to_asm();
			void compile_to_obj();
			void compile_exe();

		private:
			void parse_stmt(AST::Stmt& stmt);

			std::string get_expr_id_str(AST::Expr* expr);
			llvm::Value* get_expr_value(AST::Expr* expr);

	
		private:
			std::vector<AST::Stmt>& stmts;

			llvm::LLVMContext context;
			llvm::IRBuilder<> builder;
			llvm::Module module;

			std::unordered_map<std::string, llvm::Function*> functions;
			std::unordered_map<std::string, llvm::AllocaInst*> variables;
	};
	
};
