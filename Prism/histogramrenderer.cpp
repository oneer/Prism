#include "histogramrenderer.h"

#include <QColor>
#include <QPainter>
#include <QString>

#include <algorithm>
#include <array>

QPixmap HistogramRenderer::renderRgbHistogram(const QImage &source) const
{
    const int histogramWidth = 512;
    const int histogramHeight = 180;

    QPixmap histogram(histogramWidth, histogramHeight);
    histogram.fill(QColor("#202124"));

    if (source.isNull()) {
        return histogram;
    }

    std::array<int, 256> redBins {};
    std::array<int, 256> greenBins {};
    std::array<int, 256> blueBins {};

    const QImage image = source.convertToFormat(QImage::Format_ARGB32);
    const int sampleStep = std::max(1, image.width() * image.height() / 250000);

    int sampleIndex = 0;
    int sampledPixels = 0;
    int clippedShadowPixels = 0;
    int clippedHighlightPixels = 0;
    for (int y = 0; y < image.height(); ++y) {
        const auto *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            if ((sampleIndex++ % sampleStep) != 0) {
                continue;
            }

            const QRgb pixel = line[x];
            const int red = qRed(pixel);
            const int green = qGreen(pixel);
            const int blue = qBlue(pixel);
            ++redBins[red];
            ++greenBins[green];
            ++blueBins[blue];
            ++sampledPixels;
            if (red == 0 || green == 0 || blue == 0) {
                ++clippedShadowPixels;
            }
            if (red == 255 || green == 255 || blue == 255) {
                ++clippedHighlightPixels;
            }
        }
    }

    const int bottomPadding = 18;
    const int graphHeight = histogramHeight - bottomPadding;
    const int maxBin = std::max({
        *std::max_element(redBins.begin(), redBins.end()),
        *std::max_element(greenBins.begin(), greenBins.end()),
        *std::max_element(blueBins.begin(), blueBins.end())
    });

    QPainter painter(&histogram);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(QColor("#3a3a3a"));
    painter.drawLine(0, graphHeight, histogramWidth, graphHeight);

    if (maxBin > 0) {
        auto drawChannel = [&](const std::array<int, 256> &bins, const QColor &color) {
            painter.setPen(color);
            for (int i = 0; i < 256; ++i) {
                const int x = i * 2;
                const int height = static_cast<int>((static_cast<double>(bins[i]) / maxBin) * (graphHeight - 8));
                painter.drawLine(x, graphHeight, x, graphHeight - height);
                painter.drawLine(x + 1, graphHeight, x + 1, graphHeight - height);
            }
        };

        drawChannel(redBins, QColor(255, 80, 80, 180));
        drawChannel(greenBins, QColor(80, 220, 120, 180));
        drawChannel(blueBins, QColor(90, 150, 255, 180));
    }

    painter.setPen(QColor("#d0d0d0"));
    painter.drawText(8, histogramHeight - 5, "RGB histogram");
    if (sampledPixels > 0) {
        const double shadowPercent = clippedShadowPixels * 100.0 / sampledPixels;
        const double highlightPercent = clippedHighlightPixels * 100.0 / sampledPixels;
        painter.drawText(
            150,
            histogramHeight - 5,
            QString("Shadow clip %1%   Highlight clip %2%")
                .arg(shadowPercent, 0, 'f', 2)
                .arg(highlightPercent, 0, 'f', 2));
    }

    return histogram;
}
