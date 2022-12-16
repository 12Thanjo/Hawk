# Hawk

An experimental programming language with an LLVM backend. Syntax is not final as this is very much experimental, and things may change / evolve over time.


## My idea:
I'm creating this language with the following in mind. Hopefully I can get all of these points, but we shall see :)


### Features:
- support both static and runtime code (with no built in interpreter)
- good compile time
- good optimization (some code automatically run at compile time)
- no garbage collector
- minimal to no preprcessor
- built in static analysis / bug finder


### Guidelines:
- should be fun to use
- good and readable error messages
- "simple things should be simple" (Bjarne Stroustrup), it should be easy to teach a beginner
- minimize the ammount of boilerplate needed


## Building:
Over time building this project should become easier, but for now here is what is necessary:

- sadly only Windows is supported, at least for now
- download Visual Studio 2022
- download gcc (MinGW64)
- build / install llvm, and add it to your environment path
- install premake5, and (maybe) add it to your environment path
- run the premake script `premake5 vs2022`
- open solution in Visual Studio and build



### Updates:
The changelog can be found [here](https://github.com/12Thanjo/Hawk/blob/main/CHANGELOG.md).
