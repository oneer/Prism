#include "previewprocessor.h"

#include <algorithm>
#include <cmath>

QImage PreviewProcessor::process(const QImage &source, const PreviewParams &params) const
{
    if (source.isNull()
        || (params.exposureEv == 0.0 && params.redGain == 1.0 && params.blueGain == 1.0)) {
        return source;
    }

    QImage preview = source.convertToFormat(QImage::Format_ARGB32);
    const double exposureScale = std::pow(2.0, params.exposureEv);

    for (int y = 0; y < preview.height(); ++y) {
        auto *line = reinterpret_cast<QRgb *>(preview.scanLine(y));
        for (int x = 0; x < preview.width(); ++x) {
            const QRgb pixel = line[x];
            const int red = std::clamp(static_cast<int>(qRed(pixel) * params.redGain * exposureScale), 0, 255);
            const int green = std::clamp(static_cast<int>(qGreen(pixel) * exposureScale), 0, 255);
            const int blue = std::clamp(static_cast<int>(qBlue(pixel) * params.blueGain * exposureScale), 0, 255);
            line[x] = qRgba(red, green, blue, qAlpha(pixel));
        }
    }

    return preview;
}
