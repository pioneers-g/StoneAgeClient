cmake_minimum_required(VERSION 3.15)
project(StoneAgeClient VERSION 9.0.6.1 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Find vcpkg toolchain if not already set
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# Windows specific settings
if(WIN32)
    set(CMAKE_WIN32_EXECUTABLE TRUE)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DUNICODE)
    add_definitions(-D_UNICODE)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

# Compiler-specific options
if(MSVC)
    # MSVC options
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MDd /Zi /Od")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD /O2 /GL")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /bigobj /utf-8 /W3")

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /Zi /Od")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /O2 /GL")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /utf-8 /W3")

    # Link-time optimization for release
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "ClangCl")
    # Clang options
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
else()
    # GCC or other
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
endif()

# Collect source files automatically
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

file(GLOB_RECURSE HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

# Exclude duplicate/legacy files that were split into modules
set(EXCLUDE_FILES
    # Old pet AI files (split into pet_ai.c)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pet/petai.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pet/petai_decision.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pet/petai_settings.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pet/petaisettings.c"

    # Old protocol files (functionality moved to network_*)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/protocol.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/protocol_binary.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/protocol_handler.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/protocol_handler_data.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/protocol_handler_sync.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/protocol_util.c"

    # Old render files (split into render_*)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/render/render_queue_process.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/render/render_surface.c"

    # Old sprite files (split into sprite_*)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sprite/sprite_cache.c"

    # Old NPC files (duplicate functionality)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/npc/npc_shop.c"

    # Old pet protocol (duplicates in network_protocol)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pet/pet_protocol.c"

    # Old save files (renamed)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/save/save.c"

    # Old UI files (renamed/split)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/window.c"

    # Duplicate winmain
    "${CMAKE_CURRENT_SOURCE_DIR}/src/winmain.c"

    # Old util files (moved)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/fileutil.c"

    # Duplicate item functions (moved to inventory.c)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/item/item.c"

    # Duplicate map functions
    "${CMAKE_CURRENT_SOURCE_DIR}/src/map/map_core.c"

    # Duplicate npc functions
    "${CMAKE_CURRENT_SOURCE_DIR}/src/npc/npc_packet.c"

    # Duplicate isometric functions
    "${CMAKE_CURRENT_SOURCE_DIR}/src/map/isometric.c"

    # Duplicate font (in render.c)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/font.c"

    # Duplicate graphics functions (in directx_core.c)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/graphics.c"

    # Duplicate sprite functions
    "${CMAKE_CURRENT_SOURCE_DIR}/src/directx/directx_sprite.c"

    # Duplicate ui (g_chat in chat.c)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ui.c"

    # Duplicate battle files
    "${CMAKE_CURRENT_SOURCE_DIR}/src/battle/battle_unit.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/battle/battle_action_render.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/battle/battle_action_exec.c"

    # Duplicate gameconfig (g_config is in main.c)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game/gameconfig.c"

    # Old entity stubs (split into stubs_entity_core.c, stubs_entity_slot.c, stubs_entity_character.c)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/stubs/stubs_entity.c"

    # Old skill.c (split into skill_core.c, skill_casting.c)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/skill/skill.c"

    # Old party.c (split into party_core.c, party_module.c)
    "${CMAKE_CURRENT_SOURCE_DIR}/src/party/party.c"
)

list(REMOVE_ITEM SOURCES ${EXCLUDE_FILES})

# Create executable
add_executable(sa_client WIN32 ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(sa_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries - use DirectX libraries from Windows SDK
target_link_libraries(sa_client PRIVATE
    user32
    gdi32
    kernel32
    ws2_32
    winmm
    dsound
    dinput8
    ddraw
    advapi32
    ole32
    imm32
    shell32
    dxguid
)

# Resource file
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res/resource.rc")
    target_sources(sa_client PRIVATE res/resource.rc)
endif()

# Output directory
set_target_properties(sa_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    OUTPUT_NAME "sa_9061"
)

# Enable testing
enable_testing()

# Add test subdirectory if tests exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()
