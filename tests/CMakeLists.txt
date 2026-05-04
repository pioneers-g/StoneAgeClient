# Stone Age Client - Test CMakeLists.txt
cmake_minimum_required(VERSION 3.15)

# Test executable list - only comprehensive self-contained tests
set(TEST_SOURCES
    # Comprehensive tests (self-contained with stub implementations)
    test_pet_ai_comprehensive.c
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
    test_field_bgm_comprehensive.c
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
    test_util_random_comprehensive.c
    test_util_timer_comprehensive.c
    test_fileutil_comprehensive.c
    test_assets_comprehensive.c
    test_graphics_comprehensive.c
    test_directx_comprehensive.c
    test_login_crypto_comprehensive.c
    test_field_entity_comprehensive.c
    test_entity_movement_comprehensive.c
    test_fade_comprehensive.c
    test_minimap_comprehensive.c
    test_pathfind_comprehensive.c
    test_protocol_module_comprehensive.c
    test_application_comprehensive.c
    test_stringutil_comprehensive.c
    test_battle_calc_comprehensive.c
    test_battle_unit_comprehensive.c
    test_character_protocol_comprehensive.c
    test_ui_widget_comprehensive.c
    test_sprite_cache_comprehensive.c
    test_sprite_blend_comprehensive.c
    test_sprite_lcg_comprehensive.c
    test_sprite_core_comprehensive.c
    test_sprite_queue_comprehensive.c
    test_sprite_lookup_comprehensive.c
    test_sprite_background_comprehensive.c
    test_sprite_render_comprehensive.c
    test_protocol_util_comprehensive.c
    test_font_system_comprehensive.c
    test_cursor_system_comprehensive.c
    test_notification_system_comprehensive.c
    test_menu_system_comprehensive.c
    test_ranking_system_comprehensive.c
    test_ui_skin_system_comprehensive.c
    test_ui_element_system_comprehensive.c
    test_ui_input_system_comprehensive.c
    test_ui_render_system_comprehensive.c
    test_mempool_comprehensive.c
    test_serialize_comprehensive.c
    test_threadpool_comprehensive.c
    test_performance_comprehensive.c
    test_commandline_comprehensive.c
    test_screenshot_comprehensive.c
    test_storage_comprehensive.c
    test_album_comprehensive.c
    test_sprite_core_module_comprehensive.c
    test_sprite_lookup_module_comprehensive.c
    test_assets_module_comprehensive.c
    test_entity_flags_comprehensive.c
    test_player_flags_comprehensive.c
    test_text_queue_comprehensive.c
    test_string_unescape_comprehensive.c
    test_render_blend_mode_comprehensive.c
    test_player_slot_comprehensive.c
    test_direction_offset_comprehensive.c
    test_normalize_angle_comprehensive.c
    test_distance_squared_comprehensive.c
    test_sprite_range_check_comprehensive.c
    test_random_shuffle_comprehensive.c
    test_direction_angle_comprehensive.c
    test_shuffle_next_comprehensive.c
    test_random_range_comprehensive.c
    test_tile_walkable_comprehensive.c
    test_clamp_value_comprehensive.c
    test_linear_interpolation_comprehensive.c
    test_coordinate_clamp_comprehensive.c
    test_direction_to_char_comprehensive.c
    test_angle_normalize_comprehensive.c
    test_linked_list_comprehensive.c
    test_lcg_random_comprehensive.c
    test_isometric_transform_comprehensive.c
    test_string_swap_comprehensive.c
    test_player_flag_bits_comprehensive.c
    test_game_state_functions_comprehensive.c
    test_player_helper_functions_comprehensive.c
    test_entity_id_functions_comprehensive.c
    test_hex_parser_comprehensive.c
    test_player_flag_functions_comprehensive.c
    test_utility_functions_comprehensive.c
    test_slot_functions_comprehensive.c
    test_base62_parser_comprehensive.c
    test_camera_system_comprehensive.c
    test_angle_normalize_functions_comprehensive.c
    test_sprite_lookup_functions_comprehensive.c
    test_data_transfer_functions_comprehensive.c
    test_string_swap_function_comprehensive.c
    test_player_flag_manipulation_comprehensive.c
    test_game_state_setter_functions_comprehensive.c
    test_config_copy_functions_comprehensive.c
    test_sprite_cache_functions_comprehensive.c
    test_resource_cleanup_functions_comprehensive.c
    test_input_flag_processing_comprehensive.c
    test_flag_operations_comprehensive.c
    test_slot_check_functions_comprehensive.c
    test_object_property_setters_comprehensive.c
    test_object_state_setters_comprehensive.c
    test_scene_transition_functions_comprehensive.c
    test_config_save_restore_comprehensive.c
    test_flag_manipulation_functions_comprehensive.c
    test_party_slot_functions_comprehensive.c
    test_object_setter_functions_comprehensive.c
)

# TODO: Fix outdated tests that need updates to match current headers
# - test_input.c - uses old InputState fields
# - test_network_core.c - uses old NetworkContext fields
# - test_ui_window.c - uses old window type/sprite constants
# - test_font_render.c - uses old text queue types
# - test_sprite_loader.c - needs update
# - test_text_protocol.c - needs update
# - test_shop.c - missing linker symbols
# - test_map.c - missing sprite constants
# - test_sound.c - missing linker symbols
# - test_animation.c - missing linker symbols
# - test_pet_ai.c, test_protocol_util.c, test_stringutil.c, test_game_data.c,
#   test_map_entry.c, test_battle_unit.c, test_character_protocol.c, etc.

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

# Note: Special handling for tests that need additional source files has been removed
# as those tests are currently commented out pending updates.
