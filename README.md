# Prism

[中文版本](README_CN.md)

Prism is a Qt Widgets desktop workbench for learning and debugging image pipeline stages. The current MVP is intentionally small and runs with the Qt Creator project in `Prism/`.

## Current Features

- Open common image files with Qt image plugins.
- Preview the image in fit-to-window or actual-size mode.
- Select placeholder ISP pipeline stages.
- Adjust preview white balance with red and blue gain sliders.
- Adjust preview exposure in EV.
- Reset preview parameters.
- Export the current processed preview as PNG or JPEG.
- Inspect RGB histogram, image metadata, and log output.
- RAW file extensions appear in the open dialog, with a clear dependency message when RAW decoding is unavailable.

## Project Structure

```
Prism/
├── CMakeLists.txt              # CMake build configuration
├── main.cpp                    # Application entry point
├── mainwindow.cpp/h            # Main window interface
├── mainwindow.ui               # UI design file
├── pipelinemodel.cpp/h         # ISP pipeline data model
├── previewprocessor.cpp/h      # Image processing and preview logic
├── histogramrenderer.cpp/h     # Histogram renderer
└── build/                      # Build output directory
```

## Quick Start

### Running with Qt Creator

1. Open `Prism/CMakeLists.txt` in Qt Creator.
2. Select a desktop Qt kit, for example `Desktop Qt 6.10.2 MinGW 64-bit`.
3. Build and run.

### Build Requirements

- Qt 6.10.2 or higher
- CMake 3.20 or higher
- C++20 compatible compiler (GCC, Clang, or MSVC)

## Technology Stack

- **GUI Framework**: Qt Widgets
- **Build System**: CMake
- **Programming Language**: C++20
- **Image Processing**: Qt built-in image processing library

## Notes

The current image operations are preview-only and do not modify the source image. RAW/DNG decoding requires a RAW decoder such as LibRaw; the Qt-only build currently shows a clear message for RAW files that Qt cannot decode.

## Design Philosophy

This project is inspired by the camera imaging pipeline platform concept proposed by Karaimer and Brown at ECCV 2016. The goal is to disassemble the black-box ISP inside cameras into observable, manipulable, and experimentable stages, helping learners better understand each step of image processing.

## Planned Features

**Phase 1 Objectives**:
- Validate ISP pipeline architecture
- Provide clear visual representation of image semantics
- Support parameter adjustment and intermediate result preview
- Reserve interfaces for future extensions

**Future Features** (upcoming versions):
- Complete RAW/DNG format support
- GPU-accelerated processing
- Advanced denoising and demosaicing algorithms
- HDR image support
- AI/learned ISP extensions
- Multi-frame fusion

## License

To be determined

## Contributing

Issues and pull requests are welcome!

## Contact

For questions or suggestions, please reach out through GitHub.
