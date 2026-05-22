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
    const bool appliesToneMapping = stageIndex >= 7;
    const bool appliesGamma = stageIndex >= 8;
    const double redGain = appliesWhiteBalance ? params.redGain : 1.0;
    const double blueGain = appliesWhiteBalance ? params.blueGain : 1.0;
    const double exposureEv = appliesExposure ? params.exposureEv : 0.0;

    if (exposureEv == 0.0 && redGain == 1.0 && blueGain == 1.0 && !appliesToneMapping && !appliesGamma) {
        return source;
    }

    QImage preview = source.convertToFormat(QImage::Format_ARGB32);
    const double exposureScale = std::pow(2.0, exposureEv);
    const auto mapChannel = [appliesToneMapping, appliesGamma](double value) {
        value = std::max(0.0, value);
        if (appliesToneMapping) {
            value = value / (1.0 + value);
        }
        if (appliesGamma) {
            value = std::pow(value, 1.0 / 2.2);
        }

        return std::clamp(static_cast<int>(value * 255.0), 0, 255);
    };

    for (int y = 0; y < preview.height(); ++y) {
        auto *line = reinterpret_cast<QRgb *>(preview.scanLine(y));
        for (int x = 0; x < preview.width(); ++x) {
            const QRgb pixel = line[x];
            const int red = mapChannel(qRed(pixel) / 255.0 * redGain * exposureScale);
            const int green = mapChannel(qGreen(pixel) / 255.0 * exposureScale);
            const int blue = mapChannel(qBlue(pixel) / 255.0 * blueGain * exposureScale);
            line[x] = qRgba(red, green, blue, qAlpha(pixel));
        }
    }

    return preview;
}
