/*! \file 
	Файл содержит реализацию класса DeviceDescriptor
		DeviceDescriptor - описатель устройства, подключённого по каналу COM или USB
*/ 

#include <string>
#include <qlist.h>
#include <abstractserial.h>
#include <serialdeviceenumerator.h>
#include <qstring.h>
//#include <libusb.h>
//#include <libusbi.h>
#include <qstringlist.h>
#include <PagedArray.cpp>
#include <iostream>
#include <assert.h>
#include <DeviceDescriptor.h>

using namespace std;

/// конструктор для COM подключения
DeviceDescriptor::DeviceDescriptor(const QString Serial_Port_Name): port_type(COM)
{
	serial_port_name = Serial_Port_Name;

    //usb_dev_bus_number = 0;
    //usb_dev_addr = 0;
	//usb_dev_serial_number = QString();
};

/// конструктор для USB подключения
//DeviceDescriptor::DeviceDescriptor(const libusb_device** p_info_array_start, int p_info_idx): port_type(USB)
//{
//	serial_port_name = "";
//	libusb_device* dev = ((libusb_device**)p_info_array_start)[p_info_idx];
		
//	// сразу создаём описатель девайса
//	libusb_get_device_descriptor(dev, &usb_dev_description);

//	// получаем номер шины, к которой подключено устройство
//	usb_dev_bus_number = libusb_get_bus_number(dev);
//	usb_dev_addr = libusb_get_device_address(dev);

//	// на секунду открываем девайс, чтобы прочитать данные о нём
//	libusb_device_handle* hdev;
//	int open_res = libusb_open(dev, &hdev);
//	if (open_res == 0)
//	{
//		// вытаскиваем инфу с девайса
//		unsigned char buffer[DEVICE_DESCRIPTOR_STRING_BUFFER_SIZE];
//		int sn_get_res = libusb_get_string_descriptor_ascii(hdev, usb_dev_description.iSerialNumber, buffer, DEVICE_DESCRIPTOR_STRING_BUFFER_SIZE);
//		if (sn_get_res >= 0)
//			usb_dev_serial_number = QString((const char*)(buffer));
			
//		// закрываем девайс
//		libusb_close(hdev);
//	};
//};

/// конструктор для VIRTUAL подключения
DeviceDescriptor::DeviceDescriptor(): port_type(VIRTUAL)
{};

/// оператор присваивания
DeviceDescriptor& DeviceDescriptor::operator= (const DeviceDescriptor& origin)
{
	this->serial_port_name = origin.serial_port_name;	/// информация соответствующая COM-портовому подключению
    //this->usb_dev_description = origin.usb_dev_description; /// описание подключённого по USB устройства
    //this->usb_dev_serial_number = origin.usb_dev_serial_number; /// серийный номер устройства
    //this->usb_dev_bus_number = origin.usb_dev_bus_number; /// номер шины по которой подключено устройство
    //this->usb_dev_addr = origin.usb_dev_addr; // адрес USB устройства на USB шине

	this->port_type = origin.port_type; /// красившее обозначение типа подключения

	return (*this);
};

/// возвращает список подключённых по USB устройств и COM-портов
QList<DeviceDescriptor> DeviceDescriptor::GetDevicesList()
{
	/// хранилище для наших девайсов
	QList<DeviceDescriptor> devs; 

	// пока мы не работаем с USB устройствами
	///////// инициализируем библиотеку
	//////libusb_init(NULL);

	///////// Первым делом, роем список USB-девайсов
	//////libusb_device ** usb_dev_list;
	//////int usb_dev_count = libusb_get_device_list(NULL, &usb_dev_list); // получаем массив указателей на структуры, описывающие USB устройства
	//////	
	///////// сохраняем его в наш лист
	//////for (int i = 0; i < usb_dev_count; i++)
	//////{
	//////	devs.push_back(DeviceDescriptor((const libusb_device**)usb_dev_list, i));
	//////	libusb_ref_device(usb_dev_list[i]); // устанавливаем количество ссылок на на устройство на 1, чтобы библиотека не удаляла адресуемые записи
	//////};

	///////// (!) ВНИМАНИЕ (!) тут есть проблема, на самом деле, так как у нас выделен массив указателей на libusb_device и сами структуры libusb_device
	///////// при этом, мы не можем удалять или перемещать этот массив сами, так как он может быть внутренне связан с внутренностями библиотеки
	///////// т.е. ничего с ним делать нам нельзя, только удалять ссылки на девайсы средствами библиотеки

	///////// пожалуй, очистим ка мы этот список девайсов
	//////libusb_free_device_list(usb_dev_list, 1);

	/// Теперь роем прекрасные COM-порты
    SerialDeviceEnumerator *sde = new SerialDeviceEnumerator;	// перечислитель COM-портов
    sde->setEnabled(true);
	QList<QString> COM_devs = sde->devicesAvailable();
    sde->setEnabled(false);
    delete sde;

	// сохраняем его в наш лист
	for (QList<QString>::Iterator curr = COM_devs.begin(); curr != COM_devs.end(); curr++)
		devs.push_back(DeviceDescriptor(*curr));

	// добавляем виртуальное устройство
	devs.push_back(DeviceDescriptor());



	return devs;
};

/// Возвращает тип устройства в виде строки
QString DeviceDescriptor::GetTypeStr()
{
	switch (port_type)
	{
	case COM: return QString("COM");
	case USB: return QString("USB");
	case VIRTUAL: return QString("VIRTUAL");
	case NOT_CONNECTED: return QString("NOT_CONNECTED");
	default: 
		assert(false);
		return QString("<unknown>");
	};
};

/// возвращает id продавца
QString DeviceDescriptor::GetVendorId()
{
	switch(port_type)
	{
	case VIRTUAL: return QString();
	case COM:
		{
            SerialDeviceEnumerator *sde = new SerialDeviceEnumerator;	// перечислитель COM-портов
			sde->setDeviceName(this->serial_port_name);
            QString vendorId = sde->vendorID();
            delete sde;
            return vendorId;
		};
//	case USB:
//		return QString().setNum(usb_dev_description.idVendor);
	default: 
		assert(false);
		return QString("<unknown>");
	};
};

/// возвращает id модели
QString DeviceDescriptor::GetProductId()
{
    switch(port_type)
    {
    case VIRTUAL: return QString();
    case COM:
    {
        SerialDeviceEnumerator *sde = new SerialDeviceEnumerator;	// перечислитель COM-портов
        sde->setDeviceName(this->serial_port_name);
        QString vendorId = sde->productID();
        return vendorId;
    };
//    case USB:
//		return  QString().setNum(usb_dev_description.idProduct);
	default: 
		assert(false);
		return QString("<unknown>");
	};
};

/// возвращает серийник модели
QString DeviceDescriptor::GetDeviceSerialNumber()
{
	switch(port_type)
	{
	case VIRTUAL: return QString();
	case COM: return QString();
//	case USB: return  usb_dev_serial_number;
	default: 
		assert(false);
		return QString("<unknown>");
	};
};

/// возвращает номер шины по которой подключено устройство
QString DeviceDescriptor::GetDeviceBusNumber()
{
	switch(port_type)
	{
	case VIRTUAL: return QString();
	case COM: return QString();
//	case USB: return  QString().setNum(usb_dev_bus_number);
	default: 
		assert(false);
		return QString("<unknown>");
	};
};

/// возвращает адрес устройства на USB шине
QString DeviceDescriptor::GetDeviceAddress()
{
	switch(port_type)
	{
	case VIRTUAL: return QString();
	case COM: return QString();
//	case USB: return  QString().setNum(usb_dev_addr);
	default: 
		assert(false);
		return QString("<unknown>");
	};
};
