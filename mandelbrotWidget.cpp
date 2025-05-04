#include "MandelbrotWidget.h"
#include "mandelbrot.h"
#include "coloring.h"
#include <QPainter>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <cmath>
#include <QInputDialog>
#include <QFileDialog>
#include <QDateTime>
#include <QInputDialog>

void MandelbrotWidget::showGotoDialog() {
    bool ok1, ok2, ok3;

    double centerRe = (minRe + maxRe) / 2.0;
    double centerIm = (minIm + maxIm) / 2.0;
    double reRange = maxRe - minRe;

    double newRe = QInputDialog::getDouble(this, "Goto Position", "Center Realteil:", centerRe, -10.0, 10.0, 15, &ok1);
    if (!ok1) return;

    double newIm = QInputDialog::getDouble(this, "Goto Position", "Center Imaginärteil:", centerIm, -10.0, 10.0, 15, &ok2);
    if (!ok2) return;

    double newRange = QInputDialog::getDouble(this, "Goto Position", "Realbereichweite:", reRange, 1e-20, 4.0, 10, &ok3);
    if (!ok3) return;

    double halfRange = newRange / 2.0;

    minRe = newRe - halfRange;
    maxRe = newRe + halfRange;
    minIm = newIm - halfRange * height() / width();
    maxIm = newIm + halfRange * height() / width();

    quickRender = true;
    needsRedraw = true;
    update();

    redrawTimer.start(100); // Vollauflösung nachziehen
}


MandelbrotWidget::MandelbrotWidget(QWidget* parent)
    : QWidget(parent) {
        connect(&redrawTimer, &QTimer::timeout, this, [this]() {
            quickRender = false;
            needsRedraw = true;
            update();
        });
        redrawTimer.setSingleShot(true);
    }

void MandelbrotWidget::paintEvent(QPaintEvent*) {
    if (needsRedraw) {
        generateImage();
        needsRedraw = false;
    }

    QPainter painter(this);
    painter.drawImage(rect(), image);
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);

    QString statusText;
    if (quickRender) {
        statusText = "Rendering: Quick Preview (Low Iterations)";
    } else {
        statusText = "Rendering: Full Detail";
    }
    
    QString zoomText = QString("Zoomlevel (Realbereich): %1").arg(maxRe - minRe, 0, 'e', 2);
    double centerRe = (minRe + maxRe) / 2.0;
    double centerIm = (minIm + maxIm) / 2.0;
    QString centerText = QString("Center: (%1, %2)")
        .arg(centerRe, 0, 'f', 15)
        .arg(centerIm, 0, 'f', 15);
    
    // Hintergrund für bessere Lesbarkeit
    QRect rect(10, 10, 450, 60);
    painter.fillRect(rect, QColor(0, 0, 0, 150));
    

    painter.drawText(20, 25, statusText);
    painter.drawText(20, 45, zoomText);
    painter.drawText(20, 65, centerText);
    

}

QSize MandelbrotWidget::getRenderSize() const {
    if (quickRender) {
        return QSize(width() / 2, height() / 2);
    } else {
        return QSize(width(), height());
    }
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
    double dr = 1.0, di = 0.0; // Ableitung

    int iter = 0;
    int maxIter = computeMaxIterations(quickRender);

    while (zr * zr + zi * zi <= 4.0 && iter < maxIter) {
        double tempZr = zr * zr - zi * zi + cr;
        double tempDr = 2.0 * (zr * dr - zi * di) + 1.0;
        double tempDi = 2.0 * (zr * di + zi * dr);

        zi = 2.0 * zr * zi + ci;
        zr = tempZr;
        dr = tempDr;
        di = tempDi;
        iter++;
    }

    double modulus = sqrt(zr * zr + zi * zi);
    double derivative = sqrt(dr * dr + di * di);
    double distance = modulus * log(modulus) / derivative;

    return getColor(iter, maxIter, distance, currentMode, modulus); 
}

void MandelbrotWidget::createScreenshot() {
    bool okW, okH;

    int width = QInputDialog::getInt(this, "Screenshot", "Breite (z.B. 1920):", 1920, 100, 10000, 1, &okW);
    if (!okW) return;

    int height = QInputDialog::getInt(this, "Screenshot", "Höhe (z.B. 1080):", 1080, 100, 10000, 1, &okH);
    if (!okH) return;

    QSize targetSize(width, height);
    int iter = computeMaxIterations(false); // volle Qualität

    QImage highres = renderFractalAtSize(targetSize, iter);

    QString filename = QString("mandelbrot_%1x%2_%3.png")
        .arg(width)
        .arg(height)
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));

    QString savePath = QFileDialog::getSaveFileName(this, "Screenshot speichern", filename, "PNG Dateien (*.png)");
    if (!savePath.isEmpty()) {
        highres.save(savePath);
    }
}

QImage MandelbrotWidget::renderFractalAtSize(QSize size, int maxIter) const {
    int w = size.width();
    int h = size.height();
    QImage result(size, QImage::Format_RGB32);

    const int blockHeight = 8;
    int numBlocks = (h + blockHeight - 1) / blockHeight;
    std::atomic<int> nextBlock(0);

    int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4;

    std::vector<std::thread> threads;

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([=, &result, &nextBlock]() {
            double reFactor = (maxRe - minRe) / (w - 1);
            double imFactor = (maxIm - minIm) / (h - 1);

            while (true) {
                int blockIndex = nextBlock.fetch_add(1);
                if (blockIndex >= numBlocks) break;

                int startY = blockIndex * blockHeight;
                int endY = std::min(startY + blockHeight, h);

                for (int y = startY; y < endY; ++y) {
                    for (int x = 0; x < w; ++x) {
                        double cr = minRe + x * reFactor;
                        double ci = maxIm - y * imFactor;

                        QColor color;
                        if (isInsideCardioidOrBulb(cr, ci)) {
                            color = Qt::black;
                        } else {
                            color = computePixelColor(cr, ci);
                        }

                        result.setPixelColor(x, y, color);
                    }
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    return result;
}


void MandelbrotWidget::generateImage() {
    QSize renderSize = getRenderSize();
    int renderWidth = renderSize.width();
    int renderHeight = renderSize.height();
    image = QImage(renderSize, QImage::Format_RGB32);

    int numThreads = std::thread::hardware_concurrency();
    //printf("Number Threads: %d\n", numThreads);
    if (numThreads == 0) numThreads = 4;

    int numBlocks = (renderHeight + BLOCKHEIGHT - 1) / BLOCKHEIGHT;

    std::atomic<int> nextBlock(0);

    std::vector<std::thread> threads;
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([=, &nextBlock]() {
            double reFactor = (maxRe - minRe) / (renderWidth - 1);
            double imFactor = (maxIm - minIm) / (renderHeight - 1);
            int maxIter = computeMaxIterations(quickRender);

            while (true) {
                int blockIndex = nextBlock.fetch_add(1);
                if (blockIndex >= numBlocks) break;

                int startY = blockIndex * BLOCKHEIGHT;
                int endY = std::min(startY + BLOCKHEIGHT, renderHeight);

                for (int y = startY; y < endY; ++y) {
                    for (int x = 0; x < renderWidth; ++x) {
                        if (enableSupersampling && !quickRender) {
                            // Supersampling (nur bei FullRender)
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

    quickRender = true;
    needsRedraw = true;
    update();

    redrawTimer.start(1000);
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
    }else if (event->key() == Qt::Key_5) {
        currentMode = ColoringMode::Distance;
        needsRedraw = true;
        update();
    }else if (event->key() == Qt::Key_6) {
            currentMode = ColoringMode::Rainbow;
            needsRedraw = true;
            update();
    }else if (event->key() == Qt::Key_G) {
        showGotoDialog();
    }else if (event->key() == Qt::Key_S) {
        createScreenshot();
    }else {
        QWidget::keyPressEvent(event);
    }
}

int MandelbrotWidget::computeMaxIterations(bool quick) const {
    double scale = maxRe - minRe;
    int baseIter = static_cast<int>(500 + 10000 * std::log10(3.0 / scale));
    baseIter = std::clamp(baseIter, 500, 10000);

    if (quick) {
        int quickIter = static_cast<int>(baseIter * 0.25); // 25% der Basis-Iterationen
        return std::clamp(quickIter, 100, baseIter);
    } else {
        return baseIter;
    }
}


