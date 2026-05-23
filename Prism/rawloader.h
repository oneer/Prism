#ifndef RAWLOADER_H
#define RAWLOADER_H

#include <QImage>
#include <QString>

struct RawLoadResult
{
    QImage image;
    QString formatName;
    QString errorMessage;
};

class RawLoader
{
public:
    static bool isRawFile(const QString &filePath);
    RawLoadResult load(const QString &filePath) const;
};

#endif // RAWLOADER_H
