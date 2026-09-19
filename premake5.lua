print("[premake] generating project files...")

newoption {
    trigger = "sdl2-root",
    value = "PATH",
    description = "SDL2 installation prefix"
}

newoption {
    trigger = "glm-root",
    value = "PATH",
    description = "GLM installation prefix"
}

newoption {
    trigger = "freetype-root",
    value = "PATH",
    description = "FreeType installation prefix"
}

newoption {
    trigger = "python",
    value = "COMMAND",
    description = "Python command used for asset packaging"
}

local root = path.getabsolute(".")
local action = _ACTION or "projects"
local sdlRoot = _OPTIONS["sdl2-root"] or os.getenv("SDL2_ROOT")
local glmRoot = _OPTIONS["glm-root"] or os.getenv("GLM_ROOT")
local freetypeRoot = _OPTIONS["freetype-root"] or os.getenv("FREETYPE_ROOT")
local python = _OPTIONS["python"] or os.getenv("ECLIPSE_PYTHON") or "python3"

workspace "eclipse"
    location(path.join("build", action))
    startproject "eclipseeditor"
    configurations { "Debug", "Release" }

local function configureProject()
    language "C++"
    cppdialect "C++17"
    targetdir(path.join(root, "bin/%{cfg.buildcfg}/%{prj.name}"))
    objdir(path.join(root, "bin-obj/%{cfg.buildcfg}/%{prj.name}"))

    includedirs {
        root,
        path.join(root, "eclipse/include"),
        path.join(root, "external/box2d/include"),
        path.join(root, "external/glad/include"),
        path.join(root, "external/spdlog/include")
    }
    if sdlRoot then
        includedirs { path.join(sdlRoot, "include"), path.join(sdlRoot, "include/SDL2") }
        libdirs { path.join(sdlRoot, "lib") }
    end
    if glmRoot then
        includedirs { path.join(glmRoot, "include") }
    end
    if freetypeRoot then
        includedirs { path.join(freetypeRoot, "include/freetype2") }
        libdirs { path.join(freetypeRoot, "lib") }
    end

    filter "system:windows"
        architecture "x86_64"
        systemversion "latest"
        defines { "ECLIPSE_PLATFORM_WINDOWS", "_CRT_SECURE_NO_WARNINGS" }
        buildoptions { "/utf-8" }
        staticruntime "off"
    filter "system:macosx"
        defines { "ECLIPSE_PLATFORM_MAC" }
        xcodebuildsettings { ["MACOSX_DEPLOYMENT_TARGET"] = "10.15" }
    filter "system:linux"
        defines { "ECLIPSE_PLATFORM_LINUX" }
    filter "configurations:Debug"
        defines { "ECLIPSE_CONFIG_DEBUG", "DEBUG" }
        runtime "Debug"
        symbols "on"
    filter "configurations:Release"
        defines { "ECLIPSE_CONFIG_RELEASE", "NDEBUG" }
        runtime "Release"
        optimize "on"
    filter {}
end

local function configureApp(assetGroup)
    kind "ConsoleApp"
    configureProject()
    links { "eclipse", "box2d", "freetype" }

    filter "system:windows"
        links { "SDL2", "SDL2main", "opengl32" }
    filter "system:macosx"
        links { "SDL2", "SDL2main", "Cocoa.framework", "OpenGL.framework" }
    filter "system:linux"
        links { "SDL2", "GL", "dl" }
    filter {}

    local command = python .. ' "' .. path.join(root, "tools/package_assets.py") .. '" "' ..
        path.join(root, "assets") .. '" "%{cfg.targetdir}/assets"'
    if assetGroup then
        command = command .. " " .. assetGroup
    end
    postbuildcommands { command }

    if sdlRoot then
        filter "system:windows"
            postbuildcommands {
                '{COPYFILE} "' .. path.join(sdlRoot, "bin/SDL2.dll") .. '" "%{cfg.targetdir}"'
            }
        filter {}
    end
end

project "box2d"
    location(path.join("build", action, "box2d"))
    kind "StaticLib"
    language "C"
    targetdir(path.join(root, "bin/%{cfg.buildcfg}/%{prj.name}"))
    objdir(path.join(root, "bin-obj/%{cfg.buildcfg}/%{prj.name}"))
    files { "external/box2d/include/**.h", "external/box2d/src/**.h",
            "external/box2d/src/**.c" }
    includedirs { path.join(root, "external/box2d/include"),
                  path.join(root, "external/box2d/src") }
    filter "system:windows"
        architecture "x86_64"
        systemversion "latest"
        defines { "_CRT_SECURE_NO_WARNINGS" }
        buildoptions { "/std:c17" }
        staticruntime "off"
    filter "system:not windows"
        buildoptions { "-std=c17" }
    filter "configurations:Debug"
        symbols "on"
    filter "configurations:Release"
        optimize "on"
    filter {}

project "eclipse"
    location(path.join("build", action, "eclipse"))
    kind "StaticLib"
    configureProject()
    links { "box2d" }
    files {
        "src/**.h", "src/**.cpp",
        "core/**.h", "core/**.cpp",
        "managers/**.h", "managers/**.cpp",
        "graphics/**.h", "graphics/**.cpp",
        "input/**.h", "input/**.cpp",
        "ecs/**.h", "ecs/**.cpp",
        "components/**.h", "components/**.cpp",
        "systems/**.h", "systems/**.cpp",
        "eclipse/include/**.h", "eclipse/include/**.hpp",
        "eclipse/include/external/imgui/*.cpp",
        "external/glad/src/glad.c"
    }
    removefiles { "src/main.cpp", "eclipse/include/external/imgui/imgui_demo.cpp" }
    warnings "Extra"
    filter "toolset:msc"
        buildoptions { "/WX" }
    filter "toolset:not msc"
        buildoptions { "-Werror" }
    filter {}

project "eclipseeditor"
    location(path.join("build", action, "eclipseeditor"))
    configureApp(nil)
    files { "src/main.cpp", "editor/**.h", "editor/**.cpp" }

project "snakegame"
    location(path.join("build", action, "snakegame"))
    configureApp("snake")
    files { "games/snake/**.h", "games/snake/**.cpp" }

project "tetrisgame"
    location(path.join("build", action, "tetrisgame"))
    configureApp("tetris")
    files { "games/tetris/**.h", "games/tetris/**.cpp" }

project "eclipsetests"
    location(path.join("build", action, "eclipsetests"))
    kind "ConsoleApp"
    configureProject()
    links { "eclipse", "box2d", "freetype" }
    files { "tests/**.h", "tests/**.cpp" }
    filter "system:windows"
        links { "SDL2", "opengl32" }
    filter "system:macosx"
        links { "SDL2", "Cocoa.framework", "OpenGL.framework" }
    filter "system:linux"
        links { "SDL2", "GL", "dl" }
    filter {}
    if sdlRoot then
        filter "system:windows"
            postbuildcommands {
                '{COPYFILE} "' .. path.join(sdlRoot, "bin/SDL2.dll") .. '" "%{cfg.targetdir}"'
            }
        filter {}
    end
