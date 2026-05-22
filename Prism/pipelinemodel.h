#ifndef PIPELINEMODEL_H
#define PIPELINEMODEL_H

#include <QString>
#include <QVector>

struct PipelineStage
{
    QString id;
    QString displayName;
    QString description;
};

class PipelineModel
{
public:
    PipelineModel();

    const QVector<PipelineStage> &stages() const;
    const PipelineStage *stageAt(int row) const;

private:
    QVector<PipelineStage> pipelineStages;
};

#endif // PIPELINEMODEL_H
