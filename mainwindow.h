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

    static void CBusualData(void* u_ptr, int ch0val, int ch1val, int ch2val, int ch3val);

    ~MainWindow();

public slots:

    void gotImpedance(int channel_index, float resistance, bool overflow);
    
private:
    Ui::MainWindow *ui;
    MBN_EEG::EEG* EEG_obj;
    MBN_Interfaceless_ImpedanceMeasurer* impedancePtr;
    MBN_EEG::EEG_Event_Vector_D_UsualData* vecDataPtr;
};

#endif // MAINWINDOW_H
