#pragma once

#include "Parser.h"

namespace Hawk{

	class SemanticAnalyzer{
		public:
			SemanticAnalyzer(const std::vector<AST::Stmt*>& stmts);
			~SemanticAnalyzer() = default;

			void begin();

			inline uint get_error_count() const { return this->error_count; };
			inline uint get_warning_count() const { return this->warning_count; };

		private:
			void get_all_globals();
			void global_var_type_inference_attempt();
			void func_checking_type_inference_attempt();
			void func_checking_type_inference_attempt_impl(AST::FuncDef* func_def, AST::Stmt* stmt);
			void func_call_type_inference(AST::FuncCall* func_call);

			void final_check_all();
			void final_check_all_impl(AST::Stmt* stmt);


			AST::Type* get_expr_type(AST::Expr* expr);
			bool same_expr_type(AST::Type* type1, AST::Type* type2);


			void error(const Tokenizer::Token& token);
			void error(AST::Expr* expr);
			void error(AST::Stmt* stmt);

			void warning(const Tokenizer::Token& token);
			void warning(AST::Expr* expr);
			void warning(AST::Stmt* stmt);

			void enter_scope();
			void leave_scope();
			void add_to_scope(std::string var_name, AST::VarDecl* var_decl);
			AST::VarDecl* in_scope(const std::string& var_name);
			AST::VarDecl* in_current_scope(const std::string& var_name);
			bool in_global_scope();

		public:
			std::map<std::string, AST::VarDecl*> global_vars;
			std::map<std::string, AST::FuncDef*> functions;
	
		private:
			const std::vector<AST::Stmt*>& stmts;
			uint error_count = 0;
			uint warning_count = 0;

			bool found_return_stmt = false;
			bool printed_return_error = false;


			using Scope = std::map<std::string, AST::VarDecl*>;
			std::list<Scope> scopes;

	};
	
}
