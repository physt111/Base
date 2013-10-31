/***************************************************************************************
*
*	Низкоуровненый канал связи - эмулятор EEG устройства. Подключается к классу EEG
*	как низкоуровненый канал связи. Управляется тестировочным модулем для тестирования
*	класса EEG.
*
*	Имитирует поведение реального EEG устройства - для того, чтобы можно было производить
*	настройку, отладку ПО без реального устройства и проводить автоматическое 
*	тестирование ПО для разных поведений устройства.
*		
*
***************************************************************************************/

#ifndef MBN_EEG_PROGRAM_EMULATOR
#define MBN_EEG_PROGRAM_EMULATOR

#include "Basic_CommunicationChannel.h"
#include "Common_Communication_Channel.h"
#include <qmutex.h>
#include <qthread.h>
#include <EEG_Protocol.h>
#include <math.h>

namespace MBN_EEG
{

class EEG_Emulator;

class EEG_Device_Thread: public QThread
{
public:

	// указатель на сам эмулятор
	EEG_Emulator* const emulator_ptr;

	// флаг остановки
	bool stop_dev_thread;

public:

	// конструктор
	EEG_Device_Thread(EEG_Emulator* const __emulator_ptr): emulator_ptr(__emulator_ptr)
	{};

	// поток устройства
	virtual void run();
};


class EEG_Emulator: public VirtualCommCh, public EEG_Protocol, public Common_Communication_Channel
{
public:

	/********************************************************************
	*
	*					Обменник данными
	*
	********************************************************************/

	// мьютекс доступа входному (для виртуального устройства) буферу
	QMutex in_accsess_mutex;

	// мьютекс доступа выходному (для виртуального устройства) буферу
	QMutex out_accsess_mutex;

	//входной (для виртуального устройства) буфер байт
	vector<unsigned char> in_buffer;

	//выходной (для виртуального устройства) буфер байт
	vector<unsigned char> out_buffer;

	/********************************************************************
	*
	*			Виртуальный порт виртуального устройства
	*
	********************************************************************/

	// внутренний виртуальный канал считывания, которым будет пользоваться Common_Communication_Channel
	VirtualCommCh internal_virtual_comm_ch;

	/********************************************************************
	*
	*					Состояние устройства
	*
	********************************************************************/

	EEG_Device_Architecture::EEG_CP_BridgeState BridgeData_State;
	EEG_Device_Architecture::EEG_ChannelsCommutatorState MainCommutator_State;
	EEG_Device_Architecture::EEG_ChannelsLedsState ChannelsLeds_State;
	EEG_Device_Architecture::EEG_FunctionalLedsState FunctionalLeds_State;
	EEG_Device_Architecture::EEG_ADC_State ADC_State;
	EEG_Device_Architecture::EEG_Calibrator_State DAC_State;
	EEG_Device_Architecture::EEG_Common_State Common_State;

	/********************************************************************
	*
	*					микросостояние устройства
	*
	********************************************************************/

	/// фаза калибратора
	float calibrator_phase;
	/// сдвиг фазы калибратора за шаг
	float d_phase;

	/********************************************************************
	*
	*					Отдельный поток устройства
	*
	********************************************************************/

	// сам поток
	EEG_Device_Thread device_thread;


public:


	// конструктор
	EEG_Emulator(): 
		// инициализируем канал, который и подключается к EEG классу (in и out стоят правильно - то, что для нас вход - для EEG_класса - выход)
		VirtualCommCh(
						&out_accsess_mutex, 
						&in_accsess_mutex, 
						&out_buffer, 
						&in_buffer, 
						115200, 
						115200, 
						65536),
		MBN_EEG::EEG_Protocol	// инициализируем протокол и подвязываем его к нему хранилища и callback-и
										(
											((MBN_EEG::EEG_Data_Storage*)this),  
											((MBN_EEG::EEG_Device_States_Storage*)this),
											false,
											0, 
											&CommandAcknowledgeCallback,
											0,
											0,
											0,
											this,
											this,
											this,
											this,
											this
										),
		Common_Communication_Channel	// создаём активный канал связи, подвязываем к нему экземпляр протокола и низкоуровневый канал
										(
											&internal_virtual_comm_ch,
											(MBN_EEG::EEG_Protocol*)this,
											1000,
											1000,
											100,
											100,
											1000,
											1000,
											100,
											100,
											3,
											3,
											3,
											3
										),
		internal_virtual_comm_ch		(
											&in_accsess_mutex, 
											&out_accsess_mutex, 
											&in_buffer, 
											&out_buffer, 
											115200, 
											115200, 
											65536
										),
		device_thread(this)
	{
		// запускаем активный канал
		((Common_Communication_Channel *)this)->StartChannel();		

		// микросостояние калибратора
		calibrator_phase = 0.0;
		d_phase = 0.01;
	};


	//реакция на получение команды
    static void CommandAcknowledgeCallback(long long acked_pack_Id, CommonPackage &AnswerPkg, void* parameter_obj)
	{
		EEG_Emulator* obj = (EEG_Emulator*)parameter_obj;
		
		switch(AnswerPkg.subtype)
		{
		case C_START_TRANSLATION: // если надо начать трансляцию
			obj->device_thread.stop_dev_thread = false;
			obj->device_thread.start();
			break;
		case C_STOP_TRANSLATION: // если надо закончить трансляцию
			obj->device_thread.stop_dev_thread = true;
			break;
		};
	};


	/// приостановить устройство
	void StopDevice()
	{
		((Common_Communication_Channel *)this)->StopChannel();
	};

	/// запустить устройство
	void RunDevice()
	{
		((Common_Communication_Channel *)this)->StartChannel();
	};

};


};




#endif
