# 石器时代客户端 (Stone Age Client)

从 sa_9061.exe 逆向工程的完整源代码复刻。

## 项目结构

```
StoneAgeClient/
├── CMakeLists.txt           # CMake 构建配置
├── build.bat                # Windows 构建脚本
├── include/                 # 头文件
│   ├── types.h              # 基础类型定义
│   ├── config.h             # 配置管理
│   ├── window.h             # 窗口管理
│   ├── directx.h            # DirectX 封装
│   ├── render.h             # 渲染系统
│   ├── map.h                # 地图系统
│   ├── character.h          # 角色系统
│   ├── battle.h             # 战斗系统
│   ├── item.h               # 物品系统
│   ├── network.h            # 网络模块
│   ├── protocol.h           # 网络协议
│   ├── assets.h             # 资源加载
│   └── ui.h                 # UI系统
├── src/                     # 源文件
│   ├── main.c / winmain.c   # 入口点
│   ├── game.c               # 游戏主循环
│   └── ...                  # 其他模块
└── res/                     # 资源文件
    └── resource.rc          # Windows 资源
```

## 构建方法

### 要求
- CMake 3.15+
- Visual Studio 2019+ 或 MinGW-w64
- DirectX SDK (DirectDraw, DirectSound, DirectInput)

### 编译步骤

```bash
# 使用 Visual Studio
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -A Win32 ..
cmake --build . --config Release

# 使用 MinGW
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

输出: `build/bin/sa_9061.exe`

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

## 命令行参数

```
sa_9061.exe updated userid=<user> realbin=<path> adrnbin=<path>
              sprbin=<path> spradrnbin=<path> windowmode usealpha nodelay
```

## 函数映射

| 原始地址 | 功能 | 实现文件 |
|---------|------|---------|
| 0x004936c3 | 程序入口 | main.c |
| 0x0043f1f0 | WinMain | winmain.c |
| 0x00411a00 | DirectDraw 初始化 | directx.c |
| 0x0041e260 | 游戏初始化 | game.c |
| 0x0041db40 | 主游戏循环 | game.c |
| 0x0047dc60 | 地图渲染 | map.c |
| 0x0045e880 | 网络I/O | network.c |
| 0x00404850 | 战斗地图加载 | battle.c |

## 模块说明

### DirectX (directx.c/h)
- DirectDraw 图形渲染
- DirectSound 音频播放
- DirectInput 输入处理

### 渲染系统 (render.c/h)
- 精灵绘制
- 文本渲染
- UI 元素

### 地图系统 (map.c/h)
- 地图加载
- 瓦片渲染
- 相机控制

### 角色系统 (character.c/h)
- 玩家控制
- NPC 管理
- 宠物系统

### 战斗系统 (battle.c/h)
- 回合制战斗
- 技能系统
- 捕捉机制

### 物品系统 (item.c/h)
- 背包管理
- 装备系统
- 物品使用

### 网络模块 (network.c/h)
- TCP 连接
- 数据包处理
- 心跳机制

## 协议

数据包格式:
```
[size:2][opcode:2][checksum:2][data]
```

已识别命令:
- ClientLogin, CharLogin, CharList
- 心跳包 (30秒间隔)

## 许可证

本项目仅用于学习和研究目的。

## 人类留下的痕迹

- 本项目由AI生成，全程几乎0人类干预，是一个实验项目
- 工具: claude code、ghidra、bethington/ghidra-mcp、cc-switch、vscode、clawgod(开启claude code高级功能)
- 模型: glm5(百度千帆 - 200RMB 的coding plan(本项目大概占了约70%的请求，成本不到150RMB))
- AI Agent连续工作时长: 48小时43分