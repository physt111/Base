/*! \file
	Файл содержит классы
		COM_CommCh - Низкоуровневый интерфейс для синхронного ввода-вывода по каналу COM
*/ 

/*!
|	ID класса в котором описан метод
|	|
|	|	ID метода, вызвавшего исключение
|	|	|
|	|	|	Номер ошибки в методе
02	001	ХХ	ХХ
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
#include <DeviceDescriptor.h>
#include <Basic_CommunicationChannel.h>

#include <iostream>


using namespace std;


/*!
|	ID класса в котором описан метод
|	|
|	|	ID метода, вызвавшего исключение
|	|	|
|	|	|	Номер ошибки в методе
02	002	ХХ	ХХ
*/


/// <summary> 
/// Инициализирует канал связи с указанным устройством.
/// До вызова этой функции никакие настроики порта или операции ввода-вывода проводиться не могут.
/// </summary>
/// <param name="d"> Описатель устройства. Экземпляр класса, позволяющего идентефицировать устройство, к которому надо подключиться.
/// Как получить список доступных устройств, см: <see cref="DeviceDescriptor::GetDevicesList()">Класс перечисления совместимых устройств</see></param> 
/// <returns> Результат попытки открытия канала</returns>
bool COM_CommCh::Init(DeviceDescriptor d)
{
	this->setDeviceName(d.GetCOMname());

    std::cout<<"Device COM name = "<<this->deviceName().toStdString()<<std::endl;
		
    if (this->AbstractSerial::open(this->ReadWrite | AbstractSerial::Unbuffered))
    {
		// при небуферизованном режиме, необходимо выставить не нулевое время таймаутов !!!
		// ИНАЧЕ НИЧЕГО ЧИТАТЬСЯ НЕ БУДЕТ!!! 
		const int CharIntervalTimeout__ = 10;
		const int TotalReadConstantTimeout__ = 10;
		this->setCharIntervalTimeout(CharIntervalTimeout__); 
		this->setTotalReadConstantTimeout(TotalReadConstantTimeout__);

        std::cout<<"Device opened."<<std::endl;

		// устанавливаем состояние
		opened = true;
		return true;
	}
    else
	{
        std::cout<<"Failed to open Device"<<std::endl;
		// устанавливаем состояние
		opened = false;
		return false;
	}
};

// Деструктор
COM_CommCh::~COM_CommCh()
{
    std::cout<<"Destructor of COM_CommCh"<<std::endl;

    if (opened)
    {
        //this->AbstractSerial::close();

        std::cout<<"QSerial Device was closed"<<std::endl;
        opened = false;
    };

    //this->AbstractSerial::~AbstractSerial();
}

/// <summary> 
/// Синхронно читает из буфера приёма порта всё, что там есть и записывает это в
/// конец указанного хранилища. В хранилище данные добавляются методом push_back(...).
/// </summary>
/// <param name="T"> Тип хранилища. </param> 
/// <param name="storage"> Хранилище байт, в которое будут добавлены считанные данные. Должно поддерживать метод push_back(...).</param> 
/// <returns> Число прочитанных байт, если положительно, иначе - ошибка</returns>
//template<class T>
int COM_CommCh::readAllSyncAndPushBack(PagedArray<unsigned char> &storage)
{   
	qint64 bytes_av = this->AbstractSerial::bytesAvailable();
	if (bytes_av > 0)
	{
		QByteArray tmp_buff = this->AbstractSerial::read(this->AbstractSerial::bytesAvailable());
				
		for (int i = 0; i < tmp_buff.size(); i++)
			storage.push_back(tmp_buff[i]);
		return tmp_buff.size();
	}
	else return 0;
};

/// <summary> 
/// Синхронно выводит в порт данные из указанного места [from_index; to_index] хранилища.
/// </summary>
/// <param name="T"> Тип хранилища. </param> 
/// <param name="storage"> Хранилище байт, из которго данные будут направлены в порт. Должно поддерживать оператор[].</param> 
/// <param name="from_index"> Стартовый индекс с которого надо начать выводить данные.</param> 
/// <param name="to_index"> Конечный индекс на котором надо вывод закончить.</param> 
/// <returns> Число записанных байт, если положительно, иначе - ошибка</returns>
//template<class T>
int COM_CommCh::writeSync(PagedArray<unsigned char> &storage, int from_index, int to_index)
{
	QByteArray tmp_buff;
	for (int i = 0; i <= to_index - from_index; i++)
		tmp_buff.push_back(storage[i + from_index]);
	return this->AbstractSerial::write(tmp_buff);
};

/// <summary> 
/// Специализированная функция для установки параметров COM-порта. Делает что-то, только если 
/// канальная связь осуществляется через COM-порт (подключение было сделано к COM-порту).
/// <see cref="COM_USB_CommCh::Init">Инициализация канала. </see>
/// </summary>
/// <param name="baud_rate"> Устанавливаемая скорость.</param> 
/// <param name="direction"> Для каких направлений устанавливается скорость.</param> 
/// <param name="parity"> Устанавливаемая чётность.</param> 
/// <param name="data_bits"> Устанавливаемое количество битов данных.</param> 
/// <param name="stop_bits"> Устанавливаемое количество стоповых битов.</param> 
/// <returns> Успешность установки всех COM-порта.</returns>
bool COM_CommCh::SetCOM_params(AbstractSerial::BaudRate baud_rate, AbstractSerial::BaudRateDirectionFlag direction, 
					AbstractSerial::Parity parity,
					AbstractSerial::DataBits data_bits, 
					AbstractSerial::StopBits stop_bits,
					AbstractSerial::Flow flow)
{
		return 
			this->AbstractSerial::setBaudRate(baud_rate, direction) &&
			this->AbstractSerial::setParity(parity) &&
			this->AbstractSerial::setDataBits(data_bits) &&
			this->AbstractSerial::setStopBits(stop_bits) &&
			this->AbstractSerial::setFlowControl(flow);
};
