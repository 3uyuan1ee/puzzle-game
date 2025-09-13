# 🧩 Puzzle Game / 拼图游戏
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-green.svg)](#)
[![Qt](https://img.shields.io/badge/Qt-5%2F6-brightgreen.svg)](#)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](#)
A modern Qt-based puzzle game application with multiple difficulty levels, game modes, and networking features. Built with C++ and Qt framework.

基于Qt框架开发的现代拼图游戏应用，支持多种难度级别、游戏模式和网络功能。使用C++和Qt框架构建。

---

## 🎮 Features / 游戏特性

### Core Gameplay / 核心功能
- **Multiple Game Modes** / **多种游戏模式**:
  - Standard Puzzle: 3x3, 4x4, 8x8 grid sizes and custom (2x2 to 10x10)
  - Irregular Puzzle: 8 different shapes including triangles, pentagons, hexagons, circles
  - Jigsaw Puzzle: Classic tab-and-blank puzzle pieces
  - Custom Mode: Use your own images
  - 标准拼图：支持 3x3、4x4、8x8 网格大小和自定义（2x2到10x10）
  - 异形拼图：包含三角形、五边形、六边形、圆形等8种形状
  - 拼图拼图：经典凸凹拼图块
  - 自定义模式：使用您自己的图片

- **Interactive Controls** / **交互操作**:
  - Drag & drop puzzle pieces
  - Right-click rotation (90° increments)
  - Smart piece snapping
  - Real-time progress tracking
  - 拖拽移动拼图块
  - 右键旋转拼图块（90度增量）
  - 智能吸附功能
  - 实时进度显示

- **Audio System** / **音效系统**:
  - Background music (MP3 support)
  - Sound effects for moves, rotations, and correct placements
  - Volume control
  - 背景音乐（MP3支持）
  - 移动、旋转和正确放置的音效
  - 音量控制

- **Game Assistance** / **游戏辅助**:
  - Timer and move counter
  - Progress bar
  - Reference image display
  - Game instructions
  - 计时器和步数统计
  - 进度条显示
  - 参考图片显示
  - 游戏说明

### Network Features / 网络功能
- User login/registration system / 用户登录/注册系统
- Online leaderboards (time, moves, level rankings) / 在线排行榜（时间、步数、等级排行）
- Client-server architecture / 客户端-服务器架构
- Multiplayer support infrastructure / 多人游戏支持架构

---

## 🛠️ Technology Stack / 技术栈

- **Framework** / **开发框架**: Qt 6.6+ (C++17)
- **Build System** / **构建系统**: qmake + Make
- **Multimedia** / **多媒体**: QMediaPlayer, QAudioOutput
- **Networking** / **网络**: Custom TCP protocol / 自定义TCP协议
- **Graphics** / **图形**: QGraphicsScene, QGraphicsView
- **Platform Support** / **平台支持**: macOS, Windows, Linux

---

## 📦 Installation / 安装

### Prerequisites / 环境要求
- Qt 6.6 or higher / Qt 6.6或更高版本
- C++17 compatible compiler / C++17兼容的编译器
- macOS/Windows/Linux operating system / macOS/Windows/Linux操作系统

### Build Instructions / 构建步骤

#### Using Qt Creator / 使用Qt Creator
1. Open Qt Creator / 打开Qt Creator
2. Open project `PuzzleGame.pro` / 打开项目`PuzzleGame.pro`
3. Select build kit (recommended Qt 6.6+) / 选择构建套件（推荐Qt 6.6+）
4. Build and run / 构建并运行

#### Using Command Line / 使用命令行
```bash
# Generate Makefile / 生成Makefile
qmake PuzzleGame.pro

# Build project / 构建项目
make

# Run the application / 运行应用程序
# Debug version / Debug版本
../debug/PuzzleGame

# Release version / Release版本  
../release/PuzzleGame
```

### Build Configuration / 构建配置
The project supports multiple build configurations / 项目支持多种构建配置:
- **Debug**: With debug symbols, outputs to `../debug/` / 包含调试信息，输出到`../debug/`
- **Release**: Optimized version, outputs to `../release/` / 优化版本，输出到`../release/`
- **Profile**: Performance analysis version / 性能分析版本

---

## 📁 Project Structure / 项目结构

```
puzzle-game/
├── src/                    # Source code / 源代码
│   ├── core/              # Core game logic / 核心游戏逻辑
│   ├── ui/                # User interface components / 用户界面组件  
│   ├── network/           # Network functionality / 网络功能
│   └── utils/             # Utility functions / 工具函数
│   ├── main.cpp           # Application entry point / 应用程序入口
│   ├── DlgMenu.*          # Main menu interface / 主菜单界面
│   ├── play4x4.*          # Standard puzzle game / 标准拼图游戏
│   ├── IrregularPuzzle.*  # Irregular puzzle game / 异形拼图游戏
│   ├── JigsawPuzzle.*     # Jigsaw puzzle base class / 拼图游戏基类
│   ├── NetworkClient.*    # Network client / 网络客户端
│   └── help.*             # Help system / 帮助系统
├── res/                   # Resources / 资源文件
│   ├── img/               # Image resources / 图片资源
│   ├── sounds/            # Sound files / 音效文件
│   └── img.qrc            # Resource configuration / 资源配置
├── docs/                  # Documentation / 文档
├── tests/                 # Unit tests / 单元测试
├── examples/              # Example configurations / 示例配置
├── build/                 # Build artifacts (gitignored) / 构建产物（git忽略）
├── PuzzleGame.pro         # Project configuration / 项目配置
└── README.md              # Project documentation / 项目文档
```

---

## 🎯 How to Play / 游戏玩法

### Standard Puzzle Mode / 标准拼图模式
1. Select difficulty level (3x3, 4x4, 8x8) / 选择难度级别（3x3、4x4、8x8）
2. Choose image (built-in or custom) / 选择图片（内置或自定义）
3. Drag pieces to correct positions / 拖拽拼图块到正确位置
4. Right-click to rotate pieces / 右键旋转拼图块
5. Complete all pieces to win / 完成所有拼图块即可获胜

### Controls / 操作说明
- **Left-click drag**: Move pieces / 左键拖拽：移动拼图块
- **Right-click**: Rotate 90° / 右键点击：旋转90度
- **Reference image**: Click "Show Original" / 参考图片：点击"显示原图"
- **Restart**: Click "Restart Game" / 重新开始：点击"重新开始"
- **Return to menu**: Click "Back" / 返回菜单：点击"返回"

---

## 🧪 Testing / 测试

Run the test suite / 运行测试套件:
```bash
# Build tests / 构建测试
qmake PuzzleGame.pro CONFIG+=test
make

# Run tests / 运行测试
./tests/run_tests.sh
```

---

## 🤝 Contributing / 贡献

1. Fork the repository / Fork仓库
2. Create a feature branch / 创建功能分支 (`git checkout -b feature/amazing-feature`)
3. Commit your changes / 提交更改 (`git commit -m 'Add some amazing feature'`)
4. Push to the branch / 推送到分支 (`git push origin feature/amazing-feature`)
5. Open a Pull Request / 打开Pull Request

### Development Guidelines / 开发指南
- Follow existing code style / 遵循现有代码风格
- Write descriptive commit messages / 编写描述性提交消息
- Add tests for new functionality / 为新功能添加测试
- Update documentation / 更新文档

---

## 📝 License / 许可证

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

本项目基于MIT许可证开源 - 详情请查看[LICENSE](LICENSE)文件。

---

## 🙏 Acknowledgments / 致谢

- Qt Framework for the excellent cross-platform toolkit / Qt框架提供的优秀跨平台工具包
- All puzzle image contributors / 所有拼图图片贡献者
- Beta testers and community feedback / Beta测试者和社区反馈

---

## 📞 Contact / 联系方式

- **GitHub**: [3uyuan1ee](https://github.com/3uyuan1ee)
- **Email**: 1481059602@qq.com
- **Issues**: [Create an issue](https://github.com/3uyuan1ee/puzzle-game/issues) / [创建问题](https://github.com/3uyuan1ee/puzzle-game/issues)

---

## 🚀 Roadmap / 发展路线

- [ ] Mobile versions (iOS/Android) / 移动版本（iOS/Android）
- [ ] Additional puzzle types / 更多拼图类型
- [ ] Enhanced multiplayer features / 增强的多人游戏功能
- [ ] Puzzle editor / 拼图编辑器
- [ ] Achievement system / 成就系统
- [ ] Cloud save functionality / 云存档功能

---

Made with ❤️ using Qt and C++ / 使用Qt和C++制作 ❤️
