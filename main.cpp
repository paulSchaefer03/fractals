#include <QApplication>
#include "MandelbrotWidget.h"

//Press 1, 2, or 3 to change the coloring mode
// 1: Linear, 2: Sinusoidal, 3: Smooth

// Center Offset
// Tile-basierte Darstellung
// Lazy Evaluation

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MandelbrotWidget window;
    window.resize(1600, 1200);
    window.setWindowTitle("Mandelbrot Explorer (Qt)");
    window.show();

    return app.exec();
}
