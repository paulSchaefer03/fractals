#pragma once

#include <QWidget>
#include <QImage>
#include <QPoint>
#include <vector>
#include <QTimer>
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
    QSize getRenderSize() const;
    void showGotoDialog();
    void createScreenshot();
    QImage renderFractalAtSize(QSize size, int maxIter) const;


private:
/*     static constexpr int WIDTH = 3840;
    static constexpr int HEIGHT = 2160; */
    static constexpr int WIDTH = 1600;
    static constexpr int HEIGHT = 1200;
    static constexpr int BLOCKHEIGHT = 16; //Da muss man mal aufprobieren zwischen 8-16
/*     static constexpr int WIDTH = 400;
    static constexpr int HEIGHT = 300; */
    int computeMaxIterations(bool) const;
    

    ColoringMode currentMode = ColoringMode::Linear;
    bool enableSupersampling = true;

    double minRe = -2.0;
    double maxRe = 1.0;
    double minIm = -1.2;
    double maxIm = 1.2;

    QPoint lastMousePos;
    QImage image;
    bool needsRedraw = true;
    bool quickRender = false;

    QTimer redrawTimer; 

    void generateImage();
};
