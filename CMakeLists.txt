cmake_minimum_required(VERSION 3.20)
project(ProjectOverboss VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensure 64-bit architecture
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Project Overboss must be compiled for x64 architecture (Fallout4.exe is 64-bit).")
endif()

# Compiler optimization and warnings
if(MSVC)
    add_compile_options(
        /MP              # Multi-processor compilation
        /W3              # Warning level 3
        /permissive-     # Strict C++ conformance
        /EHsc            # Standard C++ exception handling (interops with SEH)
    )
endif()

if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

include(FetchContent)

# -----------------------------------------------------------------------------
# Dependency: MinHook (x86/x64 API Detours)
# -----------------------------------------------------------------------------
FetchContent_Declare(
    minhook
    GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
    GIT_TAG        v1.3.3
)
FetchContent_GetProperties(minhook)
if(NOT minhook_POPULATED)
    FetchContent_Populate(minhook)

    add_library(minhook STATIC
        ${minhook_SOURCE_DIR}/src/buffer.c
        ${minhook_SOURCE_DIR}/src/hook.c
        ${minhook_SOURCE_DIR}/src/trampoline.c
        ${minhook_SOURCE_DIR}/src/hde/hde32.c
        ${minhook_SOURCE_DIR}/src/hde/hde64.c
    )

    target_include_directories(minhook PUBLIC
        ${minhook_SOURCE_DIR}/include
        ${minhook_SOURCE_DIR}/src
        ${minhook_SOURCE_DIR}/src/hde
    )
endif()

# -----------------------------------------------------------------------------
# Dependency: Dear ImGui (Docking / DX11 & Win32 Backends)
# -----------------------------------------------------------------------------
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.90.4
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)

    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
    )

    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )

    target_link_libraries(imgui PUBLIC
        d3d11
        dxgi
    )
endif()

# -----------------------------------------------------------------------------
# Target: ProjectOverboss.dll
# -----------------------------------------------------------------------------
add_library(ProjectOverboss SHARED
    src/dllmain.cpp
    src/engine/memory.h
    src/engine/memory.cpp
    src/engine/creation_bridge.h
    src/engine/creation_bridge.cpp
    src/hooks/dx11_hook.h
    src/hooks/dx11_hook.cpp
    src/gui/gui.h
    src/gui/gui.cpp
    src/features/features.h
    src/features/vitals.cpp
    src/features/armory.cpp
    src/features/power_armor.cpp
    src/features/settlement.cpp
    src/features/world.cpp
    src/features/radar.h
    src/features/radar.cpp
)

target_include_directories(ProjectOverboss PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${minhook_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(ProjectOverboss PRIVATE
    minhook
    imgui
    d3d11
    dxgi
    psapi
    user32
    gdi32
)

set_target_properties(ProjectOverboss PROPERTIES
    PREFIX ""
    OUTPUT_NAME "ProjectOverboss"
)
