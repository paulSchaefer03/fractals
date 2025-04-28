#pragma once

#include <QColor>

enum class ColoringMode {
    Linear,
    Sinusoidal,
    Smooth,
    ColorMap,
    Distance  
};

QColor getColor(int iter, int maxIter, double zn, ColoringMode mode);
