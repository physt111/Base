#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "DeviceDescriptor.h"
#include <iostream>
#include "Channel_Monitor.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QList<DeviceDescriptor> devicesList = DeviceDescriptor::GetDevicesList();
    for(QList<DeviceDescriptor>::iterator curr = devicesList.begin(); curr != devicesList.end(); curr++)
        std::cout<<(*curr).ToString().toStdString()<<std::endl;

    EEG_obj = new MBN_EEG::EEG(devicesList.at(devicesList.count()-2));
    EEG_obj->CS_SetChannelLed(MBN_EEG::EEG_Device_Architecture::EEG_CommonArc::EEG_CHANNEL_A1,MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_ORANGE);

    impedancePtr = new MBN_Interfaceless_ImpedanceMeasurer();
    std::cout<<"EEG_obj = "<<EEG_obj<<std::endl;
    impedancePtr->ConnectToEEG(EEG_obj);
    connect(this->impedancePtr, SIGNAL(ImpedanceChanged(int,float,bool)), this, SLOT(gotImpedance(int,float,bool)));

    impedancePtr->SwitchOnImpedanceMode();

}

MainWindow::~MainWindow()
{
    delete ui;
    impedancePtr->ConnectToEEG(0);
    delete EEG_obj;
    delete impedancePtr;
}
void MainWindow::gotImpedance(int channel_index, float resistance, bool overflow)
{
    std::cout<<"IMPEDANCE"<<std::endl;
    std::cout<<"channel_index = "<<channel_index<<" resistance = "<<resistance<<" overflow = "<<overflow<<std::endl;
}
