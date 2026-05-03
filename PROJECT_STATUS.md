# Stone Age Client - Reverse Engineering Project

## Project Overview

This project aims to reverse engineer and reimplement the Stone Age game client (sa_9061.exe) from decompiled code analysis using Ghidra.

## Current Status

### Completed Modules

| Module | File | Status | Key Functions |
|--------|------|--------|---------------|
| Entry Point | main.c, winmain.c | ✅ Complete | FUN_0043f1f0, main_entry |
| DirectX | directx.c | ✅ Complete | FUN_00411a00 (graphics init), FUN_00412af0 (surface create) |
| Assets | assets.c | ✅ Complete | FUN_0041f7d0 (adrn load), FUN_0041fe20 (spr load), FUN_0041fbb0 (real load) |
| Network | network.c | ✅ Complete | FUN_0045e880 (network I/O), FUN_0045ec80 (buffer append), FUN_0045ee40 (line extract) |
| Sprite | sprite.c | ✅ Complete | FUN_00414190 (sprite draw), FUN_004142f0 (blit), FUN_0047e970 (alpha blend) |
| Protocol | protocol.c | ✅ Complete | 50+ packet handlers, text/binary dispatchers |
| Input | input.c | ✅ Complete | DirectInput keyboard/mouse |
| Game | game.c | ✅ Complete | FUN_0041db40 (main loop) |
| Render | render.c | ✅ Complete | 2D rendering, GDI text |
| Map | map.c | ✅ Complete | Map loading, rendering |
| Character | character.c | ✅ Complete | Movement, animation, NPCs |
| Battle | battle.c | ✅ Complete | Turn-based combat system |
| Item | item.c | ✅ Complete | Inventory, equipment |
| UI | ui.c | ✅ Complete | Basic UI rendering |
| Sound | sound.c | ✅ Complete | DirectSound audio |
| NPC | npc.c | ✅ Complete | FUN_00462f60 (dialog), FUN_00463380 (shop), FUN_00488ea0 (talk window) |
| Pet | pet.c | ✅ Complete | FUN_00463c20 (list), FUN_00463d20 (status), FUN_0045fb80 (name), FUN_0045fdc0 (skills), FUN_0045ff50 (release), FUN_00464db0 (capture) |
| Party | party.c | ✅ Complete | FUN_00465440 (update), FUN_00465460 (dispatch), party invite/join/leave/kick |
| Trade | trade.c | ✅ Complete | FUN_0045a9a0 (request), FUN_0045bb00 (complete), item/gold exchange |
| Quest | quest.c | ✅ Complete | FUN_00465d20 (list), FUN_00465de0 (update), FUN_00465f70 (achievement) |
| Mail | mail.c | ✅ Complete | FUN_0045a900 (list), FUN_0045a980 (read), FUN_0045d6f0 (send result) |
| Friend | friend.c | ✅ Complete | Friend list, block list, online status |
| Minimap | minimap.c | ✅ Complete | Radar, minimap overlay, dot indicators |
| Guild | guild.c | ✅ Complete | Guild creation, management, ranks, chat |
| Login | login.c | ✅ Complete | FUN_0045ef60, FUN_0045f4d0, authentication, character select |
| Chat | chat.c | ✅ Complete | Chat channels, whisper, party/guild chat, history |
| Skill | skill.c | ✅ Complete | Skill learning, leveling, casting, cooldowns |
| Storage | storage.c | ✅ Complete | Warehouse, item storage, gold deposit/withdraw |
| Config | gameconfig.c | ✅ Complete | Graphics, sound, network, key bindings, settings |
| Effect | effect.c | ✅ Complete | Status effects, buffs, debuffs, DoT/HoT |
| Craft | craft.c | ✅ Complete | Item synthesis, upgrade, cooking, alchemy |
| Ranking | ranking.c | ✅ Complete | Leaderboards, player rankings |
| Pathfind | pathfind.c | ✅ Complete | A* pathfinding, auto-movement, waypoints |
| Event | event.c | ✅ Complete | Game events, bonuses, scheduling |
| SaveData | savedata.c | ✅ Complete | Save/load, auto-save, backups, checksums |
| Animation | animation.c | ✅ Complete | Frame-based animations, 8 directions |
| Timer | timer.c | ✅ Complete | Game timers, callbacks, cooldowns |
| Screenshot | screenshot.c | ✅ Complete | BMP capture, DirectX surface capture |
| Cursor | cursor.c | ✅ Complete | Custom cursors, game cursor modes |
| Font | font.c | ✅ Complete | Text rendering, word wrap, shadows |
| Random | random.c | ✅ Complete | RNG, dice, weighted selection, shuffle |
| PetAI | petai.c | ✅ Complete | Pet AI decision making, presets |
| Hotkey | hotkey.c | ✅ Complete | Keyboard shortcuts, bindings |
| Notification | notification.c | ✅ Complete | Game alerts, message display |
| ChatCommand | chatcommand.c | ✅ Complete | Chat commands, filter system |
| MemPool | mempool.c | ✅ Complete | Memory pools, object pools |
| Performance | performance.c | ✅ Complete | FPS, frame time, CPU/memory monitoring |
| Scene | scene.c | ✅ Complete | Scene management, transitions, stack |
| Serialize | serialize.c | ✅ Complete | Binary serialization, save/load |
| ThreadPool | threadpool.c | ✅ Complete | Worker threads, async tasks |
| FileUtil | fileutil.c | ✅ Complete | File I/O, directory operations |
| ConfigFile | configfile.c | ✅ Complete | INI-style config parsing |
| AudioManager | audiomanager.c | ✅ Complete | BGM/SE playback, 3D audio, fading |
| Resource | resource.c | ✅ Complete | LRU cache for sprites/textures/sounds |
| GameState | gamestate.c | ✅ Complete | State machine, state stack, pause/resume |
| GameLoop | gameloop.c | ✅ Complete | Main loop, frame timing, FPS limiting |
| Application | application.c | ✅ Complete | System coordinator, config, lifecycle |
| UIWidget | uiwidget.c | ✅ Complete | UI widget system, windows, buttons, input |
| Graphics | graphics.c | ✅ Complete | 16-bit color ops, alpha blend, blitting |
| MapManager | mapmanager.c | ✅ Complete | Map loading, rendering, collision, warps |
| BattleSystem | battlesystem.c | ✅ Complete | Turn-based combat, actions, damage calculation |

### Key Technical Details

#### Packet Format
```
[size:2][opcode:2][checksum:2][data]
```

#### Pixel Formats
- 16-bit color (565/555 format)
- Alpha blending algorithms from FUN_0047e970
- MMX/SSE optimized blending routines

#### Resource Files
- `real.bin` - Real data entries (FUN_0041fbb0)
- `adrn.bin` - Address entries (FUN_0041f7d0)
- `spr.bin` - Sprite data (FUN_0041fe20)
- `spradrn.bin` - Sprite address index

#### Network Protocol Modes
- Text protocol (lines ending with \n)
- Binary protocol (structured packets)

## Build Instructions

### Requirements
- CMake 3.15+
- Visual Studio 2022 or MinGW
- Windows SDK
- DirectX 7 SDK (ddraw.lib, dsound.lib, dinput.lib)

### Build Commands
```bash
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A Win32 ..
cmake --build . --config Release
```

## Function Reference

### Graphics (directx.c)
- `graphics_init()` - FUN_00411a00: Initialize DirectDraw, create surfaces
- `graphics_create_offscreen()` - FUN_00412af0: Create offscreen surface
- `graphics_load_palette()` - FUN_00412220: Load palette from file
- `graphics_flip()` - Present back buffer
- `graphics_clear()` - Clear surface with color

### Assets (assets.c)
- `assets_load_real()` - FUN_0041fbb0: Load real.bin
- `assets_load_adrn()` - FUN_0041f7d0: Load adrn.bin with adjustments
- `assets_load_spr()` - FUN_0041fe20: Load sprite data

### Network (network.c)
- `network_process()` - FUN_0045e880: Main network I/O loop
- `network_buffer_append()` - FUN_0045ec80: Append to receive buffer
- `network_extract_line()` - FUN_0045ee40: Extract text line

### Sprite (sprite.c)
- `sprite_draw()` - FUN_00414190: Draw sprite with transparency
- `sprite_blit_transparent()` - FUN_004142f0: Transparent blit
- `sprite_blend_565()` - FUN_0047e970: Alpha blending for 565 format

### NPC (npc.c)
- `npc_init()` - Initialize NPC system with dialogue pool
- `npc_handle_dialog_packet()` - FUN_00462f60: Process NPC dialogue packets
- `npc_handle_shop_packet()` - FUN_00463380: Process NPC shop packets
- `npc_show_talk_window()` - FUN_00488ea0: Create and show talk window
- `npc_hide_talk_window()` - FUN_00489810: Hide talk window
- `npc_load_skin()` - FUN_00489070: Load talk window skins
- `npc_add_dialogue()` - Add dialogue entry to window
- `npc_clear_dialogue()` - FUN_004897d0: Clear dialogue entries
- `npc_open_shop()` - Open NPC shop interface
- `npc_buy_item()` - Buy item from shop
- `npc_sell_item()` - Sell item to shop

### UI Widget (uiwidget.c)
- `widgetmgr_init()` - Initialize widget manager with memory pools
- `widget_create_window()` - Create window widget with title bar
- `widget_create_button()` - Create clickable button
- `widget_load_skin()` - FUN_00489070: Load UI skin bitmaps
- `widget_render()` - Render widget and children
- `widgetmgr_handle_input()` - Process mouse/keyboard input

### Game Loop (game.c)
- `game_init()` - FUN_0041e260: Initialize all game systems
- `game_main_loop()` - FUN_0041db40: Main game loop with frame timing
- `game_update()` - Update game state
- `game_render()` - Render current frame

### Entry Point (main.c)
- `WinMain()` - FUN_0043f1f0: Application entry point
- `game_parse_command_line()` - Parse command line arguments:
  - userid: User identifier
  - realbin:, adrnbin:, sprbin:, spradrnbin: Data file paths
  - realtrue:, adrntrue: True data paths
  - usealpha: Enable alpha blending
  - windowmode: Windowed mode
  - nodelay: Disable network delay

### Pet (pet.c)
- `pet_init()` - Initialize pet system
- `pet_get_slot()` - Get pet by slot index
- `pet_get_selected()` - Get currently selected pet
- `pet_get_battle_pet()` - Get pet in battle
- `pet_select()` - Select pet by slot
- `pet_set_battle()` - Set battle pet
- `pet_add()` - Add pet to slot
- `pet_remove()` - Remove pet from slot
- `pet_rename()` - Rename pet
- `pet_release()` - Release pet
- `pet_feed()` - Feed pet to restore HP/MP
- `pet_capture()` - Capture enemy as pet
- `pet_handle_list_packet()` - FUN_00463c20: Process pet list packet
- `pet_handle_status_packet()` - FUN_00463d20: Process pet status packet
- `pet_handle_name_result()` - FUN_0045fb80: Process pet rename result
- `pet_handle_skill_list()` - FUN_0045fdc0: Process pet skill list
- `pet_handle_release_result()` - FUN_0045ff50: Process pet release result
- `pet_handle_capture_result()` - FUN_00464db0: Process capture result
- `pet_level_up()` - Level up pet
- `pet_gain_exp()` - Give experience to pet
- `pet_use_skill()` - Use pet skill in battle
- `pet_set_skill()` - Set pet skill
- `pet_evolve()` - Evolve pet
- `pet_fuse()` - Fuse two pets together

### Party (party.c)
- `party_init()` - Initialize party system
- `party_create()` - Create new party
- `party_disband()` - Disband party
- `party_invite()` - Invite player to party
- `party_handle_invite()` - Handle incoming invite
- `party_accept_invite()` - Accept party invite
- `party_decline_invite()` - Decline invite
- `party_handle_join()` - Handle member join
- `party_leave()` - Leave party
- `party_handle_leave()` - FUN_00465440: Handle member leave
- `party_kick()` - Kick member from party
- `party_change_leader()` - Transfer party leadership
- `party_set_exp_share()` - Set experience share mode
- `party_set_item_share()` - Set item share mode
- `party_distribute_exp()` - Distribute experience to members
- `party_chat()` - Send party chat message
- `party_get_member()` - Get member by index
- `party_find_member()` - Find member by ID

### Trade (trade.c)
- `trade_init()` - Initialize trade system
- `trade_request()` - FUN_0045a9a0: Request trade with player
- `trade_handle_request()` - Handle incoming trade request
- `trade_accept()` - Accept trade request
- `trade_decline()` - Decline trade request
- `trade_cancel()` - Cancel ongoing trade
- `trade_add_item()` - Add item to trade window
- `trade_remove_item()` - Remove item from trade
- `trade_set_gold()` - Set gold amount for trade
- `trade_lock()` - Lock/confirm trade
- `trade_handle_partner_add_item()` - FUN_0045a980: Partner added item
- `trade_handle_complete()` - FUN_0045bb00: Trade completed
- `trade_get_state()` - Get current trade state
- `trade_is_active()` - Check if in trade

### Quest (quest.c)
- `quest_init()` - Initialize quest system
- `quest_handle_list_packet()` - FUN_00465d20: Process quest list packet
- `quest_parse_list()` - Parse quest list data
- `quest_handle_update()` - FUN_00465de0: Process quest update
- `quest_handle_achievement()` - FUN_00465f70: Process achievement packet
- `quest_accept()` - Accept available quest
- `quest_complete()` - Complete active quest
- `quest_abandon()` - Abandon active quest
- `quest_claim_reward()` - Claim quest reward
- `quest_update_progress()` - Update quest objective progress
- `quest_find()` - Find quest by ID
- `quest_get_active()` - Get current active quest
- `quest_request_list()` - Request quest list from server

### Mail (mail.c)
- `mail_init()` - Initialize mail system
- `mail_request_list()` - FUN_0045a900: Request mail list
- `mail_handle_list()` - Handle mail list response
- `mail_handle_read()` - FUN_0045a980: Handle mail read
- `mail_handle_send_result()` - FUN_0045d6f0: Handle send result
- `mail_send()` - Send mail to player
- `mail_delete()` - Delete mail
- `mail_claim_items()` - Claim attached items
- `mail_claim_gold()` - Claim attached gold
- `mail_find()` - Find mail by ID
- `mail_get_unread_count()` - Get unread mail count
- `mail_refresh_list()` - Refresh mail list

### Friend (friend.c)
- `friend_init()` - Initialize friend system
- `friend_request_list()` - Request friend list from server
- `friend_handle_list()` - Handle friend list response
- `friend_add()` - Add friend by name
- `friend_handle_add()` - Handle add friend response
- `friend_remove()` - Remove friend
- `friend_handle_remove()` - Handle remove friend response
- `friend_handle_status_change()` - Handle friend online/offline status
- `friend_find_by_name()` - Find friend by name
- `friend_find_by_id()` - Find friend by ID
- `friend_get_by_index()` - Get friend by index
- `friend_get_online_count()` - Get count of online friends
- `friend_get_count()` - Get total friend count
- `friend_is_full()` - Check if friend list is full
- `block_init()` - Initialize block list
- `block_add()` - Add player to block list
- `block_remove()` - Remove player from block list
- `block_is_blocked()` - Check if player is blocked

### Minimap (minimap.c)
- `minimap_init()` - Initialize minimap system
- `minimap_render()` - Render minimap overlay
- `minimap_draw_background()` - Draw minimap background
- `minimap_draw_dots()` - Draw NPC/monster/party dots
- `minimap_draw_player()` - Draw player indicator
- `minimap_draw_border()` - Draw minimap border
- `minimap_update()` - Update minimap data
- `minimap_set_center()` - Set view center (player position)
- `minimap_set_map()` - Set current map info
- `minimap_clear_dots()` - Clear all dots
- `minimap_add_dot()` - Add dot to minimap
- `minimap_remove_dot()` - Remove dot by ID
- `minimap_world_to_minimap()` - Convert world to minimap coordinates
- `minimap_minimap_to_world()` - Convert minimap to world coordinates
- `minimap_toggle()` - Toggle visibility
- `minimap_handle_click()` - Handle click on minimap

### Guild (guild.c)
- `guild_init()` - Initialize guild system
- `guild_create()` - Create new guild
- `guild_handle_create_result()` - Handle create result
- `guild_disband()` - Disband guild
- `guild_request_info()` - Request guild info
- `guild_handle_info()` - Handle guild info response
- `guild_request_members()` - Request member list
- `guild_handle_members()` - Handle member list response
- `guild_invite()` - Invite player to guild
- `guild_handle_invite_request()` - Handle incoming invite
- `guild_accept_invite()` - Accept guild invite
- `guild_decline_invite()` - Decline guild invite
- `guild_leave()` - Leave guild
- `guild_kick()` - Kick member from guild
- `guild_set_rank()` - Set member rank
- `guild_set_notice()` - Set guild notice
- `guild_chat()` - Send guild chat
- `guild_transfer_leader()` - Transfer leadership
- `guild_contribute()` - Contribute gold to guild
- `guild_find_member()` - Find member by name
- `guild_find_member_by_id()` - Find member by ID
- `guild_is_leader()` - Check if player is guild leader
- `guild_is_in_guild()` - Check if in a guild

### Login (login.c)
- `login_init()` - Initialize login system
- `login_connect()` - Connect to login server
- `login_disconnect()` - Disconnect from server
- `login_auth()` - Authenticate with credentials
- `login_handle_result()` - FUN_0045f4d0: Handle login response
- `login_request_servers()` - Request server list
- `login_handle_servers()` - Handle server list response
- `login_select_server()` - Select game server
- `login_request_characters()` - Request character list
- `login_handle_characters()` - Handle character list response
- `login_select_character()` - Select character to play
- `login_create_character()` - Create new character
- `login_delete_character()` - Delete character
- `login_enter_game()` - Enter game with selected character
- `login_logout()` - Logout from server
- `login_get_state()` - Get current login state
- `login_update()` - Update login state machine

### Chat (chat.c)
- `chat_init()` - Initialize chat system
- `chat_send()` - Send normal chat message
- `chat_send_to()` - Send chat with specific type
- `chat_whisper()` - Send private message
- `chat_party()` - Send party chat
- `chat_guild()` - Send guild chat
- `chat_receive()` - Receive chat message
- `chat_system_message()` - Display system message
- `chat_handle_message()` - Handle chat packet
- `chat_add_history()` - Add message to history
- `chat_set_filter()` - Set channel filter flags
- `chat_input_char()` - Input character to chat
- `chat_history_up/down()` - Navigate sent history
- `chat_reply()` - Reply to last whisper
- `chat_process_command()` - Process chat commands
- `chat_save_history()` - Save chat log

### Skill (skill.c)
- `skill_init()` - Initialize skill system
- `skill_load_database()` - Load skill data
- `skill_learn()` - Learn new skill
- `skill_forget()` - Forget skill
- `skill_level_up()` - Level up skill
- `skill_gain_exp()` - Gain skill experience
- `skill_set_slot()` - Set skill to hotkey slot
- `skill_use()` - Use skill on target
- `skill_start_cast()` - Start casting skill
- `skill_cancel_cast()` - Cancel casting
- `skill_is_on_cooldown()` - Check cooldown
- `skill_handle_list()` - Handle skill list packet
- `skill_handle_effect()` - Handle skill effect packet
- `skill_get_mp_cost()` - Get MP cost for skill

### Storage (storage.c)
- `storage_init()` - Initialize storage system
- `storage_open()` - Open storage UI
- `storage_close()` - Close storage UI
- `storage_deposit_item()` - Deposit item to storage
- `storage_withdraw_item()` - Withdraw item from storage
- `storage_deposit_gold()` - Deposit gold
- `storage_withdraw_gold()` - Withdraw gold
- `storage_find_item()` - Find item by ID
- `storage_find_free_slot()` - Find empty slot
- `storage_set_filter()` - Set category filter
- `storage_handle_item_list()` - Handle item list packet
- `storage_expand()` - Expand storage capacity
- `storage_get_max_slots()` - Get max slots

### Effect (effect.c)
- `effect_init()` - Initialize effect system
- `effect_add()` - Add status effect
- `effect_remove()` - Remove effect
- `effect_remove_all()` - Remove all effects
- `effect_has()` - Check if has effect
- `effect_update()` - Update effect timers
- `effect_tick()` - Process periodic effect
- `effect_get_attack_modifier()` - Get attack modifier
- `effect_can_act()` - Check if can act
- `effect_can_cast()` - Check if can use magic
- `effect_can_move()` - Check if can move

### Craft (craft.c)
- `craft_init()` - Initialize craft system
- `craft_learn_recipe()` - Learn new recipe
- `craft_start()` - Start crafting
- `craft_update()` - Update crafting progress
- `craft_handle_result()` - Handle craft result
- `craft_check_materials()` - Check if has materials
- `craft_can_craft()` - Check if can craft
- `craft_get_success_rate()` - Get success chance

### Ranking (ranking.c)
- `ranking_init()` - Initialize ranking system
- `ranking_request()` - Request rankings from server
- `ranking_handle_list()` - Handle ranking list packet
- `ranking_get_entry()` - Get rank entry by index
- `ranking_find_player()` - Find player in rankings
- `ranking_get_own_rank()` - Get own rank

### Pathfind (pathfind.c)
- `pathfind_init()` - Initialize pathfinding system
- `pathfind_to()` - Find path to coordinates
- `pathfind_to_npc()` - Find path to NPC
- `pathfind_update()` - Update path movement
- `pathfind_get_next_direction()` - Get next move direction
- `pathfind_astar()` - A* pathfinding algorithm
- `pathfind_stop()` - Stop pathfinding

### Event (event.c)
- `event_init()` - Initialize event system
- `event_update()` - Update event schedules
- `event_get()` - Get event by ID
- `event_is_active()` - Check if event is active
- `event_get_exp_bonus()` - Get EXP bonus
- `event_get_gold_bonus()` - Get gold bonus
- `event_handle_list()` - Handle event list packet
- `event_recalculate_bonuses()` - Recalculate active bonuses

### SaveData (savedata.c)
- `savedata_init()` - Initialize save system
- `savedata_save()` - Save to slot
- `savedata_load()` - Load from slot
- `savedata_auto_save()` - Auto save
- `savedata_calculate_checksum()` - Calculate checksum
- `savedata_create_backup()` - Create backup
- `savedata_validate()` - Validate save data

### Animation (animation.c)
- `anim_init()` - Initialize animation system
- `anim_play()` - Play animation on entity
- `anim_stop()` - Stop animation
- `anim_update()` - Update all animations
- `anim_set_direction()` - Set animation direction
- `anim_direction_from_delta()` - Get direction from movement

### Timer (timer.c)
- `timer_init()` - Initialize timer system
- `timer_create()` - Create new timer
- `timer_create_oneshot()` - Create one-shot timer
- `timer_create_repeating()` - Create repeating timer
- `timer_destroy()` - Destroy timer
- `timer_update()` - Update timer system
- `timer_get_remaining()` - Get remaining time

### Screenshot (screenshot.c)
- `screenshot_init()` - Initialize screenshot system
- `screenshot_capture()` - Capture screen to file
- `screenshot_capture_game()` - Capture game window
- `screenshot_save_bmp()` - Save BMP file
- `screenshot_set_format()` - Set output format

### Cursor (cursor.c)
- `cursor_init()` - Initialize cursor system
- `cursor_load()` - Load cursor from file
- `cursor_set()` - Set current cursor
- `cursor_show()` - Show/hide cursor
- `cursor_set_game_mode()` - Set game cursor mode
- `cursor_update_context()` - Update cursor based on context

### Font (font.c)
- `font_init()` - Initialize font system
- `font_create_font()` - Create font
- `font_draw_text()` - Draw text
- `font_draw_text_shadow()` - Draw text with shadow
- `font_draw_text_outline()` - Draw text with outline
- `font_get_text_size()` - Get text dimensions
- `font_word_wrap()` - Word wrap text

### Random (random.c)
- `random_init()` - Initialize random system
- `random_seed()` - Seed the generator
- `random_next()` - Generate random number
- `random_range()` - Random in range
- `random_float()` - Random float
- `random_chance()` - Random boolean with probability
- `random_dice()` - Roll dice
- `random_weighted()` - Weighted random selection
- `random_shuffle()` - Shuffle array

### PetAI (petai.c)
- `petai_init()` - Initialize pet AI system
- `petai_load_settings()` - Load AI settings
- `petai_set_mode()` - Set AI mode for pet
- `petai_decide_action()` - Decide pet action in battle
- `petai_select_skill()` - Select best skill to use
- `petai_select_target()` - Select target enemy

### Hotkey (hotkey.c)
- `hotkey_init()` - Initialize hotkey system
- `hotkey_set_binding()` - Set hotkey binding
- `hotkey_check()` - Check if hotkey triggered
- `hotkey_process_input()` - Process keyboard input
- `hotkey_get_key_name()` - Get key name string
- `hotkey_save_config()` - Save hotkey config

### Notification (notification.c)
- `notification_init()` - Initialize notification system
- `notification_add()` - Add notification
- `notification_info()` - Add info notification
- `notification_warning()` - Add warning notification
- `notification_error()` - Add error notification
- `notification_update()` - Update notifications
- `notification_clear_all()` - Clear all notifications

### ChatCommand (chatcommand.c)
- `chatcommand_init()` - Initialize command system
- `chatcommand_process()` - Process chat input
- `chatcommand_register()` - Register custom command
- `chatcommand_get()` - Get command by name
- `chatfilter_init()` - Initialize filter
- `chatfilter_check()` - Check and filter message
- `chatfilter_add_word()` - Add filter word

### MemPool (mempool.c)
- `mempool_init()` - Initialize memory pool system
- `mempool_create()` - Create a memory pool
- `mempool_alloc()` - Allocate from pool
- `mempool_free()` - Free to pool
- `objpool_init()` - Initialize object pool
- `objpool_alloc()` - Allocate object from pool

### Performance (performance.c)
- `perf_init()` - Initialize performance monitor
- `perf_begin_frame()` - Begin frame timing
- `perf_end_frame()` - End frame timing
- `perf_get_fps()` - Get current FPS
- `perf_begin()` - Begin performance counter
- `perf_end()` - End performance counter

### Scene (scene.c)
- `scene_init()` - Initialize scene manager
- `scene_register()` - Register a scene
- `scene_change()` - Change scene immediately
- `scene_update()` - Update current scene
- `scene_render()` - Render current scene
- `scene_push()` - Push scene to stack

### Serialize (serialize.c)
- `serializer_init_write()` - Initialize for writing
- `serializer_init_read()` - Initialize for reading
- `serializer_write_u32()` - Write 32-bit value
- `serializer_read_u32()` - Read 32-bit value
- `serializer_write_string()` - Write string
- `serializer_calc_checksum()` - Calculate checksum

### ThreadPool (threadpool.c)
- `threadpool_init()` - Initialize thread pool system
- `threadpool_create()` - Create a thread pool
- `threadpool_submit()` - Submit a task
- `threadpool_wait()` - Wait for all tasks
- `threadpool_destroy()` - Destroy pool

### FileUtil (fileutil.c)
- `file_exists()` - Check if file exists
- `dir_create()` - Create directory
- `file_read_all()` - Read entire file
- `file_write_all()` - Write entire file
- `file_copy()` - Copy file
- `file_list()` - List directory contents

### ConfigFile (configfile.c)
- `configfile_init()` - Initialize config
- `configfile_load()` - Load from file
- `configfile_save()` - Save to file
- `configfile_get()` - Get string value
- `configfile_get_int()` - Get integer value
- `configfile_set()` - Set value

### AudioManager (audiomanager.c)
- `audiomanager_init()` - Initialize audio system
- `audiomanager_shutdown()` - Shutdown audio
- `audiomanager_play_bgm()` - Play background music
- `audiomanager_stop_bgm()` - Stop BGM
- `audiomanager_pause_bgm()` - Pause BGM
- `audiomanager_resume_bgm()` - Resume BGM
- `audiomanager_play_se()` - Play sound effect
- `audiomanager_play_se_3d()` - Play 3D positional sound
- `audiomanager_fade_out_bgm()` - Fade out BGM
- `audiomanager_fade_in_bgm()` - Fade in BGM
- `audiomanager_set_bgm_volume()` - Set BGM volume
- `audiomanager_set_se_volume()` - Set SE volume
- `audiomanager_play_bgm_for_map()` - Play BGM based on map ID

### Resource (resource.c)
- `resource_init()` - Initialize resource cache
- `resource_shutdown()` - Shutdown resource system
- `resource_load_sprite()` - Load and cache sprite
- `resource_get_sprite()` - Get cached sprite
- `resource_unload_sprite()` - Unload sprite
- `resource_evict_sprite()` - Evict LRU sprite
- `resource_load_texture()` - Load and cache texture
- `resource_preload_sprites()` - Batch preload sprites
- `resource_clear_all()` - Clear all caches
- `resource_get_stats()` - Get cache statistics
- `resource_update()` - Update cache (memory management)

### GameState (gamestate.c)
- `gamestate_init()` - Initialize state manager
- `gamestate_shutdown()` - Shutdown state manager
- `gamestate_change()` - Change game state
- `gamestate_request_change()` - Request deferred state change
- `gamestate_process_transition()` - Process pending transition
- `gamestate_enter()` - State enter handler
- `gamestate_exit()` - State exit handler
- `gamestate_update()` - Update current state
- `gamestate_render()` - Render current state
- `gamestate_handle_input()` - Handle input for state
- `gamestate_push()` - Push state to stack (overlay)
- `gamestate_pop()` - Pop state from stack
- `gamestate_pause()` - Pause game
- `gamestate_resume()` - Resume game
- `gamestate_can_exit()` - Check if state allows exit

### GameLoop (gameloop.c)
- `gameloop_init()` - Initialize game loop
- `gameloop_shutdown()` - Shutdown game loop
- `gameloop_run()` - Run main game loop
- `gameloop_stop()` - Stop game loop
- `gameloop_pause()` - Pause game loop
- `gameloop_resume()` - Resume game loop
- `gameloop_set_mode()` - Set game loop mode
- `gameloop_begin_frame()` - Begin frame timing
- `gameloop_end_frame()` - End frame timing
- `gameloop_update()` - Update game state
- `gameloop_render()` - Render frame
- `gameloop_get_fps()` - Get current FPS
- `gameloop_limit_fps()` - Limit frame rate
- `gameloop_get_stats()` - Get performance statistics

### Application (application.c)
- `application_init()` - Initialize application
- `application_init_systems()` - Initialize all game systems
- `application_shutdown()` - Shutdown application
- `application_run()` - Main application entry point
- `application_load_config()` - Load configuration from file
- `application_save_config()` - Save configuration to file
- `application_get_hwnd()` - Get window handle
- `application_resize()` - Handle window resize
- `application_toggle_fullscreen()` - Toggle fullscreen mode
- `application_get_state()` - Get application state
- `application_set_error()` - Set error information
- `application_get_uptime()` - Get application uptime
- `application_request_exit()` - Request application exit

### UIWidget (uiwidget.c)
- `widgetmgr_init()` - Initialize widget manager
- `widgetmgr_shutdown()` - Shutdown widget manager
- `widget_create()` - Create generic widget
- `widget_create_window()` - Create window widget
- `widget_create_button()` - Create button widget
- `widget_create_text()` - Create text widget
- `widget_create_edit()` - Create edit/text input widget
- `widget_create_image()` - Create image widget
- `widget_create_listbox()` - Create listbox widget
- `widget_create_progress()` - Create progress bar widget
- `widget_create_slot()` - Create inventory slot widget
- `widget_destroy()` - Destroy widget
- `widget_destroy_children()` - Destroy child widgets
- `widget_set_text()` - Set widget text
- `widget_set_sprite()` - Set widget sprite
- `widget_set_color()` - Set widget colors
- `widget_set_position()` - Set widget position
- `widget_set_visible()` - Set widget visibility
- `widget_set_on_click()` - Set click callback
- `widgetmgr_render()` - Render all widgets
- `widget_render()` - Render single widget
- `widgetmgr_handle_input()` - Handle input for widgets
- `widget_set_focus()` - Set focused widget
- `widget_find_at_point()` - Find widget at screen point
- `widget_load_skin()` - Load UI skin from files

### Graphics (graphics.c)
- `graphics_init()` - Initialize graphics context
- `graphics_set_format()` - Set 565 or 555 color format
- `graphics_make_color()` - Create 16-bit color
- `graphics_alpha_blend()` - Alpha blend two colors
- `graphics_draw_pixel()` - Draw single pixel
- `graphics_draw_hline()` - Draw horizontal line
- `graphics_draw_vline()` - Draw vertical line
- `graphics_draw_rect()` - Draw rectangle outline
- `graphics_fill_rect()` - Fill rectangle
- `graphics_fill_gradient()` - Fill with gradient
- `graphics_clear()` - Clear surface
- `graphics_blit_transparent()` - Blit with color key
- `graphics_blit_alpha()` - Blit with alpha blending
- `graphics_blit_scaled()` - Scale blit

### MapManager (mapmanager.c)
- `mapmgr_init()` - Initialize map manager
- `mapmgr_load()` - Load map by ID
- `mapmgr_unload()` - Unload map
- `mapmgr_change_map()` - Change current map
- `mapmgr_get_tile()` - Get tile at position
- `mapmgr_set_tile()` - Set tile at position
- `mapmgr_is_walkable()` - Check if tile is walkable
- `mapmgr_render()` - Render current map
- `mapmgr_render_layer()` - Render single layer
- `mapmgr_set_viewport()` - Set viewport position
- `mapmgr_center_view()` - Center view on position
- `mapmgr_check_collision()` - Check collision
- `mapmgr_find_warp()` - Find warp at position
- `mapmgr_get_encounter()` - Get random encounter
- `mapmgr_load_battle_map()` - Load battle map

### BattleSystem (battlesystem.c)
- `battle_init()` - Initialize battle system
- `battle_shutdown()` - Shutdown battle system
- `battle_start_encounter()` - Start battle encounter
- `battle_end()` - End battle
- `battle_get_state()` - Get current battle state
- `battle_set_state()` - Set battle state
- `battle_update()` - Update battle state machine
- `battle_begin_turn()` - Begin new turn
- `battle_queue_action()` - Queue battle action
- `battle_execute_actions()` - Execute queued actions
- `battle_calculate_damage()` - Calculate damage from attack
- `battle_calculate_healing()` - Calculate healing amount
- `battle_get_unit()` - Get battle unit by ID
- `battle_is_unit_alive()` - Check if unit is alive
- `battle_count_alive_enemies()` - Count living enemies
- `battle_count_alive_allies()` - Count living allies
- `battle_apply_damage()` - Apply damage to unit
- `battle_apply_healing()` - Apply healing to unit
- `battle_process_ai()` - Process AI for enemy unit
- `battle_get_random_target()` - Get random target
- `battle_can_use_skill()` - Check if skill can be used
- `battle_render()` - Render battle scene
- `battle_handle_input()` - Handle battle input

## Memory Map (from binary analysis)

| Address | Purpose |
|---------|---------|
| 0x004d36d0 | Battle state |
| 0x0455ef2c | Debug flag |
| 0x04560214 | Screen mode (0=640x480, 1=320x240) |
| 0x0461b3f8 | Network initialized flag |
| 0x0461b420 | Connection flag |
| 0x0464f488 | Object count |
| 0x054a90c | Graphics context pointer |

## Protocol Opcodes

### Client → Server
| Opcode | Name | Description |
|--------|------|-------------|
| 0x0001 | PKT_CLIENT_LOGIN | Login request |
| 0x0003 | PKT_CHAR_CREATE | Create character |
| 0x0005 | PKT_CHAR_DELETE | Delete character |
| 0x0006 | PKT_CHAR_LOGIN | Select character |
| 0x000E | PKT_CLIENT_MOVE | Movement |
| 0x0010 | PKT_CLIENT_ACTION | Action (attack, etc) |
| 0x0011 | PKT_CLIENT_TALK | Chat message |
| 0x0057 | PKT_HEARTBEAT | Keepalive |

### Server → Client
| Opcode | Name | Description |
|--------|------|-------------|
| 0x0002 | PKT_SV_LOGIN_RESULT | Login response |
| 0x0004 | PKT_SV_CHAR_LIST | Character list |
| 0x0016 | PKT_SV_MAP_ENTER | Enter map |
| 0x0018 | PKT_SV_MAP_DATA | Map data |
| 0x003B | PKT_SV_BATTLE_START | Battle start |
| 0x0045 | PKT_SV_BATTLE_END | Battle end |

## Next Steps

1. ~~Implement remaining packet handlers~~ (partial)
2. ~~Add NPC dialogue script system~~ ✅ Complete
3. ~~Implement complete login flow~~ ✅ Complete
4. ~~Add pet system functionality~~ ✅ Complete
5. ~~Implement party system~~ ✅ Complete
6. ~~Add trade system~~ ✅ Complete
7. ~~Implement quest system~~ ✅ Complete
8. ~~Add mail system~~ ✅ Complete
9. Implement achievement system (integrated in quest.c)
10. ~~Add friend/contact system~~ ✅ Complete
11. ~~Implement guild system~~ ✅ Complete
12. ~~Add mini-map and radar~~ ✅ Complete

## License

This is a reverse engineering project for educational purposes.
