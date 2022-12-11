#include "pch.h"
#include "Parser.h"


namespace Hawk{

	Parser::Parser(std::vector<Tokenizer::Token>& token_vector)
		: tokens(token_vector) {

	};



	void Parser::start(){
		while(this->i + 1 < tokens.size() && this->success()){
			this->statements.push_back(this->parse_statement());
			// this->i += 1;
		};
	};


	void Parser::error(){
		this->has_errored = true;
	};



	Tokenizer::Token Parser::get(){
		return this->tokens[this->i++];
	};

	Tokenizer::Token Parser::look(uint offset){
		return this->tokens[this->i + offset];
	};


	void Parser::assert_type(const Tokenizer::Token& token, TokenType type){
		if(token.type != type){
			cmd::error("Expected type ({}), got type ({})", Tokenizer::Token::type_string(type), Tokenizer::Token::type_string(token.type));
			cmd::error("\tline:    {}"
					 "\n\tcollumn: {}", token.line, token.collumn);
			this->error();
		}
	};



	//////////////////////////////////////////////////////////////////////
	// parsing

	

	AST::Stmt Parser::parse_statement(){
		AST::Stmt output;

		if(this->look().type == TokenType::id){
			if(this->look(1).type == TokenType::type_def){
				output = this->parse_assignment();
			}else if(this->look(1).type == TokenType::open_paren){
				output = this->parse_func_call();
			}
		}

		this->assert_type(this->get(), TokenType::semicolon);

		return output;
	};


	AST::Stmt Parser::parse_assignment(){
		auto id = this->get();

		this->assert_type(this->get(), TokenType::type_def);

		Tokenizer::Token type(TokenType::none);
		if( this->is_type(this->look()) ){
			type = this->get();
			// this->i += 1;
		}

		this->assert_type(this->get(), TokenType::assign);



		auto expr = this->parse_expr();

		return {
			AST::Stmt::Type::assign,
			new AST::Expr::LR {
				new AST::Expr::Val{id},
				type,
				expr
			}
		};
	};


	AST::Stmt Parser::parse_func_call(){
		auto id = this->get();
		auto parens = this->parse_parens();


		return {
			AST::Stmt::Type::func_call,
			new AST::Expr::FuncCall {
				new AST::Expr::Val{id},
				parens
			}
		};
	};




	AST::Expr* Parser::parse_expr(){
		AST::Expr* output;

		if(this->look().type == TokenType::number){
			output = new AST::Expr::Val(this->get());

		}else if(this->look().type == TokenType::id){
			output = new AST::Expr::Val(this->get());

		}else{
			cmd::error("Unknown Expr (got type {})", Tokenizer::Token::type_string(this->look().type));
			cmd::error("\tline:    {}"
					 "\n\tcollumn: {}", this->look().line, this->look().collumn);
			this->error();
		}


		if( this->is_arithmetic(this->look()) && this->success() ){
			auto op = this->get();
			output = new AST::Expr::LR{ output, op, this->parse_expr() };
		}

		return output;
	};


	std::vector<AST::Expr*> Parser::parse_parens(){
		std::vector<AST::Expr*> output;

		this->i += 1;

		while(this->look().type != TokenType::close_paren){
			output.push_back(this->parse_expr());
			// this->i += 1;
		};

		this->i += 1;

		return output;
	};


	//////////////////////////////////////////////////////////////////////
	// is

	bool Parser::is_type(const Tokenizer::Token& token){
		switch(token.type){
			case TokenType::type_int:
			case TokenType::id:
				return true;
			default:
				return false;
		};
	};


	bool Parser::is_arithmetic(const Tokenizer::Token& token){
		switch(token.type){
			case TokenType::op_plus:
				return true;
			default:
				return false;
		};
	};




	//////////////////////////////////////////////////////////////////////
	// printing

	std::string tabs(uint indent){
		std::string output;

		for(uint i = 0; i < indent; i++){
			output += "    ";
		}

		return output;
	};


	void AST::Stmt::print(uint indent){
		cmd::print("{}{} ({})", tabs(indent), "Stmt", (uint)this->type);
		this->expr->print(indent + 1);
	};



	void AST::Expr::Val::print(uint indent){
		cmd::print("{}Expr::Val ({})", tabs(indent), std::get<std::string>(this->token.value));
	};

	void AST::Expr::LR::print(uint indent){
		cmd::print("{}Expr::LR ({})", tabs(indent), std::get<std::string>(this->op.value));

		this->left->print(indent + 1);
		this->right->print(indent + 1);
	};


	void AST::Expr::FuncCall::print(uint indent){
		cmd::print("{}Expr::FuncCall", tabs(indent));

		this->id->print(indent + 1);
		cmd::print("{}params:", tabs(indent+1));
		
		for(auto* expr : this->exprs){
			expr->print(indent + 2);
		}

	};

	
};