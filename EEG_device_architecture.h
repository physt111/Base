/*! \file
	Файл содержит.... а хз что он содержит на данный момент))
*/ 

#ifndef MBN_EEG_DEVICE_ARCHITECTURE
#define MBN_EEG_DEVICE_ARCHITECTURE

#include <vector>
#include <string>
#include <PagedArray.cpp>
#include <assert.h>
#include <QSemaphore>


namespace MBN_EEG
{
	// просто класс, содержащий поле и флаг определённости этого поля
	// обёртка для значения с флагом определённости
	template<class T>
	class def_wrap
	{
	private:
		bool defined;
		T value;

	public:
		// конструктор
		def_wrap()
		{
			defined = false;
		};

		// конструктор со значением
		def_wrap(const T &val)
		{
			value = val;
			defined = true;
		};

		// установка значения
		void Set(const T &new_value)
		{
			value = new_value;
			defined = true;
		};

		// установка значения
		T& operator = (const T &new_val)
		{
			value = new_val;
			defined = true;
			return value;
		};

		// получение значения
		T Get()
		{
			return value;
		};

		// определена ли переменная
		bool Defined()
		{
			return defined;
		};

		// установить в неопределённое состояние
		void SetUndefined()
		{
			defined = false;
		};

		// проверка
		bool operator == (const T &val)
		{
			return (value == val);
		};
	};



	class EEG_Device_State;

	/// Класс содержит статическую информацию об устройстве (то как, например, стандартные режимы раборы коммутатора,
	/// идентефикаторы усиления АЦПа и т.д. 
	/// А также определяет классы-описатели состояний узлов, так как пространства состояний узлов это тоже архитектура
	class EEG_Device_Architecture
	{
	public:

		/****************************************************************************************************
		*
		*			Классы, описывающие архитектуру узлов устройства
		*	Под архитектурой понимаются те параметры, которые остаются неизменными
		*	в процессе работы прибора (без скидывания со стола, биения топором итд...)
		*   т.е. всё то, что без вмешательства инженера-разработчика или топора 8) остаётся неизменным
		*
		****************************************************************************************************/

		/// Описывает архитектуру CP21.. моста
		class EEG_CP_BridgeArc
		{
		public:

			// значения, после запуска питания по умолчанию
            /*static const unsigned short DEF_VID = 1000;
            static const unsigned short DEF_PID = 1000;
            static const unsigned char  DEF_PowerDescriptorAttributes = 0;
            static const unsigned char  DEF_PowerDescriptorMaxPower = 0;
            static const unsigned short DEF_ReleaseNumber = 0;
            static const std::string  DEF_SerialNumber;
            static const std::string  DEF_ProductDescriptionString;*/

            static const unsigned short DEF_VID;
            static const unsigned short DEF_PID;
            static const unsigned char  DEF_PowerDescriptorAttributes;
            static const unsigned char  DEF_PowerDescriptorMaxPower;
            static const unsigned short DEF_ReleaseNumber;
            static const std::string  DEF_SerialNumber;
            static const std::string  DEF_ProductDescriptionString;
		};

		/// Описывает архитектуру главного коммутатора каналов
		class EEG_ChannelsCommutatorArc
		{
		public:
			enum  EEG_ChannelsMode {CH_MODE_CALIBRATION = 0x00, CH_MODE_IMPEDANCE_MBN = 0x01, CH_MODE_EEG = 0x02, CH_MODE_IMPEDANCE_ALTONICS = 0x03};

			/// состояние регистра коммутатора 
            typedef unsigned char EEG_MainComm_OneChipState;

			/// состояние регистра коммутатора в нормальном режиме
//			static const EEG_MainComm_OneChipState   EEG_U34_State_NORMAL = 0x08;
//			static const EEG_MainComm_OneChipState   EEG_U6_State_NORMAL = 0x09;
//			static const EEG_MainComm_OneChipState   EEG_U24_State_NORMAL = 0x08;
//			static const EEG_MainComm_OneChipState   EEG_U2_State_NORMAL = 0x09;
            static const EEG_MainComm_OneChipState   EEG_U34_State_NORMAL;
            static const EEG_MainComm_OneChipState   EEG_U6_State_NORMAL;
            static const EEG_MainComm_OneChipState   EEG_U24_State_NORMAL;
            static const EEG_MainComm_OneChipState   EEG_U2_State_NORMAL;
			/// состояние регистра коммутатора в режиме калибровки
//			static const EEG_MainComm_OneChipState   EEG_U34_State_CALIBRATION = 0x10;
//			static const EEG_MainComm_OneChipState   EEG_U6_State_CALIBRATION = 0x60;
//			static const EEG_MainComm_OneChipState   EEG_U24_State_CALIBRATION = 0x10;
//			static const EEG_MainComm_OneChipState   EEG_U2_State_CALIBRATION = 0x60;
            static const EEG_MainComm_OneChipState   EEG_U34_State_CALIBRATION;
            static const EEG_MainComm_OneChipState   EEG_U6_State_CALIBRATION;
            static const EEG_MainComm_OneChipState   EEG_U24_State_CALIBRATION;
            static const EEG_MainComm_OneChipState   EEG_U2_State_CALIBRATION;



			/// состояние регистра коммутатора в режиме измерения импеданса Fp1
//			static const EEG_MainComm_OneChipState   EEG_U34_State_IMPEDANCE_MBN_Fp1 = 0x10;
//			static const EEG_MainComm_OneChipState   EEG_U6_State_IMPEDANCE_MBN_Fp1 = 0x90;
//			static const EEG_MainComm_OneChipState   EEG_U24_State_IMPEDANCE_MBN_Fp1 = 0x10;
//			static const EEG_MainComm_OneChipState   EEG_U2_State_IMPEDANCE_MBN_Fp1 = 0x8C;
            static const EEG_MainComm_OneChipState   EEG_U34_State_IMPEDANCE_MBN_Fp1;
            static const EEG_MainComm_OneChipState   EEG_U6_State_IMPEDANCE_MBN_Fp1;
            static const EEG_MainComm_OneChipState   EEG_U24_State_IMPEDANCE_MBN_Fp1;
            static const EEG_MainComm_OneChipState   EEG_U2_State_IMPEDANCE_MBN_Fp1;
			/// состояние регистра коммутатора в режиме измерения импеданса C3
//			static const EEG_MainComm_OneChipState   EEG_U34_State_IMPEDANCE_MBN_C3 = 0x10;
//			static const EEG_MainComm_OneChipState   EEG_U6_State_IMPEDANCE_MBN_C3 = 0x90;
//			static const EEG_MainComm_OneChipState   EEG_U24_State_IMPEDANCE_MBN_C3 = 0x10;
//			static const EEG_MainComm_OneChipState   EEG_U2_State_IMPEDANCE_MBN_C3 = 0x13;
            static const EEG_MainComm_OneChipState   EEG_U34_State_IMPEDANCE_MBN_C3;
            static const EEG_MainComm_OneChipState   EEG_U6_State_IMPEDANCE_MBN_C3;
            static const EEG_MainComm_OneChipState   EEG_U24_State_IMPEDANCE_MBN_C3;
            static const EEG_MainComm_OneChipState   EEG_U2_State_IMPEDANCE_MBN_C3;
			/// состояние регистра коммутатора в режиме измерения импеданса A1
//			static const EEG_MainComm_OneChipState   EEG_U34_State_IMPEDANCE_MBN_A1 = 0x10;
//			static const EEG_MainComm_OneChipState   EEG_U6_State_IMPEDANCE_MBN_A1 = 0x90;
//			static const EEG_MainComm_OneChipState   EEG_U24_State_IMPEDANCE_MBN_A1 = 0x0C;
//			static const EEG_MainComm_OneChipState   EEG_U2_State_IMPEDANCE_MBN_A1 = 0x90;
            static const EEG_MainComm_OneChipState   EEG_U34_State_IMPEDANCE_MBN_A1;
            static const EEG_MainComm_OneChipState   EEG_U6_State_IMPEDANCE_MBN_A1;
            static const EEG_MainComm_OneChipState   EEG_U24_State_IMPEDANCE_MBN_A1;
            static const EEG_MainComm_OneChipState   EEG_U2_State_IMPEDANCE_MBN_A1;
			/// состояние регистра коммутатора в режиме измерения импеданса Fp2
//			static const EEG_MainComm_OneChipState   EEG_U34_State_IMPEDANCE_MBN_Fp2 = 0x10;
//			static const EEG_MainComm_OneChipState   EEG_U6_State_IMPEDANCE_MBN_Fp2 = 0x8C;
//			static const EEG_MainComm_OneChipState   EEG_U24_State_IMPEDANCE_MBN_Fp2 = 0x10;
//			static const EEG_MainComm_OneChipState   EEG_U2_State_IMPEDANCE_MBN_Fp2 = 0x90;
            static const EEG_MainComm_OneChipState   EEG_U34_State_IMPEDANCE_MBN_Fp2;
            static const EEG_MainComm_OneChipState   EEG_U6_State_IMPEDANCE_MBN_Fp2;
            static const EEG_MainComm_OneChipState   EEG_U24_State_IMPEDANCE_MBN_Fp2;
            static const EEG_MainComm_OneChipState   EEG_U2_State_IMPEDANCE_MBN_Fp2;
			/// состояние регистра коммутатора в режиме измерения импеданса C4
//			static const EEG_MainComm_OneChipState   EEG_U34_State_IMPEDANCE_MBN_C4 = 0x10;
//			static const EEG_MainComm_OneChipState   EEG_U6_State_IMPEDANCE_MBN_C4 = 0x13;
//			static const EEG_MainComm_OneChipState   EEG_U24_State_IMPEDANCE_MBN_C4 = 0x10;
//			static const EEG_MainComm_OneChipState   EEG_U2_State_IMPEDANCE_MBN_C4 = 0x90;
            static const EEG_MainComm_OneChipState   EEG_U34_State_IMPEDANCE_MBN_C4;
            static const EEG_MainComm_OneChipState   EEG_U6_State_IMPEDANCE_MBN_C4;
            static const EEG_MainComm_OneChipState   EEG_U24_State_IMPEDANCE_MBN_C4;
            static const EEG_MainComm_OneChipState   EEG_U2_State_IMPEDANCE_MBN_C4;
			/// состояние регистра коммутатора в режиме измерения импеданса A2
//			static const EEG_MainComm_OneChipState   EEG_U34_State_IMPEDANCE_MBN_A2 = 0x0C;
//			static const EEG_MainComm_OneChipState   EEG_U6_State_IMPEDANCE_MBN_A2 = 0x90;
//			static const EEG_MainComm_OneChipState   EEG_U24_State_IMPEDANCE_MBN_A2 = 0x10;
//			static const EEG_MainComm_OneChipState   EEG_U2_State_IMPEDANCE_MBN_A2 = 0x90;
            static const EEG_MainComm_OneChipState   EEG_U34_State_IMPEDANCE_MBN_A2;
            static const EEG_MainComm_OneChipState   EEG_U6_State_IMPEDANCE_MBN_A2;
            static const EEG_MainComm_OneChipState   EEG_U24_State_IMPEDANCE_MBN_A2;
            static const EEG_MainComm_OneChipState   EEG_U2_State_IMPEDANCE_MBN_A2;

		public:
			// значения, после запуска питания по умолчанию
//			static const EEG_MainComm_OneChipState EEG_DEF_U34_State = EEG_U34_State_NORMAL;
//			static const EEG_MainComm_OneChipState EEG_DEF_U6_State =  EEG_U6_State_NORMAL;
//			static const EEG_MainComm_OneChipState EEG_DEF_U24_State = EEG_U24_State_NORMAL;
//			static const EEG_MainComm_OneChipState EEG_DEF_U2_State =  EEG_U2_State_NORMAL;
            static const EEG_MainComm_OneChipState EEG_DEF_U34_State;
            static const EEG_MainComm_OneChipState EEG_DEF_U6_State;
            static const EEG_MainComm_OneChipState EEG_DEF_U24_State;
            static const EEG_MainComm_OneChipState EEG_DEF_U2_State;


		public:
			/// Перечисление микросхем коммутатора
			enum EEG_MainComm_Chip  { chip_U2, chip_U6, chip_U24, chip_U34, chip_ALL}; 

			/// Перечисление управляемых байтом состояния пар пинов микросхемы коммутатора
			/// По совместительству они равны числам, в которых в 1 выставлены биты, соответствующие 
			/// данной паре в регистре одной микросхемы коммутатора (1 байт)
			/// см. таблицу установки регистров коммутатора а также схемотехнику
            typedef unsigned char EEG_MainComm_PinPair;

			static const EEG_MainComm_PinPair  pp_7_8 = 0x02;
			static const EEG_MainComm_PinPair  pp_6_5 = 0x01;
			static const EEG_MainComm_PinPair  pp_17_18 = 0x40;
			static const EEG_MainComm_PinPair  pp_19_20 = 0x80; 
			static const EEG_MainComm_PinPair  pp_10_9 = 0x04;
			static const EEG_MainComm_PinPair  pp_11_12 = 0x08; 
			static const EEG_MainComm_PinPair  pp_14_13 = 0x10;
			static const EEG_MainComm_PinPair  pp_16_15 = 0x20;
			static const EEG_MainComm_PinPair  pp_ALL = 0xFF;


			/// Какие есть каналы у устройства, нумерация совпадает с нумерацией каналов в посылках
            typedef unsigned char EEG_Channel;

			static const EEG_Channel EEG_Ch_A1 = 0x04;
			static const EEG_Channel EEG_Ch_A2 = 0x05;
			static const EEG_Channel EEG_Ch_Fp1 = 0x02;
			static const EEG_Channel EEG_Ch_Fp2 = 0x00;
			static const EEG_Channel EEG_Ch_C3 = 0x03;
			static const EEG_Channel EEG_Ch_C4 = 0x01;
		};

		/// Описывает архитектуру узла подсветки каналов
		class EEG_ChannelsLedsArc
		{
		public:
			/// возможные состояния подсветки канала
			enum  EEG_Led_State {LED_OFF = 0x00, LED_RED = 0x01, LED_GREEN = 0x02, LED_ORANGE = 0x03};

			// распечатка состояния в строку
			static std::string print_to_str(EEG_Led_State led_st);

			// распечатка состояния
			static void print(EEG_Led_State led_st);

		public:
			// значения, после запуска питания по умолчанию
//			static const EEG_ChannelsLedsArc::EEG_Led_State DEF_connector_Fp2_state = EEG_ChannelsLedsArc::LED_OFF;
//			static const EEG_ChannelsLedsArc::EEG_Led_State DEF_connector_C4_state = EEG_ChannelsLedsArc::LED_OFF;
//			static const EEG_ChannelsLedsArc::EEG_Led_State DEF_connector_Fp1_state = EEG_ChannelsLedsArc::LED_OFF;
//			static const EEG_ChannelsLedsArc::EEG_Led_State DEF_connector_C3_state = EEG_ChannelsLedsArc::LED_OFF;
//			static const EEG_ChannelsLedsArc::EEG_Led_State DEF_connector_A1_state = EEG_ChannelsLedsArc::LED_OFF;
//			static const EEG_ChannelsLedsArc::EEG_Led_State DEF_connector_A2_state = EEG_ChannelsLedsArc::LED_OFF;
//			static const EEG_ChannelsLedsArc::EEG_Led_State DEF_connector_GND_state = EEG_ChannelsLedsArc::LED_OFF;
            static const EEG_ChannelsLedsArc::EEG_Led_State DEF_connector_Fp2_state;
            static const EEG_ChannelsLedsArc::EEG_Led_State DEF_connector_C4_state;
            static const EEG_ChannelsLedsArc::EEG_Led_State DEF_connector_Fp1_state;
            static const EEG_ChannelsLedsArc::EEG_Led_State DEF_connector_C3_state;
            static const EEG_ChannelsLedsArc::EEG_Led_State DEF_connector_A1_state;
            static const EEG_ChannelsLedsArc::EEG_Led_State DEF_connector_A2_state;
            static const EEG_ChannelsLedsArc::EEG_Led_State DEF_connector_GND_state;
		};

		/// Описывает архитектуру узла технических диодов
		class EEG_FunctionalLedsArc
		{
		public:
			// значения, после запуска питания по умолчанию
//			static const bool DEF_DS1_state = false;
//			static const bool DEF_DS2_state = false;
            static const bool DEF_DS1_state;
            static const bool DEF_DS2_state;
		};

		/// Описывает архитектуру узла АЦПа
		class EEG_ADC_Arc
		{
		public:
			// состояние коммутатора АЦПа по одному каналу
			enum  EEG_ADS_Mux_Mode { EEG_CH_ADS_MUX_MODE__NORMAL = 0x00, 
									 EEG_CH_ADS_MUX_MODE__SHORT_CIRCUT = 0x01, 
									 EEG_CH_ADS_MUX_MODE__REF_and_RLD_IN = 0x02, 
									 EEG_CH_ADS_MUX_MODE__MVDD = 0x03, 
									 EEG_CH_ADS_MUX_MODE__TEMPERATURE = 0x04, 
									 EEG_CH_ADS_MUX_MODE__INTERNAL_TEST_SOURCE = 0x05,
									 EEG_CH_ADS_MUX_MODE__RLD_DRP = 0x06,
									 EEG_CH_ADS_MUX_MODE__RLD_DRN = 0x07};

			// распечатка состояния
			static void print(EEG_ADS_Mux_Mode mux_mode);

			// распечатка состояния в строку
			static std::string print_to_str(EEG_ADS_Mux_Mode mux_mode);
			

			// состояние усилителя АЦПа по одному каналу
			enum  EEG_ADS_Amplification {EEG_ADS_AMPLIFICATION_1x = 0x01, 
										 EEG_ADS_AMPLIFICATION_2x = 0x02, 
										 EEG_ADS_AMPLIFICATION_3x = 0x03, 
										 EEG_ADS_AMPLIFICATION_4x = 0x04, 
										 EEG_ADS_AMPLIFICATION_6x = 0x00, 
										 EEG_ADS_AMPLIFICATION_8x = 0x05,
										 EEG_ADS_AMPLIFICATION_12x = 0x06};

			// распечатка состояния
			static void print(EEG_ADS_Amplification ampl_mode);

			// распечатка состояния в строку
			static std::string print_to_str(EEG_ADS_Amplification ampl_mode);

		public:
			// значения, после запуска питания по умолчанию
			
			/// размер умолчательного массива возможных частот дискретизации (в SPS - семплах в секунду)
			static const int DEF_FrequencyTable_Length = 4;
			/// массив возможных частот дискретизации (в SPS - семплах в секунду)
            static const unsigned short DEF_FrequencyTable[DEF_FrequencyTable_Length];
			
			/// Индекс в массиве текущей установленной частоты
//          static const char DEF_FrequencyIndex = 1;
            static const char DEF_FrequencyIndex;

			/// Состояния внутренних коммутаторов каналов АЦПа - при загрузе не инициализируются и случайны
			//static const EEG_ADC_Arc::EEG_ADS_Mux_Mode DEF_Ch0_MUX_STATE = EEG_ADC_Arc::EEG_ADS_Mux_Mode::EEG_CH_ADS_MUX_MODE__UNKNOWN;
			//static const EEG_ADC_Arc::EEG_ADS_Mux_Mode DEF_Ch1_MUX_STATE = EEG_ADC_Arc::EEG_ADS_Mux_Mode::EEG_CH_ADS_MUX_MODE__UNKNOWN;
			//static const EEG_ADC_Arc::EEG_ADS_Mux_Mode DEF_Ch2_MUX_STATE = EEG_ADC_Arc::EEG_ADS_Mux_Mode::EEG_CH_ADS_MUX_MODE__UNKNOWN;
			//static const EEG_ADC_Arc::EEG_ADS_Mux_Mode DEF_Ch3_MUX_STATE = EEG_ADC_Arc::EEG_ADS_Mux_Mode::EEG_CH_ADS_MUX_MODE__UNKNOWN;

			/// Состояния усилителей каналов 
//			static const EEG_ADC_Arc::EEG_ADS_Amplification DEF_Ch0_Ampl_STATE = EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_6x;
//			static const EEG_ADC_Arc::EEG_ADS_Amplification DEF_Ch1_Ampl_STATE = EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_6x;
//			static const EEG_ADC_Arc::EEG_ADS_Amplification DEF_Ch2_Ampl_STATE = EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_6x;
//			static const EEG_ADC_Arc::EEG_ADS_Amplification DEF_Ch3_Ampl_STATE = EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_6x;
            static const EEG_ADC_Arc::EEG_ADS_Amplification DEF_Ch0_Ampl_STATE;
            static const EEG_ADC_Arc::EEG_ADS_Amplification DEF_Ch1_Ampl_STATE;
            static const EEG_ADC_Arc::EEG_ADS_Amplification DEF_Ch2_Ampl_STATE;
            static const EEG_ADC_Arc::EEG_ADS_Amplification DEF_Ch3_Ampl_STATE;

			/// Включён ли АЦП
//			static const bool DEF_TurnedOn_STATE = false;
            static const bool DEF_TurnedOn_STATE;
		};

		/// Описывает архитектуру узла Калибратора
        class EEG_CalibratorArc
		{
		public:
			// возможные формы калибровочного сигнала
			enum  EEG_Calibrator_Shape {EEG_CAL_SHAPE_SINE = 0x00, 
										EEG_CAL_SHAPE_RECTANGLE = 0x01, 
										EEG_CAL_SHAPE_TRIANGLE = 0x02};

			static const float EEG_CALIBRATION_SIGNAL_FREQUENCY_STEP_HZ;
			
			static std::string print_to_str(EEG_Calibrator_Shape cal_shape);

			static void print(EEG_Calibrator_Shape cal_shape);

		public:
			// значения, после запуска питания по умолчанию

			/// Установленная форма калибровочного сигнала
//			static const EEG_Calibrator_Shape DEF_Shape = EEG_CAL_SHAPE_SINE;
            static const EEG_Calibrator_Shape DEF_Shape;
			/// Установленная частота сигнала
			static const float DEF_Frequency;
			/// Включён ли калибратор
//			static const bool DEF_TurnedOn_STATE = false;
            static const bool DEF_TurnedOn_STATE;
		};

		/// Описывает архитектуру устройства в целом (то, что не вошло в отдельные настройки узлов)
		class EEG_CommonArc
		{
		public:
			/// перечисление каналов устройства
			enum EEG_CHANNEL {	EEG_CHANNEL_Fp2 = 0,
								EEG_CHANNEL_C4 = 1,
								EEG_CHANNEL_Fp1 = 2,
								EEG_CHANNEL_C3 = 3,
								EEG_CHANNEL_A1 = 4,
								EEG_CHANNEL_A2 = 5
							  };

			static void print(EEG_CHANNEL channel);

		public:
			// значения, после запуска питания по умолчанию

			/// ID последней ошибки устройства
//			static const int DEF_LastErrorID = 0;
            static const int DEF_LastErrorID;

			/// опрос каналов включён
//			static const bool DEF_Recording = false;
            static const bool DEF_Recording;

			/// Подождите, пожалуйста, загружаемся :)
//			static const bool DEF_Booting = true;
            static const bool DEF_Booting;

			// версия прошивки мастера
            //static const unsigned char DEF_FirmwareVersion;

			// версия схемотехники
            //static const unsigned char DEF_CircuityVersion;

			// Серийный номер. Не путать с тем, который записан в мосте! Они Записаны в разных местах (хотя, может быть, мы их сделаем одинаковыми)
            //static const unsigned short DEF_SerialNumber;

			// версия протокола
            //static const unsigned char DEF_ProtocolVersion;
		};

	public:

		/****************************************************************************************************
		*
		*			Классы, описывающие состояния узлов устройства
		*
		****************************************************************************************************/

		/// Хранит состояние CP моста устройства
		class EEG_CP_BridgeState
		{
		public:
			// поля состояния
            def_wrap<unsigned short> VID;
            def_wrap<unsigned short> PID;
            def_wrap<unsigned char>  PowerDescriptorAttributes;
            def_wrap<unsigned char>  PowerDescriptorMaxPower;
            def_wrap<unsigned short> ReleaseNumber;
			def_wrap<std::string>  SerialNumber;    // максимальная длина == 63 взята из спецификации чипа
			def_wrap<std::string>  ProductDescriptionString;   // максимальная длина == 126 взята из спецификации чипа

		public:

			// конструктор, reset_state - установить значения, равные значениям после ресета
			EEG_CP_BridgeState(bool reset_state = false);

			// распечатка состояния в строку
            std::string print_to_str();

		// отладочный функционал
		#ifdef _DEBUG
			void print();
		#endif
		};

		/// \brief Класс хранит состояние главного коммутатора.
		/// \details Программно это по-сути обёртка для беззнакового инта, только он интерпретируется в 
		/// понятиях управления коммутатором EEG: выбрать микросхему коммутатора, соединить выводы, разъединить выводы,
		/// узнать состояния выводов (соединены, разъединены).
		/// \par по 1 в регистре соответствующие пины соединены, по 0 - разъединены
		class EEG_ChannelsCommutatorState
		{
		public:
			/// состояния коммутаторов
			def_wrap<EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState>  U34_State;
			def_wrap<EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState>  U6_State;
			def_wrap<EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState>  U24_State;
			def_wrap<EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState>  U2_State;

		public:

			// конструктор по-умолчанию создаёт объект с флагом unknown
			EEG_ChannelsCommutatorState(bool reset_state = false);

			// печать себя в строку
			std::string print_to_str();

			// отладочный функционал
		#ifdef _DEBUG
			void print();
		#endif

			/*****************************************
			*			СТАНДАРТНЫЕ РЕЖИМЫ
			*****************************************/

			/// Установка режима обычного измерения
			void SetNormal();

			/// Установка режима калибровки
			void SetCalibration();

			/// Установка режима измерения импеданса МБН по указанному каналу
			void SetImpedanceMBN(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Channel channel);

			/// Является ли установленный режим нормальным
			bool IsNormal();

			/// Является ли установленный режим калибровочным
			bool IsCalibration();

			/// Является ли установленный режим измерением импеданса МБН
			bool IsImpedanceMBN(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Channel channel);

			/*****************************************
			*		ПРОИЗВОЛЬНЫЕ  РЕЖИМЫ
			*****************************************/

			/// Разъединить всё пары пинов на всех чипах
			void SetDisconnectAll();

			/// установить соединёнными указанную пару пинов на указанном коммутаторе
			void SetConnect(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_Chip chip, EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_PinPair pin_pair);

			/// установить разъединить указанную пару пинов на указанном коммутаторе
			void SetDisconnect(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_Chip chip, EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_PinPair pin_pair);

			/// возвращает, соедининена ли выбранная пара пинов. Если выбран параметр ALL, то true вернёт только в случае 
			/// если все попадающие под условие выборки пары пинов соединены
			bool IsConnected(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_Chip chip, EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_PinPair pin_pair);
		};

		/// Хранит состояние Индикации каналов
		class EEG_ChannelsLedsState
		{
		public:
			/// состояние подсветки каналов
			def_wrap<EEG_ChannelsLedsArc::EEG_Led_State> Fp2_state;
			def_wrap<EEG_ChannelsLedsArc::EEG_Led_State> C4_state;
			def_wrap<EEG_ChannelsLedsArc::EEG_Led_State> Fp1_state;
			def_wrap<EEG_ChannelsLedsArc::EEG_Led_State> C3_state;
			def_wrap<EEG_ChannelsLedsArc::EEG_Led_State> A1_state;
			def_wrap<EEG_ChannelsLedsArc::EEG_Led_State> A2_state;
			def_wrap<EEG_ChannelsLedsArc::EEG_Led_State> GND_state;

		public:

			// конструктор по-умолчанию создаёт объект с флагом unknown
			EEG_ChannelsLedsState(bool reset_state = false);
			
			// печать себя в строку
			std::string print_to_str();

			// отладочный функционал
		#ifdef _DEBUG
			void print();
		#endif
		};

		/// Хранит состояние технических светодиодов
		class EEG_FunctionalLedsState
		{
		public:
			/// состояние подсветки технических светодиодов
			def_wrap<bool> DS1_on;
			def_wrap<bool> DS2_on;

		public:

			// конструктор по-умолчанию создаёт объект с флагом unknown
			EEG_FunctionalLedsState(bool reset_state = false);

			// печать себя в строку
			std::string print_to_str();

			// отладочный функционал
		#ifdef _DEBUG
			void print();
		#endif
		};

		/// Хранит состояние АЦПа
		class EEG_ADC_State
		{
		public: 
			/// массив возможных частот дискретизации (в SPS - семплах в секунду)
            def_wrap<std::vector<unsigned short> > FrequencyTable;
			
			/// Индекс в массиве текущей установленной частоты
            def_wrap<char> CurrentFrequencyIndex;

			/// Состояния внутренних коммутаторов каналов АЦПа
			def_wrap<EEG_ADC_Arc::EEG_ADS_Mux_Mode> Ch0_MUX;
			def_wrap<EEG_ADC_Arc::EEG_ADS_Mux_Mode> Ch1_MUX;
			def_wrap<EEG_ADC_Arc::EEG_ADS_Mux_Mode> Ch2_MUX;
			def_wrap<EEG_ADC_Arc::EEG_ADS_Mux_Mode> Ch3_MUX;

			/// Состояния усилителей каналов 
			def_wrap<EEG_ADC_Arc::EEG_ADS_Amplification> Ch0_Ampl;
			def_wrap<EEG_ADC_Arc::EEG_ADS_Amplification> Ch1_Ampl;
			def_wrap<EEG_ADC_Arc::EEG_ADS_Amplification> Ch2_Ampl;
			def_wrap<EEG_ADC_Arc::EEG_ADS_Amplification> Ch3_Ampl;

			/// Включён ли АЦП
			def_wrap<bool> TurnedOn;

		public:

			// конструктор по-умолчанию создаёт объект с флагом unknown
			EEG_ADC_State(bool reset_state = false);

			/// вывод себя в строку
			std::string print_to_str();

			// отладочный функционал
		#ifdef _DEBUG
			void print();
		#endif
		};

		/// Хранит состояние калибратора (ЦАПа и его модулей управления)
		class EEG_Calibrator_State
		{
		public:
			/// Установленная форма калибровочного сигнала
			def_wrap<EEG_CalibratorArc::EEG_Calibrator_Shape> Shape;
			/// Установленная частота сигнала
			def_wrap<float> Frequency;
			/// Включён ли калибратор
			def_wrap<bool> TurnedOn;

		public:

			// конструктор по-умолчанию создаёт объект с флагом unknown
			EEG_Calibrator_State(bool reset_state = false);

			/// вывод себя в строку
			std::string print_to_str();

			// отладочный функционал
		#ifdef _DEBUG
			void print();
		#endif
		};

		/// Состояние прибора вообще, то что не вписывается однозначно в остальные узлы, 
		/// или просто относится ко всему прибору (версия исполнения итд).
		/// Хранит общие параметры прибора, которые не отнесены к остальным группам (преимушественно - режимы работы и данные МК)
		class EEG_Common_State
		{
		public:
			/// ID последней ошибки устройства
			def_wrap<int> LastErrorID;

			/// опрос каналов включён
			def_wrap<bool> Recording;

			/// Подождите, пожалуйста, загружаемся :)
			def_wrap<bool> Booting;

			// версия прошивки мастера
            def_wrap<unsigned char> FirmwareVersion;

			// версия схемотехники
            def_wrap<unsigned char> CircuityVersion;

			// Серийный номер. Не путать с тем, который записан в мосте! Они Записаны в разных местах (хотя, может быть, мы их сделаем одинаковыми)
            def_wrap<unsigned short> SerialNumber;

			// версия протокола
            def_wrap<unsigned char> ProtocolVersion;

		public:

			// конструктор по-умолчанию создаёт объект с флагом unknown
			EEG_Common_State(bool reset_state = false);

			// вывод себя в строку
			std::string print_to_str();

			// отладочный функционал
		#ifdef _DEBUG
			void print();
		#endif
		};

		/**************************************************************
		*
		*			Класс, описывающий состояние устройства
		*
		**************************************************************/

		/// Класс, который непосредственно хранит данные состояния устройства
		class EEG_Device_State
		{
		public:
			///// последовательности состояний узлов:
			EEG_Device_Architecture::EEG_CP_BridgeState BridgeData_State;
			EEG_Device_Architecture::EEG_ChannelsCommutatorState MainCommutator_State;
			EEG_Device_Architecture::EEG_ChannelsLedsState ChannelsLeds_State;
			EEG_Device_Architecture::EEG_FunctionalLedsState FunctionalLeds_State;
			EEG_Device_Architecture::EEG_ADC_State ADC_State;
			EEG_Device_Architecture::EEG_Calibrator_State DAC_State;
			EEG_Device_Architecture::EEG_Common_State Common_State;

		public:
			
			/// вывод в строку состояния устройства
			std::string print_to_str();

			/// отладочная функция
		#ifdef _DEBUG
			void print();
		#endif
		};

	public:

		/**************************************************************
		*
		*			Статический функционал архитектуры
		*
		**************************************************************/

		
        static float GetRealVoltage__uV(int ch_data, char channel_idx, EEG_ADC_State* ADC_state_ptr);

		/// Статическая функция, возвращает вольтаж в микровольтах на основе указанного коэффициента усиления канала
		/// и значения отсчёта АЦП.
		static float GetRealVoltage__uV(int ch_data, EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch_amplification);

		/// Возвразает коэффициент преобразования из отсчёта АЦПа в микровольты
		static float GetMeasurementToVoltageCoeff(EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch_amplification);

		/*/// Усиление аналоговой части (каскада перед АЦПом)
		static const float ADC_VOLTAGE_STEP;

		/// Усиление аналоговой части (каскада перед АЦПом)
		static const float ANALOG_PART_AMPLIFICATION;*/
	};





	
	
	
};



#endif
