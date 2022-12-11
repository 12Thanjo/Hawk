#include "pch.h"
#include "StaticAnalyzer.h"


namespace Hawk{

	StaticAnalyzer::StaticAnalyzer(std::vector<AST::Stmt>& statements)
		: stmts(statements) {
			
		this->add_scope();
		
	};



	void StaticAnalyzer::error(){
		// this->has_errored = true;
	};


	void StaticAnalyzer::begin(){
		
	};


	

	void StaticAnalyzer::add_scope(){
		this->scopes.emplace_back();
	};


	void StaticAnalyzer::leave_scope(){
		this->scopes.pop_back();
	};
	
};
