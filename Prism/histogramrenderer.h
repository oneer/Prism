#ifndef HISTOGRAMRENDERER_H
#define HISTOGRAMRENDERER_H

#include <QImage>
#include <QPixmap>

class HistogramRenderer
{
public:
    QPixmap renderRgbHistogram(const QImage &image) const;
};

#endif // HISTOGRAMRENDERER_H
