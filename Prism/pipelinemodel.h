#ifndef PIPELINEMODEL_H
#define PIPELINEMODEL_H

#include <QString>
#include <QVector>

struct PipelineStage
{
    QString id;
    QString displayName;
    QString description;
    QString inputDescription;
    QString outputDescription;
    QString implementationStatus;
};

class PipelineModel
{
public:
    PipelineModel();

    const QVector<PipelineStage> &stages() const;
    const PipelineStage *stageAt(int row) const;
    int indexOfStage(const QString &id) const;

private:
    QVector<PipelineStage> pipelineStages;
};

#endif // PIPELINEMODEL_H
