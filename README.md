# 🧩 Puzzle Game

A modern Qt-based puzzle game application with multiple difficulty levels, game modes, and networking features. Built with C++ and Qt framework.

## 🎮 Features

### Core Gameplay
- **Multiple Game Modes**:
  - Standard Puzzle: 3x3, 4x4, 8x8 grid sizes and custom (2x2 to 10x10)
  - Irregular Puzzle: 8 different shapes including triangles, pentagons, hexagons, circles
  - Jigsaw Puzzle: Classic tab-and-blank puzzle pieces
  - Custom Mode: Use your own images

- **Interactive Controls**:
  - Drag & drop puzzle pieces
  - Right-click rotation (90° increments)
  - Smart piece snapping
  - Real-time progress tracking

- **Audio System**:
  - Background music (MP3 support)
  - Sound effects for moves, rotations, and correct placements
  - Volume control

- **Game Assistance**:
  - Timer and move counter
  - Progress bar
  - Reference image display
  - Game instructions

### Network Features
- User login/registration system
- Online leaderboards (time, moves, level rankings)
- Client-server architecture
- Multiplayer support infrastructure

## 🛠️ Technology Stack

- **Framework**: Qt 6.6+ (C++17)
- **Build System**: qmake + Make
- **Multimedia**: QMediaPlayer, QAudioOutput
- **Networking**: Custom TCP protocol
- **Graphics**: QGraphicsScene, QGraphicsView
- **Platform Support**: macOS, Windows, Linux

## 📦 Installation

### Prerequisites
- Qt 6.6 or higher
- C++17 compatible compiler
- macOS/Windows/Linux operating system

### Build Instructions

#### Using Qt Creator
1. Open Qt Creator
2. Open project `PuzzleGame.pro`
3. Select build kit (recommended Qt 6.6+)
4. Build and run

#### Using Command Line
```bash
# Generate Makefile
qmake PuzzleGame.pro

# Build project
make

# Run the application
# Debug version
../debug/PuzzleGame

# Release version  
../release/PuzzleGame
```

### Build Configuration
The project supports multiple build configurations:
- **Debug**: With debug symbols, outputs to `../debug/`
- **Release**: Optimized version, outputs to `../release/`
- **Profile**: Performance analysis version

## 📁 Project Structure

```
puzzle-game/
├── src/                    # Source code
│   ├── core/              # Core game logic
│   ├── ui/                # User interface components  
│   ├── network/           # Network functionality
│   └── utils/             # Utility functions
│   ├── main.cpp           # Application entry point
│   ├── DlgMenu.*          # Main menu interface
│   ├── play4x4.*          # Standard puzzle game
│   ├── IrregularPuzzle.*  # Irregular puzzle game
│   ├── JigsawPuzzle.*     # Jigsaw puzzle base class
│   ├── NetworkClient.*    # Network client
│   └── help.*             # Help system
├── res/                   # Resources
│   ├── img/               # Image resources
│   ├── sounds/            # Sound files
│   └── img.qrc            # Resource configuration
├── docs/                  # Documentation
├── tests/                 # Unit tests
├── examples/              # Example configurations
├── build/                 # Build artifacts (gitignored)
├── PuzzleGame.pro         # Project configuration
└── README.md              # Project documentation
```

## 🚀 快速开始

### 环境要求
- Qt 6.0 或更高版本
- C++17 支持的编译器
- macOS/Windows/Linux 操作系统

### 构建步骤

#### 使用 Qt Creator
1. 打开 Qt Creator
2. 打开项目 `PuzzleGame.pro`
3. 选择构建套件（推荐 Qt 6.9.2）
4. 点击构建并运行

#### 使用命令行
```bash
# 生成 Makefile
qmake PuzzleGame.pro

# 编译项目
make

# 运行程序（根据构建模式选择）
# Debug 版本
../debug/PuzzleGame

# Release 版本
../release/PuzzleGame
```

### 构建配置
项目支持多种构建配置：
- **Debug**：包含调试信息，输出到 `../debug/`
- **Release**：优化版本，输出到 `../release/`
- **Profile**：性能分析版本

## 🎯 游戏玩法

### 标准拼图模式
1. 选择难度级别（3x3、4x4、8x8）
2. 选择图片（内置图片或自定义图片）
3. 拖拽拼图块到正确位置
4. 右键点击拼图块进行旋转
5. 完成所有拼图块即可获胜

### 异形拼图模式
1. 16个不同形状的拼图块
2. 将拼图块拖拽到中央拼图区域
3. 右键旋转调整角度
4. 拼图块会自动吸附到正确位置
5. 完成所有拼图块即可获胜

### 操作说明
- **左键拖拽**：移动拼图块
- **右键点击**：旋转拼图块 90 度
- **参考图片**：点击"显示原图"查看完整图片
- **重新开始**：点击"重新开始"重置游戏
- **返回菜单**：点击"返回"回到主菜单

## 🔧 开发指南

### 架构设计
项目采用 MVC 架构模式：

#### 表示层 (Presentation Layer)
- **DlgMenu**：主菜单界面
- **play4x4**：标准拼图游戏界面
- **IrregularPuzzle**：异形拼图游戏界面
- **各种对话框**：登录、排行榜、设置等

#### 模型层 (Model Layer)
- **GameState**：游戏状态管理
- **PuzzlePiece**：拼图块数据模型
- **NetworkData**：网络数据模型
- **LevelConfig**：难度配置

#### 控制层 (Control Layer)
- **GameController**：游戏逻辑控制
- **NetworkController**：网络通信控制
- **SoundController**：音效控制
- **FileController**：文件操作控制

### 核心类说明

#### 主要游戏类
- **DlgMenu**：主菜单和音乐控制
- **play4x4**：标准拼图游戏逻辑
- **IrregularPuzzle**：异形拼图游戏逻辑
- **JigsawPuzzle**：拼图游戏基类

#### 网络类
- **NetworkClient**：网络客户端
- **LoginDialog**：用户登录
- **RankingDialog**：排行榜显示

#### 辅助类
- **LevelSelect**：难度选择
- **CustomMode**：自定义模式
- **help**：帮助系统

### 音效系统
使用 QMediaPlayer 和 QAudioOutput 实现 MP3 音效播放：
- 移动音效：`move.mp3`
- 旋转音效：`rotate.mp3`
- 正确放置音效：`correct.mp3`

### 网络协议
使用自定义 TCP 协议：
- 用户认证（登录/注册）
- 排行榜同步
- 数据包结构定义在 `protocol.h`

## 🐛 常见问题

### Qt Creator 运行崩溃
确保 Qt Creator 配置使用本地构建目录：
1. 打开项目设置
2. 构建和运行 → 构建目录
3. 设置为本地路径（非 iCloud 目录）

### 字体相关错误
项目已替换为系统可用字体：
- "Microsoft YaHei" → "Arial"
- "华文行楷" → "Arial Unicode MS"

### 音效文件不支持
确保使用 MP3 格式音效文件，项目已升级到 QMediaPlayer 以支持 MP3。

### 信号连接警告
所有音乐控制按钮的信号连接已修复，使用现代 Qt 语法。

## 📞 联系方式

如有问题或建议，请通过以下方式联系：
- 1481059602@qq.com
- 13648509682

---

## 📖 更新日志

### v1.0 (2025-09-12)
- ✅ 完成基础拼图游戏功能
- ✅ 添加异形拼图模式
- ✅ 实现音效系统（MP3 支持）
- ✅ 添加网络功能
- ✅ 修复 Qt Creator 兼容性问题
- ✅ 完善用户界面和交互体验
- ✅ 添加详细的文档说明

---

**感谢使用 Qt 拼图游戏！** 🎉