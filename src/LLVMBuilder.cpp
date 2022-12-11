#include "pch.h"
#include "LLVMBuilder.h"





namespace Hawk{

	LLVMBuilder::LLVMBuilder(std::vector<AST::Stmt>& statements, const std::string& module_name)
		: stmts(statements),
		context(), builder(context), module(module_name, context) {

		this->types["void"] = this->builder.getVoidTy();

		this->types["i1"] = this->builder.getInt1Ty();
		this->types["i8"] = this->builder.getInt8Ty();
		this->types["i16"] = this->builder.getInt16Ty();
		this->types["i32"] = this->builder.getInt32Ty();
		this->types["i64"] = this->builder.getInt64Ty();
		this->types["i128"] = this->builder.getInt128Ty();

		this->types["i1p"] = llvm::Type::getInt1PtrTy(this->context, 0);
		this->types["i8p"] = this->builder.getInt8PtrTy();
		this->types["i16p"] = llvm::Type::getInt16PtrTy(this->context, 0);
		this->types["i32p"] = llvm::Type::getInt32PtrTy(this->context, 0);
		this->types["i64p"] = llvm::Type::getInt64PtrTy(this->context, 0);
	};
	

	LLVMBuilder::~LLVMBuilder(){
		// for(auto [id, func] : this->functions){
		// 	delete func;
		// }
	};





	void LLVMBuilder::build_ir(){



		// use libc's printf function
		auto printf_prototype = llvm::FunctionType::get(this->types["i8p"], true);
		this->functions["printf"] = llvm::Function::Create(printf_prototype, llvm::Function::ExternalLinkage, "printf", module);



		for(auto& stmt : this->stmts){
			this->parse_stmt(stmt);
		}

	};




	void LLVMBuilder::save_ir_to_file(){
		std::error_code EC;
		auto out = llvm::raw_fd_ostream("output.ll", EC);
		
		this->module.print(out, nullptr);
	};

	void LLVMBuilder::print_ir(){
		this->module.print(llvm::outs(), nullptr);
	};

	void LLVMBuilder::run_interpreter(){
		system("lli output.ll");
	};

	void LLVMBuilder::compile_to_asm(){
		system("llc -filetype=asm output.ll");
	};

	void LLVMBuilder::compile_to_obj(){
		system("llc -filetype=obj output.ll -o output.o");
	};

	void LLVMBuilder::compile_exe(){
		system("g++ -g output.o -o output.exe");
	};





	void LLVMBuilder::parse_stmt(AST::Stmt& stmt){

		switch(stmt.type){
			case AST::Stmt::Type::func_call: {
				this->get_expr_value(stmt.expr);

			} break; case AST::Stmt::Type::var_decl: {
				auto* lr = static_cast<AST::Expr::LR*>(stmt.expr);
				std::string id = this->get_expr_id_str(lr->left);
				auto llvm_num = this->get_expr_value(lr->right);

				auto alloca = this->builder.CreateAlloca(this->builder.getInt64Ty(), nullptr, id);
				this->builder.CreateStore(llvm_num, alloca);

				this->variables[id] = alloca;

			} break; case AST::Stmt::Type::assign: {
				auto* lr = static_cast<AST::Expr::LR*>(stmt.expr);
				std::string id = this->get_expr_id_str(lr->left);
				auto expr_value = this->get_expr_value(lr->right);

				auto alloca = this->variables[id];
				this->builder.CreateStore(expr_value, alloca);

				// this->variables[id] = alloca;

			} break; case AST::Stmt::Type::func_def: {
				auto* func_call = static_cast<AST::Expr::FuncCall*>(stmt.expr);
				auto* id = static_cast<AST::Expr::L*>(func_call->id);

				std::string id_name = this->get_expr_id_str(id->left);
				auto return_type = this->get_type(id->op);


				auto prototype = llvm::FunctionType::get(return_type, false);
				llvm::Function* func = llvm::Function::Create(prototype, llvm::Function::ExternalLinkage, id_name, this->module);
				this->functions[id_name] = func;

				llvm::BasicBlock* body = llvm::BasicBlock::Create(this->context, "body", func);
				this->builder.SetInsertPoint(body);

				for(auto& stmt : stmt.stmts){
					this->parse_stmt(stmt);
				}

				// probably will get rid of this as it will be done by sematic analysis
				if(return_type == this->types["void"]){
					this->builder.CreateRet(0);
				}

			} break; case AST::Stmt::Type::func_return: {
				auto* return_value = this->get_expr_value(stmt.expr);
				this->builder.CreateRet(return_value);

			} break; default: {
				cmd::error("LLVMBuilder recieved unknown stmt ({})", (uint)stmt.type);
			} break;
		};

	};



	std::string LLVMBuilder::get_expr_id_str(AST::Expr* expr){
		return std::get<std::string>(static_cast<AST::Expr::Val*>(expr)->token.value);
	};



	llvm::Value* LLVMBuilder::get_expr_value(AST::Expr* expr){
		if(expr->type() == AST::Expr::Type::Val){
			auto* value = static_cast<AST::Expr::Val*>(expr);
			auto value_str = std::get<std::string>(value->token.value);

			switch(value->token.type){
				case Tokenizer::Token::Type::number: return llvm::ConstantInt::get(this->builder.getInt64Ty(), std::stoull(value_str));
				case Tokenizer::Token::Type::id:     return builder.CreateLoad(this->variables[value_str]->getAllocatedType(), this->variables[value_str], value_str);
				default: cmd::error("Unkown expr type");
			};
		}else if(expr->type() == AST::Expr::Type::LR){
			auto* lr = static_cast<AST::Expr::LR*>(expr);

			auto left = this->get_expr_value(lr->left);
			auto right = this->get_expr_value(lr->right);

			// just supporting adding for now
			return this->builder.CreateAdd(left, right);
		}else if(expr->type() == AST::Expr::Type::FuncCall){
			auto func_call = static_cast<AST::Expr::FuncCall*>(expr);
			auto func_id = this->get_expr_id_str(func_call->id);

			auto params = std::vector<llvm::Value*>();

			if(func_id == "printf"){
				params.push_back( builder.CreateGlobalStringPtr("num: %lli\n") );
			}

			for(auto* expr : func_call->exprs){
				params.push_back(this->get_expr_value(expr));
			}

			return this->builder.CreateCall(this->functions[func_id], params);
		}else{
			cmd::error("Recieved invalid expr value type ({})", (uint)expr->type());
		}

	};







	llvm::Type* LLVMBuilder::get_type(Tokenizer::Token token){
		switch(token.type){
			case TokenType::type_int: return this->types["i64"];
			case TokenType::type_void: return this->types["void"];
		};

		auto type_str = std::get<std::string>(token.value);

		if(!this->types.contains(type_str)){
			cmd::error("Recieved unknown type ({})", type_str);
		}

		return this->types[type_str];

	};


	
};
