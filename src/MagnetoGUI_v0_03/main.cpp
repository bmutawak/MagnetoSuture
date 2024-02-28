#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QDebug>
#include <QElapsedTimer>
#include <QStyleFactory>

//------------------------------------------------------------------------

int main(int argc, char *argv[])
{
//    QElapsedTimer *startTimer = new QElapsedTimer();
//    startTimer->restart();

    QApplication a(argc, argv);

    MainWindow GUI;
    GUI.showMaximized();


//    qDebug() << "Start Time: " << startTimer->nsecsElapsed()/(pow(10,9));
//    startTimer->invalidate();

    return a.exec();
}

//------------------------------------------------------------------------
