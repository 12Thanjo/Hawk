#include "pch.h"
#include "Compiler.h"





namespace Hawk{
	
	Compiler::Compiler(std::map<std::string, AST::VarDecl*>& global_vars, 
					const std::map<std::string, AST::FuncDef*>& functions,
					const std::string& package_name) 
		: global_vars(global_vars), functions(functions), /*package_name(package_name),*/ 
			context(), builder(context), module(package_name, context) {
		
		this->types["int"] = this->builder.getInt64Ty();
		this->types["float"] = this->builder.getDoubleTy();
		this->types["bool"] = this->builder.getInt1Ty();
		this->types["void"] = this->builder.getVoidTy();

	}
	
	Compiler::~Compiler(){
		
	}


	void Compiler::build_ir(){
		
		for(auto [name, var_decl] : this->global_vars){
			auto var_name = var_decl->id->token.value;
			auto type_str = var_decl->type->token.value;
			auto type = this->types[type_str];
			auto is_constant = false;


			auto* global = new llvm::GlobalVariable(this->module, type, is_constant, llvm::GlobalValue::PrivateLinkage, this->get_llvm_constant(var_decl->value), var_name);
			global->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
			global->setAlignment(llvm::Align(1));
			this->global_llvm_vars[var_name] = global;
		}

		this->import_externs();


		for(auto [name, func_def] : this->functions){
			auto return_type_str = func_def->return_type->token.value;
			auto return_type = this->types[return_type_str];

			std::vector<llvm::Type*> params;
			for(auto* param : func_def->params->params){
				params.push_back(this->types[param->type->token.value]);
			}

			auto prototype = llvm::FunctionType::get(return_type, params, false);
			llvm::Function* function = llvm::Function::Create(prototype, llvm::Function::ExternalLinkage, name, this->module);
			this->llvm_functions[name] = function;
		}

		for(auto [name, func_def] : this->functions){
			auto return_type_str = func_def->return_type->token.value;
			auto* function = this->llvm_functions[name];


			llvm::BasicBlock* body = llvm::BasicBlock::Create(this->context, "entry", function);
			this->builder.SetInsertPoint(body);

			// this->llvm_functions[name] = function;

			this->enter_scope();

				uint counter = 0;
				auto ast_params = func_def->params->params;
				for(auto& arg : function->args()){
					auto arg_name = ast_params[counter]->id->token.value;
					auto arg_type = this->types[ast_params[counter]->type->token.value];


					llvm::IRBuilder<> temp_builder(body, body->begin());
					auto alloca = temp_builder.CreateAlloca(arg_type, nullptr, arg_name);

					this->builder.CreateStore(&arg, alloca);

					this->add_to_scope(arg_name, alloca);

					counter += 1;
				}

				for(auto* stmt : func_def->block->stmts){
					this->parse_stmt(stmt, func_def);
				}
			this->leave_scope();


			if(return_type_str == "void"){
				this->builder.CreateRet(0);
			}


		}


	};


	void Compiler::import_externs(){
		{
			// use libc's printf function
			auto prototype = llvm::FunctionType::get(this->builder.getInt8Ty(), true);
			auto function = llvm::Function::Create(prototype, llvm::Function::ExternalLinkage, "printf", this->module);

			this->llvm_functions["printf"] = function;
		}


	};


	




	void Compiler::parse_stmt(AST::Stmt* stmt, AST::FuncDef* func_def){
		this->just_returned = false;

		switch(stmt->get_type()){
			case AST::StmtType::VarDecl: {
				auto var_decl = static_cast<AST::VarDecl*>(stmt);
				auto var_name = var_decl->id->token.value;
				auto type_str = var_decl->type->token.value;
				auto var_type = this->types[type_str];


				llvm::AllocaInst* alloca = builder.CreateAlloca(var_type, nullptr, var_name);
				auto store = builder.CreateStore(this->get_llvm_value(var_decl->value), alloca);
				this->add_to_scope(var_name, alloca);

			} break; case AST::StmtType::ReturnStmt: {
				auto* return_stmt = static_cast<AST::ReturnStmt*>(stmt);
				auto* return_value = this->get_llvm_value(return_stmt->expr);

				builder.CreateRet(return_value);
				this->just_returned = true;

			} break; case AST::StmtType::VarAssign: {
				auto var_assign = static_cast<AST::VarAssign*>(stmt);
				auto var_name = var_assign->id->token.value;


				auto alloca = this->in_scope(var_name);
				builder.CreateStore(this->get_llvm_value(var_assign->value), alloca);

			} break; case AST::StmtType::FuncCallStmt: {
				auto func_call = static_cast<AST::FuncCallStmt*>(stmt)->expr;
				auto func_name = func_call->id->token.value;
				auto params = func_call->params->params;

				if(func_name == "printf"){
					auto format_str = builder.CreateGlobalStringPtr("Printed from libc::printf (%f)\n");

					auto first = params[0]->expr;
					auto val = this->get_llvm_value(params[0]);

					builder.CreateCall(this->llvm_functions["printf"], { format_str, val });
				}else{

					std::vector<llvm::Value*> arguments;
					arguments.reserve(params.size());
					for(auto* param : params){
						arguments.push_back(this->get_llvm_value(param));
					}

					builder.CreateCall(this->llvm_functions[func_name], arguments);
				}

			} break; case AST::StmtType::Block: {
				auto block = static_cast<AST::Block*>(stmt);

				this->enter_scope();
				for(auto* block_stmt : block->stmts){
					this->parse_stmt(block_stmt, func_def);
				}
				this->leave_scope();

			} break; case AST::StmtType::Conditional: {
				auto conditional = static_cast<AST::Conditional*>(stmt);
				auto cond_value = this->get_llvm_value(conditional->cond);

				auto current_func = builder.GetInsertBlock()->getParent();

				llvm::BasicBlock* then_block;
				llvm::BasicBlock* else_block;
				llvm::BasicBlock* merge_block;



				if(conditional->else_block != nullptr){
					then_block = llvm::BasicBlock::Create(this->context, "then", current_func);
					else_block = llvm::BasicBlock::Create(this->context, "else");
					merge_block = llvm::BasicBlock::Create(this->context, "if_merge");

					this->builder.CreateCondBr(cond_value, then_block, else_block);
				}else{
					then_block = llvm::BasicBlock::Create(this->context, "then", current_func);
					merge_block = llvm::BasicBlock::Create(this->context, "if_merge");

					this->builder.CreateCondBr(cond_value, then_block, merge_block);
				}

				

				//////////////////////////////////////////////////////////////////////
				// then

				this->builder.SetInsertPoint(then_block);
					this->parse_stmt(conditional->then_block, func_def);
					if(!this->just_returned){
						builder.CreateBr(merge_block);
					}else{
						this->just_returned = false;
					}
					then_block = builder.GetInsertBlock();


				//////////////////////////////////////////////////////////////////////
				// else

				if(conditional->else_block != nullptr){
					current_func->getBasicBlockList().push_back(else_block);
					this->builder.SetInsertPoint(else_block);
						this->parse_stmt(conditional->else_block, func_def);
						if(!this->just_returned){
							builder.CreateBr(merge_block);
						}else{
							this->just_returned = false;
						}
						else_block = builder.GetInsertBlock();

				}

				//////////////////////////////////////////////////////////////////////
				// emit merge

				current_func->getBasicBlockList().push_back(merge_block);
				this->builder.SetInsertPoint(merge_block);


			} break; default: return;
		};
	};





	llvm::Value* Compiler::get_llvm_value(AST::Expr* expr){
		switch(expr->get_type()){
			case AST::ExprType::Literal: {
				auto literal = static_cast<AST::Literal*>(expr);

				if(literal->token.type == TokenType::literal_int){
					return llvm::ConstantInt::get(this->types["int"], std::stoll(static_cast<AST::Literal*>(expr)->token.value));
				}else if(literal->token.type == TokenType::literal_float){
					return llvm::ConstantFP::get(this->types["float"], std::stod(static_cast<AST::Literal*>(expr)->token.value));
				}else if(literal->token.type == TokenType::literal_bool){
					return llvm::ConstantInt::get(this->types["bool"], static_cast<AST::Literal*>(expr)->token.value == "true" ? 1 : 0);
				}else{
					cmd::fatal("Recieved unknown literal type ({})", (int)expr->get_type());
				}

			} break;case AST::ExprType::Id: {
				auto* alloca = this->in_scope(static_cast<AST::Id*>(expr)->token.value);
				if(alloca != nullptr){
					return this->builder.CreateLoad(alloca->getAllocatedType(), alloca);
				}else{
					auto var_name = static_cast<AST::Id*>(expr)->token.value;
					auto type_name = this->global_vars[var_name]->type->token.value;


					auto global = this->global_llvm_vars[var_name];
					return this->builder.CreateLoad(this->types[type_name], global);
				}

			} break;case AST::ExprType::FuncCall: {
				auto func_call = static_cast<AST::FuncCall*>(expr);
				auto func_name = func_call->id->token.value;
				auto params = func_call->params->params;

				std::vector<llvm::Value*> arguments;
				arguments.reserve(params.size());
				for(auto* param : params){
					arguments.push_back(this->get_llvm_value(param));
				}

				return builder.CreateCall(this->llvm_functions[func_name], arguments);

			} break;case AST::ExprType::Binary: {
				auto* binary = static_cast<AST::Binary*>(expr);
				auto* left = this->get_llvm_value(binary->left);
				auto* right = this->get_llvm_value(binary->right);

				if(binary->type == nullptr){
					cmd::fatal("Compiler recieved binary expr type as nullptr");
					cmd::fatal("It should have been set in the SemanticAnalyzer");
				}

				auto expr_type = binary->type->token.value;

				if(expr_type == "int"){
					switch(binary->op.type){
						break;case TokenType::op_plus: return builder.CreateAdd(left, right, "<add>");
						break;case TokenType::op_minus: return builder.CreateSub(left, right, "<sub>");
						break;case TokenType::op_mult: return builder.CreateMul(left, right, "<mul>");
						break;case TokenType::op_div: return builder.CreateSDiv(left, right, "<div>");

						break;case TokenType::op_lt:	return builder.CreateICmpSLT(left, right, "<");
						break;case TokenType::op_lte:	return builder.CreateICmpSLE(left, right, "<=");
						break;case TokenType::op_gt:	return builder.CreateICmpSGT(left, right, ">");
						break;case TokenType::op_gte:	return builder.CreateICmpSGE(left, right, "<=");
						break;case TokenType::op_eq:	return builder.CreateICmpEQ(left, right, "==");
						break;case TokenType::op_neq:	return builder.CreateICmpNE(left, right, "!=");

						break;default: cmd::fatal("Recieved unknown binary op type ({}) for int expr", (int)binary->op.type);
					};
				}else if(expr_type == "float"){
					switch(binary->op.type){
						break;case TokenType::op_plus: return builder.CreateFAdd(left, right, "<add>");
						break;case TokenType::op_minus: return builder.CreateFSub(left, right, "<sub>");
						break;case TokenType::op_mult: return builder.CreateFMul(left, right, "<mul>");
						break;case TokenType::op_div: return builder.CreateFDiv(left, right, "<div>");

						break;case TokenType::op_lt:	return builder.CreateFCmpOLT(left, right, "<");
						break;case TokenType::op_lte:	return builder.CreateFCmpOLE(left, right, "<=");
						break;case TokenType::op_gt:	return builder.CreateFCmpOGT(left, right, ">");
						break;case TokenType::op_gte:	return builder.CreateFCmpOGE(left, right, "<=");
						break;case TokenType::op_eq:	return builder.CreateFCmpOEQ(left, right, "==");
						break;case TokenType::op_neq:	return builder.CreateFCmpONE(left, right, "!=");


						break;default: cmd::fatal("Recieved unknown binary op type ({}) for float expr", (int)binary->op.type);
					};
					
				}else if(expr_type == "bool"){
					switch(binary->op.type){
						break;case TokenType::op_eq:	return builder.CreateICmpEQ(left, right, "==");
						break;case TokenType::op_neq:	return builder.CreateICmpNE(left, right, "!=");

						break;case TokenType::op_and:	return builder.CreateLogicalAnd(left, right, "&&");
						break;case TokenType::op_or:	return builder.CreateLogicalOr(left, right, "||");

						break;default: cmd::fatal("Recieved unknown binary op type ({}) for bool expr", (int)binary->op.type);
					}
				}else{
					cmd::fatal("Recieved unknown binary expr type ({})", expr_type);
				}



			} break; default: cmd::fatal("Recieved unknown Expr type for llvm_value ({})", (int)expr->get_type());
		};

		return nullptr;
	};


	llvm::Constant* Compiler::get_llvm_constant(AST::Expr* expr){
		switch(expr->get_type()){
			case AST::ExprType::Literal: {
				auto literal = static_cast<AST::Literal*>(expr);

				if(literal->token.type == TokenType::literal_int){
					return llvm::ConstantInt::get(this->types["int"], std::stoll(static_cast<AST::Literal*>(expr)->token.value));
				}else if(literal->token.type == TokenType::literal_float){
					return llvm::ConstantInt::get(this->types["float"], std::stod(static_cast<AST::Literal*>(expr)->token.value));
				}else if(literal->token.type == TokenType::literal_bool){
					return llvm::ConstantInt::get(this->types["bool"], static_cast<AST::Literal*>(expr)->token.value == "true" ? 1 : 0);
				}else{
					cmd::fatal("Recieved unknown Literal type ({})", (int)expr->get_type());
				}

			} break;case AST::ExprType::Id: {
				return this->global_llvm_vars[static_cast<AST::Id*>(expr)->token.value]->getInitializer();

			} break; default:cmd::fatal("Recieved unknown expr type llvm_constant ({})", (int)expr->get_type());
		};

		return nullptr;
	};



	//////////////////////////////////////////////////////////////////////
	// output

	void Compiler::print_ir(){
		this->module.print(llvm::outs(), nullptr);
	};


	void Compiler::save_ir_to_file(){
		std::error_code EC;
		auto out = llvm::raw_fd_ostream("output.ll", EC);
		
		this->module.print(out, nullptr);
	};

	void Compiler::compile_to_obj(){
		system("llc -filetype=obj output.ll -o output.o");
	};

	void Compiler::compile_to_asm(){
		system("llc -filetype=asm output.ll -o output.s");
	};

	void Compiler::compile_exe(fs::path& program_path){
		std::string command = "g++ -g output.o ";

		auto path = program_path.parent_path().parent_path().parent_path().parent_path() / "obj/disable_chkstk.o";
		command += "\"" + path.string() + "\"";

		command +=  " -o output.exe";
		system(command.c_str());
	};

	void Compiler::run_interpreter(){
		system("lli output.ll");
	};



	//////////////////////////////////////////////////////////////////////
	// scoping

	void Compiler::enter_scope(){
		this->scopes.emplace_back();
	};

	void Compiler::leave_scope(){
		this->scopes.pop_back();
	};

	void Compiler::add_to_scope(std::string var_name, llvm::AllocaInst* alloca){
		this->scopes.back()[var_name] = alloca;
	};

	llvm::AllocaInst* Compiler::in_scope(const std::string& var_name){


		for(std::list<Scope>::reverse_iterator ptr = this->scopes.rbegin(); ptr != this->scopes.rend(); ptr++){
			auto scope = *ptr;
			if(scope.contains(var_name)){
				return scope[var_name];
			}
		}



		return nullptr;
	};

	llvm::AllocaInst* Compiler::in_current_scope(const std::string& var_name){
		if(this->scopes.back().contains(var_name)){
			return this->scopes.back()[var_name];
		}
		return nullptr;
	};


	bool Compiler::in_global_scope(){
		return this->scopes.size() == 1;
	};



};

