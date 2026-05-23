#ifndef PREVIEWPROCESSOR_H
#define PREVIEWPROCESSOR_H

#include <QImage>

struct PreviewParams
{
    double blackLevel = 0.0;
    double normalizeScale = 1.0;
    double redGain = 1.0;
    double blueGain = 1.0;
    double colorSaturation = 1.0;
    double exposureEv = 0.0;
};

class PreviewProcessor
{
public:
    QImage process(const QImage &source, const PreviewParams &params, int stageIndex) const;
};

#endif // PREVIEWPROCESSOR_H
