cmake_minimum_required(VERSION 3.10)
project("LCEMP")

set(LCEMP_WORKING_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Assets" CACHE STRING "The working directory for MinecraftClient")

set(CMAKE_CXX_STANDARD 11)

if(NOT WIN32)
    message(FATAL_ERROR "Windows is currently the only supported OS")
endif()

# Fix linking errors with other libs compiled with an older CRT
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Sources.cmake")

list(TRANSFORM MINECRAFT_WORLD_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.World/")
list(TRANSFORM MINECRAFT_CLIENT_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/")

add_library(MinecraftWorld STATIC ${MINECRAFT_WORLD_SOURCES})
add_executable(MinecraftClient WIN32 ${MINECRAFT_CLIENT_SOURCES})

set_property(TARGET MinecraftClient PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${LCEMP_WORKING_DIR}")

if(MSVC)
    # /MT and /MTd options set the CRT version to multi-threaded static mode
    # which is what the 4J libs were compiled with
    target_compile_options(MinecraftWorld PRIVATE /W3 /MP $<$<CONFIG:Debug>:/MTd> $<$<NOT:$<CONFIG:Debug>>:/MT> /EHsc)
    target_compile_options(MinecraftClient PRIVATE /W3 /MP $<$<CONFIG:Debug>:/MTd> $<$<NOT:$<CONFIG:Debug>>:/MT> /EHsc)
endif()

target_compile_definitions(MinecraftWorld PRIVATE
        $<$<CONFIG:Debug>:_LARGE_WORLDS;_DEBUG_MENUS_ENABLED;_DEBUG;_LIB;_CRT_NON_CONFORMING_SWPRINTFS;_CRT_SECURE_NO_WARNINGS;_WINDOWS64>
        $<$<NOT:$<CONFIG:Debug>>:_LARGE_WORLDS;_DEBUG_MENUS_ENABLED;_LIB;_CRT_NON_CONFORMING_SWPRINTFS;_CRT_SECURE_NO_WARNINGS;_WINDOWS64>
)

target_compile_definitions(MinecraftClient PRIVATE
        $<$<CONFIG:Debug>:_LARGE_WORLDS;_DEBUG_MENUS_ENABLED;_DEBUG;_CRT_NON_CONFORMING_SWPRINTFS;_CRT_SECURE_NO_WARNINGS;_WINDOWS64>
        $<$<NOT:$<CONFIG:Debug>>:_LARGE_WORLDS;_DEBUG_MENUS_ENABLED;_CRT_NON_CONFORMING_SWPRINTFS;_CRT_SECURE_NO_WARNINGS;_WINDOWS64>
)

target_include_directories(MinecraftWorld PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.World"
        "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.World/x64headers"
)

target_include_directories(MinecraftClient PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client"
        "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/Iggy/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Xbox/Sentient/Include"
)

target_link_libraries(MinecraftClient PRIVATE
        MinecraftWorld
        d3d11
        XInput9_1_0
        "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/Iggy/lib/iggy_w64.lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/Miles/lib/mss64.lib"
        $<$<CONFIG:Debug>:
        "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/4JLibs/libs/4J_Input_d.lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/4JLibs/libs/4J_Storage_d.lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/4JLibs/libs/4J_Profile_d.lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/4JLibs/libs/4J_Render_PC_d.lib"
        >
        $<$<NOT:$<CONFIG:Debug>>:
        "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/4JLibs/libs/4J_Input_r.lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/4JLibs/libs/4J_Storage_r.lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/4JLibs/libs/4J_Profile_r.lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/Minecraft.Client/Windows64/4JLibs/libs/4J_Render_PC.lib"
        >
)