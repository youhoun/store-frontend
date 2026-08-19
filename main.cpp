#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Sabai Books");
    QApplication::setOrganizationName("Sabai Books");
    QApplication::setStyle("Fusion");

    MainWindow window;
    window.show();
    return app.exec();
}
