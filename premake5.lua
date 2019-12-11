-- premake5.lua

function os.winSdkVersion()
    local reg_arch = iif( os.is64bit(), "\\Wow6432Node\\", "\\" )
    local sdk_version = os.getWindowsRegistry( "HKLM:SOFTWARE" .. reg_arch .."Microsoft\\Microsoft SDKs\\Windows\\v10.0\\ProductVersion" )
    if sdk_version ~= nil then return sdk_version end
end

-- ///////////////////////////////////////

workspace "SudokuGuider"
   filter {"system:windows", "action:vs*"}
      systemversion(os.winSdkVersion() .. ".0")

   filename "SudokuGuider"
   language "C++"
   configurations { "Debug", "Release" }
   platforms { "Static", "DLL" }
   location "local"

   filter { "platforms:Static" }
       kind "StaticLib"
       defines { "COMPILING_STATIC" }

   filter { "platforms:DLL" }
       kind "SharedLib"
       defines { "COMPILING_DLL" }

   filter { "configurations:Debug" }
      defines { "DD_DEBUG", "DEBUG" }
      runtime "Debug"
      staticruntime "on"
      symbols "On"

   filter {"configurations:Release"}
      defines { "DD_RELEASE", "RELEASE" }
      runtime "Release"      
      staticruntime "on"
      optimize "On"
      symbols "On"

project "Core"
   targetdir "local/Core/%{cfg.buildcfg}"
   includedirs { "source/Core/Public/" }

   files { "source/Core/**.h", "source/Core/**.cpp" }
   removefiles {}

project "SudokuAlgorithms"
   targetdir "local/SudokuAlgorithms/%{cfg.buildcfg}"

   links { "Core" }
   includedirs { "source/SudokuAlgorithms/Public/", "source/Core/Public/" }

   files { "source/SudokuAlgorithms/**.h", "source/SudokuAlgorithms/**.cpp" }

project "Main"
   kind "ConsoleApp"
   targetdir "local/Main/%{cfg.buildcfg}"
   -- removeplatforms { "Static", "DLL" }

   links { "Core", "SudokuAlgorithms" }
   includedirs { "source/SudokuAlgorithms/Public/", "source/Core/Public/" }

   files { "source/Main/**.h", "source/Main/**.cpp" }
