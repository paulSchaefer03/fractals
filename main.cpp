#include <QApplication>
#include "MandelbrotWidget.h"

//Press 1, 2, or 3 to change the coloring mode
// 1: Linear, 2: Sinusoidal, 3: Smooth

//Test Coordinates
//0,250201862649403
//0,000005530167356
//0,00000572
//Test Coordinates 2
//0,250201452681041
//0,000005544017638
//0,000000859
//Test Coordinates 2
//0,250201361068745
//0,000005464799947
//0,000000859

// Todo Bookmarks
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
