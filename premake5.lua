-- premake5.lua
workspace "hawk"
	architecture "x64"
	startproject "hawk"

	configurations {
		"Debug",
		"Release",
	}


	flags{
		"MultiProcessorCompile"
	}


	outputdir = "%{cfg.buildcfg}"


--------------------------------------------------------------------------------------------


project "hawk"
	location "src"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"



	targetdir ("bin/bin/" .. outputdir)
	objdir ("bin/intermediates/" .. outputdir)


	pchheader "pch.h"
	pchsource "src/pch.cpp"


	files{
		"src/**.h",
		"src/**.cpp",
	}


	includedirs{
		"C:/Program Files (x86)/LLVM/include",
		
	}


	links{
		"C:/Program Files (x86)/LLVM/lib/**.lib",
	}

	linkoptions {
		"-IGNORE:4099",
	}

	defines{
		"_SILENCE_ALL_CXX20_DEPRECATION_WARNINGS",
	}



	filter "configurations:Debug"
		symbols "on"
		buildoptions "/MDd"
		