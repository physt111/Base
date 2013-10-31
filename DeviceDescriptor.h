/*! \file 
	Файл содержит классы
		DeviceDescriptor - описатель устройства, подключённого по каналу COM или USB
*/ 


#ifndef MBN_DEVICE_DESCRIPTOR__
#define MBN_DEVICE_DESCRIPTOR__


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


using namespace std;

/// Описатель устройства, подключённого по каналу COM или USB
/// ТОЛЬКО ДЛЯ ЛОКАЛИЗАЦИИ УСТРОЙСТВА
class DeviceDescriptor
{
/// перечисления
public:  

    enum PortType {COM, USB, VIRTUAL, NOT_CONNECTED}; /// типы портов, по которым может быть подключено устройство

#define DEVICE_DESCRIPTOR_STRING_BUFFER_SIZE 256

/// поля
private:

	QString serial_port_name;	/// информация соответствующая COM-портовому подключению

	/// USB часть
    //libusb_device_descriptor usb_dev_description; /// описание подключённого по USB устройства
    //QString usb_dev_serial_number; /// серийный номер устройства
    //uint8_t usb_dev_bus_number; /// номер шины по которой подключено устройство
    //uint8_t usb_dev_addr; // адрес USB устройства на USB шине

	PortType port_type; /// красившее обозначение типа подключения

/// методы
public:

	/// конструктор для COM подключения
	DeviceDescriptor(const QString Serial_Port_Name);

	/// конструктор для USB подключения
    //DeviceDescriptor(const libusb_device** p_info_array_start, int p_info_idx);

	/// конструктор для VIRTUAL подключения
	DeviceDescriptor();

	/// оператор присваивания
    DeviceDescriptor& operator= (const DeviceDescriptor& origin);

	/// возвращает список подключённых по USB устройств и COM-портов
	static QList<DeviceDescriptor> GetDevicesList();

	/// Возвращает тип устройства
	PortType GetType()
	{
		return port_type;
	};

	/// Возвращает тип устройства в виде строки
	QString GetTypeStr();

	/// возвращает id продавца
	QString GetVendorId();

	/// возвращает id модели
	QString GetProductId();

	/// возвращает серийник модели
	QString GetDeviceSerialNumber();

	/// возвращает номер шины по которой подключено устройство
	QString GetDeviceBusNumber();

	/// возвращает адрес устройства на USB шине
	QString GetDeviceAddress();

	/// возвращает имя COM порта
	QString GetCOMname()
	{
		return serial_port_name;
	};

	// возвращает строковое описание устройства
	QString ToString()
	{
		if (port_type == COM)
			return "[COM]\tPID = " + GetProductId() + "\tVID = " + GetVendorId() + "\tCOM = " + GetCOMname() + "\tSN = " + GetDeviceSerialNumber();
		else if (port_type == USB)
			return "[USB]\tPID = "+GetProductId()+"\tVID = "+GetVendorId()+"\tBus number = "+GetDeviceBusNumber()+"\tDev.addr = "+GetDeviceAddress()+"\tSN = "+GetDeviceSerialNumber();
		else if (port_type == VIRTUAL)
            return "[VIRTUAL]\tPID = "+GetProductId()+"\tVID = "+GetVendorId()+"\tSN = "+GetDeviceSerialNumber();
		else return "<unknown type>";
	};
};

#endif
