#include "coloring.h"
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

QColor linearColoring(int iter, int maxIter) {
    if (iter == maxIter) return QColor(0, 0, 0);
    int r = 255 * iter / maxIter;
    int g = 0;
    int b = 255 * iter / maxIter;
    return QColor(r, g, b);
}

QColor sinusoidalColoring(int iter, int maxIter) {
    if (iter == maxIter) return QColor(0, 0, 0);
    double t = (double)iter / maxIter;
    int r = static_cast<int>(9 * (1 - t) * t * t * t * 255);
    int g = static_cast<int>(15 * (1 - t) * (1 - t) * t * t * 255);
    int b = static_cast<int>(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
    return QColor(r, g, b);
}

QColor smoothColoring(int iter, int maxIter, double zn) {
    if (iter == maxIter) return QColor(0, 0, 0);
    double mu = iter + 1 - log(log(zn)) / log(2.0);
    double t = mu / maxIter;
    int r = static_cast<int>(255 * t);
    int g = static_cast<int>(255 * (1 - t));
    int b = static_cast<int>(255 * (0.5 + 0.5 * cos(3.0 * t * M_PI)));
    return QColor(r, g, b);
}

QColor colorMapColoring(int iter, int maxIter) {
    static const QColor mapping[16] = {
        QColor(66, 30, 15),
        QColor(25, 7, 26),
        QColor(9, 1, 47),
        QColor(4, 4, 73),
        QColor(0, 7, 100),
        QColor(12, 44, 138),
        QColor(24, 82, 177),
        QColor(57, 125, 209),
        QColor(134, 181, 229),
        QColor(211, 236, 248),
        QColor(241, 233, 191),
        QColor(248, 201, 95),
        QColor(255, 170, 0),
        QColor(204, 128, 0),
        QColor(153, 87, 0),
        QColor(106, 52, 3)
    };

    if (iter < maxIter && iter > 0) {
        int i = iter % 16;
        return mapping[i];
    } else {
        return Qt::black;
    }
}

QColor distanceColoring(double distance) {
    double normalized = std::min(1.0, std::max(0.0, distance * 0.1)); 
    int shade = static_cast<int>(255 * (1.0 - normalized)); // 0 = hell, 255 = dunkel
    return QColor(shade, shade, shade);
}


QColor getColor(int iter, int maxIter, double zn, ColoringMode mode) {
    switch (mode) {
        case ColoringMode::Linear:
            return linearColoring(iter, maxIter);
        case ColoringMode::Sinusoidal:
            return sinusoidalColoring(iter, maxIter);
        case ColoringMode::Smooth:
            return smoothColoring(iter, maxIter, zn);
        case ColoringMode::ColorMap:
            return colorMapColoring(iter, maxIter);
        case ColoringMode::Distance:
            return distanceColoring(zn);
        default:
            return QColor(0, 0, 0);
    }
}




