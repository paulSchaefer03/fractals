#include "MandelbrotWidget.h"
#include "mandelbrot.h"
#include "coloring.h"
#include <QPainter>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <cmath>

MandelbrotWidget::MandelbrotWidget(QWidget* parent)
    : QWidget(parent) {}

void MandelbrotWidget::paintEvent(QPaintEvent*) {
    if (needsRedraw) {
        generateImage();
        needsRedraw = false;
    }

    QPainter painter(this);
    painter.drawImage(0, 0, image);
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);

    QString zoomText = QString("Zoomlevel (Realbereich): %1").arg(maxRe - minRe, 0, 'e', 2);
    painter.drawText(10, 20, zoomText);

}

bool MandelbrotWidget::isInsideCardioidOrBulb(double cr, double ci) const {
    double q = (cr - 0.25) * (cr - 0.25) + ci * ci;
    if (q * (q + (cr - 0.25)) < 0.25 * ci * ci)
        return true; // Inside main cardioid
    if ((cr + 1) * (cr + 1) + ci * ci < 0.0625)
        return true; // Inside period-2 bulb
    return false;
}

QColor MandelbrotWidget::computePixelColor(double cr, double ci) const {
    double zr = 0.0, zi = 0.0;
    int iter = 0;
    int maxIter = computeMaxIterations();

    while (zr * zr + zi * zi <= 4.0 && iter < maxIter) {
        double temp = zr * zr - zi * zi + cr;
        zi = 2.0 * zr * zi + ci;
        zr = temp;
        iter++;
    }

    double zn = sqrt(zr * zr + zi * zi);
    return getColor(iter, maxIter, zn, currentMode);
}


void MandelbrotWidget::generateImage() {
    image = QImage(width(), height(), QImage::Format_RGB32);

    int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4;

    std::vector<std::thread> threads;
    int blockSize = height() / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        int startY = i * blockSize;
        int endY = (i == numThreads - 1) ? height() : (i + 1) * blockSize;

        threads.emplace_back([=]() {
            double reFactor = (maxRe - minRe) / (width() - 1);
            double imFactor = (maxIm - minIm) / (height() - 1);

            for (int y = startY; y < endY; ++y) {
                for (int x = 0; x < width(); ++x) {
                    if (enableSupersampling) {
                        // Supersampling (2x2 Subpixel)
                        int red = 0, green = 0, blue = 0;
                        for (int subY = 0; subY < 2; ++subY) {
                            for (int subX = 0; subX < 2; ++subX) {
                                double cr = minRe + (x + (subX + 0.5) / 2.0) * reFactor;
                                double ci = maxIm - (y + (subY + 0.5) / 2.0) * imFactor;

                                QColor color;
                                if (isInsideCardioidOrBulb(cr, ci)) {
                                    color = Qt::black;
                                } else {
                                    color = computePixelColor(cr, ci);
                                }

                                red += color.red();
                                green += color.green();
                                blue += color.blue();
                            }
                        }
                        image.setPixelColor(x, y, QColor(red / 4, green / 4, blue / 4));
                    } else {
                        // Ohne Supersampling
                        double cr = minRe + x * reFactor;
                        double ci = maxIm - y * imFactor;

                        QColor color;
                        if (isInsideCardioidOrBulb(cr, ci)) {
                            color = Qt::black;
                        } else {
                            color = computePixelColor(cr, ci);
                        }
                        image.setPixelColor(x, y, color);
                    }
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }
}


void MandelbrotWidget::wheelEvent(QWheelEvent* event) {
    double zoomFactor = 0.9;

    double reCenter = (minRe + maxRe) / 2.0;
    double imCenter = (minIm + maxIm) / 2.0;
    double reRange = (maxRe - minRe);
    double imRange = (maxIm - minIm);

    if (event->angleDelta().y() > 0) {
        reRange *= zoomFactor;
        imRange *= zoomFactor;
    } else {
        reRange /= zoomFactor;
        imRange /= zoomFactor;
    }

    minRe = reCenter - reRange / 2.0;
    maxRe = reCenter + reRange / 2.0;
    minIm = imCenter - imRange / 2.0;
    maxIm = imCenter + imRange / 2.0;

    needsRedraw = true;
    update();
}

void MandelbrotWidget::mousePressEvent(QMouseEvent* event) {
    lastMousePos = event->pos();
}

void MandelbrotWidget::mouseMoveEvent(QMouseEvent* event) {
    QPoint delta = event->pos() - lastMousePos;
    lastMousePos = event->pos();

    double reRange = maxRe - minRe;
    double imRange = maxIm - minIm;

    double reDelta = -delta.x() * (reRange / width());
    double imDelta = delta.y() * (imRange / height());

    minRe += reDelta;
    maxRe += reDelta;
    minIm += imDelta;
    maxIm += imDelta;

    needsRedraw = true;
    update();
}

void MandelbrotWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_1) {
        currentMode = ColoringMode::Linear;
        needsRedraw = true;
        update();
    } else if (event->key() == Qt::Key_2) {
        currentMode = ColoringMode::Sinusoidal;
        needsRedraw = true;
        update();
    } else if (event->key() == Qt::Key_3) {
        currentMode = ColoringMode::Smooth;
        needsRedraw = true;
        update();
    } else if (event->key() == Qt::Key_4) {
        currentMode = ColoringMode::ColorMap;
        needsRedraw = true;
        update();
    } else {
        QWidget::keyPressEvent(event);
    }
}

int MandelbrotWidget::computeMaxIterations() const {
    double scale = maxRe - minRe; // Bereich auf der Realachse
    int iter = static_cast<int>(500 + 1000 * std::log10(3.0 / scale));
    return std::clamp(iter, 500, 5000); // Mindestens 500, maximal 5000
}
