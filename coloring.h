#pragma once

#include <QColor>

enum class ColoringMode {
    Linear,
    Sinusoidal,
    Smooth,
    ColorMap 
};

QColor getColor(int iter, int maxIter, double zn, ColoringMode mode);
