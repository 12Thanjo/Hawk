#include "pch.h"
#include "playground.h"


#pragma warning (push, 0)
	#include <llvm/IR/IRBuilder.h>
#pragma warning (pop)


namespace Hawk{


	#define TYPE_I8 builder.getInt8PtrTy()
	#define TYPE_I32 builder.getInt32Ty()
	#define TYPE_I64 builder.getInt64Ty()


	void playground(){
		

		// auto context = llvm::LLVMContext();
		// llvm::IRBuilder<> builder(context);


		// auto module = llvm::Module("llvm_test", context);


		// // auto *GV = new GlobalVariable(module, TYPE_I64, true, GlobalValue::PrivateLinkage, StrConstant, Name, nullptr, GlobalVariable::NotThreadLocal, AddressSpace);
		// auto *GV = new llvm::GlobalVariable(module, TYPE_I64, true, llvm::GlobalValue::PrivateLinkage, llvm::ConstantInt::get(TYPE_I64, 123456), "|--GlobalI64--|");
		// GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
		// GV->setAlignment(llvm::Align(1));

		// // create main
		// {
		// 	auto prototype = llvm::FunctionType::get(TYPE_I32, false);
		// 	llvm::Function* main_fn = llvm::Function::Create(prototype, llvm::Function::ExternalLinkage, "main", module);
		// 	llvm::BasicBlock* body = llvm::BasicBlock::Create(context, "body", main_fn);
		// 	builder.SetInsertPoint(body);
		// }


		// // use libc's printf function
		// auto printf_prototype = llvm::FunctionType::get(TYPE_I8, true);
		// auto printf_fn = llvm::Function::Create(printf_prototype, llvm::Function::ExternalLinkage, "printf", module);
		// auto format_str = builder.CreateGlobalStringPtr("num: %lli\n");


		// llvm::AllocaInst* foo_alloca = builder.CreateAlloca(TYPE_I64, nullptr, "foo");
		// {
		// 	auto foo_store = builder.CreateStore(llvm::ConstantInt::get(TYPE_I64, 256), foo_alloca);

		// 	auto num = builder.CreateLoad(foo_alloca->getAllocatedType(), foo_alloca, "foo");

		// 	builder.CreateCall(printf_fn, { format_str, num });
		// }





		// auto Vec3 = llvm::StructType::create(builder.getContext(), std::string("Vec3"));
		// Vec3->setBody(llvm::ArrayRef<llvm::Type*>({TYPE_I64, TYPE_I64, TYPE_I64}));

		// {

		// 	// llvm::AllocaInst* vec3_alloca = builder.CreateAlloca(Vec3, nullptr, "first_vec3");
		// 	// auto vec3_store = builder.CreateStore(llvm::ConstantInt::get(TYPE_I64, 101), vec3_alloca);
		// 	// auto vec3_load = builder.CreateLoad(Vec3, vec3_alloca, "vec3_load");



		// 	// auto y = builder.CreateStructGEP(Vec3, vec3_load, 0);
		// 	// builder.CreateInsertElement(vec3_load, llvm::ConstantInt::get(TYPE_I64, 101), 0);
		// 	// builder.CreateInsertElement(vec3_load, llvm::ConstantInt::get(TYPE_I64, 202), 1);
		// 	// builder.CreateInsertElement(vec3_load, llvm::ConstantInt::get(TYPE_I64, 303), 2);


		// 	// auto extract_y = builder.CreateExtractElement(vec3_load, 1);
		// 	// builder.CreateCall(printf_fn, { format_str, extract_y });

		// 	// {

		// 		llvm::Value* member_index = llvm::ConstantInt::get(context, llvm::APInt(32, 0 /*The index of the member*/, true));
		// 		llvm::Value* data = llvm::ConstantInt::get(TYPE_I64, 101);

		// 		llvm::AllocaInst* alloc = builder.CreateAlloca(Vec3, 0, "alloctmp");
		// 		builder.CreateStore(data, alloc);

		// 	// }


		// 	// // {

		// 	// 	llvm::Value* member_index = llvm::ConstantInt::get(context, llvm::APInt(32, 1 /*The index of the member*/, true));
		// 	// 	llvm::Value* data = llvm::ConstantInt::get(TYPE_I64, 202);

		// 	// 	llvm::AllocaInst* alloc = builder.CreateAlloca(Vec3, 0, "alloctmp");
		// 	// 	builder.CreateStore(data, alloc);

		// 	// // }







		// 	std::vector<llvm::Value*> indices(2);
		// 	indices[0] = llvm::ConstantInt::get(context, llvm::APInt(32, 0, true));
		// 	indices[1] = member_index;

		// 	llvm::Value* member_ptr = builder.CreateGEP(Vec3, alloc, indices, "memberptr");
		// 	llvm::Value* loaded_member = builder.CreateLoad(TYPE_I64, member_ptr, "loadtmp");


		// 	indicies[1] = llvm::ConstantInt::get(context, llvm::APInt(32, 1, true));
		// 	llvm::AllocaInst* alloc = builder.CreateAlloca(Vec3, 0, "alloctmp");


		// 	builder.CreateCall(printf_fn, { format_str, loaded_member });

		// }






		// // if else
		// {
		// 	llvm::Value* cond_value = llvm::ConstantInt::get(TYPE_I64, 0);
		// 	auto condition = builder.CreateICmpEQ(cond_value, llvm::ConstantInt::get(TYPE_I64, 0), "|--ifcond--|");

		// 	// is this the same as the main func?
		// 	llvm::Function* the_func = builder.GetInsertBlock()->getParent();

		// 	llvm::BasicBlock* then_block = llvm::BasicBlock::Create(context, "|--then--|", the_func);
		// 	llvm::BasicBlock* else_block = llvm::BasicBlock::Create(context, "|--else--|");
		// 	llvm::BasicBlock* merge_block = llvm::BasicBlock::Create(context, "|--ifcont--|");
		// 	llvm::BasicBlock* return_block = llvm::BasicBlock::Create(context, "|--return--|");

		// 	builder.CreateCondBr(condition, then_block, else_block);


		// 	//////////////////////////////////////////////////////////////////////
		// 	// then

		// 	builder.SetInsertPoint(then_block);

		// 		auto then_num = llvm::ConstantInt::get(TYPE_I64, 12);
		// 		builder.CreateCall(printf_fn, { format_str, then_num });

		// 	builder.CreateBr(return_block);

		// 	// Codegen of "then" can change the current block, so update the then_block
		// 	then_block = builder.GetInsertBlock();


		// 	//////////////////////////////////////////////////////////////////////
		// 	// else

		// 	the_func->getBasicBlockList().push_back(else_block);
		// 	builder.SetInsertPoint(else_block);

		// 		auto else_num = llvm::ConstantInt::get(TYPE_I64, 65);
		// 		builder.CreateCall(printf_fn, { format_str, else_num });

		// 	builder.CreateBr(merge_block);

		// 	// Codegen of "else" can change the current block, so update the else_block
		// 	else_block = builder.GetInsertBlock();


		// 	//////////////////////////////////////////////////////////////////////
		// 	// emit merge

		// 	the_func->getBasicBlockList().push_back(merge_block);
		// 	builder.SetInsertPoint(merge_block);

		// 	// llvm::PHINode* PN = builder.CreatePHI(TYPE_I64, 2, "|--iftmp--|");

		// 	// PN->addIncoming(then_num, then_block);
		// 	// PN->addIncoming(else_num, else_block);


		// 	// builder.CreateCall(printf_fn, { format_str, PN });

		// 	builder.CreateCall(printf_fn, { format_str, llvm::ConstantInt::get(TYPE_I64, 4) });

		// 	builder.CreateBr(return_block);

		// 	the_func->getBasicBlockList().push_back(return_block);
		// 	builder.SetInsertPoint(return_block);
		// }





		// // main return
		// {

		// 	auto ret = llvm::ConstantInt::get(TYPE_I32, 0);
		// 	builder.CreateRet(ret);
		// }


		// cmd::log("\n----------------------------------------\n");

		// // print
		// module.print(llvm::outs(), nullptr);

		// std::error_code EC;
		// auto out = llvm::raw_fd_ostream("output.ll", EC);
			

		// module.print(out, nullptr);

		// cmd::log("\n----------------------------------------\n");

		// system("lli output.ll");

	};

	
}

