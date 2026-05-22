# Prism

[English Version](README.md)

Prism 是一个基于 Qt Widgets 的桌面工作台，用于学习和调试图像处理管道阶段。当前 MVP 版本保持精简，在 `Prism/` 目录中以 Qt Creator 项目的形式运行。

## 当前功能

- 使用 Qt 图像插件打开常见的图像文件。
- 支持两种预览模式：自适应窗口或实际尺寸。
- 选择占位 ISP 管道阶段。
- 通过红蓝增益滑块调整预览白平衡。
- 通过 EV 调整预览曝光。
- 重置预览参数。
- 将当前处理的预览导出为 PNG 或 JPEG 格式。
- 检查 RGB 直方图、图像元数据和日志输出。

## 项目结构

```
Prism/
├── CMakeLists.txt              # CMake 构建配置
├── main.cpp                    # 应用入口点
├── mainwindow.cpp/h            # 主窗口界面
├── mainwindow.ui               # UI 设计文件
├── pipelinemodel.cpp/h         # ISP 管道数据模型
├── previewprocessor.cpp/h      # 图像处理和预览逻辑
├── histogramrenderer.cpp/h     # 直方图渲染器
└── build/                      # 构建输出目录
```

## 快速开始

### 使用 Qt Creator 运行

1. 在 Qt Creator 中打开 `Prism/CMakeLists.txt`。
2. 选择一个桌面 Qt kit，例如 `Desktop Qt 6.10.2 MinGW 64-bit`。
3. 构建并运行。

### 构建要求

- Qt 6.10.2 或更高版本
- CMake 3.20 或更高版本
- 支持 C++20 的 C++ 编译器（GCC、Clang 或 MSVC）

## 技术栈

- **GUI 框架**: Qt Widgets
- **构建系统**: CMake
- **编程语言**: C++20
- **图像处理**: Qt 内置图像处理库

## 注意事项

当前的图像操作仅用于预览，不会修改源图像。计划在后续版本中添加 LibRaw 库来支持 RAW 和 DNG 格式。

## 设计理念

本项目参考 Karaimer 和 Brown 在 ECCV 2016 提出的相机成像管线平台理念，目的是将相机内部不可见的 ISP 黑盒拆解成可观察、可操作、可实验的阶段，帮助学习者更好地理解图像处理的每一步。

## 功能规划

**第一版的关键目标**：
- 验证 ISP 管道架构
- 提供清晰的图像语义展示
- 支持参数调节和中间结果预览
- 为后续扩展预留接口

**计划中的功能**（后续版本）：
- RAW/DNG 格式完整支持
- GPU 加速处理
- 高级去噪和去马赛克算法
- HDR 图像支持
- AI/学习型 ISP 扩展
- 多帧融合

## 许可证

待补充

## 贡献

欢迎提交问题和拉取请求！

## 联系方式

如有问题或建议，请通过 GitHub 联系。
