# Prism

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

## Run With Qt Creator

1. Open `Prism/CMakeLists.txt` in Qt Creator.
2. Select a desktop Qt kit, for example `Desktop Qt 6.10.2 MinGW 64-bit`.
3. Build and run.

## Notes

The current image operations are preview-only and do not modify the source image. RAW/DNG support is planned for a later step after adding LibRaw.
