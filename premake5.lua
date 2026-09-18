workspace "eclipse"
    startproject "eclipseeditor"
    architecture "ARM64"

    configurations
    {
        "Debug",
        "Release"
    }

tdir = "bin/%{cfg.buildcfg}/%{prj.name}"
odir = "bin-obj/%{cfg.buildcfg}/%{prj.name}"

-- External Dependencies
externals = {}
externals["spdlog"] = "external/spdlog"
externals["glad"] = "external/glad"

-- Process Glad before anything else
include "external/glad"

project "eclipse"
    location "eclipse"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir(tdir)
    objdir(odir)

    files
    {
        "%{prj.name}/include/**.h",
        "%{prj.name}/include/**.hpp",
        "%{prj.name}/include/**.cpp",
        "src/**.h",
        "src/**.cpp",
        "core/**.h",
        "core/**.cpp",
        "managers/**.h",
        "managers/**.cpp",
        "graphics/**.h",
        "graphics/**.cpp",
        "input/**.h",
        "input/**.cpp"
    }

    removefiles
    {
        "src/main.cpp"
    }

    sysincludedirs
    {
        "%{prj.name}/include",
        "%{externals.spdlog}/include",
        "%{externals.glad}/include",
        "/opt/homebrew/include",
        "/opt/homebrew/include/SDL2"
    }

    includedirs
    {
        "src",
        "core",
        "managers",
        "graphics",
        "input"
    }

    flags
    {
        "FatalWarnings"
    }

    defines
    {
        "GLFW_INCLUDE_NONE"
    }

    filter {"system:windows", "configurations:*"}
        systemversion "latest"

        defines
        {
            "ECLIPSE_PLATFORM_WINDOWS"
        }

    filter {"system:macosx", "configurations:*"}
        xcodebuildsettings
        {
            ["MACOSX_DEPLOYMENT_TARGET"] = "10.15",
            ["UseModernBuildSystem"] = "NO"
        }

        defines
        {
            "ECLIPSE_PLATFORM_MAC"
        }

    filter {"system:linux", "configurations:*"}
        defines
        {
            "ECLIPSE_PLATFORM_LINUX"
        }

    filter "configurations:Debug"
        defines
        {
            "ECLIPSE_CONFIG_DEBUG",
            "DEBUG"
        }
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines
        {
            "ECLIPSE_CONFIG_RELEASE",
            "NDEBUG"
        }
        runtime "Release"
        symbols "off"
        optimize "on"

project "eclipseeditor"
    location "eclipseeditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    links "eclipse"

    targetdir(tdir)
    objdir(odir)

    files
    {
        "src/main.cpp"
    }

    sysincludedirs
    {
        "eclipse/include",
        "%{externals.spdlog}/include",
        "/opt/homebrew/include",
        "/opt/homebrew/include/SDL2"
    }

    includedirs
    {
        "src",
        "core",
        "managers",
        "graphics",
        "input"
    }

    flags
    {
        "FatalWarnings"
    }

    filter {"system:windows", "configurations:*"}
        systemversion "latest"

        defines
        {
            "ECLIPSE_PLATFORM_WINDOWS"
        }

        links
        {
            "SDL2"
        }

    filter {"system:macosx", "configurations:*"}
        xcodebuildsettings
        {
            ["MACOSX_DEPLOYMENT_TARGET"] = "10.15",
            ["UseModernBuildSystem"] = "NO"
        }

        defines
        {
            "ECLIPSE_PLATFORM_MAC"
        }

        libdirs
        {
            "/opt/homebrew/lib"
        }

        linkoptions
        {
            "-Wl,-framework,Cocoa"
        }

        links
        {
            "SDL2",
            "SDL2main",
            "glad"
        }

    filter {"system:linux", "configurations:*"}
        defines
        {
            "ECLIPSE_PLATFORM_LINUX"
        }

        links
        {
            "SDL2",
            "glad",
            "dl"
        }

    filter "configurations:Debug"
        defines
        {
            "ECLIPSE_CONFIG_DEBUG",
            "DEBUG"
        }
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines
        {
            "ECLIPSE_CONFIG_RELEASE",
            "NDEBUG"
        }
        runtime "Release"
        symbols "off"
        optimize "on"
