#include "pch.h"
#include "Parser.h"


namespace Hawk{

	Parser::Parser(const std::vector<Tokenizer::Token>& token_vector)
		: tokens(token_vector) {

	};


	Parser::~Parser(){
		for(auto* stmt : this->statements){
			delete stmt;
		}
	}



	void Parser::start(){
		while(this->i + 1 < tokens.size() && this->success()){
			this->statements.push_back(this->parse_stmt());
		};
	};


	Tokenizer::Token Parser::get(){
		return this->tokens[this->i++];
	};

	Tokenizer::Token Parser::peek(uint offset){
		return this->tokens[this->i + offset];
	};



	void Parser::error(const Tokenizer::Token& token){
		this->has_errored = true;
		cmd::error("\tlocation: <{}, {}>", token.line, token.collumn);
	};

	void Parser::error(){
		this->error(this->peek());
	};

	#define ERROR(...) if(this->success()){ cmd::error(__VA_ARGS__); this->error(); };


	void Parser::expected_error(Tokenizer::Token::Type expected, const Tokenizer::Token& token){
		ERROR("Error: expected {}, got {}", this->print_token(expected), this->print_token(token));
	};

	void Parser::expected_error(Tokenizer::Token::Type expected){
		this->expected_error(expected, this->peek());
	};

	bool Parser::expect(Tokenizer::Token::Type expected){
		if(this->peek().type == expected){
			this->get();
			return true;
		}else{
			this->expected_error(expected);
			return false;
		}
	};

	#define EXPECT(type) if(!this->expect(type)){ return nullptr; };



	void Parser::uncaught_error(const Tokenizer::Token& token){
		ERROR("Uncaught Error");
	};

	void Parser::uncaught_error(){
		this->uncaught_error(this->peek());
	};



	std::string Parser::print_token(const Tokenizer::Token& token){
		switch(token.type){
			case TokenType::id:				return fmt::format("[ID: {}]", token.value);
			case TokenType::literal_number:	return fmt::format("[LITERAL: {}]", token.value);
			case TokenType::literal_bool:	return fmt::format("[LITERAL: {}]", token.value);

			default: 						return Parser::print_token(token.type);
		};
	};

	std::string Parser::print_token(Tokenizer::Token::Type token){
		switch(token){
			case TokenType::none:			return "[NONE]";

			case TokenType::id:				return "[ID]";
			case TokenType::literal_number:	return "[LITERAL NUMBER]";
			case TokenType::literal_bool:	return "[LITERAL BOOL]";

			case TokenType::keyword_func:	return "[KEYWORD: func]";
			case TokenType::keyword_return:	return "[KEYWORD: return]";
			case TokenType::keyword_if:		return "[KEYWORD: if]";
			case TokenType::keyword_else:	return "[KEYWORD: else]";

			case TokenType::type_void:		return "[TYPE: void]";
			case TokenType::type_int:		return "[TYPE: int]";
			case TokenType::type_bool:		return "[TYPE: bool]";

			case TokenType::assign:			return "[OPERATOR: '=']";
			case TokenType::type_def:		return "[OPERATOR: ':']";

			case TokenType::op_plus:		return "[OPERATOR: '+']";

			case TokenType::semicolon:		return "[PUNCTUATION: ';']";
			case TokenType::comma:			return "[PUNCTUATION: ',']";
			case TokenType::open_paren:		return "[PUNCTUATION: '(']";
			case TokenType::close_paren:	return "[PUNCTUATION: ')']";
			case TokenType::open_brace:		return "[PUNCTUATION: '{']";
			case TokenType::close_brace:	return "[PUNCTUATION: '}']";

			default: 						return "[UNKNOWN TOKEN TYPE]";
		};
	};


	std::string Parser::print_token(){
		return this->print_token(this->peek());
	};


	//////////////////////////////////////////////////////////////////////
	// parsing


	// Stmt
	// 		FuncCallStmt ';'
	// 		VarDecl      ';'
	// 		VarAssign	 ';'
	// 		FuncDef      ';'
	AST::Stmt* Parser::parse_stmt(){
		AST::Stmt* output;

		switch(this->peek().type){
			case Tokenizer::Token::Type::id: {
				if(this->peek(1).type == TokenType::open_paren){
					output = this->parse_func_call_stmt();
				}else if(this->peek(1).type == TokenType::assign){
					output = this->parse_var_assign();
				}else{
					output = this->parse_var_decl();
				}
			} break;
			case Tokenizer::Token::Type::keyword_func: {
				output = this->parse_func_def();
			} break;
			case Tokenizer::Token::Type::keyword_return: {
				output = this->parse_return_stmt();
			} break;
			case Tokenizer::Token::Type::keyword_if: {
				output = this->parse_conditional();
			} break;
			default: {
				ERROR("Received invalid begin to statement ({})", this->print_token(this->peek()));
				return nullptr;
			};
		};

		EXPECT(TokenType::semicolon);

		return output;
	};


	// FuncCallStmt
	// 		FuncCall
	AST::FuncCallStmt* Parser::parse_func_call_stmt(){
		return new AST::FuncCallStmt(this->parse_func_call());
	};

	// ReturnStmt
	// 		Expr
	AST::ReturnStmt* Parser::parse_return_stmt(){
		EXPECT(TokenType::keyword_return);

		auto expr = this->parse_expr();

		if(expr == nullptr){
			ERROR("Expected expression in return statement, got ({})", this->print_token(this->peek()));
			return nullptr;
		}


		return new AST::ReturnStmt(expr);
	};


	// Block
	// 		'{' '}'
	// 		'{' Stmt+ '}'
	AST::Block* Parser::parse_block(){
		EXPECT(TokenType::open_brace);

		auto* block = new AST::Block(this->peek(-1));
		while(this->peek().type != TokenType::close_brace){
			auto stmt = this->parse_stmt();
			if(stmt == nullptr) return nullptr;

			block->stmts.push_back(stmt);
		};

		EXPECT(TokenType::close_brace);

		return block;
	};



	// VarDecl
	// 		Id ':'      '=' Expr
	// 		Id ':' Type '=' Expr
	// 		Id ':' Type         
	AST::VarDecl* Parser::parse_var_decl(){
		auto id = this->parse_id();
		if(id == nullptr){
			this->uncaught_error();
			return nullptr;
		}


		EXPECT(TokenType::type_def);


		auto type = this->parse_type();

		AST::Expr* value = nullptr;

		// option 1
		if(type == nullptr){
			EXPECT(TokenType::assign);
			value = this->parse_expr();
			if(value == nullptr){
				ERROR("Expected Expression in Variable Declaration, got ({})", this->print_token(this->peek()));
				return nullptr;
			}

		// option 2
		}else if(this->peek().type == TokenType::assign){
			EXPECT(TokenType::assign);
			value = this->parse_expr();
			if(value == nullptr){
				ERROR("Expected Expression in Variable Declaration, got ({})", this->print_token(this->peek()));
				return nullptr;
			}			

		// option 3
		}else{
			// do nothing
		}


		return new AST::VarDecl(id, type, value);

	};


	// VarAssign
	// 		Id '=' Expr
	AST::VarAssign* Parser::parse_var_assign(){
		auto id = this->parse_id();
		if(id == nullptr){
			this->uncaught_error();
			return nullptr;
		}

		EXPECT(TokenType::assign);

		auto value = this->parse_expr();
		if(value == nullptr){
			ERROR("Expected Expression in Variable Assignment, got ({})", this->print_token(this->peek()));
			return nullptr;
		}

		return new AST::VarAssign(id, value);

	};





	// FuncDef
	// 		'func' Id Params type Block
	// 		'func' Id Params      Block
	AST::FuncDef* Parser::parse_func_def(){
		EXPECT(TokenType::keyword_func);

		auto id = this->parse_id();
		if(id == nullptr){
			ERROR("Expected Id for function definition, got ({})", this->print_token(this->peek()));
			return nullptr;
		}

		auto params = this->parse_params();
		if(params == nullptr) return nullptr;


		auto return_type = this->parse_type();


		auto block = this->parse_block();
		if(params == nullptr) return nullptr;


		return new AST::FuncDef(id, return_type, params, block);
	};


	// Conditional
	// 		'if' '(' Expr ')' Block
	// 		'if' '(' Expr ')' Block 'else' Block
	// 		'if' '(' Expr ')' Block 'else' Conditional
	AST::Conditional* Parser::parse_conditional(){
		EXPECT(TokenType::keyword_if);

		EXPECT(TokenType::open_paren);
			auto cond = this->parse_expr();
			if(cond == nullptr){
				ERROR("Expected conditional expression, got ({})", this->print_token(this->peek()));
				return nullptr;
			}
		EXPECT(TokenType::close_paren);

		auto then_block = this->parse_block();

		AST::Stmt* else_block = nullptr;
		if(this->peek().type == TokenType::keyword_else){
			this->get();

			if(this->peek().type == TokenType::keyword_if){
				else_block = this->parse_conditional();
			}else{
				else_block = this->parse_block();
			}
		}


		return new AST::Conditional(cond, then_block, else_block);

	};





	// Expr
	// 		Literal
	// 		Id
	// 		FuncCall
	AST::Expr* Parser::parse_expr(){
		AST::Expr* output = nullptr;

		output = this->parse_literal();
		if(output != nullptr) return output;


		if(this->peek().type == TokenType::id){
			if(this->peek(1).type == TokenType::open_paren ){
				output = this->parse_func_call();
			}else{
				output = this->parse_id();
			}
			if(output != nullptr) return output;
		}


		return nullptr;
	};



	// FuncCall
	// 		Id Params
	AST::FuncCall* Parser::parse_func_call(){
		auto id = this->parse_id();
		auto params = this->parse_params();

		return new AST::FuncCall(id, params);
	};




	// Type
	// 		'type_keyword'
	// 		Id
	AST::Type* Parser::parse_type(){
		switch(this->peek().type){
			case TokenType::id: 		return dynamic_cast<AST::Type*>(this->parse_id());
			case TokenType::type_int:	return new AST::Type(this->get());
			case TokenType::type_void:	return new AST::Type(this->get());
			case TokenType::type_bool:	return new AST::Type(this->get());
			default: 					return nullptr;
		};
	};


	// Params
	// 		'(' ')'
	// 		'(' Param ')'
	// 		'(' (Param ',')+ Param ')'
	AST::Params* Parser::parse_params(){
		EXPECT(TokenType::open_paren);

		auto params = new AST::Params(this->peek(-1));

		while(this->peek().type != TokenType::close_paren){
			if(params->params.size() > 0){
				EXPECT(TokenType::comma);
			}

			auto param = this->parse_param();
			if(param == nullptr){
				ERROR("Expected Expression in parameter list, got ({})");
				return nullptr;
			}

			params->params.push_back(param);
		};

		EXPECT(TokenType::close_paren);

		return params;
	};


	// Param
	// 		Expr
	AST::Param* Parser::parse_param(){
		auto expr = this->parse_expr();
		if(expr != nullptr){
			return static_cast<AST::Param*>(expr);
		}else{
			return nullptr;
		}
	};


	// Literal
	// 		literal_int
	// 		literal_bool
	AST::Literal* Parser::parse_literal(){
		switch(this->peek().type){
			case TokenType::literal_number:	return new AST::Literal(this->get());
			case TokenType::literal_bool:	return new AST::Literal(this->get());
			default: 						return nullptr;
		};
	};


	AST::Id* Parser::parse_id(){
		if(this->peek().type != TokenType::id) return nullptr;

		return new AST::Id(this->get());
	};





	//////////////////////////////////////////////////////////////////////
	// printing
	std::string indentation(uint ident){
		std::string output;
		for(uint i = 0; i < ident; i++){
			output += '\t';
		}
		return output;
	};



	void AST::Id::print(uint ident){
		cmd::log("{}Id: {}", indentation(ident), this->token.value);
	};

	void AST::Type::print(uint ident){
		cmd::log("{}Type: {}", indentation(ident), this->token.value);
	};

	void AST::Literal::print(uint ident){
		cmd::log("{}Literal: {}", indentation(ident), this->token.value);
	};

	void AST::Params::print(uint ident){
		if(this->params.size() > 0){
			cmd::log("{}Params:", indentation(ident));

			for(auto* param : this->params){
				param->print(ident + 1);
			}
		}else{
			cmd::log("{}Params: (empty)", indentation(ident));
		}
	};


	void AST::Param::print(uint ident){
		cmd::log("{}Block:", indentation(ident));
		this->expr->print(ident + 1);
	};

	void AST::FuncCall::print(uint ident){
		cmd::log("{}FuncCall:", indentation(ident));
		this->id->print(ident + 1);
		this->params->print(ident + 1);
	};








	void AST::Block::print(uint ident){
		if(this->stmts.size() > 0){
			cmd::log("{}Block:", indentation(ident));

			for(auto* stmt : this->stmts){
				stmt->print(ident + 1);
			}
		}else{
			cmd::log("{}Block: (empty)", indentation(ident));
		}
	};



	void AST::VarDecl::print(uint ident){
		cmd::log("{}VarDecl:", indentation(ident));
		this->id->print(ident + 1);

		if(this->type != nullptr){
			this->type->print(ident + 1);
		}else{
			cmd::log("{}Type: auto", indentation(ident + 1));
		}

		if(this->value != nullptr){
			this->value->print(ident + 1);
		}else{
			cmd::log("{}Value: uninitialized", indentation(ident + 1));
		}
	};

	void AST::VarAssign::print(uint ident){
		cmd::log("{}VarAssign:", indentation(ident));
		this->id->print(ident + 1);
		this->value->print(ident + 1);
	};


	void AST::FuncDef::print(uint ident){
		cmd::log("{}FuncDef:", indentation(ident));
		this->id->print(ident + 1);

		if(this->return_type != nullptr){
			this->return_type->print(ident + 1);
		}else{
			cmd::log("{}Type: auto", indentation(ident + 1));
		}

		this->params->print(ident + 1);
		this->block->print(ident + 1);

	};


	void AST::FuncCallStmt::print(uint ident){
		cmd::log("{}FuncCallStmt:", indentation(ident));
		this->expr->id->print(ident + 1);
		this->expr->params->print(ident + 1);
	};

	void AST::ReturnStmt::print(uint ident){
		cmd::log("{}ReturnStmt:", indentation(ident));
		this->expr->print(ident + 1);
	};

	void AST::Conditional::print(uint ident){
		cmd::log("{}Conditional:", indentation(ident));
		this->cond->print(ident + 1);
		this->then_block->print(ident + 1);

		if(this->else_block != nullptr){
			this->else_block->print(ident + 1);
		}
	};




	
};