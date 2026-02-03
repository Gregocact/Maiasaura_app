#include <QApplication>
#include "MainWindow.h"
#include <QMessageBox>
#include <QtMessageHandler>
#include <QDebug>

static void messageHandler(QtMsgType type, const QMessageLogContext&, const QString& msg) {
    // Print to terminal
    fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());

    // Also show warnings/critical as a popup so you can't miss them
    if (type == QtWarningMsg || type == QtCriticalMsg || type == QtFatalMsg) {
        QMessageBox::warning(nullptr, "Qt message", msg);
    }
}


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}