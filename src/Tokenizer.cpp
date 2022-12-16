#include "pch.h"
#include "Tokenizer.h"


#include "CharacterStream.h"
#include "Tokenizer.h"
#include "Parser.h"


namespace Hawk{
	

	void Tokenizer::start(){
		
		while(!this->stream.end() && this->success()){
			auto next_char = this->stream.next();
			if(next_char.has_value()){

				this->character = next_char.value();
				if(this->character != '\0'){
					this->process_char();
				}else{
					// cmd::print("EOF");
					break;
				}

			}else{
				cmd::fatal("Attempted to read past the end of the file");
				this->error();
			}
		};

	};


	void Tokenizer::error(){
		this->has_errored = true;
		cmd::error("\tline:    {}"
			     "\n\tcollumn: {}", this->stream.get_line(), this->stream.get_collumn());
	};




	void Tokenizer::make_token(Token::Type type){
		this->tokens.emplace_back(type, this->stream.get_line(), this->stream.get_collumn(), this->token_val);
	};


	void Tokenizer::process_char(){

		this->token_val += this->character;
		
		if(
			!this->is_whitespace() &&
			!this->process_comments() &&
			!this->process_id() &&
			!this->process_number() &&
			!this->process_operators() &&
			!this->process_punctuation() &&
			true
		){
			cmd::error("Unable to process char [{}] (charcode: {})", this->character, (uint)this->character);
			this->error();
		}

		this->token_val = "";

	};





	bool Tokenizer::process_comments(){
		if(this->character == '/'){

			if(this->stream.peek(0) == '/'){
				this->stream.move(1);
				this->move_while_newline();
				return true;

			}else if(this->stream.peek(0) == '*'){
				uint depth = 1;
				this->stream.move(1);
				this->move_while([&](){
					if(this->character == '*' && this->stream.peek(0) == '/'){
						depth -= 1;
						this->stream.move(1);
					}else if(this->character == '/' && this->stream.peek(0) == '*'){
						depth += 1;
						this->stream.move(1);
					}

					return depth > 0;
				});
				// this->stream.move(2);
				return true;
			}

		}
		return false;
	};



	bool Tokenizer::process_id(){
		if(this->is_letter() || this->character == '_'){
			this->move_while([&](){
				auto peek = this->stream.peek();
				if(peek.has_value()){
					return this->is_letter(peek.value()) || this->is_number(peek.value()) || peek.value() == '_';
				}else{
					cmd::fatal("Attempted to read past the end of the file");
					this->error();
				}
				return false;
			});

			if(this->token_val == "void"){
				this->make_token(Token::Type::type_void);
			}else if(this->token_val == "int"){
				this->make_token(Token::Type::type_int);
			}else if(this->token_val == "bool"){
				this->make_token(Token::Type::type_bool);


			}else if(this->token_val == "func"){
				this->make_token(Token::Type::keyword_func);
			}else if(this->token_val == "return"){
				this->make_token(Token::Type::keyword_return);
			}else if(this->token_val == "if"){
				this->make_token(Token::Type::keyword_if);
			}else if(this->token_val == "else"){
				this->make_token(Token::Type::keyword_else);


			}else if(this->token_val == "true" || this->token_val == "false"){
				this->make_token(Token::Type::literal_bool);
			
			}else{
				this->make_token(Token::Type::id);
			}


			return true;
		}

		return false;
	};


	bool Tokenizer::process_number(){
		if(this->is_number()){
			this->move_while([&](){
				auto peek = this->stream.peek();
				if(peek.has_value()){
					return this->is_number(peek.value());
				}else{
					cmd::fatal("Attempted to read past the end of the file");
					this->error();
				}
				return false;
			});


			this->make_token(Token::Type::literal_number);
			return true;
		}


		return false;
	};



	bool Tokenizer::process_operators(){
		if(this->character == '='){
			this->make_token(Token::Type::assign);
			return true;
		}else if(this->character == ':'){
			this->make_token(Token::Type::type_def);
			return true;
		}else if(this->character == '@'){
			this->make_token(Token::Type::const_type_def);
			return true;
		}else if(this->character == '+'){
			this->make_token(Token::Type::op_plus);
			return true;
		}
		return false;
	};


	bool Tokenizer::process_punctuation(){

		switch(this->character){
			case ';': this->make_token(Token::Type::semicolon);   return true;
			case ',': this->make_token(Token::Type::comma); 	  return true;
			case '(': this->make_token(Token::Type::open_paren);  return true;
			case ')': this->make_token(Token::Type::close_paren); return true;
			case '{': this->make_token(Token::Type::open_brace);  return true;
			case '}': this->make_token(Token::Type::close_brace); return true;
		};


		return false;
	};






	void Tokenizer::move_while(std::function<bool()> condition){
		while(!this->stream.end() && condition()){
			auto next_char = this->stream.next();
			if(next_char.has_value()){

				this->character = next_char.value();
				if(this->character == '\0'){
					// cmd::print("EOF");
					// break;
					cmd::error("Hit the end of the file (Tokenizer)");
					this->error();
				}
				this->token_val += this->character;

			}else{
				cmd::error("Attempted to read past the end of the file (Tokenizer)");
				this->error();
			}
		};
	};



	void Tokenizer::move_while_newline(){
		this->move_while([&](){
			return this->character != '\n';
		});
	};





	//////////////////////////////////////////////////////////////////////
	// is


	bool Tokenizer::is_whitespace(){
		switch(this->character){
			case '\n':
			case '\r':
			case '\t':
			case ' ':
				return true;
			default:
				return false;	
		};
	};

	bool Tokenizer::is_letter(char _char){
		switch(_char){
			case 'a': case 'A':
			case 'b': case 'B':
			case 'c': case 'C':
			case 'd': case 'D':
			case 'e': case 'E':
			case 'f': case 'F':
			case 'g': case 'G':
			case 'h': case 'H':
			case 'i': case 'I':
			case 'j': case 'J':
			case 'k': case 'K':
			case 'l': case 'L':
			case 'm': case 'M':
			case 'n': case 'N':
			case 'o': case 'O':
			case 'p': case 'P':
			case 'q': case 'Q':
			case 'r': case 'R':
			case 's': case 'S':
			case 't': case 'T':
			case 'u': case 'U':
			case 'v': case 'V':
			case 'w': case 'W':
			case 'y': case 'Y':
			case 'z': case 'Z':
				return true;
			default:
				return false;	
		};
	};

	bool Tokenizer::is_letter(){
		return this->is_letter(this->character);
	};


	bool Tokenizer::is_number(char _char){
		switch(_char){
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				return true;
			default:
				return false;	
		};
	};

	bool Tokenizer::is_number(){
		return this->is_number(this->character);
	};



}

