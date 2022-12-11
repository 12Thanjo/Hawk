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
				FuncCall,
			};

			virtual Type type() = 0;

			struct Val;
			struct LR;
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

			AST::Expr* left;
			Tokenizer::Token op;
			AST::Expr* right;

			void print(uint indent) override;
			Expr::Type type() override { return Expr::Type::LR; };
		};


		struct Expr::FuncCall : public Expr{
			FuncCall(AST::Expr* ident, std::vector<AST::Expr*> expr_vec)
				: id(ident), exprs(expr_vec) {};

			AST::Expr* id;
			std::vector<AST::Expr*> exprs;

			void print(uint indent) override;
			Expr::Type type() override { return Expr::Type::FuncCall; };
		};



		struct Stmt{
			enum class Type{
				assign,
				func_call
			};

			Type type;
			AST::Expr* expr;

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
			AST::Stmt parse_assignment();
			AST::Stmt parse_func_call();

			AST::Expr* parse_expr();
			std::vector<AST::Expr*> parse_parens();


			//////////////////////////////////////////////////////////////////////
			// is

			bool is_type(const Tokenizer::Token& token);
			bool is_arithmetic(const Tokenizer::Token& token);



			void assert_type(const Tokenizer::Token& token, TokenType type);

		private:
			std::vector<Tokenizer::Token>& tokens;
			uint i = 0;
			bool has_errored = false;
	};
	
};