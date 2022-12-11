#pragma once


#include "Parser.h"


namespace Hawk{
	
	class StaticAnalyzer{
		public:
			StaticAnalyzer(std::vector<AST::Stmt>& statements);
			~StaticAnalyzer() = default;

			void begin();
			inline bool success() const { return !this->has_errored; };

		private:
			void error();

			void add_scope();
			void leave_scope();

	
		private:
			std::vector<AST::Stmt>& stmts;
			bool has_errored = false;

			std::vector<std::set<std::string>> scopes;
	};
	
};
