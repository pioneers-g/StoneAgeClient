# Stone Age Client - Test CMakeLists.txt
cmake_minimum_required(VERSION 3.15)

# Test executable list
set(TEST_SOURCES
    test_pet_ai.c
    test_protocol_util.c
    test_stringutil.c
    test_game_data.c
    test_map_entry.c
    test_battle_unit.c
    test_character_protocol.c
    test_skill_protocol.c
    test_pet_data.c
    test_isometric.c
    test_battle_action_exec.c
    test_pathfind.c
    test_battle_calc.c
    test_sprite.c
    test_sound.c
    test_input.c
    test_network_core.c
    test_ui_window.c
    test_font_render.c
    test_sprite_loader.c
    test_pet_ai_comprehensive.c
    test_text_protocol.c
    test_shop.c
    test_map.c
    test_battle.c
    test_gamestate.c
    test_animation.c
    test_login.c
    test_network_protocol.c
    test_fade.c
    test_chat.c
    test_item.c
    test_save.c
    test_des_crypto.c
    test_petai.c
    test_pet_protocol.c
    test_shop_protocol.c
    test_mail_protocol.c
    test_render_queue.c
    test_item_protocol.c
    test_ui_window_comprehensive.c
    test_input_system_comprehensive.c
    test_bgm_loading_comprehensive.c
    test_sound_system_comprehensive.c
    test_render_queue_comprehensive.c
    test_battle_map_load_comprehensive.c
    test_save_data_comprehensive.c
    test_isometric_render_comprehensive.c
    test_text_render_comprehensive.c
    test_character_movement_comprehensive.c
    test_npc_interaction_comprehensive.c
    test_pet_capture_comprehensive.c
    test_battle_action_dispatcher_comprehensive.c
    test_battle_handlers_comprehensive.c
    test_binary_packet_dispatcher_comprehensive.c
    test_string_parsing_comprehensive.c
    test_map_loading_comprehensive.c
    test_battle_state_machine_comprehensive.c
    test_element_system_comprehensive.c
    test_guild_system_comprehensive.c
    test_guild_module_comprehensive.c
    test_entity_system_comprehensive.c
    test_entity_action_system_comprehensive.c
    test_entity_movement_system_comprehensive.c
    test_field_update_comprehensive.c
    test_battle_action_executor_comprehensive.c
    test_text_protocol_system_comprehensive.c
    test_main_game_loop_comprehensive.c
    test_item_protocol_comprehensive.c
    test_network_io_comprehensive.c
    test_sprite_rle_comprehensive.c
    test_game_state_machine_comprehensive.c
    test_mail_protocol_comprehensive.c
    test_shop_ui_system_comprehensive.c
    test_shop_protocol_dispatcher_comprehensive.c
    test_login_system_comprehensive.c
    test_binary_protocol_dispatcher_comprehensive.c
    test_character_select_system_comprehensive.c
    test_pet_ai_settings_system_comprehensive.c
    test_party_system_comprehensive.c
    test_party_module_comprehensive.c
    test_skill_system_comprehensive.c
    test_skill_module_comprehensive.c
    test_trade_system_comprehensive.c
    test_trade_module_comprehensive.c
    test_inventory_system_comprehensive.c
    test_storage_module_comprehensive.c
    test_quest_module_comprehensive.c
    test_chat_protocol_module_comprehensive.c
    test_battle_action_attack_module_comprehensive.c
    test_effect_system_comprehensive.c
    test_event_system_comprehensive.c
    test_debug_effect.c
    test_debug_death.c
    test_animation_system_comprehensive.c
    test_craft_comprehensive.c
    test_social_comprehensive.c
    test_config_module_comprehensive.c
)

# Create test executables
foreach(TEST_SRC ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)

    # Check if file exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_SRC}")
        add_executable(${TEST_NAME} ${TEST_SRC})

        # Set as console application (not GUI)
        set_target_properties(${TEST_NAME} PROPERTIES WIN32_EXECUTABLE FALSE)

        # Include directories
        target_include_directories(${TEST_NAME} PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}
        )

        # Link against main project library if needed
        if(TARGET sa_client)
            # Standalone tests don't need the main library
        endif()

        # Platform-specific libraries
        if(WIN32)
            target_link_libraries(${TEST_NAME} PRIVATE
                ws2_32
                user32
                gdi32
            )
        endif()

        # Add test
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endif()
endforeach()

# Special handling for tests that need additional source files
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_protocol_util.c" AND EXISTS "${CMAKE_SOURCE_DIR}/src/protocol/protocol_util.c")
    target_sources(test_protocol_util PRIVATE ${CMAKE_SOURCE_DIR}/src/protocol/protocol_util.c)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_stringutil.c" AND EXISTS "${CMAKE_SOURCE_DIR}/src/util/stringutil.c")
    target_sources(test_stringutil PRIVATE ${CMAKE_SOURCE_DIR}/src/util/stringutil.c)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_game_data.c")
    if(EXISTS "${CMAKE_SOURCE_DIR}/src/protocol/protocol_util.c")
        target_sources(test_game_data PRIVATE ${CMAKE_SOURCE_DIR}/src/protocol/protocol_util.c)
    endif()
    if(EXISTS "${CMAKE_SOURCE_DIR}/src/util/stringutil.c")
        target_sources(test_game_data PRIVATE ${CMAKE_SOURCE_DIR}/src/util/stringutil.c)
    endif()
endif()
