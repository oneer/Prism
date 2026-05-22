#include "previewprocessor.h"

#include <algorithm>
#include <cmath>

QImage PreviewProcessor::process(const QImage &source, const PreviewParams &params, int stageIndex) const
{
    if (source.isNull()) {
        return source;
    }

    const bool appliesWhiteBalance = stageIndex >= 4;
    const bool appliesExposure = stageIndex >= 6;
    const double redGain = appliesWhiteBalance ? params.redGain : 1.0;
    const double blueGain = appliesWhiteBalance ? params.blueGain : 1.0;
    const double exposureEv = appliesExposure ? params.exposureEv : 0.0;

    if (exposureEv == 0.0 && redGain == 1.0 && blueGain == 1.0) {
        return source;
    }

    QImage preview = source.convertToFormat(QImage::Format_ARGB32);
    const double exposureScale = std::pow(2.0, exposureEv);

    for (int y = 0; y < preview.height(); ++y) {
        auto *line = reinterpret_cast<QRgb *>(preview.scanLine(y));
        for (int x = 0; x < preview.width(); ++x) {
            const QRgb pixel = line[x];
            const int red = std::clamp(static_cast<int>(qRed(pixel) * redGain * exposureScale), 0, 255);
            const int green = std::clamp(static_cast<int>(qGreen(pixel) * exposureScale), 0, 255);
            const int blue = std::clamp(static_cast<int>(qBlue(pixel) * blueGain * exposureScale), 0, 255);
            line[x] = qRgba(red, green, blue, qAlpha(pixel));
        }
    }

    return preview;
}
