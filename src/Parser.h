#pragma once

#include "Tokenizer.h"

namespace Hawk{

	namespace AST{

		//////////////////////////////////////////////////////////////////////
		// expressions

		enum class ExprType{
			Id,
			Type,
			Literal,
			Param,
			Params,
			FuncCall,
			Binary,
		};

		struct Expr{
			virtual ~Expr(){};

			virtual void print(uint ident) = 0;
			virtual ExprType get_type() = 0;
		};

		struct Id : public Expr {
			Id(Tokenizer::Token token) : token(token) {};

			Tokenizer::Token token;

			void print(uint ident) override;
			ExprType get_type() override { return ExprType::Id; };
		};

		struct Type : public Expr {
			Type(Tokenizer::Token token) : token(token) {};

			Tokenizer::Token token;

			void print(uint ident) override;
			ExprType get_type() override { return ExprType::Type; };
		};

		struct Literal : public Expr {
			Literal(Tokenizer::Token token) : token(token) {};

			Tokenizer::Token token;

			void print(uint ident) override;
			ExprType get_type() override { return ExprType::Literal; };
		};


		struct Param : public Expr {

			Expr* expr;

			void print(uint ident) override;
			ExprType get_type() override { return ExprType::Param; };
		};

		struct Params : public Expr {
			Params(Tokenizer::Token token): start(token) {};

			std::vector<Param*> params;
			Tokenizer::Token start;

			void print(uint ident) override;
			ExprType get_type() override { return ExprType::Params; };
		};

		struct FuncCall : public Expr {
			FuncCall(Id* id, Params* params) : id(id), params(params) {};

			Id* id;
			Params* params;


			void print(uint ident) override;
			ExprType get_type() override { return ExprType::FuncCall; };
		};


		struct Binary : public Expr {
			Binary(Expr* left, Tokenizer::Token op, Expr* right)
				: left(left), op(op), right(right) {};

			Expr* left;
			Tokenizer::Token op;
			Expr* right;


			void print(uint ident) override;
			ExprType get_type() override { return ExprType::Binary; };
		};


	
		//////////////////////////////////////////////////////////////////////
		// statements

		enum class StmtType{
			Block,
			VarDecl,
			FuncCallStmt,
			FuncDef,
			VarAssign,
			ReturnStmt,
			Conditional,
		};

		struct Stmt{
			virtual ~Stmt(){};

			virtual void print(uint ident) = 0;
			virtual StmtType get_type() = 0;
		};


		struct Block : public Stmt {
			Block(Tokenizer::Token token): start(token) {};

			std::vector<Stmt*> stmts;
			Tokenizer::Token start;

			void print(uint ident) override;
			StmtType get_type() override { return StmtType::Block; };
		};



		struct VarDecl : public Stmt {
			VarDecl(Id* id, Type* type, Expr* value): id(id), type(type), value(value) {};

			Id* id;
			Type* type;
			Expr* value;


			void print(uint ident) override;
			StmtType get_type() override { return StmtType::VarDecl; };
		};

		struct VarAssign : public Stmt {
			VarAssign(Id* id, Expr* value): id(id), value(value) {};

			Id* id;
			Expr* value;


			void print(uint ident) override;
			StmtType get_type() override { return StmtType::VarAssign; };
		};


		struct FuncCallStmt : public Stmt {
			FuncCallStmt(FuncCall* expr) : expr(expr) {};

			FuncCall* expr;

			void print(uint ident) override;
			StmtType get_type() override { return StmtType::FuncCallStmt; };
		};


		struct ReturnStmt : public Stmt {
			ReturnStmt(Expr* expr) : expr(expr) {};

			Expr* expr;

			void print(uint ident) override;
			StmtType get_type() override { return StmtType::ReturnStmt; };
		};


		struct FuncDef : public Stmt {
			FuncDef(Id* id, Type* return_type, Params* params, Block* block)
				: id(id), return_type(return_type), params(params), block(block) {};

			Id* id;
			Type* return_type;
			Params* params;
			Block* block;


			void print(uint ident) override;
			StmtType get_type() override { return StmtType::FuncDef; };
		};



		struct Conditional : public Stmt {
			Conditional(Expr* cond, Block* then, Stmt* else_block)
				: cond(cond), then_block(then), else_block(else_block) {};

			Expr* cond;
			Block* then_block;
			Stmt* else_block;
			// must be AST::Conditional or AST::Block


			void print(uint ident) override;
			StmtType get_type() override { return StmtType::Conditional; };
		};


	};

	

	class Parser{
		public:
			Parser(const std::vector<Tokenizer::Token>& token_vector);
			~Parser();

			void start();

			inline bool success() const { return !this->has_errored; };

			static std::string print_token(const Tokenizer::Token& token);
			static std::string print_token(Tokenizer::Token::Type token);
		public:
			std::vector<AST::Stmt*> statements;

		private:
			std::string print_token();

			Tokenizer::Token get();
			Tokenizer::Token peek(uint offset = 0);

			void error(const Tokenizer::Token& token);
			void error();

			void expected_error(Tokenizer::Token::Type expected, const Tokenizer::Token& token);
			void expected_error(Tokenizer::Token::Type expected);
			bool expect(Tokenizer::Token::Type expected);

			void uncaught_error(const Tokenizer::Token& token);
			void uncaught_error();


			//////////////////////////////////////////////////////////////////////
			// parsing

			// Stmt
			// 		FuncCallStmt ';'
			// 		VarDecl      ';'
			// 		VarAssign	 ';'
			// 		FuncDef      ';'
			// 		ReturnStmt   ';'
			// 		Conditional  ';'
			AST::Stmt* parse_stmt();


			// Block
			// 		'{' '}'
			// 		'{' Stmt+ '}'
			AST::Block* parse_block();


			// VarDecl
			// 		Id ':'      '=' Expr
			// 		Id ':' Type '=' Expr
			// 		Id ':' Type
			AST::VarDecl* parse_var_decl();


			// VarAssign
			// 		Id '=' Expr
			AST::VarAssign* parse_var_assign();


			// FuncCallStmt
			// 		FuncCall
			AST::FuncCallStmt* parse_func_call_stmt();


			// ReturnStmt
			// 		Expr
			AST::ReturnStmt* parse_return_stmt();


			// FuncDef
			// 		'func' Id Params type Block
			// 		'func' Id Params      Block
			AST::FuncDef* parse_func_def();

			// Conditional
			// 		'if' '(' Expr ')' Block
			// 		'if' '(' Expr ')' Block 'else' Block
			// 		'if' '(' Expr ')' Block 'else' Conditional
			AST::Conditional* parse_conditional();




			AST::Expr* parse_expr();
			AST::Expr* parse_op(AST::Expr* left, uint prec);
			uint get_op_prec(const Tokenizer::Token& op);
			AST::Expr* parse_term();


			// FuncCall
			// 		Id Params
			AST::FuncCall* parse_func_call();


			// Params
			// 		'(' ')'
			// 		'(' Param ')'
			// 		'(' (Param ',')+ Param ')'
			AST::Params* parse_params();


			// Param
			// 		Expr
			AST::Param* parse_param();


			// Type
			// 		'type_keyword'
			// 		Id
			AST::Type* parse_type();


			// Literal
			// 		literal_int
			// 		literal_bool
			AST::Literal* parse_literal();


			// Id...
			AST::Id* parse_id();


			//////////////////////////////////////////////////////////////////////
			// is

			bool is_operator(Tokenizer::Token& token);


		private:
			const std::vector<Tokenizer::Token>& tokens;
			uint i = 0;
			bool has_errored = false;
	};
	
};