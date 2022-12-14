#include "pch.h"
#include "SemanticAnalyzer.h"


namespace Hawk{


	SemanticAnalyzer::SemanticAnalyzer(const std::vector<AST::Stmt*>& stmts) : stmts(stmts) {
		this->enter_scope();

		#define GENERATE_TYPE(type) { \
			auto new_token = Tokenizer::Token(TokenType::generated); \
			new_token.value = type; \
			this->types[type] = new AST::Type(new_token); \
		}

		GENERATE_TYPE("void");
		GENERATE_TYPE("int");
		GENERATE_TYPE("float");
		GENERATE_TYPE("bool");


		#undef GENERATE_TYPE
	};

	SemanticAnalyzer::~SemanticAnalyzer(){
		for(auto [name, type] : this->types){
			delete type;
		}
	};




	#define CONTINUE() if(this->error_count != 0){return;};
	void SemanticAnalyzer::begin(){
		this->get_all_globals();
		CONTINUE();
		this->global_var_type_inference_attempt();
		CONTINUE();
		this->func_checking_type_inference_attempt();
		CONTINUE();
		this->final_check_all();
		
	};
	#undef CONTINUE



	void SemanticAnalyzer::get_all_globals(){
		for(auto* stmt : this->stmts){
			switch(stmt->get_type()){

				case AST::StmtType::VarDecl: {
					auto* var_decl = static_cast<AST::VarDecl*>(stmt);
					std::string var_name = var_decl->id->token.value;

					// check if already defined
					if(this->global_vars.contains(var_name)){
						auto already_defined = this->global_vars[var_name]->id->token;
						cmd::error("Global variable ({}) was already defined at <{}, {}>", var_name, already_defined.line, already_defined.collumn);
						this->error(var_decl->id->token);
					}else{
						this->global_vars[var_name] = var_decl;
					}


				} break; case AST::StmtType::FuncDef: {
					auto func_def = static_cast<AST::FuncDef*>(stmt);
					std::string func_name = func_def->id->token.value;

					if(this->functions.contains(func_name)){
						auto first_definition = this->global_vars[func_name]->id->token;
						this->error(func_def->id->token);
						cmd::error("\tFunction ({}) was already defined at <{}, {}>", func_name, first_definition.line, first_definition.collumn);
					}else{
						this->functions[func_name] = func_def;
					}

				} break; case AST::StmtType::VarAssign: {
					auto* var_assign = static_cast<AST::VarAssign*>(stmt);
					this->error(var_assign->id->token);
					cmd::error("\tVariable assignments cannot be in global scope");

				}break;case AST::StmtType::FuncCallStmt: {
					auto* func_call_stmt = static_cast<AST::FuncCallStmt*>(stmt);
					this->error(func_call_stmt->expr->id->token);
					cmd::error("\tFunction-call statements cannot be in global scope");

				}break;case AST::StmtType::Block: {
					auto* block = static_cast<AST::Block*>(stmt);
					this->error(block->start);
					cmd::error("\tBlock statements cannot be alone in global scope");

				}break;case AST::StmtType::ReturnStmt: {
					auto* return_stmt = static_cast<AST::ReturnStmt*>(stmt);
					this->error(return_stmt->expr);
					cmd::error("\tReturn statements cannot be in global scope");

				}break;case AST::StmtType::Conditional: {
					auto* conditional_stmt = static_cast<AST::Conditional*>(stmt);
					this->error(conditional_stmt->cond);
					cmd::error("\tConditional statements cannot be in global scope");

				}break; default: {
					cmd::fatal("Compiler Fail: Received unknown statement type (SemanticAnalyzer, line: {})", __LINE__);
					this->error_count += 1;
				}

			};
		}
	};




	void SemanticAnalyzer::global_var_type_inference_attempt(){
		for(auto [name, var_decl] : this->global_vars){
			this->add_to_scope(name, var_decl);

			if(var_decl->type != nullptr){ break; };

			if(var_decl->value->get_type() == AST::ExprType::Literal){
				var_decl->type = this->get_expr_type(var_decl->value);
			}else if(var_decl->value->get_type() == AST::ExprType::Id){
				auto id_token = static_cast<AST::Id*>(var_decl->value)->token;


				if(id_token.value == name){
					this->error(id_token);
					cmd::error("\tCannot set variable to self in a declaration");
					continue;
				}

				var_decl->type = this->get_expr_type(var_decl->value);
			}

		}
	};



	void SemanticAnalyzer::func_checking_type_inference_attempt(){
		for(auto [name, func_def] : this->functions){
			
			this->enter_scope();

			for(auto* param : func_def->params->params){
				auto param_name = param->id->token.value;

				if(this->in_current_scope(param_name)){
					auto first_definition = this->in_current_scope(param_name)->id->token;
					this->error(param->id);
					cmd::error("\tParameter({}) was already defined at <{}, {}> ", param_name, first_definition.line, first_definition.collumn);
					return;
				}

				if(this->in_scope(param_name)){
					auto first_definition = this->in_scope(param_name)->id->token;
					this->warning(param->id);
					cmd::warning("\tParameter ({}) was already defined in a parent scope at <{}, {}> ", param_name, first_definition.line, first_definition.collumn);
					cmd::warning("\tThis may cause unexpected behavior");
				}

				this->add_to_scope(param_name, new AST::VarDecl(param->id, param->type, nullptr));

			}

			for(auto* stmt : func_def->block->stmts){
				this->func_checking_type_inference_attempt_impl(func_def, stmt);
			}

			if(func_def->return_type == nullptr){
				if(this->found_return_stmt){
					this->error(func_def);
					cmd::error("\tFound return statement in function ({}) with 'void' return type", name);
				}else{
					func_def->return_type = this->types["void"];
				}
			}else if(!this->found_return_stmt && func_def->return_type->token.value != "void"){
				this->error(func_def);
				cmd::error("\tFunction ({}) does not return on all conditional paths", name);
			}

			this->leave_scope();

		}
	};


	void SemanticAnalyzer::func_checking_type_inference_attempt_impl(AST::FuncDef* func_def, AST::Stmt* stmt){
		if(this->found_return_stmt && !this->printed_return_error){
			this->error(stmt);
			cmd::error("\tFound code after return stmt");
			this->printed_return_error = true;
		}

		switch(stmt->get_type()){

			//////////////////////////////////////////////////////////////////////
			// VarDecl

			case AST::StmtType::VarDecl: {
				auto var_decl = static_cast<AST::VarDecl*>(stmt);
				auto var_name = var_decl->id->token.value;
				if(this->in_current_scope(var_name)){
					auto first_definition = this->in_current_scope(var_name)->id->token;
					this->error(var_decl->id);
					cmd::error("\tVariable ({}) was already defined at <{}, {}> ", var_name, first_definition.line, first_definition.collumn);
					return;
				}

				if(this->in_scope(var_name)){
					auto first_definition = this->in_scope(var_name)->id->token;
					this->warning(var_decl->id);
					cmd::warning("\tVariable ({}) was already defined in a parent scope at <{}, {}> ", var_name, first_definition.line, first_definition.collumn);
					cmd::warning("\tThis may cause unexpected behavior");
				}

				this->add_to_scope(var_name, var_decl);

				if(var_decl->type != nullptr){ return; };




				if(var_decl->value->get_type() == AST::ExprType::Literal || var_decl->value->get_type() == AST::ExprType::Binary){
					var_decl->type = this->get_expr_type(var_decl->value);


				}else if(var_decl->value->get_type() == AST::ExprType::Id){
					auto id_token = static_cast<AST::Id*>(var_decl->value)->token;

					if(id_token.value == var_name){
						this->error(id_token);
						cmd::error("\tCannot set variable to self in a declaration");
						return;
					}

					var_decl->type = this->get_expr_type(var_decl->value);


				}else if(var_decl->value->get_type() == AST::ExprType::FuncCall){
					auto* func_call = static_cast<AST::FuncCall*>(var_decl->value);
					auto func_call_name = func_call->id->token.value;

					this->func_call_type_inference(func_call);

					var_decl->type = this->functions[func_call_name]->return_type;


				}else{
					cmd::fatal("Compiler Fail: Received unknown expr type (SemanticAnalyzer, line: {})", __LINE__);
					this->error(var_decl->value);
					return;
				}



			} break;


			case AST::StmtType::VarAssign: break;

			//////////////////////////////////////////////////////////////////////
			// FuncCallStmt

			case AST::StmtType::FuncCallStmt: {
				auto* func_call_stmt = static_cast<AST::FuncCallStmt*>(stmt);
				auto* func_call = static_cast<AST::FuncCall*>(func_call_stmt->expr);

				this->func_call_type_inference(func_call);

			} break;

			//////////////////////////////////////////////////////////////////////
			// ReturnStmt

			case AST::StmtType::ReturnStmt: {
				auto return_stmt = static_cast<AST::ReturnStmt*>(stmt);
				this->found_return_stmt = true;

				auto return_type = this->get_expr_type(return_stmt->expr);

				if(func_def->return_type == nullptr){
					func_def->return_type = return_type;
				}else{
					if(return_type == nullptr){
						this->warning(return_stmt);
						cmd::warning("\tUnable to determine type of return statement in function ({})", func_def->id->token.value);
						return;
					}

					if(!this->same_expr_type(func_def->return_type, return_type)){
						this->error(return_stmt);
						cmd::error("\tUnmatching return types (func: {}, return: {})", func_def->return_type->token.value, return_type->token.value);
						return;
					}
				}

			}break;

			//////////////////////////////////////////////////////////////////////
			// FuncDef

			case AST::StmtType::FuncDef: {
				this->error(stmt);
				cmd::error("\tFunction definitions must be in global scope");
				cmd::error("\tThey must be outside all functions");
				return;
			} break;

			//////////////////////////////////////////////////////////////////////
			// Conditional

			case AST::StmtType::Conditional: {
				auto conditional = static_cast<AST::Conditional*>(stmt);
				this->enter_scope();
				for(auto* then_stmt : conditional->then_block->stmts){
					func_checking_type_inference_attempt_impl(func_def, then_stmt);
				}
				this->leave_scope();

				if(conditional->else_block != nullptr){
					func_checking_type_inference_attempt_impl(func_def, conditional->else_block);
				}
			}break;

			//////////////////////////////////////////////////////////////////////
			// Block

			case AST::StmtType::Block: {
				this->enter_scope();
				for(auto* block_stmt : static_cast<AST::Block*>(stmt)->stmts){
					func_checking_type_inference_attempt_impl(func_def, block_stmt);
				}
				this->leave_scope();
			} break;

			//////////////////////////////////////////////////////////////////////
			// default

			default: {
				cmd::fatal("Compiler Fail: Received unknown statement type (SemanticAnalyzer, line: {})", __LINE__);
				this->error(stmt);
			}
		};
	};







	void SemanticAnalyzer::func_call_type_inference(AST::FuncCall* func_call){
		auto func_call_name = func_call->id->token.value;

		if(func_call_name == "printf"){
			return;
		}

		if(!this->functions.contains(func_call_name)){
			this->error(func_call);
			cmd::error("\tFunction ({}) is not defined", func_call_name);
			return;
		}

		auto function = this->functions[func_call_name];

		auto& call_args = func_call->params->params;
		auto call_arg_count = call_args.size();

		auto& func_args = function->params->params;
		auto func_arg_count = func_args.size();



		if(call_arg_count != func_arg_count){
			this->error(func_call);
			cmd::error("\tInvalid number of function call arguments");
			cmd::error("\tFunction ({}) has ({}) arguments, recieved ({})", func_call_name, func_arg_count, call_arg_count);
			return;
		}


		for(int i = 0; i < call_arg_count; i++){
			if(!this->same_expr_type(
				this->get_expr_type(call_args[i]),
				func_args[i]->type
			)){

				auto correct_type = func_args[i]->type->token.value;
				auto recieved_type = this->get_expr_type(call_args[i])->token.value;

				this->error(func_call);
				cmd::error("\tIncorrect function argument type in function ({})", func_call_name);
				cmd::error("\tArgument ({}/{}) is ({}), recieved ({})", i + 1, func_arg_count, correct_type, recieved_type);
				return;

			}
		}


	};








	void SemanticAnalyzer::final_check_all(){
		// clear/reset scopes
		this->scopes.clear();
		this->enter_scope();

		// iterate through all statements
		for(auto* stmt : this->stmts){
			this->final_check_all_impl(stmt);
		}
	};

	void SemanticAnalyzer::final_check_all_impl(AST::Stmt* stmt){
		switch(stmt->get_type()){
			case AST::StmtType::Block: {
				auto block_stmt = static_cast<AST::Block*>(stmt);

				this->enter_scope();
				for(auto* block_stmt : block_stmt->stmts){
					this->final_check_all_impl(block_stmt);
				}
				this->leave_scope();

			} break;case AST::StmtType::VarDecl: {
				auto var_decl = static_cast<AST::VarDecl*>(stmt);
				auto var_name = var_decl->id->token.value;

				if(var_decl->type == nullptr){
					// type not defined
					var_decl->type = this->get_expr_type(var_decl->value);
				}else{
					// type is defined
					auto expr_type = this->get_expr_type(var_decl->value);



					if(!this->same_expr_type(var_decl->type, expr_type)){
						this->error(var_decl);
						cmd::error("\tType mismatch in definition of variable ({})", var_name);
						cmd::error("\t{{ {} : ({}) = ({}) }}", var_name, var_decl->type->token.value, expr_type->token.value);
						return;
					}
				}

				this->add_to_scope(var_name, var_decl);

			} break;case AST::StmtType::VarAssign: {
				auto var_assign = static_cast<AST::VarAssign*>(stmt);
				auto var_name = var_assign->id->token.value;

				auto* var = this->in_scope(var_name);
				if(var == nullptr){
					this->error(var_assign);
					cmd::error("\tAssignment of undefined variable ({})", var_name);
				}


				auto* var_type = var->type;
				auto* assign_type = this->get_expr_type(var_assign->value);
				if(!this->same_expr_type(var_type, assign_type)){
					this->error(var_assign);
					cmd::error("\tIncorrect expression type in assignment of variable ({})", var_name);
					cmd::error("\texpected ({}), recieved ({})", var_type->token.value, assign_type->token.value);
				}

			} break;case AST::StmtType::FuncCallStmt: {
				auto* func_call_stmt = static_cast<AST::FuncCallStmt*>(stmt);
				if(func_call_stmt->expr->id->token.value == "printf"){
					cmd::warning("Semantic Analyzer not run on arguments to printf (may cause compilation errors/seg-faults)");
				}

			} break;case AST::StmtType::ReturnStmt: {
				// do nothing
			// 	auto return_stmt = static_cast<AST::ReturnStmt*>(stmt);

			} break;case AST::StmtType::FuncDef: {
				auto func_def = static_cast<AST::FuncDef*>(stmt);

				this->enter_scope();

				for(auto* param : func_def->params->params){
					auto param_name = param->id->token.value;

					this->add_to_scope(param_name, new AST::VarDecl(param->id, param->type, nullptr));

				}

				this->final_check_all_impl(func_def->block);

				this->leave_scope();

			} break;case AST::StmtType::Conditional: {
				auto conditional = static_cast<AST::Conditional*>(stmt);


				if( !this->same_expr_type(this->types["bool"], this->get_expr_type(conditional->cond)) ){
					this->error(conditional->cond);
					cmd::error("\tConditional expressions must return type 'bool'");
				}

				this->final_check_all_impl(conditional->then_block);

				if(conditional->else_block != nullptr){
					this->final_check_all_impl(conditional->else_block);					
				}

			} break;default: {
				cmd::fatal("Compiler Fail: Received unknown statement type (SemanticAnalyzer, line: {})", __LINE__);
				this->error(stmt);
			}
		};
	};





	//////////////////////////////////////////////////////////////////////
	// expr helpers


	AST::Type* SemanticAnalyzer::get_expr_type(AST::Expr* expr){

		switch(expr->get_type()){
			case AST::ExprType::Literal: {
				std::string type_str;
				switch(static_cast<AST::Literal*>(expr)->token.type){
					break;case TokenType::literal_bool: type_str = "bool";
					break;case TokenType::literal_int: type_str = "int";
					break;case TokenType::literal_float: type_str = "float";
					break;default:
						this->error(expr);
						cmd::fatal("\tUnknown literal type ({})", Parser::print_token(static_cast<AST::Literal*>(expr)->token.type));
				};

				// auto new_token = Tokenizer::Token(TokenType::generated);
				// new_token.value = type_str;
				// return new AST::Type(new_token);
				return this->types[type_str];

			} break; case AST::ExprType::Id: {
				auto id_token = static_cast<AST::Id*>(expr)->token;

				if(!this->in_scope(id_token.value)){
					this->error(id_token);
					cmd::error("\tVariable ({}) is not defined", id_token.value);
					return nullptr;	
				}

				auto value_id = this->in_scope(id_token.value);
				if(value_id->type == nullptr){
					this->error(id_token);
					cmd::error("\tVariable ({}) doesn't have a type", id_token.value);
					return nullptr;
				};

				return new AST::Type(value_id->type->token);

			} break; case AST::ExprType::Type: {
				this->error(expr);
				cmd::fatal("\tCompiler Fail:\n\tReceived unexpected AST::Type (SemanticAnalyzer, line: {})", __LINE__);
				return nullptr;

			} break; case AST::ExprType::Param: {
				this->error(expr);
				cmd::fatal("\tUNIMPLEMENTED\tAST::Param (SemanticAnalyzer, line: {})", __LINE__);
				return nullptr;

			} break; case AST::ExprType::Params: {
				this->error(expr);
				cmd::fatal("\tUNIMPLEMENTED\tAST::Params (SemanticAnalyzer, line: {})", __LINE__);
				return nullptr;

			} break; case AST::ExprType::FuncCall: {
				auto func_call = static_cast<AST::FuncCall*>(expr);
				auto func_call_name = func_call->id->token.value;

				if(this->functions.contains(func_call_name)){
					return this->functions[func_call_name]->return_type;
				}else{
					this->error(func_call);
					cmd::error("\tFunction ({}) is not defined", func_call_name);
					return nullptr;	
				}

			} break; case AST::ExprType::Binary: {
				auto binary = static_cast<AST::Binary*>(expr);


				auto left = this->get_expr_type(binary->left);
				auto right = this->get_expr_type(binary->right);

				if(binary->type != nullptr){
					return binary->type;
				}
				
				if(left == nullptr) return nullptr;
				if(right == nullptr) return nullptr;

				if(!this->same_expr_type(left, right)){
					this->error(binary->left);
					cmd::error("\tBinary expression is invalid");
					cmd::error("\t[ ({}) {} ({}) ]", left->token.value, binary->op.value, right->token.value);
					return nullptr;
				}


				switch(binary->op.type){
					case TokenType::op_plus:
					case TokenType::op_minus:
					case TokenType::op_mult:
					case TokenType::op_div:{
						binary->type = left;
						return left;
					}

					case TokenType::op_lt:
					case TokenType::op_lte:
					case TokenType::op_gt:
					case TokenType::op_gte:
					case TokenType::op_eq: 
					case TokenType::op_neq:
					case TokenType::op_and:
					case TokenType::op_or: 
					{
						auto bool_token = Tokenizer::Token(TokenType::generated);
						bool_token.value = "bool";
						binary->type = new AST::Type(bool_token);
						return new AST::Type(bool_token);
					};

					default: {
						this->error(expr);
						cmd::fatal("\tCompiler Fail:\n\tReceived unknown binary operation type (SemanticAnalyzer, line: {})", __LINE__);
						return nullptr;
					};
				};

				
				
			} break; default: {
				this->error(expr);
				cmd::fatal("\tCompiler Fail:\n\tReceived unknown expression type (SemanticAnalyzer, line: {})", __LINE__);
				return nullptr;
			}
		};

	};


	bool SemanticAnalyzer::same_expr_type(AST::Type* type1, AST::Type* type2){
		if(type1 == nullptr || type2 == nullptr){
			return false;
		}

		return type1->token.value == type2->token.value;
	};



	//////////////////////////////////////////////////////////////////////
	// error / warnings


	void SemanticAnalyzer::error(const Tokenizer::Token& token){
		cmd::error("\nERROR: <{}, {}>", token.line, token.collumn);
		this->error_count += 1;
	};



	void SemanticAnalyzer::error(AST::Expr* expr){
		switch(expr->get_type()){
			break; case AST::ExprType::Id:  	 this->error(static_cast<AST::Id*>(expr)->token);
			break; case AST::ExprType::Type:     this->error(static_cast<AST::Type*>(expr)->token);
			break; case AST::ExprType::Literal:  this->error(static_cast<AST::Literal*>(expr)->token);
			break; case AST::ExprType::Param:	 this->error(static_cast<AST::Param*>(expr)->expr);
			break; case AST::ExprType::Params:	 this->error(static_cast<AST::Params*>(expr)->start);
			break; case AST::ExprType::FuncCall: this->error(static_cast<AST::FuncCall*>(expr)->id);
			break; case AST::ExprType::Binary:   this->error(static_cast<AST::Binary*>(expr)->left);
		};
	};


	void SemanticAnalyzer::error(AST::Stmt* stmt){
		switch(stmt->get_type()){
			break; case AST::StmtType::Block:  	  	 this->error(static_cast<AST::Block*>(stmt)->start);
			break; case AST::StmtType::VarDecl:      this->error(static_cast<AST::VarDecl*>(stmt)->id);
			break; case AST::StmtType::VarAssign: 	 this->error(static_cast<AST::VarAssign*>(stmt)->id);
			break; case AST::StmtType::FuncCallStmt: this->error(static_cast<AST::FuncCallStmt*>(stmt)->expr);
			break; case AST::StmtType::ReturnStmt: 	 this->error(static_cast<AST::ReturnStmt*>(stmt)->expr);
			break; case AST::StmtType::FuncDef: 	 this->error(static_cast<AST::FuncDef*>(stmt)->id);
			break; case AST::StmtType::Conditional:  this->error(static_cast<AST::Conditional*>(stmt)->cond);
		};
	};


	void SemanticAnalyzer::warning(const Tokenizer::Token& token){
		cmd::warning("\nWARNING: <{}, {}>", token.line, token.collumn);
		this->warning_count += 1;
	};



	void SemanticAnalyzer::warning(AST::Expr* expr){
		switch(expr->get_type()){
			break; case AST::ExprType::Id:  	 this->warning(static_cast<AST::Id*>(expr)->token);
			break; case AST::ExprType::Type:     this->warning(static_cast<AST::Type*>(expr)->token);
			break; case AST::ExprType::Literal:  this->warning(static_cast<AST::Literal*>(expr)->token);
			break; case AST::ExprType::Param:	 this->warning(static_cast<AST::Param*>(expr)->expr);
			break; case AST::ExprType::Params:	 this->warning(static_cast<AST::Params*>(expr)->start);
			break; case AST::ExprType::FuncCall: this->warning(static_cast<AST::FuncCall*>(expr)->id);
			break; case AST::ExprType::Binary:   this->warning(static_cast<AST::Binary*>(expr)->left);
		};
	};

	void SemanticAnalyzer::warning(AST::Stmt* stmt){
		switch(stmt->get_type()){
			break; case AST::StmtType::Block:  	  	 this->warning(static_cast<AST::Block*>(stmt)->start);
			break; case AST::StmtType::VarDecl:      this->warning(static_cast<AST::VarDecl*>(stmt)->id);
			break; case AST::StmtType::VarAssign: 	 this->warning(static_cast<AST::VarAssign*>(stmt)->id);
			break; case AST::StmtType::FuncCallStmt: this->warning(static_cast<AST::FuncCallStmt*>(stmt)->expr);
			break; case AST::StmtType::ReturnStmt: 	 this->warning(static_cast<AST::ReturnStmt*>(stmt)->expr);
			break; case AST::StmtType::FuncDef: 	 this->warning(static_cast<AST::FuncDef*>(stmt)->id);
			break; case AST::StmtType::Conditional:  this->warning(static_cast<AST::Conditional*>(stmt)->cond);
		};
	};



	//////////////////////////////////////////////////////////////////////
	// scoping

	void SemanticAnalyzer::enter_scope(){
		this->scopes.emplace_back();
	};

	void SemanticAnalyzer::leave_scope(){
		this->scopes.pop_back();
		this->found_return_stmt = false;
		this->printed_return_error = false;
	};

	void SemanticAnalyzer::add_to_scope(std::string var_name, AST::VarDecl* var_decl){
		this->scopes.back()[var_name] = var_decl;
	};

	AST::VarDecl* SemanticAnalyzer::in_scope(const std::string& var_name){


		for(std::list<Scope>::reverse_iterator ptr = this->scopes.rbegin(); ptr != this->scopes.rend(); ptr++){
			auto scope = *ptr;
			if(scope.contains(var_name)){
				return scope[var_name];
			}
		}

		return nullptr;
	};

	AST::VarDecl* SemanticAnalyzer::in_current_scope(const std::string& var_name){
		if(this->scopes.back().contains(var_name)){
			return this->scopes.back()[var_name];
		}
		return nullptr;
	};


	bool SemanticAnalyzer::in_global_scope(){
		return this->scopes.size() == 1;
	};


}
