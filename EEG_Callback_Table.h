/***************************************************************************************
*
*	Описывает класс:
*
*		Таблица обратных вызовов для EEG. 
*			В таблице хранятся данные, необходимые внутреннему потоку EEG класса для
*			вызова соответствующих клиентских callback-функций по какому-то событию 
*			(получения подтверждения команды, тайм аут команды). 
*		
*			в таблице хранится id посылки на которую надо вызвать ф-цию о.в. если id == 0,
*			то данная функция вызывается для всех пакетов данного типа.
*
***************************************************************************************/

#include <qmutex.h>
#include <list>

#include <CommonPackage.h>
#include <EEG_Protocol.h>
#include <qthread.h>
#include <qstring.h>

#ifndef MBN_EEG_CALLBACK_TABLE__
#define MBN_EEG_CALLBACK_TABLE__

using namespace std;

namespace MBN_EEG
{
	/***********************************************************
	*
	*				Базовый класс события
	*
	***********************************************************/
	
	/// класс события ЭЭГ, которое передаётся вектору обратного вызова, и котороый либо срабатывает либо нет
	class EEG_Event
	{
	public:

		/// типы записей в таблице обратных вызовов
		enum EEG_EVENT_TYPES
		{
			EEG_EVENT_TYPE__ANY = 0,		// спец-тип любой
			EEG_EVENT_TYPE__GOT_COMMAND_RESPONSE,   // GCRE:  реакция на результат отработки команды (отвечена либо таймаут)
			EEG_EVENT_TYPE__GOT_DATA,		// GDE:  реакция на приём данных с устройства
			EEG_EVENT_TYPE__GOT_MESSAGE,		// GME: реакция на приём сообщения от устройства
			EEG_EVENT_TYPE__SYSTEM_EVENT		// SE: системное сообщение
		};

		/// спец-подтип
		static const int EEG_EVENT_SUBTYPE__ANY = -1;		// спец-подтип - любой 

		/// подтипы событий результата обработки команды
		enum EEG_EVENT_GCRE_SUBTYPES
		{
			EEG_EVENT_GCRE_SUBTYPE__ERROR,
			EEG_EVENT_GCRE_SUBTYPE__RESET,
			EEG_EVENT_GCRE_SUBTYPE__GET_VERSION,
			EEG_EVENT_GCRE_SUBTYPE__SET_MUX,
			EEG_EVENT_GCRE_SUBTYPE__GET_PROTOCOL_VERSION,
			EEG_EVENT_GCRE_SUBTYPE__GET_FREQUENCY_TABLE,
			EEG_EVENT_GCRE_SUBTYPE__SET_SAMPLING_FREQUENCY,
			EEG_EVENT_GCRE_SUBTYPE__SET_CHANNELS_MODE,
			EEG_EVENT_GCRE_SUBTYPE__SET_CALIBRATION_SHAPE,
			EEG_EVENT_GCRE_SUBTYPE__START_CALIBRATION,
			EEG_EVENT_GCRE_SUBTYPE__STOP_CALIBRATION,
			EEG_EVENT_GCRE_SUBTYPE__START_TRANSLATION,
			EEG_EVENT_GCRE_SUBTYPE__STOP_TRANSLATION,
			EEG_EVENT_GCRE_SUBTYPE__SET_CHANNEL_LEDS,
			EEG_EVENT_GCRE_SUBTYPE__SET_AMPLIFICATION,
			EEG_EVENT_GCRE_SUBTYPE__SET_ADS_MUX
		};

		/// подтипы событий приёма данных с устройства
		enum EEG_EVENT_GDE_SUBTYPES
		{
			EEG_EVENT_GDE_SUBTYPE__USUAL_CHANNELS_DATA_WITHOUT_STIMUL,
			EEG_EVENT_GDE_SUBTYPE__USUAL_CHANNELS_DATA_WITH_STIMUL,
			EEG_EVENT_GDE_SUBTYPE__IMPEDANCE_MBN,
			EEG_EVENT_GDE_SUBTYPE__IMPEDANCE_ALTONIX
		};

		/// подтипы событий приёма сообщения от устройства
		enum EEG_EVENT_GME_SUBTYPES
		{
			EEG_EVENT_GME_SUBTYPE_DEVICE_READY_MSG = 3
		};

		/// подтипы системных событий класса EEG
		enum EEG_EVENT_SE_SUBTYPES
		{
			EEG_EVENT_SE_SUBTYPE__DEVICE_CONNECTED,
			EEG_EVENT_SE_SUBTYPE__DEVICE_DISCONNECTED,
			EEG_EVENT_SE_SUBTYPE__DEVICE_REPORTED_ERROR,
			EEG_EVENT_SE_SUBTYPE__ERROR
		};



		/// вернуть строковое представление типа события
public: static QString EventTypeToStr(EEG_EVENT_TYPES type)
		{
			switch (type)
			{
			case EEG_EVENT_TYPE__ANY: return "any";
			case EEG_EVENT_TYPE__GOT_COMMAND_RESPONSE: return "got command response";
			case EEG_EVENT_TYPE__GOT_DATA: return "got data";
			case EEG_EVENT_TYPE__GOT_MESSAGE: return "got message";
			case EEG_EVENT_TYPE__SYSTEM_EVENT: return "got system message";
			default: return "unknown";
			};
		}

		/// вернуть строковое представление подтипа события
public: static QString EventSubtypeToStr(int type , int subtype)
		{
			switch (type)
			{
			case EEG_EVENT_TYPE__ANY: return "any";
			case EEG_EVENT_TYPE__GOT_COMMAND_RESPONSE: 
				{
					switch (subtype)
					{
					case EEG_EVENT_GCRE_SUBTYPE__ERROR: return "Error";
					case EEG_EVENT_GCRE_SUBTYPE__RESET: return "Reset";
					case EEG_EVENT_GCRE_SUBTYPE__GET_VERSION: return "GetVersion";
					case EEG_EVENT_GCRE_SUBTYPE__SET_MUX: return "SetMUX";
					case EEG_EVENT_GCRE_SUBTYPE__GET_PROTOCOL_VERSION: return "GetProtocolVersion";
					case EEG_EVENT_GCRE_SUBTYPE__GET_FREQUENCY_TABLE: return "GetFrequencyTable";
					case EEG_EVENT_GCRE_SUBTYPE__SET_SAMPLING_FREQUENCY: return "SetSamplingFrequency";
					case EEG_EVENT_GCRE_SUBTYPE__SET_CHANNELS_MODE: return "SetChannelsMode";
					case EEG_EVENT_GCRE_SUBTYPE__SET_CALIBRATION_SHAPE: return "SetCalibrationShape";
					case EEG_EVENT_GCRE_SUBTYPE__START_CALIBRATION: return "StartCalibration";
					case EEG_EVENT_GCRE_SUBTYPE__STOP_CALIBRATION: return "StopCalibration";
					case EEG_EVENT_GCRE_SUBTYPE__START_TRANSLATION: return "StartTranslation";
					case EEG_EVENT_GCRE_SUBTYPE__STOP_TRANSLATION: return "StopTranslation";
					case EEG_EVENT_GCRE_SUBTYPE__SET_CHANNEL_LEDS: return "SetChannelLeds";
					case EEG_EVENT_GCRE_SUBTYPE__SET_AMPLIFICATION: return "SetAmplification";
					case EEG_EVENT_GCRE_SUBTYPE__SET_ADS_MUX: return "SetADSMUX";
					default: return "unknown";
					}
				}
			case EEG_EVENT_TYPE__GOT_DATA:
				{
				switch (subtype)
					{
					case EEG_EVENT_GDE_SUBTYPE__USUAL_CHANNELS_DATA_WITHOUT_STIMUL: return "usual channels data without stimul";
					case EEG_EVENT_GDE_SUBTYPE__USUAL_CHANNELS_DATA_WITH_STIMUL: return "usual channels data with stimul";
					case EEG_EVENT_GDE_SUBTYPE__IMPEDANCE_MBN: return "impedance MBN";
					case EEG_EVENT_GDE_SUBTYPE__IMPEDANCE_ALTONIX: return "impedance Altonix";
					default: return "unknown";
					}
				}
			case EEG_EVENT_TYPE__GOT_MESSAGE:
				{
				switch (subtype)
					{
					case EEG_EVENT_GME_SUBTYPE_DEVICE_READY_MSG: return "device ready";
					default: return "unknown";
					}
				}
			case EEG_EVENT_TYPE__SYSTEM_EVENT:
				{
				switch (subtype)
					{
					default: return "unknown";
					}
				}
			default: return "unknown";
			};
		}

	public:
		/// тип записи
		typedef int EEG_EVENT_TYPE;

		/// подтип записи
		typedef int EEG_EVENT_SUBTYPE;

	public:
		/// тип события
		const EEG_EVENT_TYPE	type;

		/// подтип события
		const EEG_EVENT_SUBTYPE subtype;

	public:

		/// конструктор
		EEG_Event(EEG_EVENT_TYPE __type, EEG_EVENT_SUBTYPE __subtype): 
			type(__type),
			subtype(__subtype)
		{};
		
		/// виртуальный деструктор
		virtual ~EEG_Event() {};

		#ifdef _DEBUG
		/// распечатать событие в стандартный вывод
		void virtual PrintEvent()
		{
			switch (type)
			{
			case EEG_EVENT_TYPE__ANY: cout<<"Any event"<<endl; break;
			case EEG_EVENT_TYPE__GOT_COMMAND_RESPONSE: 
				{
					cout<<"Got command result"<<endl;

					switch (subtype)
					{
					case EEG_EVENT_GCRE_SUBTYPE__ERROR:	cout<<"\tError"; break;
					case EEG_EVENT_GCRE_SUBTYPE__RESET: cout<<"\tReset"; break;
					case EEG_EVENT_GCRE_SUBTYPE__GET_VERSION: cout<<"\tGetVersion"; break;
					case EEG_EVENT_GCRE_SUBTYPE__SET_MUX: cout<<"\tSetMUX"; break;
					case EEG_EVENT_GCRE_SUBTYPE__GET_PROTOCOL_VERSION: cout<<"\tGetProtocolVersion"; break;
					case EEG_EVENT_GCRE_SUBTYPE__GET_FREQUENCY_TABLE: cout<<"\tGetFrequencyTable"; break;
					case EEG_EVENT_GCRE_SUBTYPE__SET_SAMPLING_FREQUENCY: cout<<"\tSetSamplingFrequency"; break;
					case EEG_EVENT_GCRE_SUBTYPE__SET_CHANNELS_MODE: cout<<"\tSetChannelsMode"; break;
					case EEG_EVENT_GCRE_SUBTYPE__SET_CALIBRATION_SHAPE: cout<<"\tSetCalibrationShape"; break;
					case EEG_EVENT_GCRE_SUBTYPE__START_CALIBRATION: cout<<"\tStartCalibration"; break;
					case EEG_EVENT_GCRE_SUBTYPE__STOP_CALIBRATION: cout<<"\tStopCalibration"; break;
					case EEG_EVENT_GCRE_SUBTYPE__START_TRANSLATION: cout<<"\tStartTranslation"; break;
					case EEG_EVENT_GCRE_SUBTYPE__STOP_TRANSLATION: cout<<"\tStopTranslation"; break;
					case EEG_EVENT_GCRE_SUBTYPE__SET_CHANNEL_LEDS: cout<<"\tSetChannelLeds"; break;
					case EEG_EVENT_GCRE_SUBTYPE__SET_AMPLIFICATION: cout<<"\tSetAmplification"; break;
					case EEG_EVENT_GCRE_SUBTYPE__SET_ADS_MUX: cout<<"\tSetADS_MUX"; break;
					};

					break;
				};
			case EEG_EVENT_TYPE__GOT_DATA:
				{
					cout<<"Got data"<<endl;

					switch(subtype)
					{
					case EEG_EVENT_GDE_SUBTYPE__USUAL_CHANNELS_DATA_WITHOUT_STIMUL: cout<<"\tUsual data"; break;
					case EEG_EVENT_GDE_SUBTYPE__USUAL_CHANNELS_DATA_WITH_STIMUL: cout<<"\tUsual data with stimul"; break;
					case EEG_EVENT_GDE_SUBTYPE__IMPEDANCE_MBN: cout<<"\tImpedance MBN data"; break;
					case EEG_EVENT_GDE_SUBTYPE__IMPEDANCE_ALTONIX: cout<<"\tImpedance Altonix data"; break;
					};

					break;
				};
			case EEG_EVENT_TYPE__GOT_MESSAGE:
				{
					cout<<"Got message"<<endl;

					switch(subtype)
					{
					case EEG_EVENT_GME_SUBTYPE_DEVICE_READY_MSG: cout<<"\tDevice ready"; break;
					};

					break;
				};
			case EEG_EVENT_TYPE__SYSTEM_EVENT: 
				{
					cout<<"System event"<<endl;
					

					switch(subtype)
					{
					case EEG_EVENT_SE_SUBTYPE__DEVICE_CONNECTED: cout<<"\tDevice connected"; break;
					case EEG_EVENT_SE_SUBTYPE__DEVICE_DISCONNECTED: cout<<"\tDevice disconnected"; break;
					case EEG_EVENT_SE_SUBTYPE__DEVICE_REPORTED_ERROR: cout<<"\tDevice reported an error"; break;
					case EEG_EVENT_SE_SUBTYPE__ERROR: cout<<"\tError"; break;
					};

					break;
				};
			};

			
		};
		#endif
	};


	/***********************************************************
	*
	*		Общий класс события для событий получения 
	*			результата по команде
	*
	***********************************************************/

	/// класс общего события результата выполнения команды
	class EEG_Event_GCRE: public EEG_Event
	{
	public:

		/// id посылки команды, результат которой известен
        const long long command_pkg_id;

		/// подтверждена ли команда
		const bool Acknoledged;

	public:
		// конструктор
        EEG_Event_GCRE(EEG_EVENT_SUBTYPE __subtype, long long __command_pkg_id, bool __Acknoledged):
			EEG_Event(EEG_EVENT_TYPE__GOT_COMMAND_RESPONSE, __subtype),
			command_pkg_id(__command_pkg_id),
			Acknoledged(__Acknoledged)
		{};
	};


	/***********************************************************
	*
	*			События получения результата по команде
	*
	***********************************************************/

	/// Событие получения результата по команде Reset
	class EEG_Event_GCRE_Error_event: public EEG_Event_GCRE
	{
	public:
		const unsigned char DeviceErrorID;

	public:
        EEG_Event_GCRE_Error_event(long long __command_pkg_id, bool __acked, unsigned char __DeviceErrorID):
			EEG_Event_GCRE(EEG_Event::EEG_EVENT_GCRE_SUBTYPE__ERROR, __command_pkg_id, __acked),
			DeviceErrorID(__DeviceErrorID)
		{};
	};

	/// Событие получения результата по команде Reset
	class EEG_Event_GCRE_Reset_event: public EEG_Event_GCRE
	{
	public:
        EEG_Event_GCRE_Reset_event(long long __command_pkg_id, bool __acked):
			EEG_Event_GCRE(EEG_Event::EEG_EVENT_GCRE_SUBTYPE__RESET, __command_pkg_id, __acked)
		{};
	};

	/// Событие получения результата по команде GetVersion
	class EEG_Event_GCRE_GetVersion_event: public EEG_Event_GCRE
	{
	public:
		const unsigned char FirmwareVersion;
		const unsigned char CircuityVersion;
		const unsigned short SerialNumber;

	public:
        EEG_Event_GCRE_GetVersion_event(long long __command_pkg_id, bool __acked, unsigned char __FirmwareVersion, unsigned char __CircuityVersion, unsigned short __SerialNumber):
			EEG_Event_GCRE(EEG_Event::EEG_EVENT_GCRE_SUBTYPE__GET_VERSION, __command_pkg_id, __acked),
			FirmwareVersion(__FirmwareVersion),
			CircuityVersion(__CircuityVersion),
			SerialNumber(__SerialNumber)
		{};
	};

	/// Событие получения результата по команде GetProtocolVersion
	class EEG_Event_GCRE_GetProtocolVersion_event: public EEG_Event_GCRE
	{
	public:
		const unsigned char ProtocolVersion;

	public:
        EEG_Event_GCRE_GetProtocolVersion_event(long long __command_pkg_id, bool __acked, unsigned char __ProtocolVersion):
			EEG_Event_GCRE(EEG_Event::EEG_EVENT_GCRE_SUBTYPE__GET_PROTOCOL_VERSION, __command_pkg_id, __acked),
			ProtocolVersion(__ProtocolVersion)
		{};
	};

	/// Событие получения результата по команде GetFrequenciesTable
	class EEG_Event_GCRE_GetFrequenciesTable_event: public EEG_Event_GCRE
	{
	public:
        const vector<unsigned short> freq_table;

	private:
		// просто создаёт копию массива по указателю, если указатель не 0, а если 0 - то возвращает пустой массив
        vector<unsigned short> get_vector_copy_by_ptr(const vector<unsigned short> *__freq_table)
		{
            if (__freq_table) return vector<unsigned short>(*__freq_table);
            else return vector<unsigned short>();
		};

	public:
		// если __freq_table == 0, оставляем freq_table пустым
        EEG_Event_GCRE_GetFrequenciesTable_event(long long __command_pkg_id, bool __acked, const vector<unsigned short> *__freq_table):
			EEG_Event_GCRE(EEG_Event::EEG_EVENT_GCRE_SUBTYPE__GET_FREQUENCY_TABLE, __command_pkg_id, __acked),
			freq_table(get_vector_copy_by_ptr(__freq_table))
		{};
	};

	/// Событие получения результата по команде SetSamplingFrequency
	class EEG_Event_GCRE_SetSamplingFrequency_event: public EEG_Event_GCRE
	{
	public:
        EEG_Event_GCRE_SetSamplingFrequency_event(long long __command_pkg_id, bool __acked):
			EEG_Event_GCRE(EEG_Event::EEG_EVENT_GCRE_SUBTYPE__SET_SAMPLING_FREQUENCY, __command_pkg_id, __acked)
		{};
	};

	/// Событие получения результата по команде SetChannelsMode
	class EEG_Event_GCRE_SetChannelsMode_event: public EEG_Event_GCRE
	{
	public:
        EEG_Event_GCRE_SetChannelsMode_event(long long __command_pkg_id, bool __acked):
		  EEG_Event_GCRE(EEG_Event::EEG_EVENT_GCRE_SUBTYPE__SET_CHANNELS_MODE, __command_pkg_id, __acked)
		{};
	};

	/// Событие получения результата по команде SetCalibrationShape
	class EEG_Event_GCRE_SetCalibrationShape_event: public EEG_Event_GCRE
	{
	public:
        EEG_Event_GCRE_SetCalibrationShape_event(long long __command_pkg_id, bool __acked):
		  EEG_Event_GCRE(EEG_Event::EEG_EVENT_GCRE_SUBTYPE__SET_CALIBRATION_SHAPE, __command_pkg_id, __acked)
		{};
	};

	/// Событие получения результата по команде SetMUX
	class EEG_Event_GCRE_SetMUX_event: public EEG_Event_GCRE
	{
	public:
        EEG_Event_GCRE_SetMUX_event(long long __command_pkg_id, bool __acked):
		  EEG_Event_GCRE(EEG_Event::EEG_EVENT_GCRE_SUBTYPE__SET_MUX, __command_pkg_id, __acked)
		{};
	};

	/// Событие получения результата по команде StartCalibration
	class EEG_Event_GCRE_StartCalibration_event: public EEG_Event_GCRE
	{
	public:
        EEG_Event_GCRE_StartCalibration_event(long long __command_pkg_id, bool __acked):
		  EEG_Event_GCRE(EEG_Event::EEG_EVENT_GCRE_SUBTYPE__START_CALIBRATION, __command_pkg_id, __acked)
		{};
	};

	/// Событие получения результата по команде StopCalibration
	class EEG_Event_GCRE_StopCalibration_event: public EEG_Event_GCRE
	{
	public:
        EEG_Event_GCRE_StopCalibration_event(long long __command_pkg_id, bool __acked):
		  EEG_Event_GCRE(EEG_Event::EEG_EVENT_GCRE_SUBTYPE__STOP_CALIBRATION, __command_pkg_id, __acked)
		{};
	};

	/// Событие получения результата по команде StartTranslation
	class EEG_Event_GCRE_StartTranslation_event: public EEG_Event_GCRE
	{
	public:
        EEG_Event_GCRE_StartTranslation_event(long long __command_pkg_id, bool __acked):
		  EEG_Event_GCRE(EEG_Event::EEG_EVENT_GCRE_SUBTYPE__START_TRANSLATION, __command_pkg_id, __acked)
		{};
	};

	/// Событие получения результата по команде StopCalibration
	class EEG_Event_GCRE_StopTranslation_event: public EEG_Event_GCRE
	{
	public:
        EEG_Event_GCRE_StopTranslation_event(long long __command_pkg_id, bool __acked):
		  EEG_Event_GCRE(EEG_Event::EEG_EVENT_GCRE_SUBTYPE__STOP_TRANSLATION, __command_pkg_id, __acked)
		{};
	};

	/// Событие получения результата по команде SetChannelLed
	class EEG_Event_GCRE_SetChannelLed_event: public EEG_Event_GCRE
	{
	public:
        EEG_Event_GCRE_SetChannelLed_event(long long __command_pkg_id, bool __acked):
		  EEG_Event_GCRE(EEG_Event::EEG_EVENT_GCRE_SUBTYPE__SET_CHANNEL_LEDS, __command_pkg_id, __acked)
		{};
	};

	/// Событие получения результата по команде SetAmplification
	class EEG_Event_GCRE_SetAmplification_event: public EEG_Event_GCRE
	{
	public:
        EEG_Event_GCRE_SetAmplification_event(long long __command_pkg_id, bool __acked):
		  EEG_Event_GCRE(EEG_Event::EEG_EVENT_GCRE_SUBTYPE__SET_AMPLIFICATION, __command_pkg_id, __acked)
		{};
	};

	/// Событие получения результата по команде SetADSMux
	class EEG_Event_GCRE_SetADSMux_event: public EEG_Event_GCRE
	{
	public:
        EEG_Event_GCRE_SetADSMux_event(long long __command_pkg_id, bool __acked):
		  EEG_Event_GCRE(EEG_Event::EEG_EVENT_GCRE_SUBTYPE__SET_ADS_MUX, __command_pkg_id, __acked)
		{};
	};


	/***********************************************************
	*
	*			События получения данных от устройства
	*
	***********************************************************/

	/// Событие получение обычных данных без стимула
	class EEG_Event_GDE_GotUsualData_event: public EEG_Event
	{
	public:

		/// данные каналов
		const int ch0_data;
		const int ch1_data;
		const int ch2_data;
		const int ch3_data;

	public:

		EEG_Event_GDE_GotUsualData_event(int ch0val, int ch1val, int ch2val, int ch3val):
		  EEG_Event(EEG_Event::EEG_EVENT_TYPE__GOT_DATA, EEG_Event::EEG_EVENT_GDE_SUBTYPE__USUAL_CHANNELS_DATA_WITHOUT_STIMUL),
			ch0_data(ch0val),
			ch1_data(ch1val),
			ch2_data(ch2val),
			ch3_data(ch3val)
		{};
	};

	/// Событие получение обычных данных со стимулом
	class EEG_Event_GDE_GotUsualDataWithStimul_event: public EEG_Event
	{
	public:

		/// данные каналов
		const int ch0_data;
		const int ch1_data;
		const int ch2_data;
		const int ch3_data;

	public:

		EEG_Event_GDE_GotUsualDataWithStimul_event(int ch0val, int ch1val, int ch2val, int ch3val):
		  EEG_Event(EEG_Event::EEG_EVENT_TYPE__GOT_DATA, EEG_Event::EEG_EVENT_GDE_SUBTYPE__USUAL_CHANNELS_DATA_WITH_STIMUL),
			ch0_data(ch0val),
			ch1_data(ch1val),
			ch2_data(ch2val),
			ch3_data(ch3val)
		{};
	};

	/// Событие получение обычных данных со стимулом
	class EEG_Event_GDE_GotImpedanceMBNData_event: public EEG_Event
	{
	public:

		/// номер канала, для которого передаются данные
		unsigned char ch_num; 

		/// данные канала
		vector<int> ch_data;

	public:

		EEG_Event_GDE_GotImpedanceMBNData_event(unsigned char channel_num, vector<int> channel_data):
		  EEG_Event(EEG_Event::EEG_EVENT_TYPE__GOT_DATA, EEG_Event::EEG_EVENT_GDE_SUBTYPE__IMPEDANCE_MBN),
			ch_num(channel_num),
			ch_data(channel_data)
		{};

		virtual ~EEG_Event_GDE_GotImpedanceMBNData_event()
		{};
	};


	/***********************************************************
	*
	*			События получения сообщения от устройства
	*
	***********************************************************/

	/// Событие получение сообщения от устройства
	class EEG_Event_GME_DeviceReady_event: public EEG_Event
	{
	public:

		EEG_Event_GME_DeviceReady_event():
		  EEG_Event(EEG_Event::EEG_EVENT_TYPE__GOT_MESSAGE, EEG_Event::EEG_EVENT_GME_SUBTYPE_DEVICE_READY_MSG)
		{};
	};


	/***********************************************************
	*
	*				Очередь событий устройства
	*
	***********************************************************/


	/// потокозащищённая очередь произошедших событий
	class EEG_Event_Queue
	{
	private:
		/// мьютекс доступа
		QMutex mutex;

		/// сама очередь сообщений
		list<EEG_Event*> events; 

	public:

		/// Добавить событие в очередь событий (в очередь добавляется указатель на событие!)
		/// new_event - указатель на объект, находящийся в куче, удалять его нельзя. Событие будет удалено автоматически в конце отработки GetEvent()
		inline void AddEvent(EEG_Event *new_event)
		{
			mutex.lock();
			events.push_back(new_event);
			mutex.unlock();
		};

		/// Возвращает указатель на событие и одновременно удаляет указатель на событие из очереди событий
		/// Если очередь событий пуста - возвращает 0
		EEG_Event* GetAndDispatchEvent()
		{
			EEG_Event* ret_val = 0;

			mutex.lock();
			// если лист не пустой, возвращаем его первую запись и удаляем её из списка
			if (events.size()) 
			{
				ret_val = events.front();
				events.pop_front();
			};
			mutex.unlock();

			return ret_val;
		};

		/// Возвращает размер очереди событий
		int GetEventsCount()
		{
			int ret_val = 0;

			mutex.lock();
			ret_val = events.size();
			mutex.unlock();

			return ret_val;
		};

		// очищает очередь событий
		void ClearEventsTable()
		{
			mutex.lock();
			for (list<EEG_Event*>::iterator curr = events.begin(); curr != events.end(); curr++)
				delete (*curr);
			events.clear();
			mutex.unlock();
		};

		// При удалении - уничтожаем, все события, под которые вручную была быделена память
		~EEG_Event_Queue()
		{
			ClearEventsTable();
		};
	};


	/***********************************************************
	*
	*			Базовый класс обработчиков событий
	*
	***********************************************************/

	
	/// базовый класс EEG_Event_Vector_Base - базовый класс обработчика события
	/// Хранит данные - кого вызвать и в каком случае (при каком событии) и что передать при вызове
	class EEG_Event_Vector_Base
	{
	private:
		// id поддерживаемых обратных вызовов
		enum EEG_EVENT_VECTOR_BASE__CALLB_TYPE
		{
			EEG_EVENT_VECTOR_BASE__CALLB_TYPE__BASIC, // ф-ция без параметров
			EEG_EVENT_VECTOR_BASE__CALLB_TYPE__ETYPE, // ф-ция принимающая тип события
			EEG_EVENT_VECTOR_BASE__CALLB_TYPE__ETYPE_ESUBTYPE, // ф-ция принимающая тип и подтип события
			EEG_EVENT_VECTOR_BASE__CALLB_TYPE__UPTR, // ф-ция принимающая пользовательский указатель
			EEG_EVENT_VECTOR_BASE__CALLB_TYPE__UPTR_ETYPE, // ф-ция принимающая  пользовательский указатель и тип события
			EEG_EVENT_VECTOR_BASE__CALLB_TYPE__UPTR_ETYPE_ESUBTYPE, // ф-ция принимающая  пользовательский указатель и тип и подтип события
		};

	public:
		/// типы поддерживаемых вызовов
		typedef void (*CB_Type__Basic)(); // ф-ция без параметров
		typedef void (*CB_Type__Etype)(EEG_Event::EEG_EVENT_TYPE e_type); // ф-ция принимающая тип события
		typedef void (*CB_Type__Etype_Esubtype)(EEG_Event::EEG_EVENT_TYPE e_type, EEG_Event::EEG_EVENT_SUBTYPE e_stype); // ф-ция принимающая тип и подтип события
		typedef void (*CB_Type__Uptr)(void* u_ptr); // ф-ция принимающая пользовательский указатель
		typedef void (*CB_Type__Uptr_Etype)(void* u_ptr, EEG_Event::EEG_EVENT_TYPE e_type); // ф-ция принимающая пользовательский указатель и тип события
		typedef void (*CB_Type__Uptr_Etype_Esubtype)(void* u_ptr, EEG_Event::EEG_EVENT_TYPE e_type, EEG_Event::EEG_EVENT_SUBTYPE e_stype); // ф-ция принимающая пользовательский указатель и тип и подтип события

	public:
		/// тип вектора
		const EEG_Event::EEG_EVENT_TYPE	type;

		/// подтип вектора
		const EEG_Event::EEG_EVENT_SUBTYPE	subtype;

		/// является ли триггер одноразовым
		const bool once;

		/// пользовательские данные
		void* const user_pointer;
		
	protected:

		/// обестипизированный указатель на функцию реакции на событие 
		/// на которое настроена данная запись
		const void* callback_ptr;

		/// показывает, какого типа callback функцию нам надо вызывать,
		/// интерпретация этого поля у каждого потомка задаётся функцией Raise,
		/// для каждого потомка интерпретация своя
		const int callback_type_id;

		/// конструктор для потомков
		EEG_Event_Vector_Base(EEG_Event::EEG_EVENT_TYPE	__vector_type,
							  EEG_Event::EEG_EVENT_SUBTYPE	__vector_subtype,
							  void *__ack_callback_ptr,
							  bool __once_fire,
							  int __callback_type_id, 
							  void *usr_ptr):
					type(__vector_type), 
					subtype(__vector_subtype),
					callback_ptr(__ack_callback_ptr),
					once(__once_fire), 
					user_pointer(usr_ptr),
					callback_type_id(__callback_type_id)
		{};

	public:

	/// макросовый шаблон конструктора   (__callback_type - тип функции обр выз в С++, __callback_type_id - число - id типа функции обратного вызова)
	#define EEG_Event_Vector_Base_CONSTRUCTOR(__callback_type, __callback_type_id)	\
		EEG_Event_Vector_Base(														\
							  EEG_Event::EEG_EVENT_TYPE	__vector_type,				\
							  EEG_Event::EEG_EVENT_SUBTYPE	__vector_subtype,		\
							  __callback_type __callback_ptr,						\
							  bool __once_fire = false,								\
							  void *user_ptr = 0									\
							 ):														\
			type(__vector_type),													\
			subtype(__vector_subtype),												\
            callback_ptr((const void*)__callback_ptr),											\
			once(__once_fire),														\
			user_pointer(user_ptr),													\
			callback_type_id(__callback_type_id)									\
		{}


		/// конструкторы для простого вызова
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Basic,	EEG_EVENT_VECTOR_BASE__CALLB_TYPE__BASIC);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Etype,	 EEG_EVENT_VECTOR_BASE__CALLB_TYPE__ETYPE);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Etype_Esubtype,	EEG_EVENT_VECTOR_BASE__CALLB_TYPE__ETYPE_ESUBTYPE);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Uptr,	EEG_EVENT_VECTOR_BASE__CALLB_TYPE__UPTR);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Uptr_Etype,	EEG_EVENT_VECTOR_BASE__CALLB_TYPE__UPTR_ETYPE);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Uptr_Etype_Esubtype,		EEG_EVENT_VECTOR_BASE__CALLB_TYPE__UPTR_ETYPE_ESUBTYPE);

	#undef EEG_Event_Vector_Base_CONSTRUCTOR

		
		/// Запускается ли этот вектор этим событием
		virtual bool TriggeredBy(const EEG_Event &eeg_event)
		{
			// если реагируем на любое событие
			if (type == EEG_Event::EEG_EVENT_TYPE__ANY) return true;
			
			// или если тип вектора и события совпадают
			if (type == eeg_event.type)
			{
				// и если подтип - любой
				if (subtype == EEG_Event::EEG_EVENT_SUBTYPE__ANY) return true;

				// или, если это наше событие
				if (subtype == eeg_event.subtype) return true;
			};

			return false;
		};

		/// выполнить вектор
		/// возвращает, был ли запущен вектор
		virtual bool RaiseIfTriggered(const EEG_Event &eeg_event)
		{	
			// если не запускаемся триггером - выходим
			if (!TriggeredBy(eeg_event)) return false;

			//вызываем нужный обработчик
			switch(callback_type_id)
			{
			case EEG_EVENT_VECTOR_BASE__CALLB_TYPE__BASIC:					(*((CB_Type__Basic)					callback_ptr))(); return true;
			case EEG_EVENT_VECTOR_BASE__CALLB_TYPE__ETYPE:					(*((CB_Type__Etype)					callback_ptr))(eeg_event.type); return true;
			case EEG_EVENT_VECTOR_BASE__CALLB_TYPE__ETYPE_ESUBTYPE:			(*((CB_Type__Etype_Esubtype)		callback_ptr))(eeg_event.type, eeg_event.subtype); return true;
			case EEG_EVENT_VECTOR_BASE__CALLB_TYPE__UPTR:					(*((CB_Type__Uptr)					callback_ptr))(user_pointer); return true;
			case EEG_EVENT_VECTOR_BASE__CALLB_TYPE__UPTR_ETYPE:				(*((CB_Type__Uptr_Etype)			callback_ptr))(user_pointer, eeg_event.type); return true;
			case EEG_EVENT_VECTOR_BASE__CALLB_TYPE__UPTR_ETYPE_ESUBTYPE:	(*((CB_Type__Uptr_Etype_Esubtype)	callback_ptr))(user_pointer, eeg_event.type, eeg_event.subtype); return true;
			};

			return false;
		};	
	};


	/***********************************************************
	*
	*			Общий класс для обработчиков событий
	*				принятия решения по команде
	*
	***********************************************************/

	/// общий класс обработчика события получения результата по команде
	class EEG_Event_Vector_GCRV: public EEG_Event_Vector_Base
	{
	protected:
		// id поддерживаемых обратных вызовов  GCRV
		enum EEG_EVENT_VECTOR_GCRV__CALLB_TYPE
		{
			EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC, // ф-ция без параметров
			EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR, // ф-ция принимающая пользовательский указатель
			EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__ESUBTYPE, // ф-ция принимающая подтип события
			EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR_ESUBTYPE, // ф-ция принимающая пользовательский указатель и подтип события

			EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK, // ф-ция принимающая command package id и acknoledge state
			EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR, // ф-ция принимающая command package id, acknoledge state и пользовательский указатель
			EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_ESUBTYPE, // ф-ция принимающая command package id, acknoledge state и подтип события
			EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_ESUBTYPE, // ф-ция принимающая command package id, acknoledge state, пользовательский указатель и подтип события

			// для команды Любой конкретной команды

			EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__SD, // ф-ция только со спец-данными
			EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR_SD, // ф-ция принимающая пользовательский указатель и спец-данные
			EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_SD, // ф-ция принимающая command package id, acknoledge state и спец-данные
			EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_SD // ф-ция принимающая command package id, acknoledge state, пользовательский указатель и спец-данные
		};

	public:

		// возможные состояния подтверждения
        enum
        {
			ACK_STATE_ANY = 0,
			ACK_STATE_ACKED,
			ACK_STATE_TIMEOUT
		};

	public:
		/// типы поддерживаемых вызовов
		typedef void (*CB_Type__Basic)();	// ф-ция без параметров
		typedef void (*CB_Type__Uptr)(void* u_ptr);	// ф-ция принимающая пользовательский указатель
		typedef void (*CB_Type__Esubtype)(EEG_Event::EEG_EVENT_SUBTYPE e_stype);	 // ф-ция принимающая подтип события
		typedef void (*CB_Type__Uptr_Esubtype)(void* u_ptr, EEG_Event::EEG_EVENT_SUBTYPE e_stype);	// ф-ция принимающая пользовательский указатель и подтип события

        typedef void (*CB_Type__CPid_Ack)(long long command_pkg_id, bool acked);		// ф-ция принимающая command package id и acknoledge state
        typedef void (*CB_Type__CPid_Ack_Uptr)(long long command_pkg_id, bool acked, void* u_ptr);	// ф-ция принимающая command package id, acknoledge state и пользовательский указатель
        typedef void (*CB_Type__CPid_Ack_Esubtype)(long long command_pkg_id, bool acked, EEG_Event::EEG_EVENT_SUBTYPE e_stype);	// ф-ция принимающая package command id, acknoledge state и подтип события
        typedef void (*CB_Type__CPid_Ack_Uptr_Esubtype)(long long command_pkg_id, bool acked, void* u_ptr, EEG_Event::EEG_EVENT_SUBTYPE e_stype);	// ф-ция принимающая package command id, acknoledge state, пользовательский указатель и подтип события


	public:

		// id посылки команды, на которую мы настроены (если 0 - мы реагируем на любую посылку с командой соответствующего типа)
        const long long command_pkg_id;


		// состояние подтверждения на которое мы реагируем
        const int ack_state;


	protected:

		/// конструктор для потомков
		EEG_Event_Vector_GCRV(														
							EEG_Event::EEG_EVENT_SUBTYPE	vector_subtype,			
							void *callback_ptr,			
							int callback_type_id,
							void *user_ptr = 0,										
                            int acknoledge_state = ACK_STATE_ANY,
							bool __once_fire = false,								
                            long long __command_pkg_id = 0
						 ):															
		EEG_Event_Vector_Base(														
								EEG_Event::EEG_EVENT_TYPE__GOT_COMMAND_RESPONSE,	
								vector_subtype,										
								callback_ptr,								
								__once_fire,										
								callback_type_id,									
								user_ptr											
							 ),														
		command_pkg_id(__command_pkg_id),											
		ack_state(acknoledge_state)													
		{};	


	public:
	

		/// макросовый шаблон конструктора   (__callback_type - тип функции обр выз в С++, __callback_type_id - число - id типа функции обратного вызова)
		#define EEG_Event_Vector_Base_CONSTRUCTOR(__callback_type, __callback_type_id)	\
			EEG_Event_Vector_GCRV(														\
								EEG_Event::EEG_EVENT_SUBTYPE	vector_subtype,			\
								__callback_type callback_ptr,							\
								void *user_ptr = 0,										\
                                int acknoledge_state = ACK_STATE_ANY,				\
								bool __once_fire = false,								\
                                long long __command_pkg_id = 0							\
							 ):															\
			EEG_Event_Vector_Base(														\
									EEG_Event::EEG_EVENT_TYPE__GOT_COMMAND_RESPONSE,	\
									vector_subtype,										\
									(void*)callback_ptr,								\
									__once_fire,										\
									__callback_type_id,									\
									user_ptr											\
								 ),														\
			command_pkg_id(__command_pkg_id),											\
			ack_state(acknoledge_state)													\
			{}																			

		// пользовательские конструкторы
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Basic, EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Uptr, EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Esubtype, EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__ESUBTYPE);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Uptr_Esubtype, EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR_ESUBTYPE);

		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__CPid_Ack, EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__CPid_Ack_Uptr, EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__CPid_Ack_Esubtype, EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_ESUBTYPE);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__CPid_Ack_Uptr_Esubtype, EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_ESUBTYPE);

		#undef EEG_Event_Vector_Base_CONSTRUCTOR


		/// Запускается ли этот вектор эти событием,
		/// Наша реакция = наша собственная реакция && реакция предка
		virtual bool TriggeredBy(const EEG_Event &eeg_event)
		{
			// только если предок реагирует на событие - мы можем на него ответить
			if (EEG_Event_Vector_Base::TriggeredBy(eeg_event))  
			{
				// должны проходить по соответствующим условиям для id посылки команды 
				// и по состоянию подтверждения
				return
					(	
						(command_pkg_id == 0) || 
						(command_pkg_id == ((EEG_Event_GCRE*)(&eeg_event))->command_pkg_id)
					)
					&&
					(	
						(ack_state == ACK_STATE_ANY) || 
						((ack_state == ACK_STATE_ACKED) && (((EEG_Event_GCRE*)(&eeg_event))->Acknoledged)) || 
						((ack_state == ACK_STATE_TIMEOUT) && (!((EEG_Event_GCRE*)(&eeg_event))->Acknoledged)) 
					);
			};

			return false;
		};

		/// выполнить вектор
		/// возвращает, был ли запущен вектор
		virtual bool RaiseIfTriggered(const EEG_Event &eeg_event)
		{			
			// если не запускаемся триггером - выходим
			if (!TriggeredBy(eeg_event)) return false;

			// получаем типизированный в соответствии с событием указатель на сообщение
			EEG_Event_GCRE *ev = (EEG_Event_GCRE *)(&eeg_event);
			
			//вызываем обработчик
			switch(callback_type_id)
			{	
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC: (*((CB_Type__Basic)callback_ptr))(); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR: (*((CB_Type__Uptr)callback_ptr))(user_pointer); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__ESUBTYPE: (*((CB_Type__Esubtype)callback_ptr))(eeg_event.subtype); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR_ESUBTYPE: (*((CB_Type__Uptr_Esubtype)callback_ptr))(user_pointer, eeg_event.subtype); return true;

			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK: (*((CB_Type__CPid_Ack)callback_ptr))(ev->command_pkg_id, ev->Acknoledged); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR: (*((CB_Type__CPid_Ack_Uptr)callback_ptr))(ev->command_pkg_id, ev->Acknoledged, user_pointer); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_ESUBTYPE: (*((CB_Type__CPid_Ack_Esubtype)callback_ptr))(ev->command_pkg_id, ev->Acknoledged, eeg_event.subtype); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_ESUBTYPE : (*((CB_Type__CPid_Ack_Uptr_Esubtype)callback_ptr))(ev->command_pkg_id, ev->Acknoledged, user_pointer, eeg_event.subtype); return true;
			};

			return false;
		};
	};


	/***********************************************************
	*
	*		Классы обработчиков событий конкретных команд
	*
	***********************************************************/


	/// вектор команды Reset
	class EEG_Event_Vector_C_Reset: public EEG_Event_Vector_GCRV
	{
	public:
		
	/// макро-шаблон конструктора 
	#define EEG_Event_CONSTRUCTOR(callback_type, callback_type_id)	\
        EEG_Event_Vector_C_Reset(callback_type __callback_ptr, void* __user_pointer = 0, int __ack_state = EEG_Event_Vector_GCRV::ACK_STATE_ANY, bool __once_fire = false,  long long for_command_pkg_id = 0 ): \
            EEG_Event_Vector_GCRV( MBN_EEG::EEG_Event::EEG_EVENT_GCRE_SUBTYPE__RESET, ( void*)__callback_ptr, callback_type_id, __user_pointer, __ack_state, __once_fire, for_command_pkg_id) {}


	// конструкторы
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Basic,			EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Uptr,			EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR);
	
	#undef EEG_Event_CONSTRUCTOR
	};

	/// вектор команды GetVersion
	class EEG_Event_Vector_C_GetVersion: public EEG_Event_Vector_GCRV
	{
	public:
		/// типы поддерживаемых вызовов
        typedef void (*CB_Type_GetVer__Data)(unsigned char FirmwareVersion, unsigned char CircuityVersion, unsigned short SerialNumber);	// ф-ция со спец-данными
        typedef void (*CB_Type_GetVer__Uptr_GetVer_Data)(void* u_ptr, unsigned char FirmwareVersion, unsigned char CircuityVersion, unsigned short SerialNumber);	// ф-ция принимающая пользовательский указатель и спец-данные
        typedef void (*CB_Type_GetVer__CPid_Ack_GetVer_Data)(long long command_pkg_id, bool acked, unsigned char FirmwareVersion, unsigned char CircuityVersion, unsigned short SerialNumber);		// ф-ция принимающая command package id и acknoledge state и спец-данные
        typedef void (*CB_Type_GetVer__CPid_Ack_Uptr_GetVer_Data)(long long command_pkg_id, bool acked, void* u_ptr, unsigned char FirmwareVersion, unsigned char CircuityVersion, unsigned short SerialNumber);	// ф-ция принимающая command package id, acknoledge state и пользовательский указатель и спец-данные

	public:	

	/// макро-шаблон конструктора 
	#define EEG_Event_CONSTRUCTOR(C_Type, C_TYPE_ID)  \
        EEG_Event_Vector_C_GetVersion(C_Type __callback_ptr, void* __user_pointer = 0, int __ack_state = EEG_Event_Vector_GCRV::ACK_STATE_ANY, bool __once_fire = false,  long long for_command_pkg_id = 0 ): \
			EEG_Event_Vector_GCRV( MBN_EEG::EEG_Event::EEG_EVENT_GCRE_SUBTYPE__GET_VERSION, (void*) __callback_ptr, C_TYPE_ID, __user_pointer, __ack_state, __once_fire, for_command_pkg_id) {}

		// пользовательские конструкторы
		EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Basic, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC);
		EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR);
		EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK);
		EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR);

		EEG_Event_CONSTRUCTOR(CB_Type_GetVer__Data, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_SD);
		EEG_Event_CONSTRUCTOR(CB_Type_GetVer__Uptr_GetVer_Data, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_SD);
		EEG_Event_CONSTRUCTOR(CB_Type_GetVer__CPid_Ack_GetVer_Data, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_SD);
		EEG_Event_CONSTRUCTOR(CB_Type_GetVer__CPid_Ack_Uptr_GetVer_Data, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_SD);

	#undef EEG_Event_CONSTRUCTOR

		/// выполнить вектор
		/// возвращает, был ли запущен вектор
		virtual bool RaiseIfTriggered(const EEG_Event &eeg_event)
		{			
			// если не запускаемся триггером - выходим
			if (!TriggeredBy(eeg_event)) return false;

			// получаем типизированный в соответствии с событием указатель на сообщение
			EEG_Event_GCRE_GetVersion_event *ev = (EEG_Event_GCRE_GetVersion_event *)(&eeg_event);

			//вызываем обработчик
			switch(EEG_Event_Vector_GCRV::callback_type_id)
			{	
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC: (*((EEG_Event_Vector_GCRV::CB_Type__Basic)EEG_Event_Vector_GCRV::callback_ptr))(); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR: (*((EEG_Event_Vector_GCRV::CB_Type__Uptr)EEG_Event_Vector_GCRV::callback_ptr))(user_pointer); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK: (*((EEG_Event_Vector_GCRV::CB_Type__CPid_Ack)EEG_Event_Vector_GCRV::callback_ptr))(ev->command_pkg_id, ev->Acknoledged); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR: (*((EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr)EEG_Event_Vector_GCRV::callback_ptr))(ev->command_pkg_id, ev->Acknoledged, user_pointer); return true;

			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__SD: (*((CB_Type_GetVer__Data)EEG_Event_Vector_GCRV::callback_ptr))(ev->FirmwareVersion, ev->CircuityVersion, ev->SerialNumber); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR_SD: (*((CB_Type_GetVer__Uptr_GetVer_Data)EEG_Event_Vector_GCRV::callback_ptr))(user_pointer, ev->FirmwareVersion, ev->CircuityVersion, ev->SerialNumber); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_SD: (*((CB_Type_GetVer__CPid_Ack_GetVer_Data)EEG_Event_Vector_GCRV::callback_ptr))(ev->command_pkg_id, ev->Acknoledged, ev->FirmwareVersion, ev->CircuityVersion, ev->SerialNumber); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_SD: (*((CB_Type_GetVer__CPid_Ack_Uptr_GetVer_Data)EEG_Event_Vector_GCRV::callback_ptr))(ev->command_pkg_id, ev->Acknoledged, user_pointer, ev->FirmwareVersion, ev->CircuityVersion, ev->SerialNumber); return true;
			};

			return false;
		};
	};

	/// вектор команды SetMux
	class EEG_Event_Vector_C_SetMux: public EEG_Event_Vector_GCRV
	{
	public:
		
	/// макро-шаблон конструктора 
	#define EEG_Event_CONSTRUCTOR(C_Type, C_type_Id) \
        EEG_Event_Vector_C_SetMux(C_Type __callback_ptr, void* __user_pointer = 0, int __ack_state = EEG_Event_Vector_GCRV::ACK_STATE_ANY, bool __once_fire = false,  long long for_command_pkg_id = 0 ):\
            EEG_Event_Vector_GCRV( MBN_EEG::EEG_Event::EEG_EVENT_GCRE_SUBTYPE__SET_MUX, ( void*)__callback_ptr, C_type_Id, __user_pointer, __ack_state, __once_fire, for_command_pkg_id) {}


	// конструкторы
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Basic,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Uptr,			EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR);
	
	#undef EEG_Event_CONSTRUCTOR
	};

	/// вектор команды GetProtocolVersion
	class EEG_Event_Vector_C_GetProtocolVersion: public EEG_Event_Vector_GCRV
	{
	public:
		/// типы поддерживаемых вызовов
		typedef void (*CB_Type_GetVer__Data)(unsigned char ProtocolVersion);	// ф-ция со спец-данными
		typedef void (*CB_Type_GetVer__Uptr_GetVer_Data)(void* u_ptr, unsigned char ProtocolVersion);	// ф-ция принимающая пользовательский указатель и спец-данные
        typedef void (*CB_Type_GetVer__CPid_Ack_GetVer_Data)(long long command_pkg_id, bool acked, unsigned char ProtocolVersion);		// ф-ция принимающая command package id и acknoledge state и спец-данные
        typedef void (*CB_Type_GetVer__CPid_Ack_Uptr_GetVer_Data)(long long command_pkg_id, bool acked, void* u_ptr, unsigned char ProtocolVersion);	// ф-ция принимающая command package id, acknoledge state и пользовательский указатель и спец-данные

	public:	

	/// макро-шаблон конструктора 
	#define EEG_Event_CONSTRUCTOR(C_Type, C_TYPE_ID)  \
        EEG_Event_Vector_C_GetProtocolVersion(C_Type __callback_ptr, void* __user_pointer = 0, int __ack_state = EEG_Event_Vector_GCRV::ACK_STATE_ANY, bool __once_fire = false,  long long for_command_pkg_id = 0 ): \
			EEG_Event_Vector_GCRV( MBN_EEG::EEG_Event::EEG_EVENT_GCRE_SUBTYPE__GET_PROTOCOL_VERSION, (void*) __callback_ptr, C_TYPE_ID, __user_pointer, __ack_state, __once_fire, for_command_pkg_id) {}

		// пользовательские конструкторы
		EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Basic, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC);
		EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR);
		EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK);
		EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR);

		EEG_Event_CONSTRUCTOR(CB_Type_GetVer__Data, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_SD);
		EEG_Event_CONSTRUCTOR(CB_Type_GetVer__Uptr_GetVer_Data, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_SD);
		EEG_Event_CONSTRUCTOR(CB_Type_GetVer__CPid_Ack_GetVer_Data, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_SD);
		EEG_Event_CONSTRUCTOR(CB_Type_GetVer__CPid_Ack_Uptr_GetVer_Data, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_SD);

	#undef EEG_Event_CONSTRUCTOR

		/// выполнить вектор
		/// возвращает, был ли запущен вектор
		virtual bool RaiseIfTriggered(const EEG_Event &eeg_event)
		{			
			// если не запускаемся триггером - выходим
			if (!TriggeredBy(eeg_event)) return false;

			// получаем типизированный в соответствии с событием указатель на сообщение
			EEG_Event_GCRE_GetProtocolVersion_event *ev = (EEG_Event_GCRE_GetProtocolVersion_event *)(&eeg_event);

			//вызываем обработчик
			switch(EEG_Event_Vector_GCRV::callback_type_id)
			{	
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC: (*((EEG_Event_Vector_GCRV::CB_Type__Basic)EEG_Event_Vector_GCRV::callback_ptr))(); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR: (*((EEG_Event_Vector_GCRV::CB_Type__Uptr)EEG_Event_Vector_GCRV::callback_ptr))(user_pointer); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK: (*((EEG_Event_Vector_GCRV::CB_Type__CPid_Ack)EEG_Event_Vector_GCRV::callback_ptr))(ev->command_pkg_id, ev->Acknoledged); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR: (*((EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr)EEG_Event_Vector_GCRV::callback_ptr))(ev->command_pkg_id, ev->Acknoledged, user_pointer); return true;

			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__SD: (*((CB_Type_GetVer__Data)EEG_Event_Vector_GCRV::callback_ptr))(ev->ProtocolVersion); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR_SD: (*((CB_Type_GetVer__Uptr_GetVer_Data)EEG_Event_Vector_GCRV::callback_ptr))(user_pointer, ev->ProtocolVersion); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_SD: (*((CB_Type_GetVer__CPid_Ack_GetVer_Data)EEG_Event_Vector_GCRV::callback_ptr))(ev->command_pkg_id, ev->Acknoledged, ev->ProtocolVersion); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_SD: (*((CB_Type_GetVer__CPid_Ack_Uptr_GetVer_Data)EEG_Event_Vector_GCRV::callback_ptr))(ev->command_pkg_id, ev->Acknoledged, user_pointer, ev->ProtocolVersion); return true;
			};

			return false;
		};
	};

	/// вектор команды GetFrequenciesTable
	class EEG_Event_Vector_C_GetFrequenciesTable: public EEG_Event_Vector_GCRV
	{
	public:
		/// типы поддерживаемых вызовов
        typedef void (*CB_Type_GetVer__Data)(const vector<unsigned short> &freqTable);	// ф-ция со спец-данными
        typedef void (*CB_Type_GetVer__Uptr_GetVer_Data)(void* u_ptr, const vector<unsigned short> &freqTable);	// ф-ция принимающая пользовательский указатель и спец-данные
        typedef void (*CB_Type_GetVer__CPid_Ack_GetVer_Data)(long long command_pkg_id, bool acked, const vector<unsigned short> &freqTable);		// ф-ция принимающая command package id и acknoledge state и спец-данные
        typedef void (*CB_Type_GetVer__CPid_Ack_Uptr_GetVer_Data)(long long command_pkg_id, bool acked, void* u_ptr, const vector<unsigned short> &freqTable);	// ф-ция принимающая command package id, acknoledge state и пользовательский указатель и спец-данные

	public:	

	/// макро-шаблон конструктора 
	#define EEG_Event_CONSTRUCTOR(C_Type, C_TYPE_ID)  \
        EEG_Event_Vector_C_GetFrequenciesTable(C_Type __callback_ptr, void* __user_pointer = 0, int __ack_state = EEG_Event_Vector_GCRV::ACK_STATE_ANY, bool __once_fire = false,  long long for_command_pkg_id = 0 ): \
			EEG_Event_Vector_GCRV( MBN_EEG::EEG_Event::EEG_EVENT_GCRE_SUBTYPE__GET_PROTOCOL_VERSION, (void*) __callback_ptr, C_TYPE_ID, __user_pointer, __ack_state, __once_fire, for_command_pkg_id) {}

		// пользовательские конструкторы
		EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Basic, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC);
		EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR);
		EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK);
		EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR);

		EEG_Event_CONSTRUCTOR(CB_Type_GetVer__Data, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_SD);
		EEG_Event_CONSTRUCTOR(CB_Type_GetVer__Uptr_GetVer_Data, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_SD);
		EEG_Event_CONSTRUCTOR(CB_Type_GetVer__CPid_Ack_GetVer_Data, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_SD);
		EEG_Event_CONSTRUCTOR(CB_Type_GetVer__CPid_Ack_Uptr_GetVer_Data, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_SD);

	#undef EEG_Event_CONSTRUCTOR

		/// выполнить вектор
		/// возвращает, был ли запущен вектор
		virtual bool RaiseIfTriggered(const EEG_Event &eeg_event)
		{			
			// если не запускаемся триггером - выходим
			if (!TriggeredBy(eeg_event)) return false;

			// получаем типизированный в соответствии с событием указатель на сообщение
			EEG_Event_GCRE_GetFrequenciesTable_event *ev = (EEG_Event_GCRE_GetFrequenciesTable_event *)(&eeg_event);

			//вызываем обработчик
			switch(EEG_Event_Vector_GCRV::callback_type_id)
			{	
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC: (*((EEG_Event_Vector_GCRV::CB_Type__Basic)EEG_Event_Vector_GCRV::callback_ptr))(); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR: (*((EEG_Event_Vector_GCRV::CB_Type__Uptr)EEG_Event_Vector_GCRV::callback_ptr))(user_pointer); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK: (*((EEG_Event_Vector_GCRV::CB_Type__CPid_Ack)EEG_Event_Vector_GCRV::callback_ptr))(ev->command_pkg_id, ev->Acknoledged); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR: (*((EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr)EEG_Event_Vector_GCRV::callback_ptr))(ev->command_pkg_id, ev->Acknoledged, user_pointer); return true;

			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__SD: (*((CB_Type_GetVer__Data)EEG_Event_Vector_GCRV::callback_ptr))(ev->freq_table); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR_SD: (*((CB_Type_GetVer__Uptr_GetVer_Data)EEG_Event_Vector_GCRV::callback_ptr))(user_pointer, ev->freq_table); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_SD: (*((CB_Type_GetVer__CPid_Ack_GetVer_Data)EEG_Event_Vector_GCRV::callback_ptr))(ev->command_pkg_id, ev->Acknoledged, ev->freq_table); return true;
			case EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR_SD: (*((CB_Type_GetVer__CPid_Ack_Uptr_GetVer_Data)EEG_Event_Vector_GCRV::callback_ptr))(ev->command_pkg_id, ev->Acknoledged, user_pointer, ev->freq_table); return true;
			};

			return false;
		};
	};

	/// вектор команды SetSamplingFrequency
	class EEG_Event_Vector_C_SetSamplingFrequency: public EEG_Event_Vector_GCRV
	{
	public:
		
	/// макро-шаблон конструктора 
	#define EEG_Event_CONSTRUCTOR(C_Type, C_type_Id) \
        EEG_Event_Vector_C_SetSamplingFrequency(C_Type __callback_ptr, void* __user_pointer = 0, int __ack_state = EEG_Event_Vector_GCRV::ACK_STATE_ANY, bool __once_fire = false,  long long for_command_pkg_id = 0 ):\
            EEG_Event_Vector_GCRV( MBN_EEG::EEG_Event::EEG_EVENT_GCRE_SUBTYPE__SET_SAMPLING_FREQUENCY, (void*)__callback_ptr, C_type_Id, __user_pointer, __ack_state, __once_fire, for_command_pkg_id) {}

	// конструкторы
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Basic,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Uptr,			EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR);
	
	#undef EEG_Event_CONSTRUCTOR
	};

	/// вектор команды SetChannelsMode
	class EEG_Event_Vector_C_SetChannelsMode: public EEG_Event_Vector_GCRV
	{
	public:
		
	/// макро-шаблон конструктора 
	#define EEG_Event_CONSTRUCTOR(C_Type, C_type_Id) \
        EEG_Event_Vector_C_SetChannelsMode(C_Type __callback_ptr, void* __user_pointer = 0, int __ack_state = EEG_Event_Vector_GCRV::ACK_STATE_ANY, bool __once_fire = false,  long long for_command_pkg_id = 0 ):\
            EEG_Event_Vector_GCRV( MBN_EEG::EEG_Event::EEG_EVENT_GCRE_SUBTYPE__SET_CHANNELS_MODE, (void*)__callback_ptr, C_type_Id, __user_pointer, __ack_state, __once_fire, for_command_pkg_id) {}

	// конструкторы
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Basic,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Uptr,			EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR);
	
	#undef EEG_Event_CONSTRUCTOR
	};

	/// вектор команды SetCalibrationShape
	class EEG_Event_Vector_C_SetCalibrationShape: public EEG_Event_Vector_GCRV
	{
	public:
		
	/// макро-шаблон конструктора 
	#define EEG_Event_CONSTRUCTOR(C_Type, C_type_Id) \
        EEG_Event_Vector_C_SetCalibrationShape(C_Type __callback_ptr, void* __user_pointer = 0, int __ack_state = EEG_Event_Vector_GCRV::ACK_STATE_ANY, bool __once_fire = false,  long long for_command_pkg_id = 0 ):\
            EEG_Event_Vector_GCRV( MBN_EEG::EEG_Event::EEG_EVENT_GCRE_SUBTYPE__SET_CALIBRATION_SHAPE, (void*)__callback_ptr, C_type_Id, __user_pointer, __ack_state, __once_fire, for_command_pkg_id) {}

	// конструкторы
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Basic,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Uptr,			EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR);
	
	#undef EEG_Event_CONSTRUCTOR
	};

	/// вектор команды StartCalibration
	class EEG_Event_Vector_C_StartCalibration: public EEG_Event_Vector_GCRV
	{
	public:
		
	/// макро-шаблон конструктора 
	#define EEG_Event_CONSTRUCTOR(C_Type, C_type_Id) \
        EEG_Event_Vector_C_StartCalibration(C_Type __callback_ptr, void* __user_pointer = 0, int __ack_state = EEG_Event_Vector_GCRV::ACK_STATE_ANY, bool __once_fire = false,  long long for_command_pkg_id = 0 ):\
            EEG_Event_Vector_GCRV( MBN_EEG::EEG_Event::EEG_EVENT_GCRE_SUBTYPE__START_CALIBRATION, (void*)__callback_ptr, C_type_Id, __user_pointer, __ack_state, __once_fire, for_command_pkg_id) {}

	// конструкторы
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Basic,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Uptr,			EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR);
	
	#undef EEG_Event_CONSTRUCTOR
	};

	/// вектор команды StopCalibration
	class EEG_Event_Vector_C_StopCalibration: public EEG_Event_Vector_GCRV
	{
	public:
		
	/// макро-шаблон конструктора 
	#define EEG_Event_CONSTRUCTOR(C_Type, C_type_Id) \
        EEG_Event_Vector_C_StopCalibration(C_Type __callback_ptr, void* __user_pointer = 0, int __ack_state = EEG_Event_Vector_GCRV::ACK_STATE_ANY, bool __once_fire = false,  long long for_command_pkg_id = 0 ):\
            EEG_Event_Vector_GCRV( MBN_EEG::EEG_Event::EEG_EVENT_GCRE_SUBTYPE__STOP_CALIBRATION, (void*)__callback_ptr, C_type_Id, __user_pointer, __ack_state, __once_fire, for_command_pkg_id) {}

	// конструкторы
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Basic,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Uptr,			EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR);
	
	#undef EEG_Event_CONSTRUCTOR
	};

	/// вектор команды StartTranslation
	class EEG_Event_Vector_C_StartTranslation: public EEG_Event_Vector_GCRV
	{
	public:
		
	/// макро-шаблон конструктора 
	#define EEG_Event_CONSTRUCTOR(C_Type, C_type_Id) \
        EEG_Event_Vector_C_StartTranslation(C_Type __callback_ptr, void* __user_pointer = 0, int __ack_state = EEG_Event_Vector_GCRV::ACK_STATE_ANY, bool __once_fire = false,  long long for_command_pkg_id = 0 ):\
            EEG_Event_Vector_GCRV( MBN_EEG::EEG_Event::EEG_EVENT_GCRE_SUBTYPE__START_TRANSLATION, (void*)__callback_ptr, C_type_Id, __user_pointer, __ack_state, __once_fire, for_command_pkg_id) {}

	// конструкторы
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Basic,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Uptr,			EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR);
	
	#undef EEG_Event_CONSTRUCTOR
	};

	/// вектор команды StopTranslation
	class EEG_Event_Vector_C_StopTranslation: public EEG_Event_Vector_GCRV
	{
	public:
		
	/// макро-шаблон конструктора 
	#define EEG_Event_CONSTRUCTOR(C_Type, C_type_Id) \
        EEG_Event_Vector_C_StopTranslation(C_Type __callback_ptr, void* __user_pointer = 0, int __ack_state = EEG_Event_Vector_GCRV::ACK_STATE_ANY, bool __once_fire = false,  long long for_command_pkg_id = 0 ):\
            EEG_Event_Vector_GCRV( MBN_EEG::EEG_Event::EEG_EVENT_GCRE_SUBTYPE__STOP_TRANSLATION, (void*)__callback_ptr, C_type_Id, __user_pointer, __ack_state, __once_fire, for_command_pkg_id) {}

	// конструкторы
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Basic,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Uptr,			EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR);
	
	#undef EEG_Event_CONSTRUCTOR
	};

	/// вектор команды SetChannelLeds
	class EEG_Event_Vector_C_SetChannelLeds: public EEG_Event_Vector_GCRV
	{
	public:
		
	/// макро-шаблон конструктора 
	#define EEG_Event_CONSTRUCTOR(C_Type, C_type_Id) \
        EEG_Event_Vector_C_SetChannelLeds(C_Type __callback_ptr, void* __user_pointer = 0, int __ack_state = EEG_Event_Vector_GCRV::ACK_STATE_ANY, bool __once_fire = false,  long long for_command_pkg_id = 0 ):\
            EEG_Event_Vector_GCRV( MBN_EEG::EEG_Event::EEG_EVENT_GCRE_SUBTYPE__SET_CHANNEL_LEDS, (void*)__callback_ptr, C_type_Id, __user_pointer, __ack_state, __once_fire, for_command_pkg_id) {}

	// конструкторы
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Basic,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Uptr,			EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR);
	
	#undef EEG_Event_CONSTRUCTOR
	};

	/// вектор команды SetAmplification
	class EEG_Event_Vector_C_SetAmplification: public EEG_Event_Vector_GCRV
	{
	public:
		
	/// макро-шаблон конструктора 
	#define EEG_Event_CONSTRUCTOR(C_Type, C_type_Id) \
        EEG_Event_Vector_C_SetAmplification(C_Type __callback_ptr, void* __user_pointer = 0, int __ack_state = EEG_Event_Vector_GCRV::ACK_STATE_ANY, bool __once_fire = false,  long long for_command_pkg_id = 0 ):\
        EEG_Event_Vector_GCRV( MBN_EEG::EEG_Event::EEG_EVENT_GCRE_SUBTYPE__SET_AMPLIFICATION, (void*)__callback_ptr, C_type_Id, __user_pointer, __ack_state, __once_fire, for_command_pkg_id) {}

	// конструкторы
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Basic,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Uptr,			EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR);
	
	#undef EEG_Event_CONSTRUCTOR
	};

	/// вектор команды SetADSMux
	class EEG_Event_Vector_C_SetADSMux: public EEG_Event_Vector_GCRV
	{
	public:
		
	/// макро-шаблон конструктора 
	#define EEG_Event_CONSTRUCTOR(C_Type, C_type_Id) \
        EEG_Event_Vector_C_SetADSMux(C_Type __callback_ptr, void* __user_pointer = 0, int __ack_state = EEG_Event_Vector_GCRV::ACK_STATE_ANY, bool __once_fire = false,  long long for_command_pkg_id = 0 ):\
            EEG_Event_Vector_GCRV( MBN_EEG::EEG_Event::EEG_EVENT_GCRE_SUBTYPE__SET_ADS_MUX, (void*)__callback_ptr, C_type_Id, __user_pointer, __ack_state, __once_fire, for_command_pkg_id) {}

	// конструкторы
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Basic,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__BASIC);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__Uptr,			EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__UPTR);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack,		EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GCRV::CB_Type__CPid_Ack_Uptr, EEG_Event_Vector_GCRV::EEG_EVENT_VECTOR_GCRV__CALLB_TYPE__CPID_ACK_UPTR);
	
	#undef EEG_Event_CONSTRUCTOR
	};


	/***********************************************************
	*
	*		Класс базового обработчика сообщений устройства
	*
	***********************************************************/

	/// общий класс обработчика события получения результата сообщения от устройства
	class EEG_Event_Vector_GMV: public EEG_Event_Vector_Base
	{
	protected:
		// id поддерживаемых обратных вызовов  GMV
		enum EEG_EVENT_VECTOR_GMV__CALLB_TYPE
		{
			EEG_EVENT_VECTOR_GMV__CALLB_TYPE__BASIC, // ф-ция без параметров
			EEG_EVENT_VECTOR_GMV__CALLB_TYPE__UPTR, // ф-ция принимающая пользовательский указатель
			EEG_EVENT_VECTOR_GMV__CALLB_TYPE__ESUBTYPE, // ф-ция принимающая подтип события
			EEG_EVENT_VECTOR_GMV__CALLB_TYPE__UPTR_ESUBTYPE, // ф-ция принимающая пользовательский указатель и подтип события

			// для команды Любого конкретного сообщения (для потомков)
			EEG_EVENT_VECTOR_GMV__CALLB_TYPE__SD, // ф-ция только со спец-данными
			EEG_EVENT_VECTOR_GMV__CALLB_TYPE__UPTR_SD, // ф-ция принимающая пользовательский указатель и спец-данные
		};

	public:
		/// типы поддерживаемых вызовов
		typedef void (*CB_Type__Basic)();	// ф-ция без параметров
		typedef void (*CB_Type__Uptr)(void* u_ptr);	// ф-ция принимающая пользовательский указатель
		typedef void (*CB_Type__Esubtype)(EEG_Event::EEG_EVENT_SUBTYPE e_stype);	 // ф-ция принимающая подтип события
		typedef void (*CB_Type__Uptr_Esubtype)(void* u_ptr, EEG_Event::EEG_EVENT_SUBTYPE e_stype);	// ф-ция принимающая пользовательский указатель и подтип события

	protected:

		/// конструктор для потомков
		EEG_Event_Vector_GMV(														
							EEG_Event::EEG_EVENT_SUBTYPE	vector_subtype,			
							void *callback_ptr,			
							int callback_type_id,
							void *user_ptr = 0,										
							bool __once_fire = false							
						 ):															
		EEG_Event_Vector_Base(														
								EEG_Event::EEG_EVENT_TYPE__GOT_MESSAGE,	
								vector_subtype,										
								callback_ptr,								
								__once_fire,										
								callback_type_id,									
								user_ptr											
							 )																								
		{};	


	public:

		/// макросовый шаблон конструктора   (__callback_type - тип функции обр выз в С++, __callback_type_id - число - id типа функции обратного вызова)
		#define EEG_Event_Vector_Base_CONSTRUCTOR(__callback_type, __callback_type_id)	\
			EEG_Event_Vector_GMV(														\
								EEG_Event::EEG_EVENT_SUBTYPE	vector_subtype,			\
								__callback_type callback_ptr,							\
								void *user_ptr = 0,										\
								bool __once_fire = false								\
							 ):															\
			EEG_Event_Vector_Base(														\
									EEG_Event::EEG_EVENT_TYPE__GOT_MESSAGE,				\
									vector_subtype,										\
									(void*)callback_ptr,								\
									__once_fire,										\
									__callback_type_id,									\
									user_ptr											\
								 )														\
			{}

		// пользовательские конструкторы
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Basic, EEG_EVENT_VECTOR_GMV__CALLB_TYPE__BASIC);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Uptr, EEG_EVENT_VECTOR_GMV__CALLB_TYPE__UPTR);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Esubtype, EEG_EVENT_VECTOR_GMV__CALLB_TYPE__ESUBTYPE);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Uptr_Esubtype, EEG_EVENT_VECTOR_GMV__CALLB_TYPE__UPTR_ESUBTYPE);

		#undef EEG_Event_Vector_Base_CONSTRUCTOR
	};

	/***********************************************************
	*
	*		Классы обработчиков событий конкретных сообщений
	*
	***********************************************************/

	/// вектор сообщения DeviceReady
	class EEG_Event_Vector_M_DeviceReady: public EEG_Event_Vector_GMV
	{
	public:

	/// макро-шаблон конструктора 
	#define EEG_Event_CONSTRUCTOR(C_Type, C_type_Id) \
		EEG_Event_Vector_M_DeviceReady(C_Type __callback_ptr, void* __user_pointer = 0, bool __once_fire = false):\
        EEG_Event_Vector_GMV(MBN_EEG::EEG_Event::EEG_EVENT_GME_SUBTYPE_DEVICE_READY_MSG, (void*)__callback_ptr, C_type_Id, __user_pointer, __once_fire) {}

	// конструкторы
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GMV::CB_Type__Basic,		EEG_Event_Vector_GMV::EEG_EVENT_VECTOR_GMV__CALLB_TYPE__BASIC);
	EEG_Event_CONSTRUCTOR(EEG_Event_Vector_GMV::CB_Type__Uptr,		EEG_Event_Vector_GMV::EEG_EVENT_VECTOR_GMV__CALLB_TYPE__UPTR);
	
	#undef EEG_Event_CONSTRUCTOR
	};



	/***********************************************************
	*
	*		Класс базового обработчика получения данных
	*
	***********************************************************/

	/// общий класс обработчика события получения данных от устройства
	class EEG_Event_Vector_GDV: public EEG_Event_Vector_Base
	{
	protected:
		// id поддерживаемых обратных вызовов  GDV
		enum EEG_EVENT_VECTOR_GDV__CALLB_TYPE
		{
			EEG_EVENT_VECTOR_GDV__CALLB_TYPE__BASIC, // ф-ция без параметров
			EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR, // ф-ция принимающая пользовательский указатель
			EEG_EVENT_VECTOR_GDV__CALLB_TYPE__ESUBTYPE, // ф-ция принимающая подтип события
			EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR_ESUBTYPE, // ф-ция принимающая пользовательский указатель и подтип события

			// для команды Любого конкретного сообщения (для потомков)
			EEG_EVENT_VECTOR_GDV__CALLB_TYPE__SD, // ф-ция только со спец-данными
			EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR_SD, // ф-ция принимающая пользовательский указатель и спец-данные
		};

	public:
		/// типы поддерживаемых вызовов
		typedef void (*CB_Type__Basic)();	// ф-ция без параметров
		typedef void (*CB_Type__Uptr)(void* u_ptr);	// ф-ция принимающая пользовательский указатель
		typedef void (*CB_Type__Esubtype)(EEG_Event::EEG_EVENT_SUBTYPE e_stype);	 // ф-ция принимающая подтип события
		typedef void (*CB_Type__Uptr_Esubtype)(void* u_ptr, EEG_Event::EEG_EVENT_SUBTYPE e_stype);	// ф-ция принимающая пользовательский указатель и подтип события

	protected:

		/// конструктор для потомков
		EEG_Event_Vector_GDV(														
							EEG_Event::EEG_EVENT_SUBTYPE	vector_subtype,			
							void *callback_ptr,			
							int callback_type_id,
							void *user_ptr = 0,										
							bool __once_fire = false							
						 ):															
		EEG_Event_Vector_Base(														
								EEG_Event::EEG_EVENT_TYPE__GOT_DATA,	
								vector_subtype,										
								callback_ptr,								
								__once_fire,										
								callback_type_id,									
								user_ptr											
							 )																								
		{};	


	public:

		/// макросовый шаблон конструктора   (__callback_type - тип функции обр выз в С++, __callback_type_id - число - id типа функции обратного вызова)
		#define EEG_Event_Vector_Base_CONSTRUCTOR(__callback_type, __callback_type_id)	\
			EEG_Event_Vector_GDV(														\
								EEG_Event::EEG_EVENT_SUBTYPE	vector_subtype,			\
								__callback_type callback_ptr,							\
								void *user_ptr = 0,										\
								bool __once_fire = false								\
							 ):															\
			EEG_Event_Vector_Base(														\
									EEG_Event::EEG_EVENT_TYPE__GOT_DATA,				\
									vector_subtype,										\
									(void*)callback_ptr,								\
									__once_fire,										\
									__callback_type_id,									\
									user_ptr											\
								 )														\
			{}

		// пользовательские конструкторы
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Basic, EEG_EVENT_VECTOR_GDV__CALLB_TYPE__BASIC);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Uptr, EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Esubtype, EEG_EVENT_VECTOR_GDV__CALLB_TYPE__ESUBTYPE);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Uptr_Esubtype, EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR_ESUBTYPE);

		#undef EEG_Event_Vector_Base_CONSTRUCTOR
	};

	/*************************************************************
	*
	*		Классы обработчиков событий приёма конкретных данных
	*
	*************************************************************/

	/// вектор события приёма обычных данных
	class EEG_Event_Vector_D_UsualData: public EEG_Event_Vector_GDV
	{
	public:

		typedef void (*CB_Type__SD)(int ch0val, int ch1val, int ch2val, int ch3val);	// ф-ция со спец-данными
		typedef void (*CB_Type__Uptr_SD)(void* u_ptr, int ch0val, int ch1val, int ch2val, int ch3val);	// ф-ция принимающая пользовательский указатель и спец-данные

	public:

	/// макро-шаблон конструктора 
	#define EEG_Event_Vector_Base_CONSTRUCTOR(C_Type, C_type_Id) \
		EEG_Event_Vector_D_UsualData(C_Type __callback_ptr, void* __user_pointer = 0, bool __once_fire = false):\
            EEG_Event_Vector_GDV(MBN_EEG::EEG_Event::EEG_EVENT_GDE_SUBTYPE__USUAL_CHANNELS_DATA_WITHOUT_STIMUL, (void*)__callback_ptr, C_type_Id, __user_pointer, __once_fire) {}

		// конструкторы
		EEG_Event_Vector_Base_CONSTRUCTOR(EEG_Event_Vector_GDV::CB_Type__Basic,		EEG_Event_Vector_GDV::EEG_EVENT_VECTOR_GDV__CALLB_TYPE__BASIC);
		EEG_Event_Vector_Base_CONSTRUCTOR(EEG_Event_Vector_GDV::CB_Type__Uptr,		EEG_Event_Vector_GDV::EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__SD,		EEG_Event_Vector_GDV::EEG_EVENT_VECTOR_GDV__CALLB_TYPE__SD);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Uptr_SD,		EEG_Event_Vector_GDV::EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR_SD);

	#undef EEG_Event_Vector_Base_CONSTRUCTOR

		/// выполнить вектор
		/// возвращает, был ли запущен вектор
		virtual bool RaiseIfTriggered(const EEG_Event &eeg_event)
		{	
			// если не запускаемся триггером - выходим
			if (!TriggeredBy(eeg_event)) return false;

			//вызываем нужный обработчик
			switch(callback_type_id)
			{
			case EEG_EVENT_VECTOR_GDV__CALLB_TYPE__BASIC:					(*((CB_Type__Basic)					callback_ptr))(); return true;
			case EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR:					(*((CB_Type__Uptr)					callback_ptr))(this->user_pointer); return true;
			case EEG_EVENT_VECTOR_GDV__CALLB_TYPE__SD:						(*((CB_Type__SD)					callback_ptr))(((EEG_Event_GDE_GotUsualData_event*)&eeg_event)->ch0_data,
																															   ((EEG_Event_GDE_GotUsualData_event*)&eeg_event)->ch1_data,
																															   ((EEG_Event_GDE_GotUsualData_event*)&eeg_event)->ch2_data,
																															   ((EEG_Event_GDE_GotUsualData_event*)&eeg_event)->ch3_data); return true;
			case EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR_SD:					(*((CB_Type__Uptr_SD)				callback_ptr))(this->user_pointer,
																															   ((EEG_Event_GDE_GotUsualData_event*)&eeg_event)->ch0_data,
																															   ((EEG_Event_GDE_GotUsualData_event*)&eeg_event)->ch1_data,
																															   ((EEG_Event_GDE_GotUsualData_event*)&eeg_event)->ch2_data,
																															   ((EEG_Event_GDE_GotUsualData_event*)&eeg_event)->ch3_data); return true;
			};

			return false;
		};		
	};

	/// вектор события приёма данных со стимулом
	class EEG_Event_Vector_D_UsualDataWithStimul: public EEG_Event_Vector_GDV
	{
	public:

		typedef void (*CB_Type__SD)(int ch0val, int ch1val, int ch2val, int ch3val);	// ф-ция со спец-данными
		typedef void (*CB_Type__Uptr_SD)(void* u_ptr, int ch0val, int ch1val, int ch2val, int ch3val);	// ф-ция принимающая пользовательский указатель и спец-данные

	public:

	/// макро-шаблон конструктора 
	#define EEG_Event_Vector_Base_CONSTRUCTOR(C_Type, C_type_Id) \
		EEG_Event_Vector_D_UsualDataWithStimul(C_Type __callback_ptr, void* __user_pointer = 0, bool __once_fire = false):\
            EEG_Event_Vector_GDV(MBN_EEG::EEG_Event::EEG_EVENT_GDE_SUBTYPE__USUAL_CHANNELS_DATA_WITH_STIMUL, (void*)__callback_ptr, C_type_Id, __user_pointer, __once_fire) {}

		// конструкторы
		EEG_Event_Vector_Base_CONSTRUCTOR(EEG_Event_Vector_GDV::CB_Type__Basic,		EEG_Event_Vector_GDV::EEG_EVENT_VECTOR_GDV__CALLB_TYPE__BASIC);
		EEG_Event_Vector_Base_CONSTRUCTOR(EEG_Event_Vector_GDV::CB_Type__Uptr,		EEG_Event_Vector_GDV::EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__SD,		EEG_Event_Vector_GDV::EEG_EVENT_VECTOR_GDV__CALLB_TYPE__SD);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Uptr_SD,		EEG_Event_Vector_GDV::EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR_SD);

	#undef EEG_Event_Vector_Base_CONSTRUCTOR

		/// выполнить вектор
		/// возвращает, был ли запущен вектор
		virtual bool RaiseIfTriggered(const EEG_Event &eeg_event)
		{	
			// если не запускаемся триггером - выходим
			if (!TriggeredBy(eeg_event)) return false;

			//вызываем нужный обработчик
			switch(callback_type_id)
			{
			case EEG_EVENT_VECTOR_GDV__CALLB_TYPE__BASIC:					(*((CB_Type__Basic)					callback_ptr))(); return true;
			case EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR:					(*((CB_Type__Uptr)					callback_ptr))(this->user_pointer); return true;
			case EEG_EVENT_VECTOR_GDV__CALLB_TYPE__SD:						(*((CB_Type__SD)					callback_ptr))(((EEG_Event_GDE_GotUsualData_event*)&eeg_event)->ch0_data,
																															   ((EEG_Event_GDE_GotUsualData_event*)&eeg_event)->ch1_data,
																															   ((EEG_Event_GDE_GotUsualData_event*)&eeg_event)->ch2_data,
																															   ((EEG_Event_GDE_GotUsualData_event*)&eeg_event)->ch3_data); return true;
			case EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR_SD:					(*((CB_Type__Uptr_SD)				callback_ptr))(this->user_pointer,
																															   ((EEG_Event_GDE_GotUsualData_event*)&eeg_event)->ch0_data,
																															   ((EEG_Event_GDE_GotUsualData_event*)&eeg_event)->ch1_data,
																															   ((EEG_Event_GDE_GotUsualData_event*)&eeg_event)->ch2_data,
																															   ((EEG_Event_GDE_GotUsualData_event*)&eeg_event)->ch3_data); return true;
			};

			return false;
		};		
	};

	/// вектор события приёма данных со стимулом
	class EEG_Event_Vector_D_ImpedanceMBN: public EEG_Event_Vector_GDV
	{
	public:

		typedef void (*CB_Type__SD)(unsigned char ch_num, const vector<int> *const ch_data);	// ф-ция со спец-данными
		typedef void (*CB_Type__Uptr_SD)(void* u_ptr, unsigned char ch_num, const vector<int> *const ch_data);	// ф-ция принимающая пользовательский указатель и спец-данные

	public:

	/// макро-шаблон конструктора 
	#define EEG_Event_Vector_Base_CONSTRUCTOR(C_Type, C_type_Id) \
		EEG_Event_Vector_D_ImpedanceMBN(C_Type __callback_ptr, void* __user_pointer = 0, bool __once_fire = false):\
            EEG_Event_Vector_GDV(MBN_EEG::EEG_Event::EEG_EVENT_GDE_SUBTYPE__IMPEDANCE_MBN, (void*)__callback_ptr, C_type_Id, __user_pointer, __once_fire) {}

		// конструкторы
		EEG_Event_Vector_Base_CONSTRUCTOR(EEG_Event_Vector_GDV::CB_Type__Basic,		EEG_Event_Vector_GDV::EEG_EVENT_VECTOR_GDV__CALLB_TYPE__BASIC);
		EEG_Event_Vector_Base_CONSTRUCTOR(EEG_Event_Vector_GDV::CB_Type__Uptr,		EEG_Event_Vector_GDV::EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__SD,		EEG_Event_Vector_GDV::EEG_EVENT_VECTOR_GDV__CALLB_TYPE__SD);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Uptr_SD,		EEG_Event_Vector_GDV::EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR_SD);

	#undef EEG_Event_Vector_Base_CONSTRUCTOR

		/// выполнить вектор
		/// возвращает, был ли запущен вектор
		virtual bool RaiseIfTriggered(const EEG_Event &eeg_event)
		{	
			// если не запускаемся триггером - выходим
			if (!TriggeredBy(eeg_event)) return false;

			//вызываем нужный обработчик
			switch(callback_type_id)
			{
			case EEG_EVENT_VECTOR_GDV__CALLB_TYPE__BASIC:					(*((CB_Type__Basic)					callback_ptr))(); return true;
			case EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR:					(*((CB_Type__Uptr)					callback_ptr))(this->user_pointer); return true;
			case EEG_EVENT_VECTOR_GDV__CALLB_TYPE__SD:						(*((CB_Type__SD)					callback_ptr))(((EEG_Event_GDE_GotImpedanceMBNData_event*)&eeg_event)->ch_num,
																															   &(((EEG_Event_GDE_GotImpedanceMBNData_event*)&eeg_event)->ch_data)
																															  ); return true;
			case EEG_EVENT_VECTOR_GDV__CALLB_TYPE__UPTR_SD:					(*((CB_Type__Uptr_SD)				callback_ptr))(this->user_pointer,
																															   ((EEG_Event_GDE_GotImpedanceMBNData_event*)&eeg_event)->ch_num,
																															   &(((EEG_Event_GDE_GotImpedanceMBNData_event*)&eeg_event)->ch_data)
																															  ); return true;
			};

			return false;
		};		
	};



	

	/****************************************************************
	*
	*		Класс базового обработчика получения системного события
	*
	****************************************************************/

	/// общий класс обработчика события получения данных от устройства
	class EEG_Event_Vector_GSEV: public EEG_Event_Vector_Base
	{
	protected:
		// id поддерживаемых обратных вызовов  GSEV
		enum EEG_EVENT_VECTOR_GSEV__CALLB_TYPE
		{
			EEG_EVENT_VECTOR_GSEV__CALLB_TYPE__BASIC, // ф-ция без параметров
			EEG_EVENT_VECTOR_GSEV__CALLB_TYPE__UPTR, // ф-ция принимающая пользовательский указатель
			EEG_EVENT_VECTOR_GSEV__CALLB_TYPE__ESUBTYPE, // ф-ция принимающая подтип события
			EEG_EVENT_VECTOR_GSEV__CALLB_TYPE__UPTR_ESUBTYPE, // ф-ция принимающая пользовательский указатель и подтип события

			// для команды Любого конкретного сообщения (для потомков)
			EEG_EVENT_VECTOR_GSEV__CALLB_TYPE__SD, // ф-ция только со спец-данными
			EEG_EVENT_VECTOR_GSEV__CALLB_TYPE__UPTR_SD, // ф-ция принимающая пользовательский указатель и спец-данные
		};

	public:
		/// типы поддерживаемых вызовов
		typedef void (*CB_Type__Basic)();	// ф-ция без параметров
		typedef void (*CB_Type__Uptr)(void* u_ptr);	// ф-ция принимающая пользовательский указатель
		typedef void (*CB_Type__Esubtype)(EEG_Event::EEG_EVENT_SUBTYPE e_stype);	 // ф-ция принимающая подтип события
		typedef void (*CB_Type__Uptr_Esubtype)(void* u_ptr, EEG_Event::EEG_EVENT_SUBTYPE e_stype);	// ф-ция принимающая пользовательский указатель и подтип события

	protected:

		/// конструктор для потомков
		EEG_Event_Vector_GSEV(														
							EEG_Event::EEG_EVENT_SUBTYPE	vector_subtype,			
							void *callback_ptr,			
							int callback_type_id,
							void *user_ptr = 0,										
							bool __once_fire = false							
						 ):															
		EEG_Event_Vector_Base(														
								EEG_Event::EEG_EVENT_TYPE__SYSTEM_EVENT,	
								vector_subtype,										
								callback_ptr,								
								__once_fire,										
								callback_type_id,									
								user_ptr											
							 )																								
		{};	


	public:

		/// макросовый шаблон конструктора   (__callback_type - тип функции обр выз в С++, __callback_type_id - число - id типа функции обратного вызова)
		#define EEG_Event_Vector_Base_CONSTRUCTOR(__callback_type, __callback_type_id)	\
			EEG_Event_Vector_GSEV(														\
								EEG_Event::EEG_EVENT_SUBTYPE	vector_subtype,			\
								__callback_type callback_ptr,							\
								void *user_ptr = 0,										\
								bool __once_fire = false								\
							 ):															\
			EEG_Event_Vector_Base(														\
									EEG_Event::EEG_EVENT_TYPE__SYSTEM_EVENT,			\
									vector_subtype,										\
									(void*)callback_ptr,								\
									__once_fire,										\
									__callback_type_id,									\
									user_ptr											\
								 )														\
			{}

		// пользовательские конструкторы
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Basic, EEG_EVENT_VECTOR_GSEV__CALLB_TYPE__BASIC);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Uptr, EEG_EVENT_VECTOR_GSEV__CALLB_TYPE__UPTR);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Esubtype, EEG_EVENT_VECTOR_GSEV__CALLB_TYPE__ESUBTYPE);
		EEG_Event_Vector_Base_CONSTRUCTOR(CB_Type__Uptr_Esubtype, EEG_EVENT_VECTOR_GSEV__CALLB_TYPE__UPTR_ESUBTYPE);

		#undef EEG_Event_Vector_Base_CONSTRUCTOR
	};

	






	/***********************************************************
	*
	*			  Класс таблицы обработчиков событий
	*			Клиентский класс - в него клиент добавляет
	*	обработчики событий, который затем отрабатывают
	*	по происходящим событиям.
	*
	***********************************************************/

	/// класс EEG_Callback_Table
	class EEG_Callback_Table
	{
	private:

		/// Мьютекс доступа ко всему объекту
		QMutex accsess_mutex;

		/// список зарегистрированных обработчиков событий
		list<EEG_Event_Vector_Base*> event_vectors_table;

	public:

		/// конструктор
		EEG_Callback_Table(): accsess_mutex()
		{};

		/// Добавить обработчик события в таблицу
		void AddEventVector(EEG_Event_Vector_Base *event_vector)
		{
			assert(event_vector);

			accsess_mutex.lock();
			event_vectors_table.push_back(event_vector);
			accsess_mutex.unlock();
		};

		/// Возвращает, есть ли указанный обработчик в таблице
		bool HasEventVector(EEG_Event_Vector_Base *event_vector)
		{
			bool result = false;

			accsess_mutex.lock();
			for (list<EEG_Event_Vector_Base*>::iterator curr = event_vectors_table.begin(); curr != event_vectors_table.end(); curr++)
				if ((*curr) == event_vector) result = true;
			accsess_mutex.unlock();

			return result;
		};

		/// Убрать обработчик из таблицы (либо не находит обработчик и возвразает false, либо находит, удаляет его и возвращает true)
		/// Возвращает, был ли найден указанный обработчик
		bool RemoveEventVector(EEG_Event_Vector_Base *event_vector)
		{
			assert(event_vector);

			accsess_mutex.lock();
			for (list<EEG_Event_Vector_Base*>::iterator curr = event_vectors_table.begin(); curr != event_vectors_table.end(); curr++)
				if ((*curr) == event_vector) 
				{
					event_vectors_table.erase(curr);
					accsess_mutex.unlock();
					return true;
				};					
			accsess_mutex.unlock();

			return false;
		};

		/// возвращает указатель на вектор-обработчик события, или 0, если индекс был вне границ массива
		const EEG_Event_Vector_Base* operator[] (const int index)
		{
			assert(index >= 0);

			accsess_mutex.lock();

			// если индекс находится вне границ таблицы
			if (index >= event_vectors_table.size()) 			
			{
				accsess_mutex.unlock();
				return 0;
			};

			// иначе находим нужную запись и возвращаем её указатель
			list<EEG_Event_Vector_Base*>::iterator curr = event_vectors_table.begin();
			int idx = 0; 
			for ( ; idx < index; curr++) { idx++; };
			EEG_Event_Vector_Base* result = (*curr);

			accsess_mutex.unlock();

			return result;
		};

		/// Возвращает количество векторов в таблице векторов
		int GetEventVectorsTableLength()
		{
			int result = 0;

			accsess_mutex.lock();

			// если индекс находится вне границ таблицы
			result = event_vectors_table.size();		
			
			accsess_mutex.unlock();

			return result;
		};

		// обработать событие - т.е. запустить последовательно все обработчики, которые реагируют на это событие
		bool ProcessEvent(const EEG_Event &ev)
		{
			bool result = false;

			accsess_mutex.lock();
			
			// бежим по таблице и запускаем те обработчики, которые запускаются на это событие
			for (list<EEG_Event_Vector_Base*>::iterator curr = event_vectors_table.begin(); curr != event_vectors_table.end(); )
			{
				list<EEG_Event_Vector_Base*>::iterator next = curr;
				next++;

				if ((*curr)->RaiseIfTriggered(ev))
					// если обработчик разовый - удаляем его
					if ((*curr)->once)
					{
						event_vectors_table.erase(curr);
						curr = next;
						continue;
					};

				curr++;
			};

			accsess_mutex.unlock();

			return result;
		};

		/// очищает таблицу обработчиков
		void Clear()
		{
			accsess_mutex.lock();
			event_vectors_table.clear();
			accsess_mutex.unlock();
		};
	};


	/***********************************************************
	*
	*			 Активный класс обработчика событий
	*
	***********************************************************/
	/// Потокобезопасен
	class EEG_EventProcessor: public EEG_Event_Queue, public EEG_Callback_Table, private QThread
	{
	private:
		/// Мьютекс доступа
		QMutex accsess_mutex;

		/// флаг останова
		bool terminateFlag;

		/// время сна в мс.
		static const int sleep_time = 20;

		/// Поток выполнения обработки сообщений
		virtual void run()
		{
			accsess_mutex.lock();
			terminateFlag = false;
			accsess_mutex.unlock();
			
			while (!terminateFlag)
			{
				EEG_Event *next_event;

				// обрабатываем все события, которые есть в очереди
				while (next_event = GetAndDispatchEvent()) // получаем событие из очереди событий		
				{
					ProcessEvent(*next_event); // обрабатываем событие по всем векторам, которые есть в таблице
					
					// высвобождаем выделенное под сообщение место
					delete next_event;
					next_event = 0;
				};				
				
				//спим, сколько можно
				msleep(sleep_time);
			};
		};

	public:

		/// конструктор по-умолчанию
		EEG_EventProcessor()
		{
			terminateFlag = false;
		};

        /// деструктор по-умолчанию
        ~EEG_EventProcessor()
        {
            StopEventThread();
        };

		/// Выставляет флаг останова потоку выполнения
		void StopEventThread()
		{            
            std::cout<<"EEG_EventProcessor::StopEventThread:::: <BEFORE stop request>  managed thread running = "<<this->isRunning()<<std::endl;

			accsess_mutex.lock();
			terminateFlag = true;
			accsess_mutex.unlock();

			this->wait();

            std::cout<<"EEG_EventProcessor::StopEventThread:::: <AFTER stop request>  managed thread running = "<<this->isRunning()<<std::endl;
		};

		/// Запускает поток выполнения
		void StartEventThread()
		{
			this->start();
		};

		/// Показывает, был ли запущен поток обработчика событий
		bool IsActive()
		{
			bool result;

			accsess_mutex.lock();
			result = !terminateFlag;
			accsess_mutex.unlock();

			return result;
		};

	};


};

#endif
