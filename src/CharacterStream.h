#pragma once

namespace Hawk{
	
	class CharacterStream{
		public:
			CharacterStream(const std::string& file_input) : file(file_input) {};
			~CharacterStream() = default;
	
			bool end();
			std::optional<char> next();
			std::optional<char> peek(int offset = 0);
			void move(int amount);

			inline uint get_line() const { return this->line; };
			inline uint get_collumn() const { return this->collumn; };

		private:
			const std::string& file;
			uint i = 0;

			uint line = 1;
			uint collumn = 1;
	};

}
