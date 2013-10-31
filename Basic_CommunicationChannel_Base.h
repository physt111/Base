/*! \file 
	Файл содержит классы
		BaseCommCh - базовый виртуальный класс для простейшего канала ввода-вывода
*/ 

/*!
|	ID класса в котором описан метод
|	|
|	|	ID метода, вызвавшего исключение
|	|	|
|	|	|	Номер ошибки в методе
02	001	ХХ	ХХ
*/

#ifndef MBN_BASIC_COMMUNICATION_CHANNEL_BASE__
#define MBN_BASIC_COMMUNICATION_CHANNEL_BASE__


#include <string>
#include <qlist.h>
#include <abstractserial.h>
#include <serialdeviceenumerator.h>
#include <qstring.h>
//#include <libusb.h>  //my insertion
//#include <libusbi.h> //my insertion
#include <qstringlist.h>
#include <PagedArray.cpp>
#include <DeviceDescriptor.h>

#include <iostream>


using namespace std;


/// <summary> Виртуальный класс, описывающий общие элементарные функции ввода-вывода для канала связи
class BaseCommCh: public QIODevice
{
protected:
	/// флаг открытия устройства
	bool opened;

public:

    /// Виртуальный деструктор
    virtual ~BaseCommCh()
    {
    }

	/// подключены ли мы к устройству
	bool isOpened()
	{
		return opened;
	};

	/// <summary> 
	/// Синхронно читает из буфера приёма порта всё, что там есть и записывает это в
	/// конец указанного хранилища. В хранилище данные добавляются методом push_back(...).
	/// </summary>
	/// <param name="T"> Тип хранилища. </param> 
	/// <param name="storage"> Хранилище байт, в которое будут добавлены считанные данные. Должно поддерживать метод push_back(...).</param> 
	/// <returns> Число прочитанных байт, если положительно, иначе - ошибка</returns>
	//template<class T>
	virtual int readAllSyncAndPushBack(PagedArray<unsigned char> &storage) = 0;
	
	/// <summary> 
	/// Синхронно выводит в порт данные из указанного места [from_index; to_index] хранилища.
	/// </summary>
	/// <param name="T"> Тип хранилища. </param> 
	/// <param name="storage"> Хранилище байт, из которго данные будут направлены в порт. Должно поддерживать оператор[].</param> 
	/// <param name="from_index"> Стартовый индекс с которого надо начать выводить данные.</param> 
	/// <param name="to_index"> Конечный индекс на котором надо вывод закончить.</param> 
	/// <returns> Число записанных байт, если положительно, иначе - ошибка</returns>
	//template<class T>
	virtual int writeSync(PagedArray<unsigned char> &storage, int from_index, int to_index) = 0;

	/// Возвращает доступное для чтения число байт
    virtual long long bytesAvailable() = 0;

	// Возвращает установленную максимальную скорость чтения в байтах в секунду
	virtual int GetReadingSpeed() = 0;

	// Возвращает установленную максимальную скорость записи в байтах в секунду
	virtual int GetWritingSpeed() = 0;

	// Возвращает размер буфера ввода канала
	virtual int GetInBufferSize() = 0;

};


#endif
