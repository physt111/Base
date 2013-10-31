#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "EEG.h"
#include "Channel_Monitor.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:

    void gotImpedance(int channel_index, float resistance, bool overflow);
    
private:
    Ui::MainWindow *ui;
    MBN_EEG::EEG* EEG_obj;
    MBN_Interfaceless_ImpedanceMeasurer* impedancePtr;
};

#endif // MAINWINDOW_H
