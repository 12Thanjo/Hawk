#pragma once



namespace Hawk{
	class CharacterStream;
	
	class Tokenizer{
		public:
			Tokenizer(CharacterStream& char_stream) : stream(char_stream) {};
			~Tokenizer() = default;


			void start();

			struct Token{
				enum class Type;
				Type type;
				std::variant<std::string, uint64_t, double, char> value = "";
				uint line;
				uint collumn;

				static std::string type_string(Type type);
			};

			inline bool success() const { return !this->has_errored; };

		public:
			std::vector<Token> tokens;

		private:
			void make_token(Token::Type type);
			void process_char();

			bool process_comments();
			bool process_id();
			bool process_number();
			bool process_operators();
			bool process_punctuation();

			void move_while(std::function<bool()> condition);
			void move_while_newline();

			bool is_whitespace();
			bool is_letter(char _char);
			bool is_letter();
			bool is_number(char _char);
			bool is_number();

			void error();

		private:
			CharacterStream& stream;
			
			char character;
			std::string token_val = "";

			bool has_errored = false;
	};




	enum class Tokenizer::Token::Type{
		none,

		id,
		number,

		keyword_func,
		keyword_return,

		type_int,

		assign,
		type_def,

		op_plus,

		semicolon,
		open_paren,
		close_paren,
		open_brace,
		close_brace,
	};


}

