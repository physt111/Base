#ifndef MBN_EEG_CLASS__
#define MBN_EEG_CLASS__

#include <Common_Communication_Channel.h>
#include <Basic_CommunicationChannel_Base.h>
#include <Basic_CommunicationChannel.h>
#include <EEG_Data_Storage.h>
#include <EEG_Device_States_Storage.h>
#include <EEG_Package.h>
#include <EEG_device_architecture.h>
#include <EEG_Protocol.h>
#include <qsemaphore.h>
#include <qthread.h>
#include <abstractserial.h>
#include <QMutex>
#include <QWaitCondition>
#include <EEG_Callback_Table.h>
#include "EEG_program_emulator.h"


namespace MBN_EEG
{
	// указатель на элементарный низкоуровневый канал связи
	class BaseCommChannelPtr
	{
	public:
		BaseCommCh * low_level_channel_ptr;

		BaseCommChannelPtr(BaseCommCh * ch_ptr)
		{
			low_level_channel_ptr = ch_ptr;
		};
	};

	/// Класс, полностью энкапсулирующий устройство EEG_IT
	class EEG:	public DeviceDescriptor, 
				public MBN_EEG::EEG_Data_Storage,
				public MBN_EEG::EEG_Device_States_Storage, 
				private BaseCommChannelPtr,
				public MBN_EEG::EEG_Protocol,
				public Common_Communication_Channel
	{
	private:
		/***********************************************************
		*
		*				Семафор доступа к классу
		*
		***********************************************************/
		QSemaphore accsess_semaphore;


		/***********************************************************
		*
		*				Внутренние компоненты
		*
		***********************************************************/

		/// Процессор очереди событий
		EEG_EventProcessor eventProcessor;



		#pragma region Флаги состояния

	private:
		/***********************************************************
		*
		*					Флаги состояния
		*
		***********************************************************/

		/// Подключён к устройству
		bool Connected;

		/// Включена генерация сообщений и асинхронный вызов пользовательских callback-функций
		bool EventsEnabled;

		#pragma endregion

		#pragma region Констанстные параметры

		/***********************************************************
		*
		*					Констанстные параметры
		*
		***********************************************************/

		/// параметры управления памяти активного канала (размеры указаны для байтов - в байтах, для посылок - в посылках)
		static const int ACTIVE_CHANNEL_IN_BYTES_QUEUE_PAGE_SIZE = 1024; //65536;
		static const int ACTIVE_CHANNEL_OUT_BYTES_QUEUE_PAGE_SIZE = 1024;
		static const int ACTIVE_CHANNEL_IN_PACKS_QUEUE_PAGE_SIZE = 1024; //8192;
		static const int ACTIVE_CHANNEL_OUT_PACKS_QUEUE_PAGE_SIZE = 128;

		static const int ACTIVE_CHANNEL_IN_BYTES_RESERVED_SIZE = 1024; //65536;
		static const int ACTIVE_CHANNEL_OUT_BYTES_RESERVED_SIZE = 1024;
		static const int ACTIVE_CHANNEL_IN_PACKS_RESERVED_SIZE = 1024; //8192;
		static const int ACTIVE_CHANNEL_OUT_PACKS_RESERVED_SIZE = 128;	

		static const int ACTIVE_CHANNEL_IN_BYTES_HEADER_STEP = 1024;
		static const int ACTIVE_CHANNEL_OUT_BYTES_HEADER_STEP = 1024;
		static const int ACTIVE_CHANNEL_IN_PACKS_HEADER_STEP = 1024;
		static const int ACTIVE_CHANNEL_OUT_PACKS_HEADER_STEP = 1024;
								

		#pragma endregion

		#pragma region Блок функций добавления сообщений в очередь на основании пришедшей посылки

		/********************************************************************
		*
		*			функций добавления сообщений в очередь на 
		*				основании пришедшей посылки
		*
		********************************************************************/

		/// Создаёт соответствующее переданной посылке (и остальным параметрам) событие получения результата по команде
		/// и помещает его в очередь сообщений на обработку
        void AddAckedCommandEventToEventQueue(long long original_command_pack_Id, CommonPackage &CommandAnswerPkg)
		{
			switch(CommandAnswerPkg.subtype)
			{
			case EEG_Protocol::A_ERROR:  eventProcessor.AddEvent (new MBN_EEG::EEG_Event_GCRE_Error_event( original_command_pack_Id, true, 0 )); break;
			case EEG_Protocol::A_GET_FREQUENCY_TABLE: 
				eventProcessor.AddEvent (	new MBN_EEG::EEG_Event_GCRE_GetFrequenciesTable_event 
												(
													original_command_pack_Id, 
													true, 
													&(MBN_EEG::EEG_C_GetFrequencyTable::A_GetFrequenciesList<vector<unsigned char> >(CommandAnswerPkg, CommandAnswerPkg.direct_data))
												)
										);  break;
			case EEG_Protocol::A_GET_PROTOCOL_VERSION: 
				eventProcessor.AddEvent (	new MBN_EEG::EEG_Event_GCRE_GetProtocolVersion_event
												(
													original_command_pack_Id, 
													true, 
													MBN_EEG::EEG_C_GetProtocolVersion::GetProtocolVersion<vector<unsigned char> >(CommandAnswerPkg, CommandAnswerPkg.direct_data)
												)
										);  break;
			case EEG_Protocol::A_GET_VERSION: 
				eventProcessor.AddEvent (	new MBN_EEG::EEG_Event_GCRE_GetVersion_event
												(
													original_command_pack_Id, 
													true, 
													MBN_EEG::EEG_C_GetVersion::GetFirmwareVersion<vector<unsigned char> >(CommandAnswerPkg, CommandAnswerPkg.direct_data),
													MBN_EEG::EEG_C_GetVersion::GetCircuityVersion<vector<unsigned char> >(CommandAnswerPkg, CommandAnswerPkg.direct_data),
													MBN_EEG::EEG_C_GetVersion::GetSerialNumber<vector<unsigned char> >(CommandAnswerPkg, CommandAnswerPkg.direct_data)
												)
										);  break;
			case EEG_Protocol::A_RESET: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_Reset_event(original_command_pack_Id, true));  break;
			case EEG_Protocol::A_SET_ADS_MUX: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_SetADSMux_event(original_command_pack_Id, true));  break;
			case EEG_Protocol::A_SET_AMPLIFICATION: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_SetAmplification_event(original_command_pack_Id, true));  break;
			case EEG_Protocol::A_SET_CALIBRATION_SHAPE: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_SetCalibrationShape_event(original_command_pack_Id, true));  break;
			case EEG_Protocol::A_SET_CHANNELS_MODE: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_SetChannelsMode_event(original_command_pack_Id, true));  break;
			case EEG_Protocol::A_SET_CHANNEL_LEDS: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_SetChannelLed_event(original_command_pack_Id, true));  break;
			case EEG_Protocol::A_SET_MUX: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_SetMUX_event(original_command_pack_Id, true));  break;
			case EEG_Protocol::A_SET_SAMPLING_FREQUENCY: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_SetSamplingFrequency_event(original_command_pack_Id, true));  break;
			case EEG_Protocol::A_START_CALIBRATION: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_StartCalibration_event(original_command_pack_Id, true));  break;
			case EEG_Protocol::A_START_TRANSLATION: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_StartTranslation_event(original_command_pack_Id, true));  break;
			case EEG_Protocol::A_STOP_CALIBRATION: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_StopCalibration_event(original_command_pack_Id, true));  break;
			case EEG_Protocol::A_STOP_TRANSLATION: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_StopTranslation_event(original_command_pack_Id, true));  break;
			};
		};

		/// Создаёт соответствующее переданной посылке (и остальным параметрам) событие получения результата по команде
		/// и помещает его в очередь сообщений на обработку
        void AddTimeoutedCommandEventToEventQueue(long long original_command_pack_Id, int type, int subtype)
		{
			switch(subtype)
			{
			case EEG_Protocol::A_ERROR:  eventProcessor.AddEvent (new MBN_EEG::EEG_Event_GCRE_Error_event( original_command_pack_Id, false, 0 )); break;
			case EEG_Protocol::A_GET_FREQUENCY_TABLE: 
				eventProcessor.AddEvent ( new MBN_EEG::EEG_Event_GCRE_GetFrequenciesTable_event(original_command_pack_Id, false, 0) );  break;
			case EEG_Protocol::A_GET_PROTOCOL_VERSION: 
				eventProcessor.AddEvent ( new MBN_EEG::EEG_Event_GCRE_GetProtocolVersion_event(original_command_pack_Id, false, 0) );  break;
			case EEG_Protocol::A_GET_VERSION: 
				eventProcessor.AddEvent ( new MBN_EEG::EEG_Event_GCRE_GetVersion_event(original_command_pack_Id, false, 0,0,0) );  break;
			case EEG_Protocol::A_RESET: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_Reset_event(original_command_pack_Id, false));  break;
			case EEG_Protocol::A_SET_ADS_MUX: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_SetADSMux_event(original_command_pack_Id, false));  break;
			case EEG_Protocol::A_SET_AMPLIFICATION: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_SetAmplification_event(original_command_pack_Id, false));  break;
			case EEG_Protocol::A_SET_CALIBRATION_SHAPE: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_SetCalibrationShape_event(original_command_pack_Id, false));  break;
			case EEG_Protocol::A_SET_CHANNELS_MODE: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_SetChannelsMode_event(original_command_pack_Id, false));  break;
			case EEG_Protocol::A_SET_CHANNEL_LEDS: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_SetChannelLed_event(original_command_pack_Id, false));  break;
			case EEG_Protocol::A_SET_MUX: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_SetMUX_event(original_command_pack_Id, false));  break;
			case EEG_Protocol::A_SET_SAMPLING_FREQUENCY: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_SetSamplingFrequency_event(original_command_pack_Id, false));  break;
			case EEG_Protocol::A_START_CALIBRATION: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_StartCalibration_event(original_command_pack_Id, false));  break;
			case EEG_Protocol::A_START_TRANSLATION: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_StartTranslation_event(original_command_pack_Id, false));  break;
			case EEG_Protocol::A_STOP_CALIBRATION: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_StopCalibration_event(original_command_pack_Id, false));  break;
			case EEG_Protocol::A_STOP_TRANSLATION: eventProcessor.AddEvent(new MBN_EEG::EEG_Event_GCRE_StopTranslation_event(original_command_pack_Id, false));  break;
			};
		};

		/// Создаёт соответствующее переданной посылке (и остальным параметрам) событие получения данных
		/// и помещает его в очередь сообщений на обработку
		void AddDataEventToEventQueue(CommonPackage &DataPkg)
		{
			switch(DataPkg.subtype)
			{
			case EEG_Protocol::D_USUAL_CHANNELS_DATA_WITHOUT_STIMUL: 
				{
					int ch0val;
					int ch1val;
					int ch2val;
					int ch3val;
					MBN_EEG::EEG_D_UsualData::GetChannelsData<vector<unsigned char> >(DataPkg, DataPkg.direct_data, ch0val, ch1val, ch2val, ch3val);

					eventProcessor.AddEvent ( new MBN_EEG::EEG_Event_GDE_GotUsualData_event(ch0val, ch1val, ch2val, ch3val) );  
					break;
				}
			case EEG_Protocol::D_USUAL_CHANNELS_DATA_WITH_STIMUL: 
				{
					int ch0val;
					int ch1val;
					int ch2val;
					int ch3val;
					MBN_EEG::EEG_D_UsualData_With_Stimul::GetChannelsData<vector<unsigned char> >(DataPkg, DataPkg.direct_data, ch0val, ch1val, ch2val, ch3val);

					eventProcessor.AddEvent ( new MBN_EEG::EEG_Event_GDE_GotUsualDataWithStimul_event(ch0val, ch1val, ch2val, ch3val) );  
					break;
				}
			case EEG_Protocol::D_IMPEDANCE_MBN: 
				{
					// получаем данные посылки
					int ch_num = MBN_EEG::EEG_D_ImpedanceMBN::GetChannelNumber<vector<unsigned char> >(DataPkg, DataPkg.direct_data);
						// отсчёты
					vector<int> ch_vals;
							// изменяем размер массива под полученное число отсчётов
					ch_vals.resize(MBN_EEG::EEG_D_ImpedanceMBN::GetSamplesCount(DataPkg));

					MBN_EEG::EEG_D_ImpedanceMBN::GetChannelData<vector<unsigned char> , vector<int> > (DataPkg, DataPkg.direct_data, ch_vals, 0);
					// добавляем сообшение в очередь
					eventProcessor.AddEvent ( new MBN_EEG::EEG_Event_GDE_GotImpedanceMBNData_event(ch_num, ch_vals) );  
					break;
				}
			};
		};

		/// Создаёт соответствующее переданной посылке (и остальным параметрам) событие получения сообщения
		/// и помещает его в очередь сообщений на обработку
		void AddMessageEventToEventQueue(CommonPackage &MsgPkg)
		{
			switch(MsgPkg.subtype)
			{
			case EEG_Protocol::M_DEVICE_READY: eventProcessor.AddEvent ( new MBN_EEG::EEG_Event_GME_DeviceReady_event() );  break;
			};
		};

		#pragma endregion

		#pragma region блок Callback функций, вызываемых из потока активного канала связи

		/********************************************************************
		*
		*						Callback-функции, 
		*	 вызываемые из потока ввода-вывода активного канала связи
		*
		********************************************************************/

		/// функция вызывается из потока обработки входных и выходных данных
		/// как только получено подтверждение о выполнении команды устройством
        static void CommandAcknowledgeCallback(long long acked_pack_Id, CommonPackage &AnswerPkg, void* parameter_obj)
		{			
			// указатель на объект EEG должен быть не 0
			assert(parameter_obj != 0);

			EEG *this_ptr = (EEG*)parameter_obj;

			// ждём монопольного доступа к объекту
			this_ptr->accsess_semaphore.acquire();

				//// получаем индекс в таблице нашей записи
				//int rec_idx = this_ptr->SemaphoreIndexOfID(acked_pack_Id);
				//
				//// если команда была отправлена в асинхронном режиме, тогда её никто не ждёт - и соответственно, семафор найден не будет
				//// ну, и, соответственно, освобождать некого
				//if (rec_idx == -1)
				//{
				//	this_ptr->accsess_semaphore.release();
				//	return;
				//};				

				//// записываем результат выполнения команды
				//this_ptr->semaphore_table[rec_idx].result = true;

				//// записываем посылку ответа со встроенными данными
				//this_ptr->semaphore_table[rec_idx].AnswerPkg = AnswerPkg;

				//// освобождаем семафор ожидания команды
				//this_ptr->semaphore_table[rec_idx].semaphore->release();			

				//// если включена система событий, ставим соответствующее сообщение в очередь сообщений
				//if (this_ptr->EventsEnabled)
				//	this_ptr->AddAckedCommandEventToEventQueue(acked_pack_Id, AnswerPkg);


				// получаем индекс в таблице нашей записи
				int rec_idx = this_ptr->SemaphoreIndexOfID(acked_pack_Id);
				
				// если команда была отправлена в асинхронном режиме, тогда её никто не ждёт - и соответственно, семафор найден не будет
				// ну, и, соответственно, освобождать некого
				// а если в синхронном - то команду ждет семафор - и мы его освободим!
				if (rec_idx >= 0)
				{
					// записываем результат выполнения команды
					this_ptr->semaphore_table[rec_idx].result = true;

					// записываем посылку ответа со встроенными данными
					this_ptr->semaphore_table[rec_idx].AnswerPkg = AnswerPkg;

					// освобождаем семафор ожидания команды
					this_ptr->semaphore_table[rec_idx].semaphore->release();
				};				
						
				// если включена система событий, ставим соответствующее сообщение в очередь сообщений
				if (this_ptr->EventsEnabled)
					this_ptr->AddAckedCommandEventToEventQueue(acked_pack_Id, AnswerPkg);

			// освобождаем семафор доступа к объекту
			this_ptr->accsess_semaphore.release();
		};

		/// функция вызывается из потока обработки входных и выходных данных
		/// как только принято решение о таймауте выполнения команды
        static void CommandAcknowledgeTimeoutCallback(long long timeout_pack_Id, int type, int subtype, void* parameter_obj)
		{
			// указатель на объект EEG должен быть не 0
			assert(parameter_obj != 0);

			EEG *this_ptr = (EEG*)parameter_obj;

			// ждём монопольного доступа к объекту
			this_ptr->accsess_semaphore.acquire();

				// получаем индекс в таблице нашей записи
				int rec_idx = this_ptr->SemaphoreIndexOfID(timeout_pack_Id);

				// если команда была отправлена в асинхронном режиме, тогда её никто не ждёт - и соответственно, семафор найден не будет
				// ну, и, соответственно, освобождать некого
				if (rec_idx == -1)
				{
					this_ptr->accsess_semaphore.release();
					return;
				};				

				// записываем результат выполнения команды
				this_ptr->semaphore_table[rec_idx].result = false;

				// освобождаем семафор ожидания команды
				this_ptr->semaphore_table[rec_idx].semaphore->release(); 

				// если включена система событий, ставим соответствующее сообщение в очередь сообщений
				if (this_ptr->EventsEnabled)
					this_ptr->AddTimeoutedCommandEventToEventQueue(timeout_pack_Id, type, subtype);

			// освобождаем семафор доступа к объекту
			this_ptr->accsess_semaphore.release();
		};

		/// функция вызывается из потока обработки входных и выходных данных
		/// в данной версии функция нужна только для асинхронной работы класса, 
		/// поэтому подключается только при включении асинронного режима
		/// как только получены данные с устройства
		static void DataCallback(CommonPackage &DataPkg, void* parameter_obj_ptr)
		{
			// указатель на объект EEG должен быть не 0
			assert(parameter_obj_ptr != 0);

			EEG *this_ptr = (EEG*)parameter_obj_ptr;

			// ждём монопольного доступа к объекту
			this_ptr->accsess_semaphore.acquire();

				// если включена система событий, ставим соответствующее сообщение в очередь сообщений
				if (this_ptr->EventsEnabled)
					this_ptr->AddDataEventToEventQueue(DataPkg);

			// освобождаем семафор доступа к объекту
			this_ptr->accsess_semaphore.release();
		};

		/// функция вызывается из потока обработки входных и выходных данных
		/// в данной версии функция нужна только для асинхронной работы класса, 
		/// поэтому подключается только при включении асинронного режима
		/// как только получено сообщение от устройства
		static void MessageCallback(CommonPackage &MessagePkg, void* parameter_obj_ptr)
		{
			// указатель на объект EEG должен быть не 0
			assert(parameter_obj_ptr != 0);

			EEG *this_ptr = (EEG*)parameter_obj_ptr;

			// ждём монопольного доступа к объекту
			this_ptr->accsess_semaphore.acquire();

				// если включена система событий, ставим соответствующее сообщение в очередь сообщений
				if (this_ptr->EventsEnabled)
					this_ptr->AddMessageEventToEventQueue(MessagePkg);

			// освобождаем семафор доступа к объекту
			this_ptr->accsess_semaphore.release();
		};

		/// функция вызывается из потока обработки входных и выходных данных
		/// в данной версии функция нужна только для асинхронной работы класса, 
		/// поэтому подключается только при включении асинронного режима
		/// как только обнаружена ошибка протокола или более высокого уровня
		static void ErrorCallback(simple_error, void* parameter_obj_ptr)
		{
			//// указатель на объект EEG должен быть не 0
			//assert(parameter_obj_ptr != 0);

			//EEG *this_ptr = (EEG*)parameter_obj_ptr;


		};

		#pragma endregion

		#pragma region Таблица семафоров ожидания ответа устройства (и сопряжёные функции и классы)
		/***********************************************************
		*
		*	Таблица семафоров ожидания функциями синхронных команд
		*			выполнения команд устройством
		*
		***********************************************************/

		/// запись в таблице семафоров ожидания синхронных команд
		struct SemaphoreRec
		{
			/// ID команды на которую ждём ответ
            long long id;
			/// указатель на семафор, которого ждём после отправки команды
			QSemaphore  *semaphore;

			//QWaitCondition*  got_reply; 


			/// результат выполнения команды. True - успешно, False - не удалось.
			bool result;
			/// посылка ответа на команду с включёнными в неё данными
			CommonPackage AnswerPkg;
		};

		/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		///			ВНИМАНИЕ! УКАЗАТЕЛИ НА СЕМАФОРЫ 
		///		УПРАВЛЯЮТСЯ ВРУЧНУЮ !!!!! new и delete
		/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		/// таблица семафоров ожидания синхронных команд
		vector<SemaphoreRec>  semaphore_table;

		/// поиск по таблице семафоров, возвращает индекс записи с id == ID, или -1, если не удалось найти
        int SemaphoreIndexOfID(long long ID)
		{
			// массив пуст - возвращаемся
			if (!semaphore_table.size()) return -1;

			// используем, что массив по определению сортирован по ID
			int LO = 0;
			int HI = semaphore_table.size() - 1;

			if (semaphore_table[LO].id == ID) return LO;
			if (semaphore_table[HI].id == ID) return HI;

			// Бинарный поиск
			while (LO != HI)
			{
				int MID = (LO + HI)/2;

				if (semaphore_table[MID].id == ID) return MID;
				else if (semaphore_table[MID].id < ID) LO = MID;
				else HI = MID;
			};

			return LO;
		};

		#pragma endregion

	public:

		/***********************************************************
		*
		*			Структуры ответных данных устройства
		*
		***********************************************************/

		/// ответ на команду GetVersion
		struct GetVersionReply
		{
			unsigned char FirmwareVersion;
			unsigned char CircuityVersion;
			unsigned short SerialNumber;
		};


	public:

		#pragma region  Конструктор

		// вспомогательная функция для создания, подключения и настройки нужного низкоуровневого канала
		BaseCommCh * __create_low_level_channel(DeviceDescriptor &device)
		{
			if (device.GetType() == DeviceDescriptor::COM)
			{
				// создаём соответствующий дескриптору устройства низкоуровневый канал
				BaseCommCh * _comm_channel = new COM_CommCh();

				// инициализирует низкоуровневый канал (открывает порт ввода-вывода)
				((COM_CommCh*)_comm_channel)->Init(device);
				
				// настраиваем COM-порт
				((COM_CommCh*)_comm_channel)->SetCOM_params(::AbstractSerial::BaudRate921600, ::AbstractSerial::AllBaud,
							::AbstractSerial::ParityNone, 
							::AbstractSerial::DataBits8, 
							::AbstractSerial::StopBits1,
							::AbstractSerial::FlowControlOff);

				// продуваем буфера порта
				((COM_CommCh*)_comm_channel)->flush();

				return _comm_channel;
			}
			else if (device.GetType() == DeviceDescriptor::VIRTUAL)
			{
				// создаём соответствующий дескриптору устройства низкоуровневый канал
				BaseCommCh * _comm_channel = new EEG_Emulator();

				return _comm_channel;
			}
				return 0;
		};


		/*******************************************************************
		*
		*						Конструктор
		*	Создаёт хранилище данных, хранилище состояний устройства,
		*	протокол связи, активный канал связи, запускает последний
		*	и подключается к устройству, указанному конструктору
		*
		*******************************************************************/
		EEG(DeviceDescriptor device):	Connected(false),
										accsess_semaphore(1),	// инициализируем семафор доступа к объекту
										DeviceDescriptor(device), // копируем себе данные дескриптора устройства к которому подключились
										MBN_EEG::EEG_Protocol	// инициализируем протокол и подвязываем его к нему хранилища и callback-и
										(
											((MBN_EEG::EEG_Data_Storage*)this),  
											((MBN_EEG::EEG_Device_States_Storage*)this),
											true,
											&ErrorCallback, 
											&CommandAcknowledgeCallback, 
											&CommandAcknowledgeTimeoutCallback,
											&DataCallback,
											&MessageCallback,
											this,
											this,
											this,
											this,
											this
										),
										BaseCommChannelPtr(__create_low_level_channel(device)),	// создаём низкоуровневый канал связи в зависимости от типа устройства
										Common_Communication_Channel	// создаём активный канал связи, подвязываем к нему экземпляр протокола и низкоуровневый канал
										(
											this->low_level_channel_ptr,
											(MBN_EEG::EEG_Protocol*)this,
											ACTIVE_CHANNEL_IN_BYTES_QUEUE_PAGE_SIZE,
											ACTIVE_CHANNEL_OUT_BYTES_QUEUE_PAGE_SIZE,
											ACTIVE_CHANNEL_IN_PACKS_QUEUE_PAGE_SIZE,
											ACTIVE_CHANNEL_OUT_PACKS_QUEUE_PAGE_SIZE,
											ACTIVE_CHANNEL_IN_BYTES_RESERVED_SIZE,
											ACTIVE_CHANNEL_OUT_BYTES_RESERVED_SIZE,
											ACTIVE_CHANNEL_IN_PACKS_RESERVED_SIZE,
											ACTIVE_CHANNEL_OUT_PACKS_RESERVED_SIZE,
											ACTIVE_CHANNEL_IN_BYTES_HEADER_STEP,
											ACTIVE_CHANNEL_OUT_BYTES_HEADER_STEP,
											ACTIVE_CHANNEL_IN_PACKS_HEADER_STEP,
											ACTIVE_CHANNEL_OUT_PACKS_HEADER_STEP
										)
		{
			// управляющие флаги состояния
			EventsEnabled = false;
			MBN_EEG::EEG_Protocol::internalDataStorageEnabled = false;
			MBN_EEG::EEG_Protocol::internalDeviceStatesStorageEnabled = false;

			// запускаем активный канал
            std::cout<<"StartChannel pre";
            ((Common_Communication_Channel *)this)->StartChannel();
            //(static_cast<Common_Communication_Channel*> (this))->StartChannel();

			// обновляем флаг состояния
			Connected = true;
		};

		#pragma endregion 

		
		#pragma region  Деструктор

		~EEG()
		{
            std::cout<<"Entering the destructor of EEG"<<std::endl;



            // отключить цикл обработки событий
            this->DisableEvents();

            // для монопольного доступа к объекту
            accsess_semaphore.acquire();

			// высвобождаем память под таблицу семафоров
			for (int i = 0; i < semaphore_table.size(); i++)
				if (semaphore_table[i].semaphore) delete semaphore_table[i].semaphore;

            accsess_semaphore.release();

            // вручную остановим канал
            Common_Communication_Channel::StopChannel();

            // высвобождает выделенные вручную ресурсы
            if (this->low_level_channel_ptr)
                delete this->low_level_channel_ptr;
		};


		#pragma endregion 

private: 
		
		/*********************************************************************
		*
		*			Общая функция приёма-получения синхронных команд
		*
		*********************************************************************/

		// Отправляет посылку команды устройству и затем блокирует поток до получения подтверждения выполнения команды
		// либо до принятия решения о таймауте выполнения, затем при таймауте - просто возвращает false,
		// а при получении ответа на команду возвращает true и записывает в answer_pkg ответную посылку.
		// command_pkg - командная посылка со встроенными данными,
		// answer_pkg - ответная посылка со встроенными данными, которую заполняет функция, если пришёл ответ на команду.
		// Возвращает успешность выполнения команды (успешность = удачно отправили и устройство корректно ответило).
		inline bool CS_DoPostAndWait(const CommonPackage &command_pkg, CommonPackage &answer_pkg)
		{
			// ждём монопольного доступа к объекту
			accsess_semaphore.acquire();
				
				// отправляем команду
                long long comm_pkg_id = PostPackage(command_pkg, command_pkg.direct_data);

				// если отправка не удалась, освобождаем объект и выходим
				if (!comm_pkg_id)
				{
					accsess_semaphore.release();
					return false;
				};

				// запись для таблицы семафоров. (!) СЕМАФОР СОЗДАЁТСЯ В КУЧЕ ВРУЧНУЮ (!)
				SemaphoreRec rec;
				rec.id = comm_pkg_id;
				rec.semaphore = new QSemaphore(1);
                // сами лочим семафор1
                rec.semaphore->acquire();
				
				/// запись втыкаем в таблицу
				semaphore_table.push_back(rec);
			
			// освобождаем семафор доступа к объекту
			accsess_semaphore.release();

			// ждём освобождения семафора из таблицы семафоров - он должен быть освобождён 
			// потоком приёма-передачи по таймауту подтверждения или по получении подтверждения 
			// данной посылки
            rec.semaphore->acquire();
			rec.semaphore->release();

			// тут мы уже дождались результата запроса

			// снова ждём монопольного доступа к объекту
			accsess_semaphore.acquire();

				// получаем индекс в таблице нашей записи (важно - так как пока мы ждали несколькими строчками выже
				// содерджание таблицы семафоров могло быть обновлено другими потоками
				int rec_idx = SemaphoreIndexOfID(comm_pkg_id);
				assert(rec_idx >= 0);

				// получаем результат выполнения команды
				bool result = semaphore_table[rec_idx].result;

				// при хорошем ответе на команду копируем ответную посылку вызывающему
				if (result)
					answer_pkg = semaphore_table[rec_idx].AnswerPkg;

				// удаляем вручную семафор из таблицы
				delete semaphore_table[rec_idx].semaphore;
				semaphore_table[rec_idx].semaphore = 0;
				
				// удаляем саму запись из таблицы семафоров
				semaphore_table.erase(semaphore_table.begin() + SemaphoreIndexOfID(rec.id));

			// снова освобождаем семафор доступа к объекту
			accsess_semaphore.release();

			return result;
		};

public:

		#pragma region Функции получения флагов состояния

		/***********************************************************
		*
		*			Функции получения флагов состояния
		*
		***********************************************************/

		// подключились ли мы к устройству
		// если true - активный канал работает и подключён к устройству
		inline bool IsConnected()
		{
			return Connected;
		};

		#pragma endregion

		#pragma region Блок команд прибору (синхронных и асинхронных)

		/************************************************************************
		*
		*				Блок команд прибору (синхронных и асинхронных)
		*
		************************************************************************/

		// Синхронная отправка команды перезагрузки микроконтроллера устройства. Блокирует поток до получения подтверждения выполнения команды
		// либо до принятия решения о таймауте выполнения.
		// Возвращает успешность выполнения операции команды.
		bool CS_Reset()
		{
			/// создаём посылки
			CommonPackage command_pkg;
			CommonPackage answer_pkg;

            long long offset = 0;
            command_pkg.direct_data.resize(MBN_EEG::EEG_C_Reset::C_GetSize());
            MBN_EEG::EEG_C_Reset::CreatePackage<vector<unsigned char> >(command_pkg, command_pkg.direct_data, offset);

			// отправляем команду устройству и ждём результатов и получаем результаты
			if (!CS_DoPostAndWait(command_pkg, answer_pkg))
				return false;
			else // тут команда отправилясь и получен корректный ответ на неё
				return true;
		};

		// Acинхронная отправка команды перезагрузки. 
		// Просто отправляет устройству команду Reset
		// Возвращает id отправленной посылки, если id == 0, значит отправка не удалась
		bool CAS_Reset()
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
            vector<unsigned char> bytes(MBN_EEG::EEG_C_Reset::C_GetSize());
            MBN_EEG::EEG_C_Reset::CreatePackage<vector<unsigned char> >(pkg, bytes, offset);
				
            long long pkg_id = PostPackage(pkg, bytes);

			// если отправка не удалась, освобождаем объект и выходим
			if (!pkg_id) return false;
			else return true;
		};


		// Синхронная отправка команды получения версии и серийного номера. Блокирует поток до получения подтверждения выполнения команды
		// либо до принятия решения о таймауте выполнения.
		// Если получает reply указатель, то заполняет соответствующую структуру.
		// Возвращает успешность выполнения операции команды.
		bool CS_GetVersion(GetVersionReply *reply = 0)
		{
			/// создаём посылки
			CommonPackage command_pkg;
			CommonPackage answer_pkg;

            long long offset = 0;
			command_pkg.direct_data.resize(MBN_EEG::EEG_C_GetVersion::C_GetSize()); 
            MBN_EEG::EEG_C_GetVersion::CreatePackage<vector<unsigned char> >(command_pkg, command_pkg.direct_data, offset);

			// отправляем команду устройству и ждём результатов и получаем результаты
			if (!CS_DoPostAndWait(command_pkg, answer_pkg))
				return false;
			else // тут команда отправилясь и получен корректный ответ на неё
			{
				// если надо - выводим данные
				if (reply)
				{
					reply->CircuityVersion = MBN_EEG::EEG_C_GetVersion::GetCircuityVersion< vector <unsigned char> >( answer_pkg, answer_pkg.direct_data);
					reply->FirmwareVersion = MBN_EEG::EEG_C_GetVersion::GetFirmwareVersion< vector <unsigned char> >( answer_pkg, answer_pkg.direct_data);
					reply->SerialNumber = MBN_EEG::EEG_C_GetVersion::GetSerialNumber< vector <unsigned char> >( answer_pkg, answer_pkg.direct_data);
				}

				return true;
			}
		};

		// Acинхронная отправка команды получения версии и серийного номера. 
		// Просто отправляет устройству команду GetVersion
		// Возвращает успешность отправки команды устройству.
		bool CAS_GetVersion()
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			vector<unsigned char> bytes(MBN_EEG::EEG_C_GetVersion::C_GetSize()); 
            MBN_EEG::EEG_C_GetVersion::CreatePackage<vector<unsigned char> >(pkg, bytes, offset);
				
            long long pkg_id = PostPackage(pkg, bytes);

			// если отправка не удалась, освобождаем объект и выходим
			if (!pkg_id) return false;
			else return true;
		};


		// Синхронная отправка команды установки главного коммутатора прибора. Блокирует поток до получения подтверждения выполнения команды
		// либо до принятия решения о таймауте выполнения.
		// U##_state - значение регистра соответствующей микросхемы коммутатора U##
		// Возвращает успешность выполнения операции команды.
		bool CS_SetMUX(unsigned char U34_state, unsigned char U24_state, unsigned char U6_state, unsigned char U2_state)
		{
			/// создаём посылки
			CommonPackage command_pkg;
			CommonPackage answer_pkg;

            long long offset = 0;
			command_pkg.direct_data.resize(MBN_EEG::EEG_C_SetMux::C_GetSize()); 
            MBN_EEG::EEG_C_SetMux::CreatePackage<vector<unsigned char> >(command_pkg, command_pkg.direct_data, offset, U34_state, U6_state, U24_state, U2_state);

			// отправляем команду устройству и ждём результатов и получаем результаты
			if (!CS_DoPostAndWait(command_pkg, answer_pkg))
				return false;
			else // тут команда отправилясь и получен корректный ответ на неё
				return true;
		};

		// Acинхронная отправка команды установки главного коммутатора прибора. 
		// Просто отправляет устройству команду SetMUX
		// Возвращает успешность отправки команды устройству.
		bool CAS_SetMUX(unsigned char U34_state, unsigned char U24_state, unsigned char U6_state, unsigned char U2_state)
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			vector<unsigned char> bytes(MBN_EEG::EEG_C_SetMux::C_GetSize()); 
            MBN_EEG::EEG_C_SetMux::CreatePackage<vector<unsigned char> >(pkg, bytes, offset, U34_state, U6_state, U24_state, U2_state);
				
            long long pkg_id = PostPackage(pkg, bytes);

			// если отправка не удалась, освобождаем объект и выходим
			if (!pkg_id) return false;
			else return true;
		};


		// Синхронная отправка команды получения версии протокола связи. Блокирует поток до получения подтверждения выполнения команды
		// либо до принятия решения о таймауте выполнения.
		// Если получает protocol_version указатель, то записывает туда полученный номер протокола.
		// Возвращает успешность выполнения операции команды.
		bool CS_GetProtocolVersion(unsigned short *protocol_version_reply = 0)
		{
			/// создаём посылки
			CommonPackage command_pkg;
			CommonPackage answer_pkg;

            long long offset = 0;
			command_pkg.direct_data.resize(MBN_EEG::EEG_C_GetProtocolVersion::C_GetSize()); 
            MBN_EEG::EEG_C_GetProtocolVersion::CreatePackage<vector<unsigned char> >(command_pkg, command_pkg.direct_data, offset);

			// отправляем команду устройству и ждём результатов и получаем результаты
			if (!CS_DoPostAndWait(command_pkg, answer_pkg))
				return false;
			else // тут команда отправилась и получен корректный ответ на неё
			{
				// если надо - выводим данные
				if (protocol_version_reply)
					(*protocol_version_reply) = MBN_EEG::EEG_C_GetProtocolVersion::GetProtocolVersion< vector <unsigned char> >( answer_pkg, answer_pkg.direct_data);

				return true;
			}
		};

		// Acинхронная отправка команды получения версии и серийного номера. 
		// Просто отправляет устройству команду GetProtVersion
		// Возвращает успешность отправки команды устройству.
		bool CAS_GetProtocolVersion()
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			vector<unsigned char> bytes(MBN_EEG::EEG_C_GetProtocolVersion::C_GetSize()); 
            MBN_EEG::EEG_C_GetProtocolVersion::CreatePackage<vector<unsigned char> >(pkg, bytes, offset);
				
            long long pkg_id = PostPackage(pkg, bytes);

			// если отправка не удалась, освобождаем объект и выходим
			if (!pkg_id) return false;
			else return true;
		};


		// Синхронная отправка команды получения списка доступных частот сэмплинга. Блокирует поток до получения подтверждения выполнения команды
		// либо до принятия решения о таймауте выполнения.
		// Если получает freq_array_reply указатель, то записывает туда полученный список частот.
		// Возвращает успешность выполнения операции команды.
        bool CS_GetFrequenciesTable(vector<unsigned short> *freq_array_reply = 0)
		{
			/// создаём посылки
			CommonPackage command_pkg;
			CommonPackage answer_pkg;

            long long offset = 0;
			command_pkg.direct_data.resize(MBN_EEG::EEG_C_GetFrequencyTable::C_GetSize()); 
            MBN_EEG::EEG_C_GetFrequencyTable::CreatePackage<vector<unsigned char> >(command_pkg, command_pkg.direct_data, offset);

			// отправляем команду устройству и ждём результатов и получаем результаты
			if (!CS_DoPostAndWait(command_pkg, answer_pkg))
				return false;
			else // тут команда отправилась и получен корректный ответ на неё
			{
				// если надо - выводим данные
				if (freq_array_reply)
					(*freq_array_reply) = MBN_EEG::EEG_C_GetFrequencyTable::A_GetFrequenciesList< vector <unsigned char> >( answer_pkg, answer_pkg.direct_data);

				return true;
			}
		};

		// Acинхронная отправка команды получения списка доступных частот сэмплинга. 
		// Просто отправляет устройству команду GetTableFreq.
		// Возвращает успешность отправки команды устройству.
		bool CAS_GetFrequenciesTable()
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			vector<unsigned char> bytes(MBN_EEG::EEG_C_GetFrequencyTable::C_GetSize()); 
            MBN_EEG::EEG_C_GetFrequencyTable::CreatePackage<vector<unsigned char> >(pkg, bytes, offset);
				
            long long pkg_id = PostPackage(pkg, bytes);

			// если отправка не удалась, освобождаем объект и выходим
			if (!pkg_id) return false;
			else return true;
		};


		// Синхронная отправка команды установки частоты сэмплинга. Блокирует поток до получения подтверждения выполнения команды
		// либо до принятия решения о таймауте выполнения.
		// FrequencyIndexToSet - индекс частоты в списке частот прибора, которую надо установить
		// SetFrequencyIndex_reply - адрес, куда записывать индекс реально установленной частоты, если 0, то ничего туда не кладёт
		// Возвращает успешность выполнения операции команды.
		bool CS_SetSamplingFrequency(unsigned char FrequencyIndexToSet, unsigned char *SetFrequencyIndex_reply = 0)
		{
			/// создаём посылки
			CommonPackage command_pkg;
			CommonPackage answer_pkg;

            long long offset = 0;
			command_pkg.direct_data.resize(MBN_EEG::EEG_C_SetSamplingFrequency::C_GetSize()); 
            MBN_EEG::EEG_C_SetSamplingFrequency::CreatePackage<vector<unsigned char> >(command_pkg, command_pkg.direct_data, offset, FrequencyIndexToSet);

			// отправляем команду устройству и ждём результатов и получаем результаты
			if (!CS_DoPostAndWait(command_pkg, answer_pkg))
				return false;
			else // тут команда отправилась и получен корректный ответ на неё
			{
				// если надо - выводим данные
				if (SetFrequencyIndex_reply)
					(*SetFrequencyIndex_reply) = MBN_EEG::EEG_C_SetSamplingFrequency::A_GetFrequencySetIndex< vector <unsigned char> >( answer_pkg, answer_pkg.direct_data);

				return true;
			}
		};

		// Acинхронная отправка команды установки частоты сэмплинга.
		// Просто отправляет устройству команду SetSamplingFrequency.
		// Возвращает успешность отправки команды устройству.
		bool CAS_SetSamplingFrequency(unsigned char FrequencyIndexToSet)
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			vector<unsigned char> bytes(MBN_EEG::EEG_C_SetSamplingFrequency::C_GetSize()); 
            MBN_EEG::EEG_C_SetSamplingFrequency::CreatePackage<vector<unsigned char> >(pkg, bytes, offset, FrequencyIndexToSet);
				
            long long pkg_id = PostPackage(pkg, bytes);

			// если отправка не удалась, освобождаем объект и выходим
			if (!pkg_id) return false;
			else return true;
		};

		// Acинхронная отправка команды установки частоты сэмплинга. 
		// Просто отправляет устройству команду SetSampFreq.
		// FrequencyIndexToSet - индекс частоты в списке частот прибора, которую надо установить
		// Возвращает успешность отправки команды устройству.
		bool CAS_GetFrequenciesTable(unsigned char FrequencyIndexToSet)
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			vector<unsigned char> bytes(MBN_EEG::EEG_C_SetSamplingFrequency::C_GetSize()); 
            MBN_EEG::EEG_C_SetSamplingFrequency::CreatePackage<vector<unsigned char> >(pkg, bytes, offset, FrequencyIndexToSet);
				
            long long pkg_id = PostPackage(pkg, bytes);

			// если отправка не удалась, освобождаем объект и выходим
			if (!pkg_id) return false;
			else return true;
		};


		// Синхронная отправка команды установки шаблонного режима работы (SetChannelsMode). Блокирует поток до получения подтверждения выполнения команды
		// либо до принятия решения о таймауте выполнения.
		// channels_mode - режим работы устройства
		// ch#_active - активность #-го канала (сейчас игнорируется устройством)
		// Возвращает успешность выполнения операции команды.
		bool CS_SetChannelsMode(MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_ChannelsMode channels_mode,
								bool ch0_active = true, bool ch1_active = true, bool ch2_active = true, bool ch3_active = true)
		{
			/// создаём посылки
			CommonPackage command_pkg;
			CommonPackage answer_pkg;

            long long offset = 0;
			command_pkg.direct_data.resize(MBN_EEG::EEG_C_SetChannelsMode::C_GetSize()); 
            MBN_EEG::EEG_C_SetChannelsMode::CreatePackage<vector<unsigned char> >(command_pkg, command_pkg.direct_data, offset, channels_mode, ch0_active, ch1_active, ch2_active, ch3_active);

			// отправляем команду устройству и ждём результатов и получаем результаты
			if (!CS_DoPostAndWait(command_pkg, answer_pkg))
				return false;
			else // тут команда отправилась и получен корректный ответ на неё
				return true;
		};

		// Acинхронная отправка команды установки шаблонного режима работы (SetChannelsMode). 
		// Просто отправляет устройству команду SetChannelsMode.
		// channels_mode - режим работы устройства
		// ch#_active - активность #-го канала (сейчас игнорируется устройством)
		// Возвращает успешность отправки команды устройству.
		bool CAS_SetChannelsMode(MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_ChannelsMode channels_mode,
								bool ch0_active = true, bool ch1_active = true, bool ch2_active = true, bool ch3_active = true)
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			vector<unsigned char> bytes(MBN_EEG::EEG_C_SetChannelsMode::C_GetSize()); 
            MBN_EEG::EEG_C_SetChannelsMode::CreatePackage<vector<unsigned char> >(pkg, bytes, offset, channels_mode, ch0_active, ch1_active, ch2_active, ch3_active);
				
            long long pkg_id = PostPackage(pkg, bytes);

			// если отправка не удалась, освобождаем объект и выходим
			if (!pkg_id) return false;
			else return true;
		};


		// Синхронная отправка команды установки формы и частоты калибровочного сигнала (CalSetShape). Блокирует поток до получения подтверждения выполнения команды
		// либо до принятия решения о таймауте выполнения.
		// calibration_shape - форма сигнала
		// frequency - частота сигнала в герцах (Hz)
		// Возвращает успешность выполнения операции команды.
		bool CS_SetCalibrationShape(MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_Calibrator_Shape calibration_shape, float frequency)
		{
			/// создаём посылки
			CommonPackage command_pkg;
			CommonPackage answer_pkg;

            long long offset = 0;
			command_pkg.direct_data.resize(MBN_EEG::EEG_C_SetCalibrationShape::C_GetSize()); 
            MBN_EEG::EEG_C_SetCalibrationShape::CreatePackage<vector<unsigned char> >(command_pkg, command_pkg.direct_data, offset, calibration_shape, frequency);

			// отправляем команду устройству и ждём результатов и получаем результаты
			if (!CS_DoPostAndWait(command_pkg, answer_pkg))
				return false;
			else // тут команда отправилась и получен корректный ответ на неё
				return true;
		};

		// Acинхронная отправка команды установки формы и частоты калибровочного сигнала (CalSetShape). 
		// Просто отправляет устройству команду CalSetShape.
		// calibration_shape - форма сигнала
		// frequency - частота сигнала в герцах (Hz)
		// Возвращает успешность отправки команды устройству.
		bool CAS_SetCalibrationShape(MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_Calibrator_Shape calibration_shape, float frequency)
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			vector<unsigned char> bytes(MBN_EEG::EEG_C_SetCalibrationShape::C_GetSize()); 
            MBN_EEG::EEG_C_SetCalibrationShape::CreatePackage<vector<unsigned char> >(pkg, bytes, offset, calibration_shape, frequency);
				
            long long pkg_id = PostPackage(pkg, bytes);

			// если отправка не удалась, освобождаем объект и выходим
			if (!pkg_id) return false;
			else return true;
		};


		// Синхронная отправка команды установки подсветки каналов (SetChLed). Блокирует поток до получения подтверждения выполнения команды
		// либо до принятия решения о таймауте выполнения.
		// channel - канал, которому надо задать режим подсветки
		// led_state - режим подсветки
		// Возвращает успешность выполнения операции команды.
		bool CS_SetChannelLed(MBN_EEG::EEG_Device_Architecture::EEG_CommonArc::EEG_CHANNEL channel, MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::EEG_Led_State led_state)
		{
			/// создаём посылки
			CommonPackage command_pkg;
			CommonPackage answer_pkg;

            long long offset = 0;
			command_pkg.direct_data.resize(MBN_EEG::EEG_C_SetChannelLeds::C_GetSize()); 
            MBN_EEG::EEG_C_SetChannelLeds::CreatePackage<vector<unsigned char> >(command_pkg, command_pkg.direct_data, offset, channel, led_state);

			// отправляем команду устройству и ждём результатов и получаем результаты
			if (!CS_DoPostAndWait(command_pkg, answer_pkg))
				return false;
			else // тут команда отправилась и получен корректный ответ на неё
				return true;
		};

		// Acинхронная отправка команды установки подсветки каналов (SetChLed). 
		// Просто отправляет устройству команду SetChLed.
		// channel - канал, которому надо задать режим подсветки
		// led_state - режим подсветки
		// Возвращает успешность отправки команды устройству.
		bool CAS_SetChannelLed(MBN_EEG::EEG_Device_Architecture::EEG_CommonArc::EEG_CHANNEL channel, MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::EEG_Led_State led_state)
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			vector<unsigned char> bytes(MBN_EEG::EEG_C_SetChannelLeds::C_GetSize()); 
            MBN_EEG::EEG_C_SetChannelLeds::CreatePackage<vector<unsigned char> >(pkg, bytes, offset, channel, led_state);
				
            long long pkg_id = PostPackage(pkg, bytes);

			// если отправка не удалась, освобождаем объект и выходим
			if (!pkg_id) return false;
			else return true;
		};


		// Синхронная отправка команды установки усиления в каналах АЦП (SetAmp). Блокирует поток до получения подтверждения выполнения команды
		// либо до принятия решения о таймауте выполнения.
		// channel#_ampl - усиление в канале АЦПа #-го канала
		// Возвращает успешность выполнения операции команды.
		bool CS_SetADS_Amplification(MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification channel0_ampl,
									 MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification channel1_ampl,
									 MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification channel2_ampl,
									 MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification channel3_ampl)
		{
			/// создаём посылки
			CommonPackage command_pkg;
			CommonPackage answer_pkg;

            long long offset = 0;
			command_pkg.direct_data.resize(MBN_EEG::EEG_C_SetAmplification::C_GetSize()); 
            MBN_EEG::EEG_C_SetAmplification::CreatePackage<vector<unsigned char> >(command_pkg, command_pkg.direct_data, offset, channel0_ampl, channel1_ampl, channel2_ampl, channel3_ampl);

			// отправляем команду устройству и ждём результатов и получаем результаты
			if (!CS_DoPostAndWait(command_pkg, answer_pkg))
				return false;
			else // тут команда отправилась и получен корректный ответ на неё
				return true;
		};

		// Acинхронная отправка команды установки усиления в каналах АЦП (SetAmp). 
		// Просто отправляет устройству команду SetAmp.
		// channel#_ampl - усиление в канале АЦПа #-го канала
		// Возвращает успешность отправки команды устройству.
		bool CAS_SetADS_Amplification(MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification channel0_ampl,
									  MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification channel1_ampl,
									  MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification channel2_ampl,
									  MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification channel3_ampl)
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			vector<unsigned char> bytes(MBN_EEG::EEG_C_SetAmplification::C_GetSize()); 
            MBN_EEG::EEG_C_SetAmplification::CreatePackage<vector<unsigned char> >(pkg, bytes, offset, channel0_ampl, channel1_ampl, channel2_ampl, channel3_ampl);
				
            long long pkg_id = PostPackage(pkg, bytes);

			// если отправка не удалась, освобождаем объект и выходим
			if (!pkg_id) return false;
			else return true;
		};


		// Синхронная отправка команды установки режима работы комутаторов каналов АЦПа (TEST_ADS_mux). Блокирует поток до получения подтверждения выполнения команды
		// либо до принятия решения о таймауте выполнения.
		// channel#_MUXmode - режим работы коммутатора #-го канала АЦПа 
		// Возвращает успешность выполнения операции команды.
		bool CS_SetADS_MUX( MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode channel0_MUXmode,
							MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode channel1_MUXmode,
							MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode channel2_MUXmode,
							MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode channel3_MUXmode)
		{
			/// создаём посылки
			CommonPackage command_pkg;
			CommonPackage answer_pkg;

            long long offset = 0;
			command_pkg.direct_data.resize(MBN_EEG::EEG_C_SetADSMux::C_GetSize()); 
            MBN_EEG::EEG_C_SetADSMux::CreatePackage<vector<unsigned char> >(command_pkg, command_pkg.direct_data, offset, channel0_MUXmode, channel1_MUXmode, channel2_MUXmode, channel3_MUXmode);

			// отправляем команду устройству и ждём результатов и получаем результаты
			if (!CS_DoPostAndWait(command_pkg, answer_pkg))
				return false;
			else // тут команда отправилась и получен корректный ответ на неё
				return true;
		};

		// Acинхронная отправка команды установки режима работы комутаторов каналов АЦПа (TEST_ADS_mux). 
		// Просто отправляет устройству команду TEST_ADS_mux.
		// channel#_MUXmode - режим работы коммутатора #-го канала АЦПа 
		// Возвращает успешность отправки команды устройству.
		bool CAS_SetADS_MUX( MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode channel0_MUXmode,
							 MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode channel1_MUXmode,
							 MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode channel2_MUXmode,
							 MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode channel3_MUXmode)
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			vector<unsigned char> bytes(MBN_EEG::EEG_C_SetADSMux::C_GetSize()); 
            MBN_EEG::EEG_C_SetADSMux::CreatePackage<vector<unsigned char> >(pkg, bytes, offset, channel0_MUXmode, channel1_MUXmode, channel2_MUXmode, channel3_MUXmode);
				
            long long pkg_id = PostPackage(pkg, bytes);

			// если отправка не удалась, освобождаем объект и выходим
			if (!pkg_id) return false;
			else return true;
		};


		// Синхронная отправка команды запуска генерации калибровочного сигнала (CalStart). Блокирует поток до получения подтверждения выполнения команды
		// либо до принятия решения о таймауте выполнения.
		// Возвращает успешность выполнения операции команды.
		bool CS_StartCalibration()
		{
			/// создаём посылки
			CommonPackage command_pkg;
			CommonPackage answer_pkg;

            long long offset = 0;
			command_pkg.direct_data.resize(MBN_EEG::EEG_C_StartCalibration::C_GetSize()); 
            MBN_EEG::EEG_C_StartCalibration::CreatePackage<vector<unsigned char> >(command_pkg, command_pkg.direct_data, offset);

			// отправляем команду устройству и ждём результатов и получаем результаты
			if (!CS_DoPostAndWait(command_pkg, answer_pkg))
				return false;
			else // тут команда отправилясь и получен корректный ответ на неё
				return true;
		};

		// Acинхронная отправка команды запуска генерации калибровочного сигнала (CalStart). 
		// Просто отправляет устройству команду CalStart
		// Возвращает успешность отправки команды устройству.
		bool CAS_StartCalibration()
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			vector<unsigned char> bytes(MBN_EEG::EEG_C_StartCalibration::C_GetSize()); 
            MBN_EEG::EEG_C_StartCalibration::CreatePackage<vector<unsigned char> >(pkg, bytes, offset);
				
            long long pkg_id = PostPackage(pkg, bytes);

			// если отправка не удалась, освобождаем объект и выходим
			if (!pkg_id) return false;
			else return true;
		};

		// Синхронная отправка команды останова генерации калибровочного сигнала (CalStop). Блокирует поток до получения подтверждения выполнения команды
		// либо до принятия решения о таймауте выполнения.
		// Возвращает успешность выполнения операции команды.
		bool CS_StopCalibration()
		{
			/// создаём посылки
			CommonPackage command_pkg;
			CommonPackage answer_pkg;

            long long offset = 0;
			command_pkg.direct_data.resize(MBN_EEG::EEG_C_StopCalibration::C_GetSize()); 
            MBN_EEG::EEG_C_StopCalibration::CreatePackage<vector<unsigned char> >(command_pkg, command_pkg.direct_data, offset);

			// отправляем команду устройству и ждём результатов и получаем результаты
			if (!CS_DoPostAndWait(command_pkg, answer_pkg))
				return false;
			else // тут команда отправилясь и получен корректный ответ на неё
				return true;
		};

		// Acинхронная отправка команды останова генерации калибровочного сигнала (CalStop). 
		// Просто отправляет устройству команду CalStop
		// Возвращает успешность отправки команды устройству.
		bool CAS_StopCalibration()
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			vector<unsigned char> bytes(MBN_EEG::EEG_C_StopCalibration::C_GetSize()); 
            MBN_EEG::EEG_C_StopCalibration::CreatePackage<vector<unsigned char> >(pkg, bytes, offset);
				
            long long pkg_id = PostPackage(pkg, bytes);

			// если отправка не удалась, освобождаем объект и выходим
			if (!pkg_id) return false;
			else return true;
		};


		// Синхронная отправка команды запуска сэмплинга и трансляции данных по USB (Start_Rec). Блокирует поток до получения подтверждения выполнения команды
		// либо до принятия решения о таймауте выполнения.
		// Возвращает успешность выполнения операции команды.
		bool CS_StartRecording()
		{
			/// создаём посылки
			CommonPackage command_pkg;
			CommonPackage answer_pkg;

            long long offset = 0;
			command_pkg.direct_data.resize(MBN_EEG::EEG_C_StartTranslation::C_GetSize()); 
            MBN_EEG::EEG_C_StartTranslation::CreatePackage<vector<unsigned char> >(command_pkg, command_pkg.direct_data, offset);

			// отправляем команду устройству и ждём результатов и получаем результаты
			if (!CS_DoPostAndWait(command_pkg, answer_pkg))
				return false;
			else // тут команда отправилясь и получен корректный ответ на неё
				return true;
		};

		// Acинхронная отправка команды запуска сэмплинга и трансляции данных по USB (Start_Rec). 
		// Просто отправляет устройству команду Start_Rec
		// Возвращает успешность отправки команды устройству.
		bool CAS_StartRecording()
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			vector<unsigned char> bytes(MBN_EEG::EEG_C_StartTranslation::C_GetSize()); 
            MBN_EEG::EEG_C_StartTranslation::CreatePackage<vector<unsigned char> >(pkg, bytes, offset);
				
            long long pkg_id = PostPackage(pkg, bytes);

			// если отправка не удалась, освобождаем объект и выходим
			if (!pkg_id) return false;
			else return true;
		};


		// Синхронная отправка команды останова сэмплинга и трансляции данных по USB (Stop_Rec). Блокирует поток до получения подтверждения выполнения команды
		// либо до принятия решения о таймауте выполнения.
		// Возвращает успешность выполнения операции команды.
		bool CS_StopRecording()
		{
			/// создаём посылки
			CommonPackage command_pkg;
			CommonPackage answer_pkg;

            long long offset = 0;
			command_pkg.direct_data.resize(MBN_EEG::EEG_C_StopTranslation::C_GetSize()); 
            MBN_EEG::EEG_C_StopTranslation::CreatePackage<vector<unsigned char> >(command_pkg, command_pkg.direct_data, offset);

			// отправляем команду устройству и ждём результатов и получаем результаты
			if (!CS_DoPostAndWait(command_pkg, answer_pkg))
				return false;
			else // тут команда отправилясь и получен корректный ответ на неё
				return true;
		};

		// Acинхронная отправка команды останова сэмплинга и трансляции данных по USB (Stop_Rec). 
		// Просто отправляет устройству команду Stop_Rec
		// Возвращает успешность отправки команды устройству.
		bool CAS_StopRecording()
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			vector<unsigned char> bytes(MBN_EEG::EEG_C_StopTranslation::C_GetSize()); 
            MBN_EEG::EEG_C_StopTranslation::CreatePackage<vector<unsigned char> >(pkg, bytes, offset);
				
            long long pkg_id = PostPackage(pkg, bytes);

			// если отправка не удалась, освобождаем объект и выходим
			if (!pkg_id) return false;
			else return true;
		};

	#pragma endregion
		
		#pragma region Интерфейс управления обработчиками событий

		/// Добавить обработчик события в таблицу
		/// Векторы создаются клиентским кодом и им же и удаляются, мы только храним указатели.
		void AddEventVector(EEG_Event_Vector_Base *event_vector) {eventProcessor.AddEventVector(event_vector); };			

		/// Возвращает, есть ли указанный обработчик в таблице
		bool HasEventVector(EEG_Event_Vector_Base *event_vector) {return eventProcessor.HasEventVector(event_vector); };

		/// Убрать обработчик из таблицы (либо не находит обработчик и возвразает false, либо находит, удаляет его и возвращает true)
		/// Возвращает, был ли найден указанный обработчик
		/// Векторы создаются клиентским кодом и им же и удаляются, мы только храним указатели.
		bool RemoveEventVector(EEG_Event_Vector_Base *event_vector) { return eventProcessor.RemoveEventVector(event_vector); };

		// возвращает указатель на вектор-обработчик события, или 0, если индекс был вне границ массива
		const EEG_Event_Vector_Base* GetEventVector(const int index) { return eventProcessor[index]; };			

		// обработать событие - т.е. запустить последовательно все обработчики, которые реагируют на это событие
		bool ProcessEvent(const EEG_Event &ev) { return eventProcessor.ProcessEvent(ev); };		

		/// очищает таблицу обработчиков
		/// Векторы создаются клиентским кодом и им же и удаляются, мы только храним указатели.
		void ClearEventVectors() { return eventProcessor.ClearEventsTable(); };	

		/// Возвращает количество векторов в таблице векторов-обработчиков событий
		int GetEventVectorsTableLength() { return eventProcessor.GetEventVectorsTableLength(); };

		#pragma endregion

		#pragma region Управление режимами устройства

		/// Включает систему генерации событий устройства и вызова пользовательских обработчиков этих событий
		void EnableEvents()
		{
			// включаем процессор событий
			eventProcessor.StartEventThread();

			// ждём монопольного доступа к объекту
			accsess_semaphore.acquire();

			// ставим флаг асинхронной работы
			EventsEnabled = true;

			// подключаем функции обратного вызова
			MBN_EEG::EEG_Protocol::SetDataCallback(&DataCallback, this);
			MBN_EEG::EEG_Protocol::SetMessageCallback(&MessageCallback, this);
			MBN_EEG::EEG_Protocol::SetErrorCallback(&ErrorCallback, this);
			MBN_EEG::EEG_Protocol::SetAcknowledgeCallback(&CommandAcknowledgeCallback, this);
			MBN_EEG::EEG_Protocol::SetAcknowledgeTimeoutCallback(&CommandAcknowledgeTimeoutCallback, this);

			// отпускаем доступ к объекту
			accsess_semaphore.release();
		};

		/// Возвращает включена ли система генерации событий
		bool isEventSystemEnabled()
		{
			return EventsEnabled;
		};

		/// Выключает систему генерации событий устройства и вызова пользовательских обработчиков этих событий
		void DisableEvents()
		{
			// ждём монопольного доступа к объекту
			accsess_semaphore.acquire();

			EventsEnabled = false;

			// отключаем функции обратного вызова
			MBN_EEG::EEG_Protocol::RemoveDataCallback();
			MBN_EEG::EEG_Protocol::RemoveMessageCallback();
			MBN_EEG::EEG_Protocol::RemoveErrorCallback();
			MBN_EEG::EEG_Protocol::RemoveAcknowledgeCallback();
			MBN_EEG::EEG_Protocol::RemoveAcknowledgeTimeoutCallback();

			// отпускаем доступ к объекту
			accsess_semaphore.release();

			// выключаем процессор событий
			eventProcessor.StopEventThread();

			// очищаем очередь сообщений
			eventProcessor.ClearEventsTable();
		};



		/// Включает внутреннее хранение данных каналов в объекте EEG
		void EnableInternalDataStorage()
		{
			// ждём монопольного доступа к объекту
			accsess_semaphore.acquire();

			// ставим флаг асинхронной работы
			internalDataStorageEnabled = true;

			// отпускаем доступ к объекту
			accsess_semaphore.release();
		};

		/// Возвращает включено ли внутреннее хранение данных каналов в объекте EEG
		bool isInternalDataStorageEnabled()
		{
			return internalDataStorageEnabled;
		};

		/// Выключает внутреннее хранение данных каналов в объекте EEG, но не удаляет уже записанные данные
		void DisableInternalDataStorage()
		{
			// ждём монопольного доступа к объекту
			accsess_semaphore.acquire();

			// ставим флаг асинхронной работы
			internalDataStorageEnabled = false;

			// отпускаем доступ к объекту
			accsess_semaphore.release();
		};



		/// Включает внутреннее хранение данных каналов в объекте EEG
		void EnableInternalDeviceStatesStorage()
		{
			// ждём монопольного доступа к объекту
			accsess_semaphore.acquire();

			// ставим флаг асинхронной работы
			internalDeviceStatesStorageEnabled = true;

			// отпускаем доступ к объекту
			accsess_semaphore.release();
		};

		/// Возвращает включено ли внутреннее хранение данных каналов в объекте EEG
		bool isInternalDeviceStatesStorageEnabled()
		{
			return internalDeviceStatesStorageEnabled;
		};

		/// Выключает внутреннее хранение данных каналов в объекте EEG, но не удаляет уже записанные данные
		void DisableInternalDeviceStatesStorage()
		{
			// ждём монопольного доступа к объекту
			accsess_semaphore.acquire();

			// ставим флаг асинхронной работы
			internalDeviceStatesStorageEnabled = false;

			// отпускаем доступ к объекту
			accsess_semaphore.release();
		};

		#pragma endregion

	};
}


#endif
