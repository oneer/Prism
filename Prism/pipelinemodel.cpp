#include "pipelinemodel.h"

PipelineModel::PipelineModel()
    : pipelineStages({
          {"raw-decode", "01 Raw Decode", "Load source pixels into the working pipeline.", "Image file pixels after Qt image loading.", "Working RGB preview image.", "Implemented: loads common image formats through QImageReader."},
          {"black-level", "02 Black Level", "Subtract sensor black level offset.", "Working RGB preview image.", "Preview after black level correction.", "Implemented: subtracts the Black Level offset control."},
          {"normalize", "03 Normalize", "Normalize pixel values into the working range.", "Black level corrected preview.", "Normalized working preview.", "Implemented: applies the Normalize scale control."},
          {"demosaic", "04 Demosaic", "Reconstruct RGB pixels from a Bayer pattern.", "Normalized sensor mosaic data.", "RGB preview image.", "Placeholder: loaded images are already RGB, so no demosaic is applied."},
          {"white-balance", "05 White Balance", "Apply red and blue channel gain controls.", "RGB preview image.", "White balanced RGB preview.", "Implemented: red and blue gain sliders affect this stage and later stages."},
          {"color-matrix", "06 Color Matrix", "Transform camera colors into display colors.", "White balanced RGB preview.", "Display color RGB preview.", "Implemented: applies a saturation-style color matrix preview."},
          {"exposure", "07 Exposure", "Adjust preview brightness in exposure stops.", "Display color RGB preview.", "Exposure adjusted preview.", "Implemented: EV slider affects this stage and later stages."},
          {"tone-mapping", "08 Tone Mapping", "Map high dynamic range values into display range.", "Exposure adjusted preview.", "Tone mapped preview.", "Implemented: simple luminance compression is applied."},
          {"gamma", "09 Gamma", "Apply display gamma response.", "Tone mapped preview.", "Gamma encoded preview.", "Implemented: display gamma 2.2 preview is applied."},
          {"display-preview", "10 Display Preview", "Show the final preview output.", "Gamma encoded preview.", "Final preview image.", "Implemented: displays the current final pipeline preview."},
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
