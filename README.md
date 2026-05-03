# 石器时代客户端 (Stone Age Client)

从 sa_9061.exe 逆向工程的完整源代码复刻。

## 项目结构

```
StoneAgeClient/
├── include/                  # 头文件 (~120个)
├── src/                      # 源文件 (~160个，27个模块目录)
│   ├── main.c / winmain.c   # 入口点
│   ├── stubs.c               # 未实现函数的占位符
│   ├── animation/            # 动画系统 (core, direction, state)
│   ├── assets/               # 资源加载 (adrn, spr, real)
│   ├── battle/               # 战斗系统 (state, calc, render, action, unit, protocol)
│   ├── character/            # 角色系统
│   ├── chat/                 # 聊天系统
│   ├── config/               # 配置管理
│   ├── craft/                # 合成系统
│   ├── directx/              # DirectX封装 (DDraw, DSound, DInput)
│   ├── effect/               # 状态效果 (buff/debuff)
│   ├── event/                # 游戏事件
│   ├── fade/                 # 淡入淡出效果
│   ├── field/                # 场景管理
│   ├── game/                 # 游戏主循环、状态机、应用生命周期
│   ├── graphics/             # 16位颜色操作、alpha混合
│   ├── guild/                # 公会系统
│   ├── input/                # 键盘/鼠标输入、热键、IME
│   ├── inventory/            # 背包系统
│   ├── item/                 # 物品系统
│   ├── login/                # 登录认证 (DES加密、角色/服务器选择)
│   ├── mail/                 # 邮件系统
│   ├── map/                  # 地图加载、等距渲染、碰撞检测
│   ├── network/              # TCP网络I/O、二进制/文本协议分发
│   ├── npc/                  # NPC对话、商店
│   ├── party/                # 组队系统
│   ├── pathfind/             # A*寻路
│   ├── pet/                  # 宠物系统 (AI、捕捉、战斗)
│   ├── protocol/             # 协议工具函数
│   ├── quest/                # 任务系统
│   ├── render/               # 精灵绘制、文本渲染、队列处理
│   ├── save/                 # 存档系统
│   ├── shop/                 # 商店UI
│   ├── skill/                # 技能系统
│   ├── social/               # 好友/黑名单
│   ├── sound/                # DirectSound音频 (BGM/SE)
│   ├── sprite/               # 精灵RLE解码、缓存、混合
│   ├── storage/              # 仓库系统
│   ├── text_protocol/        # 文本协议处理器
│   ├── trade/                # 交易系统
│   ├── ui/                   # UI控件 (窗口、按钮、小地图、排名、皮肤)
│   └── util/                 # 工具 (内存池、计时器、序列化、截图、线程池)
├── tests/                    # ~95个独立测试可执行文件
├── res/                      # Windows资源文件
├── memory/                   # AI记忆 (项目分析文档)
├── configure.bat             # CMake配置脚本 (仅首次运行)
├── rebuild.bat               # 增量编译脚本
├── CMakePresets.json         # CMake预设 (x86/x64 Debug/Release)
├── CMakeLists.txt            # CMake主构建文件
├── vcpkg.json                # vcpkg依赖清单
└── Makefile                  # MinGW备选构建
```

## 构建方法

### 要求

- Windows 10/11
- Visual Studio 2022 Community (路径: `D:\Program Files\Microsoft Visual Studio\2022\Community`)
- vcpkg (路径: `D:\code\vcpkg`)
- DirectX 7 SDK (ddraw.lib, dsound.lib, dinput8.lib)

### 编译步骤

```batch
# 首次：执行配置 (生成 out/build/x64-debug)
configure.bat

# 后续：增量编译
rebuild.bat
```

`configure.bat` 调用 `cmake --preset x64-debug`，使用 Ninja 生成器。
`rebuild.bat` 调用 `cmake --build out/build/x64-debug --config Debug`。

输出: `out/build/x64-debug/bin/sa_9061.exe`

## 运行测试

测试位于 `tests/` 目录，每个 `.c` 文件编译为独立的测试可执行文件：

```bash
# 构建后，在 out/build/x64-debug 下运行全部测试
cd out/build/x64-debug
ctest --output-on-failure

# 运行单个测试
ctest -R test_battle_state_machine

# 或直接运行exe
tests/test_battle_state_machine_comprehensive.exe
```

测试使用独立的 assert 框架，无外部测试库依赖。

## 命令行参数

```
sa_9061.exe updated userid=<user> realbin=<path> adrnbin=<path>
              sprbin=<path> spradrnbin=<path> windowmode usealpha nodelay
```

## 数据文件

游戏需要以下数据文件 (来自原版):

```
data/
├── real.bin           # 主数据文件
├── adrn.bin           # 地址索引
├── spr.bin            # 精灵数据
├── spradrn.bin        # 精灵地址
├── realtrue.bin       # 真实数据
├── adrntrue.bin       # 真实地址
├── pal/Palet_1.sap    # 调色板
└── battleMap/
    └── battle*.sab    # 战斗地图
```

## 协议

数据包格式:
```
[size:2][opcode:2][checksum:2][data]
```

协议分两种模式:
- **文本协议**: 以 `\n` 结尾，约40+命令 (XY, XYC, XYD等)
- **二进制协议**: 结构化数据包，50+命令类型，使用Base-62编码的位掩码

常用命令码:

### Client → Server
| Opcode | 名称 | 说明 |
|--------|------|------|
| 0x0001 | PKT_CLIENT_LOGIN | 登录请求 |
| 0x0003 | PKT_CHAR_CREATE | 创建角色 |
| 0x0005 | PKT_CHAR_DELETE | 删除角色 |
| 0x0006 | PKT_CHAR_LOGIN | 选择角色 |
| 0x000E | PKT_CLIENT_MOVE | 移动 |
| 0x0010 | PKT_CLIENT_ACTION | 动作 (攻击等) |
| 0x0011 | PKT_CLIENT_TALK | 聊天消息 |
| 0x0057 | PKT_HEARTBEAT | 心跳 (30秒间隔) |

### Server → Client
| Opcode | 名称 | 说明 |
|--------|------|------|
| 0x0002 | PKT_SV_LOGIN_RESULT | 登录响应 |
| 0x0004 | PKT_SV_CHAR_LIST | 角色列表 |
| 0x0016 | PKT_SV_MAP_ENTER | 进入地图 |
| 0x0018 | PKT_SV_MAP_DATA | 地图数据 |
| 0x003B | PKT_SV_BATTLE_START | 战斗开始 |
| 0x0045 | PKT_SV_BATTLE_END | 战斗结束 |

## 关键技术细节

### 图形系统
- DirectDraw 7，16位色深 (565/555格式)
- MMX/SSE优化的alpha混合
- 等距(菱形)地图渲染
- RLE压缩精灵解码
- 两次渲染文本系统 (阴影+前景)
- 9-sprite网格UI窗口

### 加密
- DES算法，自定义S-box，密钥 `"f;encor1c"`
- 存档XOR加密
- 登录凭证加密传输

### 战斗系统
- 9状态回合制状态机
- 7元素系统 (地水火风光暗无)
- 30+动作类型 (攻击、技能、物品、宠物、逃跑、捕捉)
- 伤害公式、命中/暴击率、捕捉率计算

### 协议
- DBCS (双字节)字符支持，用于中文编码
- Base-62位掩码解析
- `|` 分隔字段的文本协议
- 选路/轮询网络I/O (FUN_0045e880)

## 已实现模块

动画、资源、战斗、角色、聊天、配置、合成、DirectX、效果、事件、淡入淡出、场景、游戏循环/状态机、图形、公会、输入、背包、物品、登录认证、邮件、地图/等距渲染、网络、NPC、组队、寻路、宠物/AI、协议、任务、渲染、存档、商店、技能、好友、音频、精灵、仓库、文本协议、交易、UI控件、工具

## 许可证

本项目仅用于学习和研究目的。

## 人类留下的痕迹

- 本项目由AI生成，全程几乎0人类干预，是一个实验项目
- 工具: claude code、ghidra、bethington/ghidra-mcp、cc-switch、vscode、clawgod(开启claude code高级功能)
- 模型: glm5(百度千帆 - 200RMB 的coding plan(本项目大概占了约70%的请求，成本不到150RMB))
- AI Agent连续工作时长: 48小时43分(从2026年5月1日10:42分开始)
