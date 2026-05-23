#include "rawloader.h"

#include <QColor>
#include <QFileInfo>
#include <QStringList>

#ifdef PRISM_HAS_LIBRAW
#include <libraw/libraw.h>
#endif

bool RawLoader::isRawFile(const QString &filePath)
{
    const QString suffix = QFileInfo(filePath).suffix().toLower();
    return QStringList({"dng", "raw", "nef", "cr2", "cr3", "arw"}).contains(suffix);
}

RawLoadResult RawLoader::load(const QString &filePath) const
{
#ifdef PRISM_HAS_LIBRAW
    RawLoadResult result;
    result.formatName = "RAW";

    LibRaw processor;
    int status = processor.open_file(QFileInfo(filePath).absoluteFilePath().toLocal8Bit().constData());
    if (status != LIBRAW_SUCCESS) {
        result.errorMessage = "LibRaw could not open this RAW file:\n" + QString::fromLocal8Bit(libraw_strerror(status));
        return result;
    }

    status = processor.unpack();
    if (status != LIBRAW_SUCCESS) {
        result.errorMessage = "LibRaw could not unpack this RAW file:\n" + QString::fromLocal8Bit(libraw_strerror(status));
        return result;
    }

    processor.imgdata.params.output_bps = 8;
    processor.imgdata.params.use_camera_wb = 1;
    status = processor.dcraw_process();
    if (status != LIBRAW_SUCCESS) {
        result.errorMessage = "LibRaw could not process this RAW file:\n" + QString::fromLocal8Bit(libraw_strerror(status));
        return result;
    }

    libraw_processed_image_t *rawImage = processor.dcraw_make_mem_image(&status);
    if (!rawImage || status != LIBRAW_SUCCESS) {
        if (rawImage) {
            LibRaw::dcraw_clear_mem(rawImage);
        }
        result.errorMessage = "LibRaw could not create a preview image:\n" + QString::fromLocal8Bit(libraw_strerror(status));
        return result;
    }

    if (rawImage->type != LIBRAW_IMAGE_BITMAP || rawImage->colors < 3 || rawImage->bits != 8) {
        LibRaw::dcraw_clear_mem(rawImage);
        result.errorMessage = "LibRaw returned an unsupported RAW preview format.";
        return result;
    }

    QImage image(rawImage->width, rawImage->height, QImage::Format_ARGB32);
    const int colors = rawImage->colors;
    const unsigned char *source = rawImage->data;
    for (unsigned int y = 0; y < rawImage->height; ++y) {
        auto *line = reinterpret_cast<QRgb *>(image.scanLine(static_cast<int>(y)));
        for (unsigned int x = 0; x < rawImage->width; ++x) {
            const unsigned int offset = (y * rawImage->width + x) * colors;
            line[x] = qRgba(source[offset], source[offset + 1], source[offset + 2], 255);
        }
    }

    LibRaw::dcraw_clear_mem(rawImage);
    result.image = image;
    return result;
#else
    Q_UNUSED(filePath);

    RawLoadResult result;
    result.formatName = "RAW";
    result.errorMessage =
        "RAW decoding is not available in this Qt-only build yet.\n\n"
        "Install and connect a RAW decoder such as LibRaw, or convert the file to TIFF/PNG first.";
    return result;
#endif
}
