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
			DefParam,
			DefParams,
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
			~Param(){ delete this->expr; };

			Expr* expr;

			void print(uint ident) override;
			ExprType get_type() override { return ExprType::Param; };
		};

		struct Params : public Expr {
			Params(Tokenizer::Token token): start(token) {};
			~Params(){
				for(auto* param : this->params){
					delete param;
				}
			};

			std::vector<Param*> params;
			Tokenizer::Token start;

			void print(uint ident) override;
			ExprType get_type() override { return ExprType::Params; };
		};

		struct DefParam : public Expr {
			DefParam(Id* id, Type* type) : id(id), type(type) {};
			~DefParam(){
				delete this->id;
				delete this->type;
			};

			Id* id;
			Type* type;

			void print(uint ident) override;
			ExprType get_type() override { return ExprType::DefParam; };
		};

		struct DefParams : public Expr {
			DefParams(Tokenizer::Token token): start(token) {};
			~DefParams(){
				for(auto* param : this->params){
					delete param;
				}
			};

			std::vector<DefParam*> params;
			Tokenizer::Token start;

			void print(uint ident) override;
			ExprType get_type() override { return ExprType::Params; };
		};

		struct FuncCall : public Expr {
			FuncCall(Id* id, Params* params) : id(id), params(params) {};
			~FuncCall(){
				delete this->id;
				delete this->params;
			};

			Id* id;
			Params* params;


			void print(uint ident) override;
			ExprType get_type() override { return ExprType::FuncCall; };
		};


		struct Binary : public Expr {
			Binary(Expr* left, Tokenizer::Token op, Expr* right)
				: left(left), op(op), right(right) {};
			~Binary(){
				delete this->left;
				delete this->right;
			};

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
			~Block(){
				for(auto* stmt : this->stmts){
					delete stmt;
				}
			};

			std::vector<Stmt*> stmts;
			Tokenizer::Token start;

			void print(uint ident) override;
			StmtType get_type() override { return StmtType::Block; };
		};



		struct VarDecl : public Stmt {
			VarDecl(Id* id, Type* type, Expr* value): id(id), type(type), value(value) {};
			~VarDecl(){
				delete this->id;
				// this causes a segfault for some reason
				// delete this->type;
				delete this->value;
			};

			Id* id;
			Type* type;
			Expr* value;


			void print(uint ident) override;
			StmtType get_type() override { return StmtType::VarDecl; };
		};

		struct VarAssign : public Stmt {
			VarAssign(Id* id, Expr* value): id(id), value(value) {};
			~VarAssign(){
				delete this->id;
				delete this->value;
			};

			Id* id;
			Expr* value;


			void print(uint ident) override;
			StmtType get_type() override { return StmtType::VarAssign; };
		};


		struct FuncCallStmt : public Stmt {
			FuncCallStmt(FuncCall* expr) : expr(expr) {};
			~FuncCallStmt(){
				delete this->expr;
			};

			FuncCall* expr;

			void print(uint ident) override;
			StmtType get_type() override { return StmtType::FuncCallStmt; };
		};


		struct ReturnStmt : public Stmt {
			ReturnStmt(Expr* expr) : expr(expr) {};
			~ReturnStmt(){
				delete this->expr;
			};

			Expr* expr;

			void print(uint ident) override;
			StmtType get_type() override { return StmtType::ReturnStmt; };
		};


		struct FuncDef : public Stmt {
			FuncDef(Id* id, Type* return_type, DefParams* params, Block* block)
				: id(id), return_type(return_type), params(params), block(block) {};
			~FuncDef(){
				delete this->id;
				delete this->return_type;
				delete this->params;
				delete this->block;
			};

			Id* id;
			Type* return_type;
			DefParams* params;
			Block* block;


			void print(uint ident) override;
			StmtType get_type() override { return StmtType::FuncDef; };
		};



		struct Conditional : public Stmt {
			Conditional(Expr* cond, Block* then, Stmt* else_block)
				: cond(cond), then_block(then), else_block(else_block) {};
			~Conditional(){
				delete this->cond;
				delete this->then_block;
				delete this->else_block;
			};

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
			// 		'func' Id DefParams Type Block
			// 		'func' Id DefParams      Block
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


			// DefParams
			// 		'(' ')'
			// 		'(' DefParam ')'
			// 		'(' (DefParam ',')+ DefParam ')'
			AST::DefParams* parse_def_params();


			// DefParam
			// 		Id ':' Type
			AST::DefParam* parse_def_param();


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