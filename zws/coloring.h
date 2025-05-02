#pragma once

#include <QColor>

enum class ColoringMode {
    Linear,
    Sinusoidal,
    Smooth,
    ColorMap,
    Distance,
    Rainbow  
};

QColor getColor(int iter, int maxIter, double zn, ColoringMode mode, double modulus);
