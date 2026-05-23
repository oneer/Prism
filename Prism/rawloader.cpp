#include "rawloader.h"

#include <QFileInfo>
#include <QStringList>

bool RawLoader::isRawFile(const QString &filePath)
{
    const QString suffix = QFileInfo(filePath).suffix().toLower();
    return QStringList({"dng", "raw", "nef", "cr2", "cr3", "arw"}).contains(suffix);
}

RawLoadResult RawLoader::load(const QString &filePath) const
{
    Q_UNUSED(filePath);

    RawLoadResult result;
    result.formatName = "RAW";
    result.errorMessage =
        "RAW decoding is not available in this Qt-only build yet.\n\n"
        "Install and connect a RAW decoder such as LibRaw, or convert the file to TIFF/PNG first.";
    return result;
}
