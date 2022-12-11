#include "pch.h"



#pragma warning (push, 0)
// #include <llvm/ExecutionEngine/ExecutionEngine.h>
// #include <llvm/ExecutionEngine/GenericValue.h>
// #include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/IR/IRBuilder.h>

#pragma warning (pop)

#include <windows.h>

// #include "cpp_test.h"


void run_dll(){
	std::string library_path = "dll_test.dll";
	std::string func_name = "test";
	// int the_num = 12;


    HINSTANCE module_handle = LoadLibraryA(library_path.c_str());
	if(module_handle == NULL){
		// PH_FATAL("Unable to get process list");
		// return false;
	}



	int (*opfunc) ();
	// opfunc = GetProcAddress(lib_handle, "add_one");
	opfunc = ( int(*)() ) GetProcAddress(module_handle, func_name.c_str());


	if(opfunc == NULL){
		std::cout << "couldn't load\n";
	}


	opfunc();



	FreeLibrary(module_handle);
};







void save_ir_to_file(llvm::Module& module){
	std::error_code EC;
	auto out = llvm::raw_fd_ostream("output.ll", EC);
	
	module.print(out, nullptr);

};

void compile_ir(){
	system("llc -filetype=obj output.ll -o output.o");
	system("g++ -g output.o cpp_test.o -o output.exe");
};

void print_ir(llvm::Module& module){
	module.print(llvm::outs(), nullptr);
};

void run_ir(){
	system("lli output.ll");
};

void run_exe(){
	system("output.exe");
};


llvm::Function* cpp_string_fn;
llvm::Function* cpp_string_to_c_str_fn;


namespace Type{
	// built in

	llvm::IntegerType* i32;
	llvm::IntegerType* i64;

	llvm::PointerType* i8p;



	llvm::PointerType* c_str;

	namespace cpp{
		llvm::StructType* string;
	};



	void create(llvm::IRBuilder<>& builder){
		Type::i32 = builder.getInt32Ty();
		Type::i64 = builder.getInt64Ty();
		Type::i8p = builder.getInt8PtrTy();


		Type::c_str = Type::i8p;



		// define StringType
		Type::cpp::string = llvm::StructType::create(builder.getContext(), std::string("std::string"));
		Type::cpp::string->setBody(llvm::ArrayRef<llvm::Type*>({Type::i8p}));



	};

};



void test(llvm::IRBuilder<>& builder, llvm::Function* printf_fn){
	// auto format_str = builder.CreateGlobalStringPtr("hello world\n");



	{
		// print int
		auto num = llvm::ConstantInt::get(Type::i64, 12);
		auto format_str = builder.CreateGlobalStringPtr("num: %lli\n");

		builder.CreateCall(printf_fn, { format_str, num });
	}


	{
		// print string
		auto str_value = std::string("test string");
		auto string_ptr = builder.CreateGlobalStringPtr(str_value.c_str());

		auto cpp_string = builder.CreateCall(cpp_string_fn, {string_ptr});

		auto to_c_str = builder.CreateCall(cpp_string_to_c_str_fn, {cpp_string});

		auto format_str = builder.CreateGlobalStringPtr("str: %s\n");
		builder.CreateCall(printf_fn, { format_str, to_c_str });
	}



	{
		// variables


		auto num_256 = llvm::ConstantInt::get(Type::i64, 256);
		auto num_512 = llvm::ConstantInt::get(Type::i64, 512);


		
		llvm::AllocaInst* foo_alloca = builder.CreateAlloca(Type::i64, nullptr, "foo");
		auto foo_store = builder.CreateStore(num_256, foo_alloca);



		auto bar_load = builder.CreateLoad(foo_alloca->getAllocatedType(), foo_alloca, "bar");


		llvm::AllocaInst* bar_alloca = builder.CreateAlloca(Type::i64, nullptr, "bar");
		auto bar_store = builder.CreateStore(num_512, bar_alloca);



		auto foo_format_str = builder.CreateGlobalStringPtr("foo: %lli\n");
		auto bar_format_str = builder.CreateGlobalStringPtr("bar: %lli\n");


		auto foo_load = builder.CreateLoad(foo_alloca->getAllocatedType(), foo_alloca, "foo");
		bar_load = builder.CreateLoad(Type::i64, bar_alloca, "bar");

		builder.CreateCall(printf_fn, { bar_format_str, foo_load});
		builder.CreateCall(printf_fn, { foo_format_str, bar_load});


		// adding them together
		auto added = builder.CreateAdd(foo_load, bar_load, "<add>");
		foo_store = builder.CreateStore(added, foo_alloca);


		foo_load = builder.CreateLoad(foo_alloca->getAllocatedType(), foo_alloca, "foo");

		builder.CreateCall(printf_fn, { builder.CreateGlobalStringPtr("foo added: %lli\n"), foo_load});
	}

};



int llvm_main(){

	auto context = llvm::LLVMContext();
	llvm::IRBuilder<> builder(context);

	Type::create(builder);



	auto module = llvm::Module("llvm_test", context);



	// build a 'main' function
	auto prototype = llvm::FunctionType::get(Type::i32, false);
	llvm::Function* main_fn = llvm::Function::Create(prototype, llvm::Function::ExternalLinkage, "main", module);
	llvm::BasicBlock* body = llvm::BasicBlock::Create(context, "body", main_fn);
	builder.SetInsertPoint(body);



	// use libc's printf function
	auto printf_prototype = llvm::FunctionType::get(Type::cpp::string, true);
	auto printf_fn = llvm::Function::Create(printf_prototype, llvm::Function::ExternalLinkage, "printf", module);




	auto cpp_string_prototype = llvm::FunctionType::get(Type::cpp::string, llvm::ArrayRef<llvm::Type*>{Type::c_str}, false);
	cpp_string_fn = llvm::Function::Create(cpp_string_prototype, llvm::Function::ExternalLinkage, "cpp_string", module);

	auto cpp_string_to_c_str_prototype = llvm::FunctionType::get(Type::c_str, llvm::ArrayRef<llvm::Type*>{Type::cpp::string}, false);
	cpp_string_to_c_str_fn = llvm::Function::Create(cpp_string_to_c_str_prototype, llvm::Function::ExternalLinkage, "cpp_string_to_c_str", module);



	// call printf with our string
	// auto format_str = builder.CreateGlobalStringPtr("hello world\n");
	// builder.CreateCall(printf_fn, { format_str });



	test(builder, printf_fn);




	// return 0 from main
	auto ret = llvm::ConstantInt::get(Type::i32, 0);
	builder.CreateRet(ret);



	save_ir_to_file(module);
	compile_ir();

	print_ir(module);
	std::cout << "\n-------------------------\n\n";

	// run_ir();
	run_exe();
	// run_dll();

	std::cin.get();


	return 0;
};