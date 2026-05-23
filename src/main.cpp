#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("cachyos-updater");
    QApplication::setApplicationDisplayName("CachyOS Updater");
    QApplication::setApplicationVersion(CACHYOS_UPDATER_VERSION);
    QApplication::setOrganizationName("CachyOS Updater");
    QApplication::setDesktopFileName("cachyos-updater");
    QApplication::setWindowIcon(QIcon::fromTheme("cachyos-updater",
        QIcon(":/icons/cachyos-updater.svg")));

    QCommandLineParser parser;
    parser.setApplicationDescription("Actualizador grafico simple para CachyOS");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    MainWindow window;
    window.show();
    return app.exec();
}
