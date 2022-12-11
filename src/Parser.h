#pragma once

#include "Tokenizer.h"

namespace Hawk{

	namespace AST{

		struct Expr{
			virtual ~Expr(){};
			virtual void print(uint indent) = 0;

			enum class Type{
				Val,
				LR,
				L,
				R,
				FuncCall,
			};

			virtual Type type() = 0;

			struct Val;
			struct LR;
			struct L;
			struct R;
			struct FuncCall;
		};

		struct Expr::Val : public Expr{
			Val(Tokenizer::Token tok) : token(tok) {};

			Tokenizer::Token token;

			void print(uint indent) override;
			Expr::Type type() override { return Expr::Type::Val; };
		};


		struct Expr::LR : public Expr{
			LR(AST::Expr* left_expr, Tokenizer::Token oper, AST::Expr* right_expr)
				: left(left_expr), op(oper), right(right_expr) {};
			~LR(){
				delete this->left;
				delete this->right;
			};

			AST::Expr* left;
			Tokenizer::Token op;
			AST::Expr* right;

			void print(uint indent) override;
			Expr::Type type() override { return Expr::Type::LR; };
		};

		struct Expr::L : public Expr{
			L(AST::Expr* left_expr, Tokenizer::Token oper)
				: left(left_expr), op(oper) {};

			~L(){
				delete this->left;
			};

			AST::Expr* left;
			Tokenizer::Token op;

			void print(uint indent) override;
			Expr::Type type() override { return Expr::Type::L; };
		};

		struct Expr::R : public Expr{
			R(Tokenizer::Token oper, AST::Expr* right_expr)
				: op(oper), right(right_expr) {};

			~R(){
				delete this->right;
			};

			Tokenizer::Token op;
			AST::Expr* right;

			void print(uint indent) override;
			Expr::Type type() override { return Expr::Type::R; };
		};


		struct Expr::FuncCall : public Expr{
			FuncCall(AST::Expr* ident, std::vector<AST::Expr*> expr_vec)
				: id(ident), exprs(expr_vec) {};

			~FuncCall(){
				delete this->id;
				for(auto* expr : this->exprs){
					delete expr;
				}
			};

			AST::Expr* id;
			std::vector<AST::Expr*> exprs;

			void print(uint indent) override;
			Expr::Type type() override { return Expr::Type::FuncCall; };
		};



		struct Stmt{
			enum class Type{
				var_decl,
				assign,
				func_call,
				func_def,
				func_return,
			};

			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			// don't forget I need to delete the expr (can't have a deconstructor)
			

			Type type;
			AST::Expr* expr;
			std::vector<Stmt> stmts; //block

			void print(uint indent);
		};
		
	};

	using TokenType = Tokenizer::Token::Type;

	class Parser{
		public:
			Parser(std::vector<Tokenizer::Token>& token_vector);
			~Parser() = default;

			void start();

			inline bool success() const { return !this->has_errored; };

		public:
			std::vector<AST::Stmt> statements;

		private:
			Tokenizer::Token get();
			Tokenizer::Token look(uint offset = 0);
			void error();


			//////////////////////////////////////////////////////////////////////
			// parse
			AST::Stmt parse_statement();
			AST::Stmt parse_var_declaration();
			AST::Stmt parse_assignment();
			AST::Stmt parse_func_call_stmt();
			AST::Stmt parse_func_def();
			std::vector<AST::Stmt> parse_block();
			AST::Stmt parse_return();


			AST::Expr* parse_expr();
			std::vector<AST::Expr*> parse_parens();
			std::vector<AST::Expr*> parse_paren_defs();
			AST::Expr* parse_func_call();


			//////////////////////////////////////////////////////////////////////
			// is

			bool is_type(const Tokenizer::Token& token);
			bool is_arithmetic(const Tokenizer::Token& token);


			//////////////////////////////////////////////////////////////////////
			// get

			Tokenizer::Token get_id();



			void assert_type(const Tokenizer::Token& token, TokenType type);

		private:
			std::vector<Tokenizer::Token>& tokens;
			uint i = 0;
			bool has_errored = false;
	};
	
};