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

#ifndef MBN_BASIC_COMMUNICATION_CHANNEL
#define MBN_BASIC_COMMUNICATION_CHANNEL


#include <string>
#include <qlist.h>
#include <abstractserial.h>
#include <serialdeviceenumerator.h>
#include <qstring.h>
//#include <libusb.h> my insertion
//#include <libusbi.h> my insertion
#include <qstringlist.h>
#include <PagedArray.cpp>
#include <DeviceDescriptor.h>
#include <Basic_CommunicationChannel_Base.h>
#include <qmutex.h>

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

/// Низкоуровневый интерфейс для синхронного ввода-вывода по каналу COM
class COM_CommCh: public BaseCommCh, public AbstractSerial
{
protected:

	/// Перегружаем виртуальную функцию BaseCommCh::readData её реализацией в BaseCommCh::AbstractSerial
	virtual qint64 	readData(char * data, qint64 maxSize)
	{
		return this->AbstractSerial::readData(data, maxSize);
	};
	
	/// Перегружаем виртуальную функцию BaseCommCh::writeData её реализацией в BaseCommCh::AbstractSerial
	virtual qint64 	writeData(const char * data, qint64 maxSize)
	{
		return this->AbstractSerial::writeData(data, maxSize);
	};

public:

    // Деструктор
    virtual ~COM_CommCh();


	// Возвращает установленную максимальную скорость чтения в байтах в секунду
	virtual int GetReadingSpeed()
	{
        return 921600/8; // скорость нашего устройства //this->AbstractSerial::baudRateVal(AbstractSerial::InputBaud) / 8;
	};

	// Возвращает установленную максимальную скорость записи в байтах в секунду
	virtual int GetWritingSpeed()
	{
        return 921600/8; // скорость нашего устройства //this->AbstractSerial::baudRateVal(AbstractSerial::OutputBaud) / 8;
	};

	/// Возвращает доступное для чтения число байт
    virtual long long bytesAvailable()
	{
		return this->AbstractSerial::bytesAvailable();
	};

	/// Перегружаем виртуальную функцию BaseCommCh::close её реализацией в BaseCommCh::AbstractSerial
	virtual void close()
	{
		this->AbstractSerial::close();
	};

	// Возвращает размер буфера ввода канала
	virtual int GetInBufferSize()
	{
		//return this->AbstractSerial::readBufferSize();
		return 16384;  // временное значение, так как readBufferSize() не работает
	};

	/// <summary> 
	/// Инициализирует канал связи с указанным устройством.
	/// До вызова этой функции никакие настроики порта или операции ввода-вывода проводиться не могут.
	/// </summary>
	/// <param name="d"> Описатель устройства. Экземпляр класса, позволяющего идентефицировать устройство, к которому надо подключиться.
	/// Как получить список доступных устройств, см: <see cref="DeviceDescriptor::GetDevicesList()">Класс перечисления совместимых устройств</see></param> 
	/// <returns> Результат попытки открытия канала</returns>
    bool Init(DeviceDescriptor d);

	/// <summary> 
	/// Синхронно читает из буфера приёма порта всё, что там есть и записывает это в
	/// конец указанного хранилища. В хранилище данные добавляются методом push_back(...).
	/// </summary>
	/// <param name="T"> Тип хранилища. </param> 
	/// <param name="storage"> Хранилище байт, в которое будут добавлены считанные данные. Должно поддерживать метод push_back(...).</param> 
	/// <returns> Число прочитанных байт, если положительно, иначе - ошибка</returns>
	//template<class T>
	virtual int readAllSyncAndPushBack(PagedArray<unsigned char> &storage);

	/// <summary> 
	/// Синхронно выводит в порт данные из указанного места [from_index; to_index] хранилища.
	/// </summary>
	/// <param name="T"> Тип хранилища. </param> 
	/// <param name="storage"> Хранилище байт, из которго данные будут направлены в порт. Должно поддерживать оператор[].</param> 
	/// <param name="from_index"> Стартовый индекс с которого надо начать выводить данные.</param> 
	/// <param name="to_index"> Конечный индекс на котором надо вывод закончить.</param> 
	/// <returns> Число записанных байт, если положительно, иначе - ошибка</returns>
	//template<class T>
	virtual int writeSync(PagedArray<unsigned char> &storage, int from_index, int to_index);

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
	bool SetCOM_params(AbstractSerial::BaudRate baud_rate, AbstractSerial::BaudRateDirectionFlag direction, 
						AbstractSerial::Parity parity,
						AbstractSerial::DataBits data_bits, 
						AbstractSerial::StopBits stop_bits,
						AbstractSerial::Flow flow);
}; 










/// Низкоуровневый интерфейс для синхронного ввода-вывода виртуальному каналу
// виртуальный канал связи которым будет пользоваться
// активный коммуникационный канал эмулятора EEG для получения данных
class VirtualCommCh: public BaseCommCh
{
private:

	virtual qint64 readData(char *data, qint64 maxlen)
	{
		return 0;
	};

    virtual qint64 writeData(const char *data, qint64 len)
	{
		return 0;
	};

public:

    // Деструктор
    virtual ~VirtualCommCh()
    { };

private:
	// указатель на мьютекс доступа к входной очереди
	QMutex *in_mutex_ptr;

	// указатель на мьютекс доступа к выходной очереди
	QMutex *out_mutex_ptr;

	//входной (для виртуального устройства) буфер байт
	vector<unsigned char> *in_buffer_ptr;

	//выходной (для виртуального устройства) буфер байт
	vector<unsigned char> *out_buffer_ptr;

	// скорость чтения из канала - в байтах в секунду
	const int reading_speed;
	// скорость записи в канал - в байтах в секунду
	const int writing_speed;
	// размер приёмного буфера канала
	const int in_buffer_size;

public:

	/// <summary> 
	/// Синхронно читает из буфера приёма порта всё, что там есть и записывает это в
	/// конец указанного хранилища. В хранилище данные добавляются методом push_back(...).
	/// </summary>
	/// <param name="T"> Тип хранилища. </param> 
	/// <param name="storage"> Хранилище байт, в которое будут добавлены считанные данные. Должно поддерживать метод push_back(...).</param> 
	/// <returns> Число прочитанных байт, если положительно, иначе - ошибка</returns>
	//template<class T>
	virtual int readAllSyncAndPushBack(PagedArray<unsigned char> &storage)
	{
		// блокируем входную очередь
		in_mutex_ptr->lock();

		// выводим данные из выходной очереди виртуального устройства во входную очередь хоста
		for (int i = 0; i < in_buffer_ptr->size(); i++)
			storage.push_back((*in_buffer_ptr)[i]);

		int bytes_red = in_buffer_ptr->size();

		// очищаем выходную очередь
		in_buffer_ptr->clear();

		// высвобожждаем входную очередь
		in_mutex_ptr->unlock();

		return bytes_red;
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
	virtual int writeSync(PagedArray<unsigned char> &storage, int from_index, int to_index)
	{
		// блокируем выходную очередь
		out_mutex_ptr->lock();

		out_buffer_ptr->reserve(out_buffer_ptr->size() + to_index - from_index + 1);

		// выводим данные из выходной очереди виртуального устройства во входную очередь хоста
		for (int i = from_index; i <= to_index; i++)
			out_buffer_ptr->push_back(storage[i]);

		// высвобожждаем объект
		out_mutex_ptr->unlock();

		return (to_index - from_index + 1);
	};

	/// Возвращает доступное для чтения число байт
    virtual long long bytesAvailable()
	{
        long long result = 0;

		// блокируем объект
		in_mutex_ptr->lock();

		result = in_buffer_ptr->size();

		// высвобожждаем объект
		in_mutex_ptr->unlock();

		return result;
	};

	// Возвращает установленную максимальную скорость чтения в байтах в секунду
	virtual int GetReadingSpeed()
	{
		return reading_speed;
	};

	// Возвращает установленную максимальную скорость записи в байтах в секунду
	virtual int GetWritingSpeed()
	{
		return writing_speed;
	};

	// Возвращает размер буфера ввода канала
	virtual int GetInBufferSize()
	{
		return in_buffer_size;
	};


	// Конструктор всея виртуального канала
	VirtualCommCh(	QMutex *__in_mutex_ptr,
					QMutex *__out_mutex_ptr,
					vector<unsigned char> *__in_buffer_ptr,
					vector<unsigned char> *__out_buffer_ptr,
					int __reading_speed,
					int __writing_speed,
					int __in_buffer_size
				 ):
	in_mutex_ptr(__in_mutex_ptr), 
	out_mutex_ptr(__out_mutex_ptr), 
	in_buffer_ptr(__in_buffer_ptr), 
	out_buffer_ptr(__out_buffer_ptr),
	reading_speed(__reading_speed), 
	writing_speed(__writing_speed), 
	in_buffer_size(__in_buffer_size)
	{};
};




#endif
