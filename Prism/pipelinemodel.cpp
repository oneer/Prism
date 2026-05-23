#include "pipelinemodel.h"

PipelineModel::PipelineModel()
    : pipelineStages({
          {"raw-decode", "01 Raw Decode", "Load source pixels into the working pipeline."},
          {"black-level", "02 Black Level", "Subtract sensor black level offset."},
          {"normalize", "03 Normalize", "Normalize pixel values into the working range."},
          {"demosaic", "04 Demosaic", "Reconstruct RGB pixels from a Bayer pattern."},
          {"white-balance", "05 White Balance", "Apply red and blue channel gain controls."},
          {"color-matrix", "06 Color Matrix", "Transform camera colors into display colors."},
          {"exposure", "07 Exposure", "Adjust preview brightness in exposure stops."},
          {"tone-mapping", "08 Tone Mapping", "Map high dynamic range values into display range."},
          {"gamma", "09 Gamma", "Apply display gamma response."},
          {"display-preview", "10 Display Preview", "Show the final preview output."},
      })
{
}

const QVector<PipelineStage> &PipelineModel::stages() const
{
    return pipelineStages;
}

const PipelineStage *PipelineModel::stageAt(int row) const
{
    if (row < 0 || row >= pipelineStages.size()) {
        return nullptr;
    }

    return &pipelineStages[row];
}

int PipelineModel::indexOfStage(const QString &id) const
{
    for (int row = 0; row < pipelineStages.size(); ++row) {
        if (pipelineStages[row].id == id) {
            return row;
        }
    }

    return -1;
}
