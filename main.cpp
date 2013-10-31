#include <QtGui/QApplication>
#include "mainwindow.h"
#include "EEG.h"
#include <iostream>

int joo = 0;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
