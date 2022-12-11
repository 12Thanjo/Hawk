#include "pch.h"
#include "LLVMBuilder.h"





namespace Hawk{

	LLVMBuilder::LLVMBuilder(std::vector<AST::Stmt>& statements, const std::string& module_name)
		: stmts(statements),
		context(), builder(context), module(module_name, context)
	{};
	

	LLVMBuilder::~LLVMBuilder(){
		// for(auto [id, func] : this->functions){
		// 	delete func;
		// }
	};





	void LLVMBuilder::build_ir(){
		auto prototype = llvm::FunctionType::get(this->builder.getInt32Ty(), false);
		llvm::Function* main_fn = llvm::Function::Create(prototype, llvm::Function::ExternalLinkage, "main", this->module);
		llvm::BasicBlock* body = llvm::BasicBlock::Create(this->context, "body", main_fn);
		this->builder.SetInsertPoint(body);



		// use libc's printf function
		auto printf_prototype = llvm::FunctionType::get(builder.getInt8PtrTy(), true);
		this->functions["printf"] = llvm::Function::Create(printf_prototype, llvm::Function::ExternalLinkage, "printf", module);



		// auto format_str = this->builder.CreateGlobalStringPtr("hello world\n");
		// this->builder.CreateCall(printf_fn, { format_str });

		this->parse_stmts();

		auto ret = llvm::ConstantInt::get(this->builder.getInt32Ty(), 0);
		this->builder.CreateRet(ret);
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





	void LLVMBuilder::parse_stmts(){
		for(auto& stmt : this->stmts){

			switch(stmt.type){
				case AST::Stmt::Type::func_call: {
					auto func_call = static_cast<AST::Expr::FuncCall*>(stmt.expr);
					auto func_id = this->get_expr_id_str(func_call->id);

					auto llvm_num = this->get_expr_value(func_call->exprs[0]);

					auto format_str = builder.CreateGlobalStringPtr("num: %lli\n");

					this->builder.CreateCall(this->functions[func_id], llvm::ArrayRef<llvm::Value*>{ format_str, llvm_num });
				} break; case AST::Stmt::Type::assign: {
					auto* lr = static_cast<AST::Expr::LR*>(stmt.expr);
					std::string id = this->get_expr_id_str(lr->left);
					auto llvm_num = this->get_expr_value(lr->right);

					auto alloca = this->builder.CreateAlloca(this->builder.getInt64Ty(), nullptr, id);
					this->builder.CreateStore(llvm_num, alloca);

					this->variables[id] = alloca;
				} break; default: {
					cmd::error("LLVMBuilder recieved unknown stmt");
				} break;
			};


		}
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
		}

	};

	
};
