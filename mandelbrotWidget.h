#pragma once

#include <QWidget>
#include <QImage>
#include <QPoint>
#include <vector>
#include <thread>
#include "coloring.h"

class MandelbrotWidget : public QWidget {
    Q_OBJECT
public:
    MandelbrotWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    bool isInsideCardioidOrBulb(double cr, double ci) const;
    QColor computePixelColor(double cr, double ci) const;

private:
/*     static constexpr int WIDTH = 3840;
    static constexpr int HEIGHT = 2160; */
    static constexpr int WIDTH = 1600;
    static constexpr int HEIGHT = 1200;
    int computeMaxIterations() const;
    

    ColoringMode currentMode = ColoringMode::Linear;
    bool enableSupersampling = true;

    double minRe = -2.0;
    double maxRe = 1.0;
    double minIm = -1.2;
    double maxIm = 1.2;

    QPoint lastMousePos;
    QImage image;
    bool needsRedraw = true;

    void generateImage();
};
