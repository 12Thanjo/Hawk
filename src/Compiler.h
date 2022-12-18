#pragma once

#pragma warning (push, 0)
	#include <llvm/IR/IRBuilder.h>
#pragma warning (pop)


#include "Parser.h"

namespace Hawk{

	class Compiler{
			public:
				Compiler(std::map<std::string, AST::VarDecl*>& global_vars, 
					const std::map<std::string, AST::FuncDef*>& functions,
					const std::string& package_name);
				~Compiler();


				void build_ir();
				void import_externs();


				void print_ir();
				void save_ir_to_file();
				void compile_to_obj();
				void compile_to_asm();
				void compile_exe(fs::path& program_path);
				void run_interpreter();

			private:
				llvm::Value* get_llvm_value(AST::Expr* expr);
				llvm::Constant* get_llvm_constant(AST::Expr* expr);

				void parse_stmt(AST::Stmt* stmt, AST::FuncDef* func_def);

		
			private:
				std::map<std::string, AST::VarDecl*>& global_vars;
				const std::map<std::string, AST::FuncDef*>& functions;
				// const std::string& package_name;


				llvm::LLVMContext context;
				llvm::IRBuilder<> builder;
				llvm::Module module;

				std::map<std::string, llvm::Type*> types;
				std::map<std::string, llvm::GlobalVariable*> global_llvm_vars;
				std::map<std::string, llvm::Function*> llvm_functions;


				void enter_scope();
				void leave_scope();
				void add_to_scope(std::string var_name, llvm::AllocaInst* alloca);
				llvm::AllocaInst* in_scope(const std::string& var_name);
				llvm::AllocaInst* in_current_scope(const std::string& var_name);
				bool in_global_scope();

				using Scope = std::map<std::string, llvm::AllocaInst*>;
				std::list<Scope> scopes;


				bool just_returned = false;
		};	

};
