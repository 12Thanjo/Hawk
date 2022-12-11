#include "pch.h"
#include "CharacterStream.h"


namespace Hawk{
	
	bool CharacterStream::end(){
		return this->i + 1 == file.size();
	};

	std::optional<char> CharacterStream::next(){
		if(!this->end()){
		    auto character = this->file[this->i];
			this->i += 1;
		    if(character == '\n'){
		        this->line += 1;
		        this->collumn = 1; 
		        // line_str = "";
		    // }else if(character == '\t'){
		    //     this->collumn += 4;
		    }else{
		        this->collumn += 1;
		        // line_str += character;
		    }
		    return std::optional<char>{character};
		}else{
			return std::nullopt;
		}
	};


	std::optional<char> CharacterStream::peek(int offset){
		if(this->i + offset < this->file.size()){
			return this->file[this->i + offset];
		}else{
			return std::nullopt;
		}
	};


	void CharacterStream::move(int amount){
		this->i += amount;
	};


}
