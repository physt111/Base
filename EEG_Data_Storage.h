/***************************************************************************************
*
*	Хранилище данных, пришедших с EEG_IT
*
*  Потенциально, с функциями экспорта в базу данных, импорта из неё, 
*  сохранения данных на диск, итд.... но пока что, только выдача в 
*  клиентскую часть программы запрошенных данных.
*
***************************************************************************************/

#ifndef MBN_EEG_DATA_STORAGE
#define MBN_EEG_DATA_STORAGE


#include <assert.h>
#include <PagedArray.cpp>
#include <QDateTime>
#include <QSemaphore>
#include <EEG_device_architecture.h>

namespace MBN_EEG
{

/// Порция публичных данных канала EEG
class EEG_ChData
{
public:
	/// измеренное значение вольтажа
	float Voltage;
	/// оценённое время измерения
	QDateTime Time;

	/// Конструктор
	EEG_ChData(float voltage, QDateTime time)
	{
		Voltage = voltage;
		Time = time;
	};
};

/// Порция данных канала EEG
class EEG_ChDataAtom
{
public:
	/// измеренное значение вольтажа
	float Voltage;
	/// оценённое время измерения
	QDateTime Time;
	/// Индекс описателя состояния прибора (индекс в массиве состояний), в котором был получен данный отсчёт
	int DevStateIdx;

	/// Конструктор по умолчанию
	EEG_ChDataAtom()
	{
		Voltage = 0;
		Time = QDateTime::currentDateTime();
		DevStateIdx = 0;
	};

	/// Конструктор
	EEG_ChDataAtom(float voltage, QDateTime time, int devStateIdx = 0)
	{
		Voltage = voltage;
		Time = time;
		DevStateIdx = devStateIdx;
	};
};

/// Хранилище данных одного канала
class EEG_Channel_Data_Strorage
{
private:
	/**************************************************************
	*
	*					Семафор доступа
	*
	**************************************************************/
	QSemaphore accsess_sem;

	/**************************************************************
	*
	*					Сами данные канала
	*	
	**************************************************************/
	PagedArray<EEG_ChDataAtom> Data;

public:

	// конструктор по-умолчанию
	EEG_Channel_Data_Strorage():
	  accsess_sem(1)
	{};

	/// возвращает размер массива данных (в количестве записей - не в байтах)
	int length()
	{
		// ждём, освобождения массива другими потоками
		accsess_sem.acquire();

		int tmp = Data.size();

		// освобождаем доступ
		accsess_sem.release();

		return tmp;
	};

	/// возвращает значение вольтажа по указанному индексу в массиве отсчётов
	float voltage(int idx)
	{
		assert(idx >= 0);

		// ждём, освобождения массива другими потоками
		accsess_sem.acquire();

		float tmp = Data[idx].Voltage;

		// освобождаем доступ
		accsess_sem.release();

		return tmp;
	};

	/// возвращает значение времени по указанному индексу в массиве отсчётов
	QDateTime time(int idx)
	{
		assert(idx >= 0);

		// ждём, освобождения массива другими потоками
		accsess_sem.acquire();

		QDateTime tmp = Data[idx].Time;

		// освобождаем доступ
		accsess_sem.release();

		return tmp;
	};

	/// возвращает структуру, содержащую вольтаж и соответствующее время указанной точки в массиве отсчётов
	const EEG_ChData operator[] (int idx)
	{
		assert(idx >= 0);

		// ждём, освобождения массива другими потоками
		accsess_sem.acquire();

		EEG_ChData tmp(Data[idx].Voltage, Data[idx].Time);

		// освобождаем доступ
		accsess_sem.release();

		return tmp;
	};

	/// возвращает значение индекса описателя состояния прибора по указанному индексу в массиве отсчётов
	int dev_state_idx(int idx)
	{
		assert(idx >= 0);

		// ждём, освобождения массива другими потоками
		accsess_sem.acquire();

		int tmp = Data[idx].DevStateIdx;

		// освобождаем доступ
		accsess_sem.release();

		return tmp;
	};

	/// Добавляем точку в конец массива
	void Add(float voltage, QDateTime time, int DevStateIdx = 0)
	{
		// ждём, освобождения массива другими потоками
		accsess_sem.acquire();

		Data.push_back(EEG_ChDataAtom(voltage, time, DevStateIdx));

		// освобождаем доступ
		accsess_sem.release();
	};

};

/// Хранилище данных для EEG_IT
/// Потокобезопасно.
class EEG_Data_Storage
{
private:
	/**************************************************************
	*
	*				Сами данные прибора
	*
	*		Всего ШЕСТЬ каналов (A1, A2, C3, C4, Fp1, Fp2). 
	*	Нумерация каналов [0,..,5] - в какой последовательности 
	*	- см. EEG_Device_Architecture::ChannelsCommutatorArc
	**************************************************************/
	EEG_Channel_Data_Strorage Data_A1;
	EEG_Channel_Data_Strorage Data_A2;

	EEG_Channel_Data_Strorage Data_C3;
	EEG_Channel_Data_Strorage Data_C4;

	EEG_Channel_Data_Strorage Data_Fp1;
	EEG_Channel_Data_Strorage Data_Fp2;


public:

	/// Оператор [] возвращает по ссылке указанный канал
	EEG_Channel_Data_Strorage& operator[] (EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Channel ch)
	{
		// полагаем, что мы что-то всё-таки хотим
		assert( (ch == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_A1) ||
				(ch == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_A2) ||
				(ch == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_C3) ||
				(ch == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_C4) ||
				(ch == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_Fp1) ||
				(ch == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_Fp2));

		switch(ch)
		{
		case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_A1: return Data_A1;
		case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_A2: return Data_A2;
		case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_C3: return Data_C3;
		case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_C4: return Data_C4;
		case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_Fp1: return Data_Fp1;
		case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_Fp2: return Data_Fp2;
		default: return Data_A1;
		};
	};

};



};

#endif
