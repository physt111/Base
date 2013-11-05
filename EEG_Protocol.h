/*! \file
	Файл содержит класс интерпретатора. И более ничего.
*/ 



#include <vector>
#include <EEG_device_architecture.h>
#include <Common_Protocol.h>
#include <CommonPackage.h>
#include <CommonByteStreamParser.h>
#include <EEG_Data_Storage.h>
#include <EEG_device_architecture.h>
#include <EEG_Device_States_Storage.h>
#include <qdatetime.h>
#include <QString>
//#include <qexception.h>
#include <SimpleError.h>

#ifndef MBN_EEG_PROTOCOL__
#define MBN_EEG_PROTOCOL__

using namespace std;
using MBN_EEG::EEG_Device_Architecture;

/// Пространство имён для всего, что связано непосредственно с EEG устройством
namespace MBN_EEG
{
	/****************************************************************************************
	*
	*									Парсер байт-кода
	*
	*****************************************************************************************/


	/// <summary> Значит так,... Класс содержит 
	///		*) данные, хранящие состояние парсинга очереди данных в данный момент
	///		*) в том числе ссылки на очереди посылок и данных
	/// А также - чисто виртуальную функцию - собсна - сам парсер, которая должна быть определена в производном классе. </summary>
	template<class ByteStorage, class PackagesStorage>
	class EEG_ByteStreamParser: public CommonByteStreamParser<ByteStorage, PackagesStorage>
	{
		/**********************************************************************************
		*			Какие данные надо хранить? 
		*
		*указатель - \/				указатель - \/
		*      ОЧЕРЕДЬ БАЙТ					ОЧЕРЕДЬ ПОСЫЛОК
		*             ^								^
		*             |								|
		*           где остановились			какую посылку парсим
		* 
		**********************************************************************************/

	public:

		EEG_ByteStreamParser(bool __computer_side_mode): 
            CommonByteStreamParser<ByteStorage, PackagesStorage>(__computer_side_mode)
		{};

		EEG_ByteStreamParser(ByteStorage *bytes, PackagesStorage *packs, bool __computer_side_mode): 
            CommonByteStreamParser<ByteStorage, PackagesStorage>(bytes, packs, __computer_side_mode)
		{};

		/**********************************************************************************
		*			
		*
		*			Функция парсинга байт-кода устройства
		* 
		**********************************************************************************/

		/// Парсит данные из byte_storage и записывает результатат в packs_storage
		void ParseBytes();

		/// парсит участок байт-последовательности, начиная с указанной позиции, результаты парсинга пишет в dst_pkg
        inline void ParseOnePackage(long long starting_byte_idx, CommonPackage & dst_pkg);

		// проверяет, может ли существовать такая пара из подтипа комманды и её размера
		bool SubtypeAndSizePairOK(int subtype, int size);

		// проверяет, корректны ли спец-данные комманды
        bool SpecialDataIsOK(int command_subtype, ByteStorage & bytes, long long special_data_start_idx, int special_data_len);

		// в зависимости от направления передачи посылки выставляет тип посылки
		void SetPackageType(CommonPackage &pkg, int pkg_number, bool in_direction);
	};


	/****************************************************************************************
	*
	*				Общая основа для протокола, Статичен. Содержит общие методы и сведения
	*		для поддержания протокола EEG.
	*
	*****************************************************************************************/
	class EEG_Protocol: public CommonProtocol
	{
		/// Интерпретатор системы комманд. Класс интерпретатора, который выполняет двустороннее преобразование из/в байт-последовательности данных 
		/// последовательного канала связи в/из удобный для программы формат.
		/// \par Класс базируется на протоколе <a href="file://localhost/W:/projects/eeg_it/programms/testing_program/Документация по железу/Система команд EEG_IT v3.docx">системы комманд</a>
		/// Например, последовательность: {0x55, Size, 0x04, 0x01, CRC1, CRC2} он преобразует к виду
		/// "пришёл ответ на комманду GetProtVersion, ошибок нет, версия протокола 1"
		/// Или запрос "Получить доступные частоты" преобразует к виду {0x55, Size, 0x05, CRC1, CRC2}
		/// \par 
	
		/// <summary> Содержит общие для всех посылок методы и перечисления. Среди методов, например,
		/// генерация заголовка пакета, вычисление CRC.
		/// </summary>
		/// Построение байт-последовательности комманды для отправки на устройство (по байтам):
		/// {	
		///		- 0х55 (это всегдашний стартовый байт)
		///		- размер посылки в байтах (прям, все байты от первого до последнего)
		///		- номер комманды (по протоколу)
		///		- {последовательность байт - специфических данных комманды, если есть}
		///		- CRC1 (LO: младший байт CRC (беззнакового числа, равного алгебраической сумме всех байт посылки КРОМЕ стартового и самих CRC байт))           
		///		- CRC2 (HI: старший байт CRC) <br><br>		
		/// }

	public:
		/// <summary> Возможные типы посылок </summary>
		enum EEG_PackageType {
								COMMAND, 
								ANSWER, 
								DATA,
								MESSAGE
							 };
		
		/// <summary> Возможные подтипы комманд  (НОМЕР РАВЕН НОМЕРУ ПО ПРОТОКОЛУ)</summary>
		enum EEG_Command_Subtype {	
									C_ERROR = 0, 
									C_RESET = 1, 
									C_GET_VERSION = 2, 
									C_SET_MUX = 3, 
									C_GET_PROTOCOL_VERSION = 4, 
									C_GET_FREQUENCY_TABLE = 5,
									C_SET_SAMPLING_FREQUENCY = 6, 
									C_SET_CHANNELS_MODE = 8, 
									C_SET_CALIBRATION_SHAPE = 14,
									C_START_CALIBRATION = 16, 
									C_STOP_CALIBRATION = 17, 
									C_START_TRANSLATION = 18, 
									C_STOP_TRANSLATION = 19,
									C_SET_CHANNEL_LEDS = 25, 
									C_SET_AMPLIFICATION = 26, 
									C_SET_ADS_MUX = 27
								 }; 
		/// переводит из EEG_Command_Subtype в строку с названием команды
	public: static QString GetCommandNameByNumber(int command_number);

		/// <summary> Возможные подтипы ответов (сейчас - те же, что и комманды, за исключением RESET, т.к. 
		/// на RESET устройство не отвечает) (НОМЕР РАВЕН НОМЕРУ ПО ПРОТОКОЛУ) </summary>
		enum EEG_Answer_Subtype {	
									A_ERROR = 0,  
									A_RESET = 1,
									A_GET_VERSION = 2, 
									A_SET_MUX = 3, 
									A_GET_PROTOCOL_VERSION = 4, 
									A_GET_FREQUENCY_TABLE = 5,
									A_SET_SAMPLING_FREQUENCY = 6, 
									A_SET_CHANNELS_MODE = 8, 
									A_SET_CALIBRATION_SHAPE = 14,
									A_START_CALIBRATION = 16, 
									A_STOP_CALIBRATION = 17, 
									A_START_TRANSLATION = 18, 
									A_STOP_TRANSLATION = 19,
									A_SET_CHANNEL_LEDS = 25, 
									A_SET_AMPLIFICATION = 26, 
									A_SET_ADS_MUX = 27
								 }; 

		/// <summary> Возможные подтипы данных </summary>
		enum EEG_Data_Subtype {		
									D_USUAL_CHANNELS_DATA_WITHOUT_STIMUL = 151,
									D_USUAL_CHANNELS_DATA_WITH_STIMUL = 152,
									D_IMPEDANCE_MBN = 153,
									D_IMPEDANCE_ALTONIX = 154
								 }; 

		/// <summary> Возможные подтипы сообщений устройства </summary>
		enum EEG_Message_Subtype {		
									M_DEVICE_READY = 28
								 }; 


		/// Ошибки уровня протокола и ниже
        static const simple_error EEG_PROTOCOL_ERR_GOT_DAMAGED_PACK;
		static const simple_error EEG_PROTOCOL_ERR_ANSWER_ON_NOT_EXISTING_COMMAND;
        static const simple_error EEG_PROTOCOL_ERR_COMMAND_HAS_BEEN_SKIPPED;
        static const simple_error EEG_PROTOCOL_ERR_COMMAND_REPLY_TIMEOUT;

	public:

		/// Стартовый байт любой посылки
		static const unsigned char Starting_Byte = 0x55;
		/// Размер обязательных данных пакета (в байтах) для отправки устройству
		static const short Common_Data_Size = 5;
		/// Размер заголовка пакета
		static const short Header_Data_Size = 3;
		/// Смещение первого байта специализированных данных в посылке комманды от начала посылки
		static const short Special_Data_Offset = 3;

		//////////////////////////////////////////////////////////////////////// 
		// Общий для всех посылок EEG функционал, генерация оболочки посылки
		////////////////////////////////////////////////////////////////////////

	protected: 

		/**************************************************************************
		 *	СТАТИЧЕСКИЕ МЕТОДЫ ДЛЯ ПОМОЩИ В СОЗДАНИИ БАЙТ КОДОВ ПОСЫЛОК
		 **************************************************************************/
	protected:
		/// <summary> Служебная комманда для создания буфера пакета и записи в этот 
		///		буфер заголовка пакета (первые 3 байта), чтобы не писать каждый раз. 
		/// </summary>
		/// <param name="CommandDataSize"> 	Размер опциональных данных для комманды (это размер 
		///		пакета без заголовка (3 байта) и CRC байтов (2 байта)) 
		/// </param>
		/// <param name="CommandNumber"> Номер команды по протоколу(см. 
		///		<a href="file://localhost/W:/projects/eeg_it/programms/testing_program/Документация по железу/Система команд EEG_IT v3.docx">
		///		систему комманд</a>) в соответствии с протоколом. 
		/// </param>
		/// <returns> Созданный буфер данных для посылки с заполненными байтами заголовка. </returns>
		template <class DstStorage>
        static inline void write_package_header(DstStorage &out_package_data_storage, long long &curr_write_byte, short SpecialDataSize, short PackageCode)
		{
			// записываем стартовый байт
			out_package_data_storage[curr_write_byte + 0] = Starting_Byte;
			// записываем размер посылки (НЕ ВКЛЮЧАЯ стартовый байт и байт размера)
			out_package_data_storage[curr_write_byte + 1] = Common_Data_Size + SpecialDataSize;
			// записываем номер комманды
			out_package_data_storage[curr_write_byte + 2] = PackageCode;

			curr_write_byte += 3;
		};

		/// <summary> Служебная комманда для установки проверочных байтов CRC1 и CRC2 в пакет. См. описание EEG_Package для 
		///		дополнительных сведений, о том, как считается CRC
		/// </summary>
		/// <param name="pack'> Ссылка на буфер пакета со всеми заполненными байтами, 
		///		кроме последних двух и (опционально) стартового байта.
		///	</param>
		template <class DstStorage>
        static inline void set_package_CRC_bytes(DstStorage &out_package_data_storage, long long starting_pkg_byte_offset, int special_data_length, long long& curr_write_byte)
		{
			// количество байт CRC
			const int CRC_BYTES_COUNT = 2;
			// итоговая контрольная сумма
			unsigned short CRC = 0u;
			// индексы [от; до] по которым бежим для получения CRC
			int min_idx = starting_pkg_byte_offset + 1;
			int max_idx = starting_pkg_byte_offset + Common_Data_Size + special_data_length - CRC_BYTES_COUNT - 1;

			// бежим по байтам посылки начиная со второго по порядку байта включительно (по всем кроме стартового байта и тех, что отведены для CRC1,2, которые лежат в конце посылки)
			for (unsigned char i = min_idx; i <= max_idx; i++)
				CRC += out_package_data_storage[i];

			// записываем в пакет оба байта контрольной суммы 
			out_package_data_storage[max_idx + 1] = CRC & 0xFF;			// слева младший байт
			out_package_data_storage[max_idx + 2] = (CRC>>8) & 0xFF;	// справа старший

			// смещаем текущее положение вывода
			curr_write_byte += CRC_BYTES_COUNT;
		};

		/**************************************************************************************
		 *	СТАТИЧЕСКИЕ МЕТОДЫ ДЛЯ ПОМОЩИ В СОЗДАНИИ ПОСЫЛОК И ЗАПОЛНЕНИИ ИХ БАЙТ КОДОВ
		 **************************************************************************************/
	protected:

		/// <summary> 
		///		Вариативное создание посылки: данные посылки (её байт-код) сохраняются во внешнем хранилище
		///		(начиная с указанного стартового индекса), которое должно поддерживать обращение по индексу [] и
		///		иметь достаточно места, чтобы вместить данные посылки
		///		При удалении такой посылки, её данные не уничтожаются.
		///	</summary>
		/// <param name="pkg"> Структура посылки, которую надо инициализировать </param>
		template< class DstStorage>
		static inline void create_pkg(CommonPackage &pkg, 
                                      DstStorage &place_for_package_data, long long &start_writing_offset,
									  int pkg_type, int pkg_subtype, int PackageCode,
									  int special_data_length, CommonPackage::PackageDirection dir 
									  )
		{
			// задаём тип посылки
			pkg.type = pkg_type;
			// задаём подтип посылки
			pkg.subtype = pkg_subtype;	
			// задаём направление посылки
			pkg.direction = dir;

			// внутреннее или внешнее хранение данных
			if ((void*)(&(pkg.direct_data)) == (void*)(&place_for_package_data))
			{
				pkg.data_location = CommonPackage::INTERNAL;
				start_writing_offset = 0;
			}
			else pkg.data_location = CommonPackage::EXTERNAL;
			// задаём полный размер данных посылки
			pkg.package_data_length = Common_Data_Size + special_data_length;
			// задаём смещение данных в массив байт
			pkg.first_byte_stream_offset = start_writing_offset;

			// записываем заголовок пакета в байт последовательность и смещаемся на начало спец-данных
			write_package_header<DstStorage>(place_for_package_data, start_writing_offset, special_data_length, PackageCode);

			// смещаемся на начало CRC байт
			start_writing_offset += special_data_length;

			// штампуем CRC байт в посылку
			set_package_CRC_bytes<DstStorage>(place_for_package_data, pkg.first_byte_stream_offset, special_data_length, start_writing_offset);

			// задаём корректность посылки посылки
			pkg.correctness = CommonPackage::CORRECT;
		};

		template< class DstStorage>
		static inline void create_simple_command_pkg(CommonPackage &pkg, 
                                      DstStorage &place_for_package_data, long long &starting_byte_offset,
									  EEG_Command_Subtype Command
									  )
		{
			create_pkg<DstStorage>(pkg, place_for_package_data, starting_byte_offset, COMMAND,
								   Command, Command, 0, CommonPackage::OUT_DIR);
		};

	public:

		/**************************************************************************************
		 *	ЧЕЛОВЕЧЕСКИЕ ОПИСАНИЯ ПОСЫЛОК
		 **************************************************************************************/

		/// возвращает общий класс посылки (по данному протоколу)
		virtual string GetPackageTypeString(int type, int subtype);
		/// возвращает название посылки (по данному протоколу)
		virtual string GetPackageNameString(int type, int subtype);

	public:

		/**************************************************************************************
		 *	ОТЛАДОЧНЫЕ СТАТИЧЕСКИЕ МЕТОДЫ
		 **************************************************************************************/
		#ifdef _DEBUG
		// проверяет, может ли существовать такая пара из подтипа комманды и её размера
		static void PrintPackage(CommonPackage pkg)
		{
			if (pkg.direction == pkg.OUT_DIR)
				cout<<">>"<<" ";
			else if (pkg.direction == pkg.IN_DIR)
				cout<<"<<"<<" ";

			cout<<pkg.date_time.toString("hh:mm ss::zzz").toStdString()<<"ms";

			cout<<"\t<ID:"<<pkg.id<<">"<<endl;

			if (pkg.correctness == CommonPackage::CORRECT)
			{
				switch(pkg.type)
				{
				case EEG_Protocol::COMMAND:
					{
						cout<<"\t[comm]\t"; 
						if (pkg.ack)
							cout<<"Ack.\t"; 
						else
							cout<<"NOT Ack.\t"; 
						break;
					};
				case EEG_Protocol::ANSWER: cout<<"\t\t\t[answ]\t"; break;
				case EEG_Protocol::DATA: cout<<"\t\t\t[data]\t"; break;
				case EEG_Protocol::MESSAGE: cout<<"\t\t\t[msg]\t"; break;
				default: cout<<"<!UNKNOWN TYPE!>\t"; break;
				};

				if (pkg.type == EEG_Protocol::ANSWER)
					switch(pkg.subtype)
					{
					case EEG_Protocol::A_ERROR: cout<<"A_ERROR"; return;
					case EEG_Protocol::A_RESET: cout<<"A_RESET"; return;
					case EEG_Protocol::A_GET_VERSION: cout<<"A_GET_VERSION"; return;
					case EEG_Protocol::A_SET_MUX:	cout<<"A_SET_MUX"; return;
					case EEG_Protocol::A_GET_PROTOCOL_VERSION: cout<<"A_GET_PROTOCOL_VERSION"; return;
					case EEG_Protocol::A_GET_FREQUENCY_TABLE: cout<<"A_GET_FREQUENCY_TABLE"; return;
					case EEG_Protocol::A_SET_SAMPLING_FREQUENCY: cout<<"A_SET_SAMPLING_FREQUENCY"; return;
					case EEG_Protocol::A_SET_CHANNELS_MODE: cout<<"A_SET_CHANNELS_MODE"; return;
					case EEG_Protocol::A_SET_CALIBRATION_SHAPE: cout<<"A_SET_CALIBRATION_SHAPE"; return;
					case EEG_Protocol::A_START_CALIBRATION: cout<<"A_START_CALIBRATION"; return;
					case EEG_Protocol::A_STOP_CALIBRATION: cout<<"A_STOP_CALIBRATION"; return;
					case EEG_Protocol::A_START_TRANSLATION: cout<<"A_START_TRANSLATION"; return;
					case EEG_Protocol::A_STOP_TRANSLATION: cout<<"A_STOP_TRANSLATION"; return;
					case EEG_Protocol::A_SET_CHANNEL_LEDS: cout<<"A_SET_CHANNEL_LEDS"; return;
					case EEG_Protocol::A_SET_AMPLIFICATION: cout<<"A_SET_AMPLIFICATION"; return;
					case EEG_Protocol::A_SET_ADS_MUX: cout<<"A_SET_ADS_MUX"; return;
					default: cout<<"<!!!damaged!!!>"; return;
					}
				else if (pkg.type == EEG_Protocol::COMMAND)
					switch(pkg.subtype)
					{
					case EEG_Protocol::C_ERROR: cout<<"C_ERROR"; return;
					case EEG_Protocol::C_RESET: cout<<"C_RESET"; return;
					case EEG_Protocol::C_GET_VERSION: cout<<"C_GET_VERSION"; return;
					case EEG_Protocol::C_SET_MUX:	cout<<"C_SET_MUX"; return;
					case EEG_Protocol::C_GET_PROTOCOL_VERSION: cout<<"C_GET_PROTOCOL_VERSION"; return;
					case EEG_Protocol::C_GET_FREQUENCY_TABLE: cout<<"C_GET_FREQUENCY_TABLE"; return;
					case EEG_Protocol::C_SET_SAMPLING_FREQUENCY: cout<<"C_SET_SAMPLING_FREQUENCY"; return;
					case EEG_Protocol::C_SET_CHANNELS_MODE: cout<<"C_SET_CHANNELS_MODE"; return;
					case EEG_Protocol::C_SET_CALIBRATION_SHAPE: cout<<"C_SET_CALIBRATION_SHAPE"; return;
					case EEG_Protocol::C_START_CALIBRATION: cout<<"C_START_CALIBRATION"; return;
					case EEG_Protocol::C_STOP_CALIBRATION: cout<<"C_STOP_CALIBRATION"; return;
					case EEG_Protocol::C_START_TRANSLATION: cout<<"C_START_TRANSLATION"; return;
					case EEG_Protocol::C_STOP_TRANSLATION: cout<<"C_STOP_TRANSLATION"; return;
					case EEG_Protocol::C_SET_CHANNEL_LEDS: cout<<"C_SET_CHANNEL_LEDS"; return;
					case EEG_Protocol::C_SET_AMPLIFICATION: cout<<"C_SET_AMPLIFICATION"; return;
					case EEG_Protocol::C_SET_ADS_MUX: cout<<"C_SET_ADS_MUX"; return;
					default: cout<<"<!!!damaged!!!>"; return;
					}
				else if  (pkg.type == EEG_Protocol::DATA)
					switch(pkg.subtype)
					{
					case EEG_Protocol::D_USUAL_CHANNELS_DATA_WITHOUT_STIMUL: cout<<"D_USUAL_CHANNELS_DATA_WITHOUT_STIMUL"; return;
					case EEG_Protocol::D_USUAL_CHANNELS_DATA_WITH_STIMUL: cout<<"D_USUAL_CHANNELS_DATA_WITH_STIMUL"; return;
					case EEG_Protocol::D_IMPEDANCE_MBN: cout<<"D_IMPEDANCE_MBN"; return;
					case EEG_Protocol::D_IMPEDANCE_ALTONIX:	cout<<"D_IMPEDANCE_ALTONIX"; return;
					default: cout<<"<!!!damaged!!!>"; return;
					}
				else if  (pkg.type == EEG_Protocol::MESSAGE)
					switch(pkg.subtype)
					{
					case EEG_Protocol::M_DEVICE_READY: cout<<"M_DEVICE_READY"; return;
					default: cout<<"<!!!damaged!!!>"; return;
					};
			}
			else if (pkg.correctness == CommonPackage::UNKNOWN_CORRECTNESS) 
				cout<<"<unknown at the moment>";
			else cout<<"<!!!damaged!!!>";


		};

		template<class Storage>
		static void PrintPackageDetailed(CommonPackage *pkg, Storage &bytes)
		{
			MBN_EEG::EEG_Protocol::PrintPackage(*pkg);
			if (pkg->correctness == CommonPackage::CORRECT)
				switch((*pkg).type)
				{
				case MBN_EEG::EEG_Protocol::DATA:
					if ((*pkg).subtype == MBN_EEG::EEG_Protocol::D_USUAL_CHANNELS_DATA_WITHOUT_STIMUL)
					{
                        int ch0 = 0;
                        int ch1 = 0;
                        int ch2 = 0;
                        int ch3 = 0;

						MBN_EEG::EEG_D_UsualData::GetChannelsData<Storage>((*pkg), bytes, ch0, ch1, ch2, ch3);

						cout<<"\t\t\t\tch0 =\t"<<ch0<<endl;
						cout<<"\t\t\t\tch1 =\t"<<ch1<<endl;
						cout<<"\t\t\t\tch2 =\t"<<ch2<<endl;
						cout<<"\t\t\t\tch3 =\t"<<ch3<<endl;
					}
					else if ((*pkg).subtype == MBN_EEG::EEG_Protocol::D_USUAL_CHANNELS_DATA_WITH_STIMUL)
					{
                        int ch0 = 0;
                        int ch1 = 0;
                        int ch2 = 0;
                        int ch3 = 0;
							
						MBN_EEG::EEG_D_UsualData_With_Stimul::GetChannelsData((*pkg), bytes, ch0, ch1, ch2, ch3);

						cout<<"\t\t\t\tch0 =\t"<<ch0<<endl;
						cout<<"\t\t\t\tch1 =\t"<<ch1<<endl;
						cout<<"\t\t\t\tch2 =\t"<<ch2<<endl;
						cout<<"\t\t\t\tch3 =\t"<<ch3<<endl;
					}
					else if ((*pkg).subtype == MBN_EEG::EEG_Protocol::D_IMPEDANCE_MBN)
					{
						unsigned char ChNum = MBN_EEG::EEG_D_ImpedanceMBN::GetChannelNumber<Storage>((*pkg), bytes);

						cout<<"\t\t\t\t ChNum = "<<ChNum<<endl;

                        vector<int> tmp;
						tmp.resize(MBN_EEG::EEG_D_ImpedanceMBN::GetSamplesCount(*pkg));
                        MBN_EEG::EEG_D_ImpedanceMBN::GetChannelData<Storage, vector<int>>((*pkg), bytes, tmp, 0);

						for (int i = 0; i < tmp.size(); i++)
							cout<<"\t\t\t\t\t"<<tmp[i]<<endl;
					};
					break;
				case MBN_EEG::EEG_Protocol::ANSWER:
					{
						switch ((*pkg).subtype)
						{
						case MBN_EEG::EEG_Protocol::A_GET_VERSION: 
							cout<<endl;
							cout<<"\t\t\t\tFirmware Version: ";
								printf("%x", MBN_EEG::EEG_C_GetVersion::GetFirmwareVersion<Storage>((*pkg), bytes)); cout<<endl;
							cout<<"\t\t\t\tCircuity Version: ";
								printf("%x", MBN_EEG::EEG_C_GetVersion::GetCircuityVersion<Storage>((*pkg), bytes)); cout<<endl;
							cout<<"\t\t\t\tSerial Number: "<< MBN_EEG::EEG_C_GetVersion::GetSerialNumber<Storage>((*pkg), bytes)<<endl;
							break;
						case MBN_EEG::EEG_Protocol::A_SET_MUX:
							cout<<endl;
							cout<<"\t\t\t\tU34 mux: ";
								printf("%x", MBN_EEG::EEG_C_SetMux::GetMUX_U34_State<Storage>((*pkg), bytes)); cout<<endl;
							cout<<"\t\t\t\tU6 mux: ";
								printf("%x", MBN_EEG::EEG_C_SetMux::GetMUX_U6_State<Storage>((*pkg), bytes)); cout<<endl;
							cout<<"\t\t\t\tU24 mux: ";
								printf("%x", MBN_EEG::EEG_C_SetMux::GetMUX_U24_State<Storage>((*pkg), bytes)); cout<<endl;
							cout<<"\t\t\t\tU2 mux: ";
								printf("%x", MBN_EEG::EEG_C_SetMux::GetMUX_U2_State<Storage>((*pkg), bytes)); cout<<endl;
							break;
						case MBN_EEG::EEG_Protocol::A_GET_PROTOCOL_VERSION: 
							cout<<endl;
							cout<<"\t\t\t\tProtocol Ver. : ";
								printf("%x", MBN_EEG::EEG_C_GetProtocolVersion::GetProtocolVersion<Storage>((*pkg), bytes)); cout<<endl;
							break;
						case MBN_EEG::EEG_Protocol::A_GET_FREQUENCY_TABLE: 
							{
							cout<<endl;
                            vector<unsigned short> tmp = MBN_EEG::EEG_C_GetFrequencyTable::A_GetFrequenciesList<Storage>((*pkg), bytes);
							cout<<"\t\t\t\tFreq. table (len = "<<tmp.size()<<endl;

							for (int i = 0; i < tmp.size(); i++)
								cout<<"\t\t\t\t\t"<<i<<":  "<<tmp[i]<<"sps"<<endl;

							break;
							}
						case MBN_EEG::EEG_Protocol::A_SET_SAMPLING_FREQUENCY: 
							{
								cout<<endl;
								cout<<"\t\t\t\tFreq. set Idx = "<<MBN_EEG::EEG_C_SetSamplingFrequency::A_GetFrequencySetIndex<Storage>((*pkg), bytes);
								cout<<endl;
								break;
							}
						case MBN_EEG::EEG_Protocol::A_SET_CHANNELS_MODE: 
							{
								cout<<endl;
								cout<<"\t\t\t\t";
								switch (MBN_EEG::EEG_C_SetChannelsMode::A_GetChannelsModeSet<Storage>((*pkg), bytes))
								{
								case  MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::CH_MODE_EEG: cout<<"EEG"; break;
								case  MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::CH_MODE_CALIBRATION: cout<<"Calibration"; break;
								case  MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::CH_MODE_IMPEDANCE_MBN: cout<<"Impedance_MBN"; break;
								case  MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::CH_MODE_IMPEDANCE_ALTONICS: cout<<"Impedance_Altonics"; break;
								}
								cout<<endl;

								if (MBN_EEG::EEG_C_SetChannelsMode::A_IsCh0_activeSet<Storage>((*pkg), bytes)) cout<<"\t\t\t\tCh0 is Active"<<endl;
								else cout<<"\t\t\t\tCh0 is NOT active"<<endl;
								if (MBN_EEG::EEG_C_SetChannelsMode::A_IsCh1_activeSet<Storage>((*pkg), bytes)) cout<<"\t\t\t\tCh1 is Active"<<endl;
								else cout<<"\t\t\t\tCh1 is NOT active"<<endl;
								if (MBN_EEG::EEG_C_SetChannelsMode::A_IsCh2_activeSet<Storage>((*pkg), bytes)) cout<<"\t\t\t\tCh2 is Active"<<endl;
								else cout<<"\t\t\t\tCh2 is NOT active"<<endl;
								if (MBN_EEG::EEG_C_SetChannelsMode::A_IsCh3_activeSet<Storage>((*pkg), bytes)) cout<<"\t\t\t\tCh3 is Active"<<endl;
								else cout<<"\t\t\t\tCh3 is NOT active"<<endl;
									
								break;
							}
						case  MBN_EEG::EEG_Protocol::A_SET_CALIBRATION_SHAPE: 
							{
								cout<<endl;
								cout<<"\t\t\t\t";
								switch (MBN_EEG::EEG_C_SetCalibrationShape::A_GetCalibrationSignalShape_Set<Storage>((*pkg), bytes))
								{
								case  MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_CAL_SHAPE_SINE: cout<<"Sine"; break;
								case  MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_CAL_SHAPE_RECTANGLE: cout<<"Rectangle"; break;
								case  MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_CAL_SHAPE_TRIANGLE: cout<<"Triangle"; break;
								}
								cout<<endl;
									
								cout<<"\t\t\t\tFrequency = "<<MBN_EEG::EEG_C_SetCalibrationShape::A_GetCalibrationFreq_Set<Storage>((*pkg), bytes)<<"Hz"<<endl;
									
								break;
							}
						case MBN_EEG::EEG_Protocol::A_SET_CHANNEL_LEDS: 
							{
								cout<<endl;
								cout<<"\t\t\t\t";
								cout<<"Ch. number = "<<(unsigned int)(MBN_EEG::EEG_C_SetChannelLeds::A_GetChannelNumber_Set<Storage>((*pkg), bytes));
								cout<<endl;

								cout<<"\t\t\t\t";
								switch (MBN_EEG::EEG_C_SetChannelLeds::A_GetChannelLedColor_Set<Storage>((*pkg), bytes))
								{
								case  MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_OFF: cout<<"<off>"; break;
								case  MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_RED: cout<<"Red"; break;
								case  MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_GREEN: cout<<"Green"; break;
								case  MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_ORANGE: cout<<"Orange"; break;
								}
								cout<<endl;
								break;
							}
						case MBN_EEG::EEG_Protocol::A_SET_AMPLIFICATION: 
							{
								cout<<endl;
								cout<<"\t\t\t\t";
								cout<<"ch0:\t";
								switch (MBN_EEG::EEG_C_SetAmplification::A_GetChannel0_Amplification_Set<Storage>((*pkg), bytes))
								{
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_1x: cout<<"1x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_2x: cout<<"2x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_3x: cout<<"3x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_4x: cout<<"4x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_6x: cout<<"6x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_8x: cout<<"8x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_12x: cout<<"12x"; break;
								}
								cout<<endl;

								cout<<"\t\t\t\t";
								cout<<"ch1:\t";
								switch (MBN_EEG::EEG_C_SetAmplification::A_GetChannel1_Amplification_Set<Storage>((*pkg), bytes))
								{
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_1x: cout<<"1x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_2x: cout<<"2x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_3x: cout<<"3x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_4x: cout<<"4x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_6x: cout<<"6x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_8x: cout<<"8x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_12x: cout<<"12x"; break;
								}
								cout<<endl;

								cout<<"\t\t\t\t";
								cout<<"ch2:\t";
								switch (MBN_EEG::EEG_C_SetAmplification::A_GetChannel2_Amplification_Set<Storage>((*pkg), bytes))
								{
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_1x: cout<<"1x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_2x: cout<<"2x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_3x: cout<<"3x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_4x: cout<<"4x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_6x: cout<<"6x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_8x: cout<<"8x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_12x: cout<<"12x"; break;
								}
								cout<<endl;

								cout<<"\t\t\t\t";
								cout<<"ch3:\t";
								switch (MBN_EEG::EEG_C_SetAmplification::A_GetChannel3_Amplification_Set<Storage>((*pkg), bytes))
								{
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_1x: cout<<"1x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_2x: cout<<"2x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_3x: cout<<"3x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_4x: cout<<"4x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_6x: cout<<"6x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_8x: cout<<"8x"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_12x: cout<<"12x"; break;
								}
								cout<<endl;
								break;
							}
						case MBN_EEG::EEG_Protocol::A_SET_ADS_MUX: 
							{
								cout<<endl;
								cout<<"\t\t\t\t";
								cout<<"ch0:\t";
								switch (MBN_EEG::EEG_C_SetADSMux::A_GetChannel_0_MUX_Set<Storage>((*pkg), bytes))
								{
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__NORMAL: cout<<"Normal"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__SHORT_CIRCUT: cout<<"Short Circuit"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__REF_and_RLD_IN: cout<<"RLD_IN"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__MVDD: cout<<"MVDD"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__TEMPERATURE: cout<<"Temperature Sensor"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__INTERNAL_TEST_SOURCE: cout<<"ADS internal test signal"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__RLD_DRP: cout<<"RLD_DRP"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__RLD_DRN: cout<<"RLD_DRN"; break;
								}
								cout<<endl;

								cout<<"\t\t\t\t";
								cout<<"ch1:\t";
								switch (MBN_EEG::EEG_C_SetADSMux::A_GetChannel_1_MUX_Set<Storage>((*pkg), bytes))
								{
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__NORMAL: cout<<"Normal"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__SHORT_CIRCUT: cout<<"Short Circuit"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__REF_and_RLD_IN: cout<<"RLD_IN"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__MVDD: cout<<"MVDD"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__TEMPERATURE: cout<<"Temperature Sensor"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__INTERNAL_TEST_SOURCE: cout<<"ADS internal test signal"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__RLD_DRP: cout<<"RLD_DRP"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__RLD_DRN: cout<<"RLD_DRN"; break;
								}
								cout<<endl;

								cout<<"\t\t\t\t";
								cout<<"ch2:\t";
								switch (MBN_EEG::EEG_C_SetADSMux::A_GetChannel_2_MUX_Set<Storage>((*pkg), bytes))
								{
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__NORMAL: cout<<"Normal"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__SHORT_CIRCUT: cout<<"Short Circuit"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__REF_and_RLD_IN: cout<<"RLD_IN"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__MVDD: cout<<"MVDD"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__TEMPERATURE: cout<<"Temperature Sensor"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__INTERNAL_TEST_SOURCE: cout<<"ADS internal test signal"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__RLD_DRP: cout<<"RLD_DRP"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__RLD_DRN: cout<<"RLD_DRN"; break;
								}
								cout<<endl;

								cout<<"\t\t\t\t";
								cout<<"ch3:\t";
								switch (MBN_EEG::EEG_C_SetADSMux::A_GetChannel_3_MUX_Set<Storage>((*pkg), bytes))
								{
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__NORMAL: cout<<"Normal"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__SHORT_CIRCUT: cout<<"Short Circuit"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__REF_and_RLD_IN: cout<<"RLD_IN"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__MVDD: cout<<"MVDD"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__TEMPERATURE: cout<<"Temperature Sensor"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__INTERNAL_TEST_SOURCE: cout<<"ADS internal test signal"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__RLD_DRP: cout<<"RLD_DRP"; break;
								case MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__RLD_DRN: cout<<"RLD_DRN"; break;
								}
								cout<<endl;


								break;
							}
						};

						break;
					}
				};
		};

		#endif

	public:

		/**************************************************************************************
		 *	
		 *
		 *				Информация о том, что и где лежит (из специализированных данных), 
		 *	хранилища данных, какие-нибудь флаги состояния, массивы состояний устройства итд...
		 *				А также информация о текущем состоянии канала связи
		 *
		 *
		 **************************************************************************************/

		/// Указатель на хранилище данных каналов
		EEG_Data_Storage *channels_data_ptr;

		/// Указатель на хранилище состояний устройства
		EEG_Device_States_Storage *device_states_ptr;

		// парсер байт-кода посылок ЭЭГ
        MBN_EEG::EEG_ByteStreamParser<PagedArray<unsigned char>, PagedArray<CommonPackage> >  BSP;

		/// всё хорошо, работаем без ошибок
		bool all_is_ok;

		/// индекс самой ранней неподтверждённой посылки в массиве посылок выходной очереди
		int unsubmitted_pkg_idx;

		/// Количество ошибок протокола. Если число - больше MAX_ERRORS_COUNT (скорее всего за какой-то промежуток времени (типа часа)), то фатал еррор
		int errors_count;

		/// Список ошибок
		vector<simple_error> errors_list;

		/// максимальное число битых посылок, которое допускаем до того как сгенерировать исключение о сбое в протоколе
		static const int MAX_PROTOCOL_ERRORS_COUNT = 4;


		/***********************************************************
		*
		*					Флаги состояния
		*
		***********************************************************/

		/// работаем в режиме хоста
		const bool computer_side_mode;

		/// Включено внутреннее сохранение данных
		bool internalDataStorageEnabled;

		/// Включено внутреннее сохранение состояний устройства
		bool internalDeviceStatesStorageEnabled;


	private:
		/**************************************************************************************
		 *	
		 *
		 *					Указатели на функции обратного вызова
		 *
		 *
		 **************************************************************************************/

		/// Вызывается при ошибке, ей передаётся информация об ошибке
		void (*ErrorCallback)(simple_error, void* parameter_obj_ptr);

		/// Вызывается при получении подтверждения комманды ей передаётся id подтверждённой посылки
        void (*AckCallback)(long long acked_pack_Id, CommonPackage &AnswerPkg, void* parameter_obj_ptr);

		/// Вызывается при таймауте ожидания подтверждения выполнения команды, ей передайтся id просроченной посылки
        void (*AckTimeoutCallback)(long long timeout_pack_Id, int type, int subtype, void* parameter_obj_ptr);

		/// Вызывается при получении данных от устройства
		void (*DataCallback)(CommonPackage &DataPkg, void* parameter_obj_ptr);

		/// Вызывается при получении сообщения от устрйоства
		void (*MessageCallback)(CommonPackage &MessagePkg, void* parameter_obj_ptr);

		/**************************************************************************************
		 *	
		 *
		 *					Дополнительные параметры для функций обратного вызова
		 *
		 *
		 **************************************************************************************/

		// доп параметр для ErrorCallback
		void *error_callback_object_ptr;

		// доп параметр для AckCallback
		void *acknowledge_callback_object_ptr;

		// доп параметр для AckTimeoutCallback
		void *acknowledge_timeout_callback_object_ptr;

		// доп параметр для DataCallback
		void *data_callback_object_ptr;

		// доп параметр для MessageCallback
		void *message_callback_object_ptr;

		

		/// записывает в очередь на вывод посылку и её байт код
        void __post_package(const CommonPackage &pack, PagedArray<CommonPackage> *Out_pkg_Ptr, PagedArray<unsigned char> *Out_byte_Ptr);

	public:


		/**************************************************************************************
		 *	
		 *
		 *				Функционал обработки и распределения пришедших посылок
		 *
		 *
		 **************************************************************************************/

        virtual void ManagePackageSequenses(PagedArray<CommonPackage> *Out_pkg_Ptr, PagedArray<unsigned char> *Out_byte_Ptr,
                                            PagedArray<CommonPackage> *In_pkg_Ptr,  PagedArray<unsigned char> *In_byte_Ptr,
											int next_pack_to_write_idx);

		/// обработать пришедшую посылку с данными
		///		*) Преобразовать данные в вольтаж
		///		*) Записать данные вольтажа в хранилище
		/// pkg - посылка с типом DATA
        void __process_data_pkg(CommonPackage &pkg, PagedArray<unsigned char> *In_byte_Ptr, PagedArray<unsigned char> *Out_byte_Ptr);

		/// ФУНКЦИЯ ИЗМЕНЯЮЩАЯ (ТОЧНЕЕ СОХРАНЯЮЩАЯ) СОСТОЯНИЕ УСТРОЙСТВА ПРИ ПОЛУЧЕНИИ ПОДТВЕРЖДЕНИЯ О ВЫПОЛНЕНИИ КОММАНДЫ
		/// pkg - посылка с типом ANSWER
        void __process_comm_pkg(CommonPackage &pkg, PagedArray<unsigned char> *In_byte_Ptr, PagedArray<unsigned char> *Out_byte_Ptr);

		/// ФУНКЦИЯ ИЗМЕНЯЮЩАЯ (ТОЧНЕЕ СОХРАНЯЮЩАЯ) СОСТОЯНИЕ УСТРОЙСТВА ПРИ ПОЛУЧЕНИИ СООБЩЕНИЯ ОТ УСТРОЙСТВА
		/// pkg - посылка с типом MESSGAE
        void __process_message_pkg(CommonPackage &pkg, PagedArray<unsigned char> *In_byte_Ptr);

		/// для режима эмуляции устройства, - отвечает на пришедшую с хоста команду
        void __dev_answer(CommonPackage &pkg, PagedArray<CommonPackage> *Out_pkg_Ptr, PagedArray<unsigned char> *Out_byte_Ptr,
                                 PagedArray<unsigned char> *In_byte_Ptr);

		/// Пробует применить указанную посылку для подтверждения выполнения комманды из выходной очереди.
		/// Сначала пробует применить её, конечно, к Unsubmitted_pkg_idx посылке, но если это не удаётся,
		/// то применяет её к ближайшей более поздней посылке.
		/// Возвращает индекс посылки в выходной очереди, к которой указанная посылка была применёна как подтверждение
		/// Если найти соответствующую посылку не удалось - возвращает -1
        int __apply_ack(CommonPackage &in_pkg, int Unsubmitted_pkg_idx,
                              PagedArray<CommonPackage> *In_pkg_Ptr,  PagedArray<unsigned char> *In_byte_Ptr,
                              PagedArray<CommonPackage> *Out_pkg_Ptr, PagedArray<unsigned char> *Out_byte_Ptr,
							  int next_pack_to_write_idx);

		/// Функция применяет политику протокола к очередям сообщений в целом,
		/// определяет на какие команды был получен ответ,
		/// на какие был получен изменённый ответ,
		/// на какие ответ не был получен,
		/// решает вопрос о состоянии канала связи и устройства (сбой - норма)
        void __apply_protocol_policy(PagedArray<CommonPackage> *In_pkg_Ptr, PagedArray<CommonPackage> *Out_pkg_Ptr, PagedArray<unsigned char> *In_byte_Ptr, PagedArray<unsigned char> *Out_byte_Ptr);

		/**************************************************************************************
		 *
		 *					Установка и сброс функций обратного вызова
		 *
		 **************************************************************************************/

		/// Устанавливает функцию обратного вызова на подтверждение посылки
		/// object_ptr - указатель, который будет передаваться в callback-функцию при её вызове
		/// если ackCallback == 0 то функция обратного вызова не вызывается
        void SetAcknowledgeCallback(void (*ackCallback)(long long acked_pack_Id, CommonPackage &AnswerPkg, void *object_ptr), void *object_ptr = 0)
		{			
			AckCallback = ackCallback;
			acknowledge_callback_object_ptr = object_ptr;
		};

		/// Останавливает вызов callback функции по получении подтверждения выполнения команды
		void RemoveAcknowledgeCallback()
		{
			AckCallback = 0;
		};

		/// Устанавливает функцию обратного вызова на таймаут посылки
		/// object_ptr - указатель, который будет передаваться в callback-функцию при её вызове
		/// если ackCallback == 0 то функция обратного вызова не вызывается
        void SetAcknowledgeTimeoutCallback(void (*ackTimeoutCallback)(long long timeout_pack_Id, int type, int subtype, void *object_ptr), void *object_ptr = 0)
		{
			AckTimeoutCallback = ackTimeoutCallback;
			acknowledge_timeout_callback_object_ptr = object_ptr;
		};

		/// Останавливает вызов callback функции по таймауту команды
		void RemoveAcknowledgeTimeoutCallback()
		{
			AckTimeoutCallback = 0;
		};

		/// Устанавливает функцию обратного вызова на обнаружение ошибки протокола
		/// object_ptr - указатель, который будет передаваться в callback-функцию при её вызове
		/// если ErrorCallback == 0 то функция обратного вызова не вызывается
		void SetErrorCallback(void (*errorCallback)(simple_error error_info, void *object_ptr), void *object_ptr = 0)
		{
			errorCallback = ErrorCallback;
			error_callback_object_ptr = object_ptr;
		};

		/// Останавливает вызов callback функции по обнаружению ошибки
		void RemoveErrorCallback()
		{
			ErrorCallback = 0;
		};

		/// Устанавливает функцию обратного вызова на получение данных от устройства
		/// object_ptr - указатель, который будет передаваться в callback-функцию при её вызове
		/// если dataCallback == 0 то функция обратного вызова не вызывается
		void SetDataCallback(void (*dataCallback)(CommonPackage &DataPkg, void* parameter_obj_ptr), void *object_ptr = 0)
		{
			DataCallback = dataCallback;
			data_callback_object_ptr = object_ptr;
		};

		/// Останавливает вызов callback функции по получению данных от устройства
		void RemoveDataCallback()
		{
			DataCallback = 0;
		};

		/// Устанавливает функцию обратного вызова на получение сообщения от устройства
		/// object_ptr - указатель, который будет передаваться в callback-функцию при её вызове
		/// если messageCallback == 0 то функция обратного вызова не вызывается
		void SetMessageCallback(void (*messageCallback)(CommonPackage &MessagePkg, void* parameter_obj_ptr), void *object_ptr = 0)
		{
			MessageCallback = messageCallback;
			message_callback_object_ptr = object_ptr;
		};

		/// Останавливает вызов callback функции по получению сообщения от устройства
		void RemoveMessageCallback()
		{
			MessageCallback = 0;
		};



		/******************************************************************
		*
		*
		*						Конструктор
		*
		*
		******************************************************************/


		// требует указать место хранения данных
		EEG_Protocol(EEG_Data_Storage *data_storage_ptr, EEG_Device_States_Storage *device_states_storage_ptr, bool __computer_side_mode,
					 void (*protocolErrorCallback)(simple_error, void* obj_ptr) = 0,
                     void (*ackCallback)(long long acked_pack_Id, CommonPackage &AnswerPkg, void* obj_ptr) = 0,
                     void (*ackTimeoutCallback)(long long timeout_pack_Id, int type, int subtype, void* obj_ptr) = 0,
					 void (*dataCallback)(CommonPackage &DataPkg, void* parameter_obj_ptr) = 0,
					 void (*messageCallback)(CommonPackage &MessagePkg, void* parameter_obj_ptr) = 0,
					 void *error_callback_ptr_parameter = 0,
					 void *acknowledge_callback_ptr_parameter = 0,
					 void *acknowledge_timeout_callback_ptr_parameter = 0,
					 void *data_callback_object_ptr_parameter = 0,
					 void *message_callback_object_ptr_parameter = 0
					 ): 
			computer_side_mode(__computer_side_mode),
			BSP(__computer_side_mode)
		{
			this->StreamParserPtr = &BSP;

			// хранилище данных
			channels_data_ptr = data_storage_ptr;
			// хранилище состояний устройства
			device_states_ptr = device_states_storage_ptr;


			all_is_ok = true;

			unsubmitted_pkg_idx = 0;

			pkg_idx_to_process = 0;

			errors_count = 0;

			/// обратный вызов
			ErrorCallback = protocolErrorCallback;
			AckCallback = ackCallback;
			AckTimeoutCallback = ackTimeoutCallback;
			DataCallback = dataCallback;
			MessageCallback = messageCallback;

			/// параметры, передаваемый в функцию обратного вызова
			error_callback_object_ptr = error_callback_ptr_parameter;
			acknowledge_callback_object_ptr = acknowledge_callback_ptr_parameter;
			acknowledge_timeout_callback_object_ptr = acknowledge_timeout_callback_ptr_parameter;
			data_callback_object_ptr = data_callback_object_ptr_parameter;
			message_callback_object_ptr = message_callback_object_ptr_parameter;
        };

	};

	
	/****************************************************************************************
	*
	*				Пока для эстетизма, общий класс посылки
	*
	*****************************************************************************************/

	class EEG_Package: protected EEG_Protocol
	{
	public:
		/*virtual int GetSize()
		{
			return Common_Data_Size;
		};*/
	};

	/****************************************************************************************
	*
	*	Посылки обеспечивают генерацию байт-кода соответствующих комманд и/или 
	*					парсинг значений из принятых данных
	*
	*						ПОСЫЛКИ КОММАНД И ОТВЕТОВ К НИМ
	*
	*****************************************************************************************/

	/// <summary>Рестарт контроллера</summary>
	class EEG_C_Reset: public EEG_Package
	{
	public:

		/// <summary> Создаёт посылку команды C_RESET. Заполняет указанную структуру command_pkg и генерирует 
		/// соответствующую последовательность байт</summary>
		template< class Storage >
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset)
		{
			create_simple_command_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, C_RESET);
		};

		/// <summary>Размер посылки команды C_RESET</summary>
        static inline int C_GetSize()
		{
			return Common_Data_Size;
        };

		/// <summary>Размер посылки ответа на команду C_RESET</summary>
		static inline int A_GetSize()
		{
			return Common_Data_Size;
        };

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool C_IsCorrectSize(int size)
		{
            return C_GetSize() == size;
        };

		/// <summary>Проверяет, возможен ли такой размер посылки ответа</summary>
		static inline bool A_IsCorrectSize(int size)
		{
			return A_GetSize() == size;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool A_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!A_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool C_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!C_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};
	};

	/// <summary>Получение версии и серийного номера устройства</summary>
	class EEG_C_GetVersion: public EEG_Package
	{
	public:
		static const int C_SPECIAL_DATA_SIZE = 0;
		static const int A_SPECIAL_DATA_SIZE = 4;


		/// Генерирует по указанному удресу в указанном хранилище последовательность спец-байтов этой посылки
		/// Начиная с offset
		template< class Storage>
		static inline void write_special_device_data(Storage &out_byte_storage, int offset, 
			unsigned char firmware_version, unsigned char circuity_version, unsigned short serial_number)
		{
			out_byte_storage[offset + 0] = firmware_version;
			out_byte_storage[offset + 1] = circuity_version;
			out_byte_storage[offset + 2] =  (unsigned char)(serial_number>>8);
			out_byte_storage[offset + 3] =  (unsigned char)(serial_number>>0);
		};

	public:
		/// <summary> Создаёт посылку команды C_GET_VERSION. Заполняет указанную структуру command_pkg и генерирует 
		/// соответствующую последовательность байт</summary>
		template< class Storage >
        static inline void CreatePackage(CommonPackage &command_pkg, Storage &place_for_package_data, long long &curr_storage_offset)
		{
			create_simple_command_pkg<Storage>(command_pkg, place_for_package_data, curr_storage_offset, C_GET_VERSION);
		};

		/// <summary> Создаёт посылку ответа устройства на команду C_GET_VERSION. Заполняет указанную структуру command_pkg и генерирует 
		/// соответствующую последовательность байт</summary>
		template< class Storage >
        static inline void CreateDevicePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
												  unsigned char firmware_version, unsigned char circuity_version, unsigned short serial_number)
		{
			write_special_device_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, firmware_version, circuity_version, serial_number); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, ANSWER, A_GET_VERSION,
				A_GET_VERSION, A_SPECIAL_DATA_SIZE, CommonPackage::IN_DIR); // общая часть
		};	

		/// <summary> Из указанной структуры answer_pkg ответа A_GET_VERSION и связанного с ней хранилища байт-последовательности
		/// вытаскивает Версию прошивки мастера</summary>
		template< class Storage >
        static inline unsigned char GetFirmwareVersion(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 0];
		};

		/// <summary> Из указанной структуры answer_pkg ответа A_GET_VERSION и связанного с ней хранилища байт-последовательности
		/// вытаскивает Версию схемотехнического исполнения устройства</summary>
		template< class Storage >
        static inline unsigned char GetCircuityVersion(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 1];
		};

		/// <summary> Из указанной структуры answer_pkg ответа A_GET_VERSION и связанного с ней хранилища байт-последовательности
		/// вытаскивает серийный номер устройства</summary>
		template< class Storage >
        static inline unsigned short GetSerialNumber(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return 
				(package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 2]<<8) |
				(package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 3]);
		};

		/// <summary>Размер посылки команды C_GET_VERSION</summary>
		static inline int C_GetSize()
		{
			return Common_Data_Size + C_SPECIAL_DATA_SIZE;
		}

		/// <summary>Размер посылки команды A_GET_VERSION</summary>
		static inline int A_GetSize()
		{
			return Common_Data_Size + A_SPECIAL_DATA_SIZE;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool C_IsCorrectSize(int size)
		{
			return C_GetSize() == size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки ответа</summary>
		static inline bool A_IsCorrectSize(int size)
		{
			return A_GetSize() == size;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool A_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!A_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool C_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!C_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};
	};

	/// <summary>Прямая установка коммутатора устройства</summary>
	class EEG_C_SetMux: EEG_Package
	{
	public:
		static const int C_SPECIAL_DATA_SIZE = 4;
		static const int A_SPECIAL_DATA_SIZE = 4;
		
	private: 
		/// Генерирует по указанному удресу в указанном хранилище последовательность спец-байтов этой посылки,
		/// начиная с offset
		template < class Storage >
		static inline void write_special_data(Storage &out_byte_storage, int offset, 
			unsigned char U34_state, unsigned char U6_state, unsigned char U24_state, unsigned char U2_state)
		{
			out_byte_storage[offset] = U34_state;
			out_byte_storage[offset + 1] = U6_state;
			out_byte_storage[offset + 2] = U24_state;
			out_byte_storage[offset + 3] = U2_state;
		};

	public:
	
		/// <summary> Создаёт посылку команды C_SET_MUX. Заполняет указанную структуру command_pkg и генерирует 
		/// соответствующую последовательность байт</summary>
		template< class Storage >
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
								  unsigned char U34_state, unsigned char U6_state, unsigned char U24_state, unsigned char U2_state)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, U34_state, U6_state, U24_state, U2_state); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, COMMAND, C_SET_MUX,
				C_SET_MUX, C_SPECIAL_DATA_SIZE, CommonPackage::OUT_DIR); // общая часть
		};

		/// <summary> Создаёт посылку ответа устройства на команду C_SET_MUX. Заполняет указанную структуру command_pkg и генерирует 
		/// соответствующую последовательность байт</summary>
		template< class Storage >
        static inline void CreateDevicePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
								  unsigned char U34_state, unsigned char U6_state, unsigned char U24_state, unsigned char U2_state)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, U34_state, U6_state, U24_state, U2_state); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, ANSWER, A_SET_MUX,
				A_SET_MUX, A_SPECIAL_DATA_SIZE, CommonPackage::IN_DIR); // общая часть
		};	

		/// <summary> Из указанной структуры answer_pkg ответа A_SET_MUX и связанного с ней хранилища байт-последовательности
		/// вытаскивает состояние коммутатора U34</summary>
		template< class Storage >
        static inline unsigned char GetMUX_U34_State(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 0];
		};

		/// <summary> Из указанной структуры answer_pkg ответа A_SET_MUX и связанного с ней хранилища байт-последовательности
		/// вытаскивает состояние коммутатора U6</summary>
		template< class Storage >
        static inline unsigned char GetMUX_U6_State(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 1];
		};

		/// <summary> Из указанной структуры answer_pkg ответа A_SET_MUX и связанного с ней хранилища байт-последовательности
		/// вытаскивает состояние коммутатора U24</summary>
		template< class Storage >
        static inline unsigned char GetMUX_U24_State(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 2];
		};

		/// <summary> Из указанной структуры answer_pkg ответа A_SET_MUX и связанного с ней хранилища байт-последовательности
		/// вытаскивает состояние коммутатора U2</summary>
		template< class Storage >
        static inline unsigned char GetMUX_U2_State(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 3];
		};

		/// <summary>Размер посылки команды C_SET_MUX</summary>
		static inline int C_GetSize()
		{
			return Common_Data_Size + C_SPECIAL_DATA_SIZE;
		}

		/// <summary>Размер посылки команды A_SET_MUX</summary>
		static inline int A_GetSize()
		{
			return Common_Data_Size + A_SPECIAL_DATA_SIZE;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool C_IsCorrectSize(int size)
		{
			return C_GetSize() == size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки ответа</summary>
		static inline bool A_IsCorrectSize(int size)
		{
			return A_GetSize() == size;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool A_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!A_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool C_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!C_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};
	};
	
	/// <summary>Получение версию протокола устройства</summary>
	class EEG_C_GetProtocolVersion: public EEG_Package
	{
	public:
		static const int C_SPECIAL_DATA_SIZE = 0;
		static const int A_SPECIAL_DATA_SIZE = 1;

		/// Генерирует по указанному удресу в указанном хранилище последовательность спец-байтов этой посылки
		/// Начиная с offset
		template< class Storage>
		static inline void write_special_device_data(Storage &out_byte_storage, int offset, 
			unsigned char protocol_version)
		{
			out_byte_storage[offset + 0] = protocol_version;
		};

	public:

		/// <summary> Создаёт посылку команды C_GET_PROTOCOL_VERSION. Заполняет указанную структуру command_pkg и генерирует 
		/// соответствующую последовательность байт</summary>
		template< class Storage >
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset)
		{
			create_simple_command_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, C_GET_PROTOCOL_VERSION);
		};


		/// <summary> Создаёт посылку ответа устройства на команду C_GET_VERSION. Заполняет указанную структуру command_pkg и генерирует 
		/// соответствующую последовательность байт</summary>
		template< class Storage >
        static inline void CreateDevicePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
												  unsigned char protocol_version)
		{
			write_special_device_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, protocol_version); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, ANSWER, A_GET_PROTOCOL_VERSION,
				A_GET_PROTOCOL_VERSION, A_SPECIAL_DATA_SIZE, CommonPackage::IN_DIR); // общая часть
		};	


		/// <summary> Из указанной структуры answer_pkg ответа A_GET_PROTOCOL_VERSION и связанного с ней хранилища байт-последовательности
		/// вытаскивает версию протокола обмена между устройством и компьютером</summary>
		template< class Storage >
        static inline unsigned char GetProtocolVersion(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 0];
		};

		/// <summary>Размер посылки команды C_GET_PROTOCOL_VERSION</summary>
		static inline int C_GetSize()
		{
			return Common_Data_Size + C_SPECIAL_DATA_SIZE;
		}

		/// <summary>Размер посылки команды A_GET_PROTOCOL_VERSION</summary>
		static inline int A_GetSize()
		{
			return Common_Data_Size + A_SPECIAL_DATA_SIZE;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool C_IsCorrectSize(int size)
		{
			return C_GetSize() == size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки ответа</summary>
		static inline bool A_IsCorrectSize(int size)
		{
			return A_GetSize() == size;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool A_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!A_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool C_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!C_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};
	};

	/// <summary>Получение перечня всех возможных частот дискретизации</summary>
	class EEG_C_GetFrequencyTable: public EEG_Package
	{
	public:
		static const int C_SPECIAL_DATA_SIZE = 0;
		static const int A_SPECIAL_DATA_SIZE = -1; // -1, если размер комманды может варьироваться

		/// Генерирует по указанному удресу в указанном хранилище последовательность спец-байтов этой посылки
		/// Начиная с offset
		template< class Storage>
		static inline void write_special_device_data(Storage &out_byte_storage, int offset, 
			vector<unsigned short> &table)
		{
			for (int i = 0; i < table.size(); i++)
			{
				out_byte_storage[offset + 2*i] = (unsigned char)(table[i]>>0);
				out_byte_storage[offset + 2*i + 1] = (unsigned char)(table[i]>>8);
			};
		};

	public:

		template< class Storage >
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset)
		{
			create_simple_command_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, C_GET_FREQUENCY_TABLE);
		};

		/// <summary> Создаёт посылку ответа устройства на команду C_GET_FREQUENCY_TABLE. Заполняет указанную структуру command_pkg и генерирует 
		/// соответствующую последовательность байт</summary>
		template< class Storage >
        static inline void CreateDevicePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
												  vector<unsigned short> &table)
		{
			int special_data_size = table.size() * 2;

			write_special_device_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, table); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, ANSWER, A_GET_FREQUENCY_TABLE,
				A_GET_FREQUENCY_TABLE, special_data_size, CommonPackage::IN_DIR); // общая часть
		};	

		/// <summary>Размер посылки команды C_GET_FREQUENCY_TABLE</summary>
		static inline int C_GetSize()
		{
			return Common_Data_Size + C_SPECIAL_DATA_SIZE;
		}

		/// <summary>Размер посылки команды A_GET_FREQUENCY_TABLE по умолчанию не известен</summary>
		static inline int A_GetSize()
		{
			return -1;
		};

		/// <summary>Размер посылки команды A_GET_FREQUENCY_TABLE для указанного количества частот</summary>
		static inline int A_GetSize(int freq_count)
		{
			return Common_Data_Size + 2 * freq_count;
		};

		/// <summary>Возвращает количество частот, пересичленных прибором, как возможные</summary>
        static inline unsigned char A_GetFrequenciesCount(CommonPackage &answer_pkg)
		{
			return (answer_pkg.package_data_length - Common_Data_Size)/2;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool C_IsCorrectSize(int size)
		{
			return C_GetSize() == size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки ответа. size - полный размер посылки</summary>
		static inline bool A_IsCorrectSize(int size)
		{
			if ((size - Common_Data_Size) % 2 == 0) return true;
			else return false;
		};

		/// <summary>Возвращает массив, содержащий частоты, которые поддерживает прибор (в семплах в секунду)</summary>
		template< class Storage >
        static inline vector<unsigned short> A_GetFrequenciesList(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
            vector<unsigned short> out(A_GetFrequenciesCount(answer_pkg));

			for (int i = 0; i < out.size(); i++)
			{
                out[i] = (((unsigned short)(package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 2*i + 1]))<<8) |
					     package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 2*i + 0];
			};

			return out;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool A_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!A_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool C_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!C_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};
	};

	/// <summary>Задать указанную частоту дискретизации (частота указана своим индексом в таблице частот, возвращаемой прибором
	/// по комманде C_GET_FREQUENCY_TABLE</summary>
	class EEG_C_SetSamplingFrequency: EEG_Package
	{
	public:
		static const int C_SPECIAL_DATA_SIZE = 1;
		static const int A_SPECIAL_DATA_SIZE = 1;
		
	private: 
		/// Генерирует по указанному удресу в указанном хранилище последовательность спец-байтов этой посылки
		/// Начиная с offset, который обновляется и после отработки указывает на следующий после последнего записанного
		/// байта байт.
		template < class Storage >
		static inline void write_special_data(Storage &out_byte_storage, int offset, unsigned char Index_of_Frequency_in_Table)
		{
			out_byte_storage[offset] = Index_of_Frequency_in_Table;
		};

	public:
	
		template< class Storage >
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
								  unsigned char Index_of_Frequency_in_Table)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, Index_of_Frequency_in_Table); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, COMMAND, C_SET_SAMPLING_FREQUENCY,
				C_SET_SAMPLING_FREQUENCY, C_SPECIAL_DATA_SIZE, CommonPackage::OUT_DIR); // общая часть
		};

		template< class Storage >
        static inline void CreateDevicePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
								  unsigned char Index_of_Frequency_in_Table)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, Index_of_Frequency_in_Table); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, ANSWER, A_SET_SAMPLING_FREQUENCY,
				A_SET_SAMPLING_FREQUENCY, A_SPECIAL_DATA_SIZE, CommonPackage::OUT_DIR); // общая часть
		};

		/// <summary>Размер посылки команды C_SET_SAMPLING_FREQUENCY</summary>
		static inline int C_GetSize()
		{
			return Common_Data_Size + C_SPECIAL_DATA_SIZE;
		}

		/// <summary>Размер посылки команды A_SET_SAMPLING_FREQUENCY</summary>
		static inline int A_GetSize()
		{
			return Common_Data_Size + A_SPECIAL_DATA_SIZE;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool C_IsCorrectSize(int size)
		{
			return C_GetSize() == size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки ответа</summary>
		static inline bool A_IsCorrectSize(int size)
		{
			return A_GetSize() == size;
		};

		/// <summary>Возвращает индекс частоты, возвращённой прибором</summary>
		template< class Storage >
        static inline unsigned short A_GetFrequencySetIndex(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 0];
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool A_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!A_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool C_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!C_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};
	};
				
	/// <summary> Установить режим работы всех каналов с возможным выбором занулённых</summary>
	class EEG_C_SetChannelsMode: EEG_Package
	{
	public:
		static const int C_SPECIAL_DATA_SIZE = 2;
		static const int A_SPECIAL_DATA_SIZE = 2;
		//enum  EEG_ChannelsMode {CH_MODE_CALIBRATION = 0x00, CH_MODE_IMPEDANCE_MBN = 0x01, CH_MODE_EEG = 0x02, CH_MODE_IMPEDANCE_ALTONICS = 0x03};
		
	private: 
		/// Генерирует по указанному удресу в указанном хранилище последовательность спец-байтов этой посылки
		/// Начиная с offset, который обновляется и после отработки указывает на следующий после последнего записанного
		/// байта байт.
		template < class Storage >
		static inline void write_special_data(Storage &out_byte_storage, int offset, 
			MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_ChannelsMode channels_mode, 
			bool ch0_active, bool ch1_active, bool ch2_active, bool ch3_active)
		{
			// первый байт задаёт режим (проще говоря, устанавливает коммутатор)
			out_byte_storage[offset] = (unsigned char)(channels_mode);

			// второй байт, управляющий занулением каналов
			unsigned char D1 = 0xF0;

			if (!ch0_active) D1 = D1 | (1<<0);
			if (!ch1_active) D1 = D1 | (1<<1);
			if (!ch2_active) D1 = D1 | (1<<2);
			if (!ch3_active) D1 = D1 | (1<<3);

			out_byte_storage[offset + 1] = D1;
		};

	public:
	
		template< class Storage >
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
								  MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_ChannelsMode ChannelsMode, bool ch0_active, bool ch1_active, bool ch2_active, bool ch3_active)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, ChannelsMode, ch0_active, ch1_active, ch2_active, ch3_active); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, COMMAND, C_SET_CHANNELS_MODE,
				C_SET_CHANNELS_MODE, C_SPECIAL_DATA_SIZE, CommonPackage::OUT_DIR); // общая часть
		};

		template< class Storage >
        static inline void CreateDevicePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
								  MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_ChannelsMode ChannelsMode, bool ch0_active, bool ch1_active, bool ch2_active, bool ch3_active)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, ChannelsMode, ch0_active, ch1_active, ch2_active, ch3_active); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, ANSWER, A_SET_CHANNELS_MODE,
				A_SET_CHANNELS_MODE, A_SPECIAL_DATA_SIZE, CommonPackage::IN_DIR); // общая часть
		};

		/// <summary>Размер посылки команды C_SET_CHANNELS_MODE</summary>
		static inline int C_GetSize()
		{
			return Common_Data_Size + C_SPECIAL_DATA_SIZE;
		}

		/// <summary>Размер посылки команды A_SET_CHANNELS_MODE</summary>
		static inline int A_GetSize()
		{
			return Common_Data_Size + A_SPECIAL_DATA_SIZE;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool C_IsCorrectSize(int size)
		{
			return C_GetSize() == size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки ответа</summary>
		static inline bool A_IsCorrectSize(int size)
		{
			return A_GetSize() == size;
		};

		/// <summary>Возвращает вернувшийся в ответе прибора режим работы каналов</summary>
		template< class Storage >
		static inline MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_ChannelsMode A_GetChannelsModeSet(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_ChannelsMode(package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 0]);
		};

		/// <summary>Возвращает вернувшуюся в ответе прибора активность 0-го канала</summary>
		template< class Storage >
		static inline bool A_IsCh0_activeSet(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return !((package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 1]) & (1<<0));
		};

		/// <summary>Возвращает вернувшуюся в ответе прибора активность 1-го канала</summary>
		template< class Storage >
		static inline bool A_IsCh1_activeSet(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return !((package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 1]) & (1<<1));
		};

		/// <summary>Возвращает вернувшуюся в ответе прибора активность 2-го канала</summary>
		template< class Storage >
		static inline bool A_IsCh2_activeSet(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return !((package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 1]) & (1<<2));
		};

		/// <summary>Возвращает вернувшуюся в ответе прибора активность 3-го канала</summary>
		template< class Storage >
		static inline bool A_IsCh3_activeSet(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return !((package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 1]) & (1<<3));
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool A_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!A_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			// проверяем байт режимма
			switch(bytes[special_data_start_idx])
			{
			case MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::CH_MODE_CALIBRATION: break;
			case MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::CH_MODE_IMPEDANCE_MBN: break;
			case MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::CH_MODE_EEG: break;
			case MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::CH_MODE_IMPEDANCE_ALTONICS: break;
			default: return false;
			};

			// проверяем байт установки занулённых каналов
			if (((0xF0) & bytes[special_data_start_idx + 1]) != 0xF0)
				return false;

			return true;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool C_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!C_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};
	};

	/// <summary> Установить форму и частоту калибровочного сигнала</summary>
	class EEG_C_SetCalibrationShape: EEG_Package
	{
	public:
		static const int C_SPECIAL_DATA_SIZE = 5;
		static const int A_SPECIAL_DATA_SIZE = 5;
		//enum  EEG_Calibration_Signal_Shape {EEG_CAL_SHAPE_SINE = 0x00, EEG_CAL_SHAPE_RECTANGLE = 0x01, EEG_CAL_SHAPE_TRIANGLE = 0x02};	
		
	private: 
		/// Генерирует по указанному удресу в указанном хранилище последовательность спец-байтов этой посылки
		/// Начиная с offset, который обновляется и после отработки указывает на следующий после последнего записанного
		/// байта байт.
		template < class Storage >
		static inline void write_special_data(Storage &out_byte_storage, int offset, MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_Calibrator_Shape cal_shape, 
			float frequency_Hz)
		{
			static const float EEG_CALIBRATION_SIGNAL_FREQUENCY_STEP_HZ = 0.01;

			// первый байт задаёт форму сигнала
			out_byte_storage[offset] = (unsigned char)(cal_shape);

			// если частота меньше нуля...
			if (frequency_Hz < 0.0) frequency_Hz = 0.0;
			// переводим частоту в частоту, кратную базовой
			unsigned int F = frequency_Hz / EEG_CALIBRATION_SIGNAL_FREQUENCY_STEP_HZ;

			// байты [1; 4], задают частоту сигнала, сначала млаюшие байты, потом старшие
			out_byte_storage[offset + 1] = (F>>0) & 0xFF;
			out_byte_storage[offset + 2] = (F>>8) & 0xFF;
			out_byte_storage[offset + 3] = (F>>16) & 0xFF;
			out_byte_storage[offset + 4] = (F>>24) & 0xFF;
		};

	public:
	
		template< class Storage >
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
								  MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_Calibrator_Shape cal_shape, float frequency_Hz)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, cal_shape, frequency_Hz); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, COMMAND, C_SET_CALIBRATION_SHAPE,
				C_SET_CALIBRATION_SHAPE, C_SPECIAL_DATA_SIZE, CommonPackage::OUT_DIR); // общая часть
		};

		template< class Storage >
        static inline void CreateDevicePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
								  MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_Calibrator_Shape cal_shape, float frequency_Hz)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, cal_shape, frequency_Hz); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, ANSWER, A_SET_CALIBRATION_SHAPE,
				A_SET_CALIBRATION_SHAPE, A_SPECIAL_DATA_SIZE, CommonPackage::IN_DIR); // общая часть
		};

		/// <summary>Размер посылки команды C_SET_CALIBRATION_SHAPE</summary>
		static inline int C_GetSize()
		{
			return Common_Data_Size + C_SPECIAL_DATA_SIZE;
		}

		/// <summary>Размер посылки команды A_SET_CALIBRATION_SHAPE</summary>
		static inline int A_GetSize()
		{
			return Common_Data_Size + A_SPECIAL_DATA_SIZE;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool C_IsCorrectSize(int size)
		{
			return C_GetSize() == size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки ответа</summary>
		static inline bool A_IsCorrectSize(int size)
		{
			return A_GetSize() == size;
		};

		/// <summary>Возвращает вернувшуюся в ответе прибора форму калибровочного сигнала</summary>
		template< class Storage >
		static inline MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_Calibrator_Shape A_GetCalibrationSignalShape_Set(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_Calibrator_Shape(package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 0]);
		};

		/// <summary>Возвращает вернувшуюся в ответе прибора частоту калибровочного сигнала</summary>
		template< class Storage >
		static inline float A_GetCalibrationFreq_Set(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			unsigned int coeff = 0x00;

			coeff |= package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 4];
			coeff <<= 8;
			coeff |= package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 3];
			coeff <<= 8;
			coeff |= package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 2];
			coeff <<= 8;
			coeff |= package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 1];

			return float(coeff * MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_CALIBRATION_SIGNAL_FREQUENCY_STEP_HZ);
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool A_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!A_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			// проверяем байт режимма
			switch(bytes[special_data_start_idx])
			{
			case MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_CAL_SHAPE_SINE: break;
			case MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_CAL_SHAPE_RECTANGLE: break;
			case MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_CAL_SHAPE_TRIANGLE: break;
			default: return false;
			};

			return true;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool C_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!C_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};
	};
			
	/// <summary> Запустить генерацию калибровочного сигнала выбранной формы и частоты </summary>
	class EEG_C_StartCalibration: public EEG_Package
	{
	public:
		static const int C_SPECIAL_DATA_SIZE = 0;
		static const int A_SPECIAL_DATA_SIZE = 0;

	public:
		template< class Storage >
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset)
		{
			create_simple_command_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, C_START_CALIBRATION);
		};

		template< class Storage >
        static inline void CreateDevicePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset)
		{
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, ANSWER, A_START_CALIBRATION,
				A_START_CALIBRATION, A_SPECIAL_DATA_SIZE, CommonPackage::IN_DIR); // общая часть
		};

		/// <summary>Размер посылки команды C_START_CALIBRATION</summary>
		static inline int C_GetSize()
		{
			return Common_Data_Size;
		}

		/// <summary>Размер посылки команды A_START_CALIBRATION</summary>
		static inline int A_GetSize()
		{
			return Common_Data_Size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool C_IsCorrectSize(int size)
		{
			return C_GetSize() == size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки ответа</summary>
		static inline bool A_IsCorrectSize(int size)
		{
			return A_GetSize() == size;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool A_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!A_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool C_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!C_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};
	};

	/// <summary> Остановить генерацию калибровочного сигнала выбранной формы и частоты </summary>
	class EEG_C_StopCalibration: public EEG_Package
	{
	public:
		static const int C_SPECIAL_DATA_SIZE = 0;
		static const int A_SPECIAL_DATA_SIZE = 0;

	public:
		template< class Storage >
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset)
		{
			create_simple_command_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, C_STOP_CALIBRATION);
		};

		template< class Storage >
        static inline void CreateDevicePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset)
		{
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, ANSWER, A_STOP_CALIBRATION,
				A_STOP_CALIBRATION, A_SPECIAL_DATA_SIZE, CommonPackage::IN_DIR); // общая часть
		};

		/// <summary>Размер посылки команды C_STOP_CALIBRATION</summary>
		static inline int C_GetSize()
		{
			return Common_Data_Size;
		}

		/// <summary>Размер посылки команды A_STOP_CALIBRATION</summary>
		static inline int A_GetSize()
		{
			return Common_Data_Size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool C_IsCorrectSize(int size)
		{
			return C_GetSize() == size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки ответа</summary>
		static inline bool A_IsCorrectSize(int size)
		{
			return A_GetSize() == size;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool A_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!A_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool C_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!C_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};
	};

	/// <summary> Запустить опрос выбранных каналов и трансляцию данных в порт </summary>
	class EEG_C_StartTranslation: public EEG_Package
	{
	public:
		static const int C_SPECIAL_DATA_SIZE = 0;
		static const int A_SPECIAL_DATA_SIZE = 0;

	public:
		template< class Storage >
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset)
		{
			create_simple_command_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, C_START_TRANSLATION);
		};

		template< class Storage >
        static inline void CreateDevicePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset)
		{
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, ANSWER, A_START_TRANSLATION,
				A_START_TRANSLATION, A_SPECIAL_DATA_SIZE, CommonPackage::IN_DIR); // общая часть
		};

		/// <summary>Размер посылки команды C_START_TRANSLATION</summary>
		static inline int C_GetSize()
		{
			return Common_Data_Size;
		}

		/// <summary>Размер посылки команды A_START_TRANSLATION</summary>
		static inline int A_GetSize()
		{
			return Common_Data_Size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool C_IsCorrectSize(int size)
		{
			return C_GetSize() == size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки ответа</summary>
		static inline bool A_IsCorrectSize(int size)
		{
			return A_GetSize() == size;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool A_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!A_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool C_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!C_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};
	};

	/// <summary> Остановить опрос выбранных каналов и трансляцию данных в порт </summary>
	class EEG_C_StopTranslation: public EEG_Package
	{
	public:
		static const int C_SPECIAL_DATA_SIZE = 0;
		static const int A_SPECIAL_DATA_SIZE = 0;

	public:
		template< class Storage >
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset)
		{
			create_simple_command_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, C_STOP_TRANSLATION);
		};

		template< class Storage >
        static inline void CreateDevicePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset)
		{
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, ANSWER, A_STOP_TRANSLATION,
				A_STOP_TRANSLATION, A_SPECIAL_DATA_SIZE, CommonPackage::IN_DIR); // общая часть
		};

		/// <summary>Размер посылки команды C_STOP_TRANSLATION</summary>
		static inline int C_GetSize()
		{
			return Common_Data_Size;
		}

		/// <summary>Размер посылки команды A_STOP_TRANSLATION</summary>
		static inline int A_GetSize()
		{
			return Common_Data_Size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool C_IsCorrectSize(int size)
		{
			return C_GetSize() == size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки ответа</summary>
		static inline bool A_IsCorrectSize(int size)
		{
			return A_GetSize() == size;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool A_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!A_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool C_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!C_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};
	};
									
	/// <summary> Установить режим подстветки каналов </summary>							
	class EEG_C_SetChannelLeds: EEG_Package
	{
	public:
		static const int C_SPECIAL_DATA_SIZE = 2;
		static const int A_SPECIAL_DATA_SIZE = 2;
		
	private: 
		/// Генерирует по указанному удресу в указанном хранилище последовательность спец-байтов этой посылки
		/// Начиная с offset, который обновляется и после отработки указывает на следующий после последнего записанного
		/// байта байт.
		template < class Storage >
		static inline void write_special_data(Storage &out_byte_storage, int offset, unsigned char ChannelID, 
			MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::EEG_Led_State Color)
		{
			out_byte_storage[offset] = ChannelID;
			out_byte_storage[offset + 1] = (unsigned char)(Color);
		};

	public:
	
		template< class Storage >
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
								  unsigned char ChannelID, MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::EEG_Led_State Color)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, ChannelID, Color); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, COMMAND, C_SET_CHANNEL_LEDS,
				C_SET_CHANNEL_LEDS, C_SPECIAL_DATA_SIZE, CommonPackage::OUT_DIR); // общая часть
		};

		template< class Storage >
        static inline void CreateDevicePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
								  unsigned char ChannelID, MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::EEG_Led_State Color)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, ChannelID, Color); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, ANSWER, A_SET_CHANNEL_LEDS,
				A_SET_CHANNEL_LEDS, A_SPECIAL_DATA_SIZE, CommonPackage::IN_DIR); // общая часть
		};

		/// <summary>Размер посылки команды C_SET_CHANNEL_LEDS</summary>
		static inline int C_GetSize()
		{
			return Common_Data_Size + C_SPECIAL_DATA_SIZE;
		}

		/// <summary>Размер посылки команды A_SET_CHANNEL_LEDS</summary>
		static inline int A_GetSize()
		{
			return Common_Data_Size + A_SPECIAL_DATA_SIZE;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool C_IsCorrectSize(int size)
		{
			return C_GetSize() == size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки ответа</summary>
		static inline bool A_IsCorrectSize(int size)
		{
			return A_GetSize() == size;
		};

		/// <summary>Возвращает вернувшуюся в ответе прибора номер канала</summary>
		template< class Storage >
        static inline unsigned char A_GetChannelNumber_Set(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 0];
		};

		/// <summary>Возвращает вернувшуюся в ответе прибора цвет индикатора канала</summary>
		template< class Storage >
		static inline MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::EEG_Led_State A_GetChannelLedColor_Set(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::EEG_Led_State(package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 1]);
		};


		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool A_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!A_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			// проверяем номер канала
			if (bytes[special_data_start_idx] >= 6) return false;

			// проверяем байт цвета
			switch(bytes[special_data_start_idx + 1])
			{
			case MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_OFF: break;
			case MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_RED: break;
			case MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_GREEN: break;
			case MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_ORANGE: break;
			default: return false;
			};

			return true;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool C_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!C_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};
	};

	/// <summary> Установить усиление в канале </summary>							
	class EEG_C_SetAmplification: EEG_Package
	{
	public:
		static const int C_SPECIAL_DATA_SIZE = 4;
		static const int A_SPECIAL_DATA_SIZE = 4;
		
	private: 
		/// Генерирует по указанному удресу в указанном хранилище последовательность спец-байтов этой посылки
		/// Начиная с offset, который обновляется и после отработки указывает на следующий после последнего записанного
		/// байта байт.
		template < class Storage >
		static inline void write_special_data(Storage &out_byte_storage, int offset, 
			EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch0_ampl, 
			EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch1_ampl, 
			EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch2_ampl, 
			EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch3_ampl)
		{
			out_byte_storage[offset] = (unsigned char)(ch0_ampl);
			out_byte_storage[offset + 1] = (unsigned char)(ch1_ampl);
			out_byte_storage[offset + 2] = (unsigned char)(ch2_ampl);
			out_byte_storage[offset + 3] = (unsigned char)(ch3_ampl);
		};

	public:
	
		template< class Storage >
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
				EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch0_ampl, 
				EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch1_ampl, 
				EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch2_ampl, 
				EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch3_ampl)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, ch0_ampl, ch1_ampl, ch2_ampl, ch3_ampl); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, COMMAND, C_SET_AMPLIFICATION,
				C_SET_AMPLIFICATION, C_SPECIAL_DATA_SIZE, CommonPackage::OUT_DIR); // общая часть
		};

		template< class Storage >
        static inline void CreateDevicePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
				EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch0_ampl, 
				EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch1_ampl, 
				EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch2_ampl, 
				EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch3_ampl)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, ch0_ampl, ch1_ampl, ch2_ampl, ch3_ampl); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, ANSWER, A_SET_AMPLIFICATION,
				A_SET_AMPLIFICATION, A_SPECIAL_DATA_SIZE, CommonPackage::IN_DIR); // общая часть
		};
		
		/// <summary>Размер посылки команды C_SET_AMPLIFICATION</summary>
		static inline int C_GetSize()
		{
			return Common_Data_Size + C_SPECIAL_DATA_SIZE;
		}

		/// <summary>Размер посылки команды A_SET_AMPLIFICATION</summary>
		static inline int A_GetSize()
		{
			return Common_Data_Size + A_SPECIAL_DATA_SIZE;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool C_IsCorrectSize(int size)
		{
			return C_GetSize() == size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки ответа</summary>
		static inline bool A_IsCorrectSize(int size)
		{
			return A_GetSize() == size;
		};

		
		/// <summary>Возвращает вернувшуюся в ответе прибора усиление в канале</summary>
		template< class Storage >
		static inline EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification A_GetChannel0_Amplification_Set(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification(package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 0]);
		};

		/// <summary>Возвращает вернувшуюся в ответе прибора усиление в канале</summary>
		template< class Storage >
		static inline EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification A_GetChannel1_Amplification_Set(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification(package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 1]);
		};

		/// <summary>Возвращает вернувшуюся в ответе прибора усиление в канале</summary>
		template< class Storage >
		static inline EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification A_GetChannel2_Amplification_Set(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification(package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 2]);
		};

		/// <summary>Возвращает вернувшуюся в ответе прибора усиление в канале</summary>
		template< class Storage >
		static inline EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification A_GetChannel3_Amplification_Set(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification(package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 3]);
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool A_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!A_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			for (int i = 0; i <= 3; i++)
			{
				// проверяем байт цвета
				switch(bytes[special_data_start_idx + i])
				{
				case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_1x: break;
				case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_2x: break;
				case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_3x: break;
				case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_4x: break;
				case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_6x: break;
				case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_8x: break;
				case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_12x: break;
				default: return false;
				};
			};		

			return true;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool C_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!C_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};
	};

	/// <summary> Установить усиление в канале </summary>							
	class EEG_C_SetADSMux: EEG_Package
	{
	public:
		static const int C_SPECIAL_DATA_SIZE = 4;
		static const int A_SPECIAL_DATA_SIZE = 4;
		
	private: 
		/// Генерирует по указанному удресу в указанном хранилище последовательность спец-байтов этой посылки
		/// Начиная с offset
		template < class Storage >
		static inline void write_special_data(Storage &out_byte_storage, int offset, 
			MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode ch0_mode, 
			MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode ch1_mode, 
			MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode ch2_mode, 
			MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode ch3_mode)
		{
			out_byte_storage[offset] = (unsigned char)(ch0_mode);
			out_byte_storage[offset + 1] = (unsigned char)(ch1_mode);
			out_byte_storage[offset + 2] = (unsigned char)(ch2_mode);
			out_byte_storage[offset + 3] = (unsigned char)(ch3_mode);
		};

	public:
	
		template< class Storage >
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode ch0_mode, 
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode ch1_mode, 
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode ch2_mode, 
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode ch3_mode)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, ch0_mode, ch1_mode, ch2_mode, ch3_mode); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, COMMAND, C_SET_ADS_MUX,
				C_SET_ADS_MUX, C_SPECIAL_DATA_SIZE, CommonPackage::OUT_DIR); // общая часть
		};

		template< class Storage >
        static inline void CreateDevicePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode ch0_mode, 
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode ch1_mode, 
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode ch2_mode, 
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode ch3_mode)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, ch0_mode, ch1_mode, ch2_mode, ch3_mode); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, ANSWER, A_SET_ADS_MUX,
				A_SET_ADS_MUX, A_SPECIAL_DATA_SIZE, CommonPackage::IN_DIR); // общая часть
		};

		/// <summary>Размер посылки команды C_SET_ADS_MUX</summary>
		static inline int C_GetSize()
		{
			return Common_Data_Size + C_SPECIAL_DATA_SIZE;
		}

		/// <summary>Размер посылки команды A_SET_ADS_MUX</summary>
		static inline int A_GetSize()
		{
			return Common_Data_Size + A_SPECIAL_DATA_SIZE;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool C_IsCorrectSize(int size)
		{
			return C_GetSize() == size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки ответа</summary>
		static inline bool A_IsCorrectSize(int size)
		{
			return A_GetSize() == size;
		};

		/// <summary>Возвращает вернувшуюся в ответе прибора установку коммутатора канала</summary>
		template< class Storage >
		static inline MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode A_GetChannel_0_MUX_Set(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode(package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 0]);
		};

		/// <summary>Возвращает вернувшуюся в ответе прибора установку коммутатора канала</summary>
		template< class Storage >
		static inline MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode A_GetChannel_1_MUX_Set(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode(package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 1]);
		};

		/// <summary>Возвращает вернувшуюся в ответе прибора установку коммутатора канала</summary>
		template< class Storage >
		static inline MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode A_GetChannel_2_MUX_Set(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode(package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 2]);
		};

		/// <summary>Возвращает вернувшуюся в ответе прибора установку коммутатора канала</summary>
		template< class Storage >
		static inline MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode A_GetChannel_3_MUX_Set(CommonPackage &answer_pkg, Storage &package_bytes_storage)
		{
			return MBN_EEG::EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode(package_bytes_storage[answer_pkg.first_byte_stream_offset + Header_Data_Size + 3]);
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool A_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!A_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;	

			return true;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool C_IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!C_IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};
	};

	/****************************************************************************************
	*
	*									ПОСЫЛКИ ДАННЫХ
	*
	*****************************************************************************************/

	/// <summary> Обычные данные без стимула </summary>	
	class EEG_D_UsualData: EEG_Package
	{
	public:
		static const int SPECIAL_DATA_SIZE = 12;

	private: 

		/// Генерирует по указанному удресу в указанном хранилище последовательность спец-байтов этой посылки
		/// Начиная с offset
		template< class Storage>
		static inline void write_special_data(Storage &out_byte_storage, int offset, 
            int ch0val, int ch1val, int ch2val, int ch3val)
		{
            out_byte_storage[offset + 0] =  (unsigned char)(ch0val>>16);
            out_byte_storage[offset + 1] =  (unsigned char)(ch0val>>8);
            out_byte_storage[offset + 2] =  (unsigned char)(ch0val>>0);
			offset+=3;
            out_byte_storage[offset + 0] =  (unsigned char)(ch1val>>16);
            out_byte_storage[offset + 1] =  (unsigned char)(ch1val>>8);
            out_byte_storage[offset + 2] =  (unsigned char)(ch1val>>0);
			offset+=3;
            out_byte_storage[offset + 0] =  (unsigned char)(ch2val>>16);
            out_byte_storage[offset + 1] =  (unsigned char)(ch2val>>8);
            out_byte_storage[offset + 2] =  (unsigned char)(ch2val>>0);
			offset+=3;
            out_byte_storage[offset + 0] =  (unsigned char)(ch3val>>16);
            out_byte_storage[offset + 1] =  (unsigned char)(ch3val>>8);
            out_byte_storage[offset + 2] =  (unsigned char)(ch3val>>0);
			offset+=3;
		};

	public:
	
		template< class Storage>
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
                unsigned int ch0val, unsigned int ch1val, unsigned int ch2val, unsigned int ch3val)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, ch0val, ch1val, ch2val, ch3val); // спец-данные
			create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, DATA, D_USUAL_CHANNELS_DATA_WITHOUT_STIMUL,
								D_USUAL_CHANNELS_DATA_WITHOUT_STIMUL, SPECIAL_DATA_SIZE, CommonPackage::IN_DIR); // общая часть
		};

		/// Записывает в указанные переменные значения отсчётов каналов D0, D1, D2, D3
		/// Данные АЦП в посылке хранятся в дополнительном коде, поэтому, собсна и делаем не только склейку байт,
		/// но и расширение с 3-х байтового д.к. до 4-х байтового д.к.
		template< class Storage>
		static inline void GetChannelsData(CommonPackage &pkg, Storage &package_data,
                                          int &ch0val, int &ch1val, int &ch2val, int &ch3val)
		{
			int offset = 0;

			ch0val = 0x00000000; 
			ch0val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0];
			ch0val <<= 8;
			ch0val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 1];
			ch0val <<= 8;
			ch0val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 2];
			// если число отрицательное (старший бит в д.к. равен 1) заполняем пробелы 1
			if (package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0] & (0x80))
				ch0val |= 0xFF000000;
			offset += 3;

			ch1val = 0x00000000; 
			ch1val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0];
			ch1val <<= 8;
			ch1val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 1];
			ch1val <<= 8;
			ch1val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 2];
			// если число отрицательное (старший бит в д.к. равен 1) заполняем пробелы 1
			if (package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0] & (0x80))
				ch1val |= 0xFF000000;
			offset += 3;

			ch2val = 0x00000000; 
			ch2val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0];
			ch2val <<= 8;
			ch2val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 1];
			ch2val <<= 8;
			ch2val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 2];
			// если число отрицательное (старший бит в д.к. равен 1) заполняем пробелы 1
			if (package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0] & (0x80))
				ch2val |= 0xFF000000;
			offset += 3;

			ch3val = 0x00000000; 
			ch3val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0];
			ch3val <<= 8;
			ch3val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 1];
			ch3val <<= 8;
			ch3val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 2];
			// если число отрицательное (старший бит в д.к. равен 1) заполняем пробелы 1
			if (package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0] & (0x80))
				ch3val |= 0xFF000000;
		};

		/// <summary>Размер посылки данных D_USUAL_CHANNELS_DATA_WITHOUT_STIMUL</summary>
		static inline int GetSize()
		{
			return Common_Data_Size + SPECIAL_DATA_SIZE;
		}

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool IsCorrectSize(int size)
		{
			if (size == Common_Data_Size + SPECIAL_DATA_SIZE)
			{
                //cout<<"IsCorrectSize returned true";
				return true;
            }
			else 
			{
                //cout<<"IsCorrectSize returned false";
				return false;
			};
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;	

			return true;
		};
	};

	/// <summary> Обычные данные но со стимулом </summary>	
	class EEG_D_UsualData_With_Stimul: EEG_Package
	{
	public:
		static const int SPECIAL_DATA_SIZE = 12;
		
	private: 

		/// Генерирует по указанному удресу в указанном хранилище последовательность спец-байтов этой посылки
		/// Начиная с offset
		template< class Storage>
		static inline void write_special_data(Storage &out_byte_storage, int offset, 
            int ch0val, int ch1val, int ch2val, int ch3val)
		{
            out_byte_storage[offset + 0] =  (unsigned char)(ch0val>>16);
            out_byte_storage[offset + 1] =  (unsigned char)(ch0val>>8);
            out_byte_storage[offset + 2] =  (unsigned char)(ch0val>>0);
			offset+=3;
            out_byte_storage[offset + 0] =  (unsigned char)(ch1val>>16);
            out_byte_storage[offset + 1] =  (unsigned char)(ch1val>>8);
            out_byte_storage[offset + 2] =  (unsigned char)(ch1val>>0);
			offset+=3;
            out_byte_storage[offset + 0] =  (unsigned char)(ch2val>>16);
            out_byte_storage[offset + 1] =  (unsigned char)(ch2val>>8);
            out_byte_storage[offset + 2] =  (unsigned char)(ch2val>>0);
			offset+=3;
            out_byte_storage[offset + 0] =  (unsigned char)(ch3val>>16);
            out_byte_storage[offset + 1] =  (unsigned char)(ch3val>>8);
            out_byte_storage[offset + 2] =  (unsigned char)(ch3val>>0);
			offset+=3;
		};

	public:
	
		template< class Storage>
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
                unsigned int ch0val, unsigned int ch1val, unsigned int ch2val, unsigned int ch3val)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, ch0val, ch1val, ch2val, ch3val); // спец-данные
            create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, DATA, D_USUAL_CHANNELS_DATA_WITH_STIMUL,
                                D_USUAL_CHANNELS_DATA_WITH_STIMUL, SPECIAL_DATA_SIZE); // общая часть
		};

		/// Записывает в указанные переменные значения отсчётов каналов D0, D1, D2, D3
		template< class Storage>
		static inline void GetChannelsData(CommonPackage &pkg, Storage &package_data,
                                          int &ch0val, int &ch1val, int &ch2val, int &ch3val)
		{
			int offset = 0;

			ch0val = 0x00000000; 
			ch0val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0];
			ch0val <<= 8;
			ch0val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 1];
			ch0val <<= 8;
			ch0val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 2];
			// если число отрицательное (старший бит в д.к. равен 1) заполняем пробелы 1
			if (package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0] & (0x80))
				ch0val |= 0xFF000000;
			offset += 3;

			ch1val = 0x00000000; 
			ch1val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0];
			ch1val <<= 8;
			ch1val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 1];
			ch1val <<= 8;
			ch1val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 2];
			// если число отрицательное (старший бит в д.к. равен 1) заполняем пробелы 1
			if (package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0] & (0x80))
				ch1val |= 0xFF000000;
			offset += 3;

			ch2val = 0x00000000; 
			ch2val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0];
			ch2val <<= 8;
			ch2val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 1];
			ch2val <<= 8;
			ch2val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 2];
			// если число отрицательное (старший бит в д.к. равен 1) заполняем пробелы 1
			if (package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0] & (0x80))
				ch2val |= 0xFF000000;
			offset += 3;

			ch3val = 0x00000000; 
			ch3val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0];
			ch3val <<= 8;
			ch3val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 1];
			ch3val <<= 8;
			ch3val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 2];
			// если число отрицательное (старший бит в д.к. равен 1) заполняем пробелы 1
			if (package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0] & (0x80))
				ch3val |= 0xFF000000;
		};

		/// <summary>Размер посылки данных D_USUAL_CHANNELS_DATA_WITH_STIMUL</summary>
		static inline int GetSize()
		{
			return Common_Data_Size + SPECIAL_DATA_SIZE;
		}

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool IsCorrectSize(int size)
		{
			if (size == Common_Data_Size + SPECIAL_DATA_SIZE)
				return true;
			else 
				return false;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;	

			return true;
		};
	};
	
	/// <summary> Данные во время измерения импеданса МБН </summary>	
	class EEG_D_ImpedanceMBN: EEG_Package
	{
	public:
		static const int SAMPLES_COUNT = 40;
		static const int SPECIAL_DATA_SIZE = SAMPLES_COUNT * 3 + 1;
		static const int SAMPLES_COUNT_SHORT_PACK = 10;
		static const int SPECIAL_DATA_SIZE_SHORT_PACK = SAMPLES_COUNT_SHORT_PACK * 3 + 1;
		
		
	private: 

		/// Генерирует по указанному удресу в указанном хранилище последовательность спец-байтов этой посылки
        /// Начиная с offset. DAC_vals - хранилище данных int для канала channel_number
		template< class Storage, class DAC_vals_storage>
		static inline void write_special_data(Storage &out_byte_storage, int offset,
            unsigned char channel_number,
			DAC_vals_storage &DAC_vals, int from_idx)
		{
			// пишем номер канала
			out_byte_storage[offset] = channel_number;
            offset++;
			// пишем данные канала
			for (int i = from_idx; i < from_idx + SAMPLES_COUNT; i++)
			{
                out_byte_storage[offset + 0] =  (unsigned char)(DAC_vals[i]>>16);
                out_byte_storage[offset + 1] =  (unsigned char)(DAC_vals[i]>>8);
                out_byte_storage[offset + 2] =  (unsigned char)(DAC_vals[i]>>0);
				offset+=3;
			};
		};

	public:
	
		template< class Storage, class DAC_vals_storage>
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
                unsigned char channel_number,
				DAC_vals_storage &DAC_vals, int from_idx)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, channel_number, DAC_vals, from_idx); // спец-данные
            create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, DATA, D_IMPEDANCE_MBN,
                                D_IMPEDANCE_MBN, SPECIAL_DATA_SIZE); // общая часть
		};

		/// Записывает в указанное хранилище значения отсчётов канала из послыки
		template< class Storage, class DAC_vals_storage>
		static inline void GetChannelData(CommonPackage &pkg, Storage &package_data,
										   DAC_vals_storage &DAC_vals_receiver, int from_idx)
		{		
			// число семплов в текущей посылке
			int samples = (pkg.package_data_length - Common_Data_Size - 1) / 3;

			// пишем данные канала
			for (int i = from_idx; i < from_idx + samples; i++)
			{
                int ch_val = 0x00000000;

				ch_val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + 1 + (i - from_idx) * 3 + 0];
				ch_val <<= 8;
				ch_val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + 1 + (i - from_idx) * 3 + 1];
				ch_val <<= 8;
				ch_val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + 1 + (i - from_idx) * 3 + 2];

				// если число отрицательное (старший бит в д.к. равен 1) заполняем пробелы 1
				if (package_data[pkg.first_byte_stream_offset + Header_Data_Size + 1 + (i - from_idx) * 3 + 0] & (0x80))
					ch_val |= 0xFF000000;

				DAC_vals_receiver[i] = ch_val;
			};
		};

		/// Возвращает номер канала, данные которого находнятся в посылке
		template< class Storage >
        static inline unsigned char GetChannelNumber(CommonPackage &pkg, Storage &package_data)
		{
			return package_data[pkg.first_byte_stream_offset + Header_Data_Size];
		};

		/// Возвращает количество отсчётов ADC в посылке
        static inline unsigned char GetSamplesCount(CommonPackage &pkg)
		{
			return (pkg.package_data_length - Common_Data_Size - 1) / 3;
		};

		/// <summary>Размер посылки данных D_IMPEDANCE_MBN</summary>
		static inline int GetSize()
		{
			//return Common_Data_Size + SPECIAL_DATA_SIZE;
			return -1;
		}

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool IsCorrectSize(int size)
		{
			// если число байт данных в посылке кратно трём, то ок
			if ((size - Common_Data_Size - 1) % 3 == 0)			
				return true;
			else 
				return false;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;	

			return true;
		};
	};

	/// <summary> Данные во время измерения импеданса Альтоника </summary>	
	class EEG_D_ImpedanceAltonics: EEG_Package
	{
	public:
		static const int SPECIAL_DATA_SIZE = 7;

	private: 

		/// Генерирует по указанному удресу в указанном хранилище последовательность спец-байтов этой посылки
		/// Начиная с offset
		template< class Storage>
		static inline void write_special_data(Storage &out_byte_storage, int offset, 
            unsigned char ChannelNumber, int Ysin, int Ycos)
		{
			out_byte_storage[offset] = ChannelNumber;
			offset ++;

            out_byte_storage[offset + 0] =  (unsigned char)(Ysin>>16);
            out_byte_storage[offset + 1] =  (unsigned char)(Ysin>>8);
            out_byte_storage[offset + 2] =  (unsigned char)(Ysin>>0);

			offset+=3;
            out_byte_storage[offset + 0] =  (unsigned char)(Ycos>>16);
            out_byte_storage[offset + 1] =  (unsigned char)(Ycos>>8);
            out_byte_storage[offset + 2] =  (unsigned char)(Ycos>>0);
		};

	public:
	
		template< class Storage>
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset,
                unsigned char ChannelNumber, int Ysin, int Ycos)
		{
			write_special_data<Storage>(place_for_package_data, curr_storage_offset + Header_Data_Size, ChannelNumber, Ysin, Ycos); // спец-данные
            create_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, DATA, D_IMPEDANCE_ALTONIX,
                D_IMPEDANCE_ALTONIX, SPECIAL_DATA_SIZE); // общая часть
		};

		/// Возвращает Ysin для указанной посылки
		template< class Storage>
        static inline unsigned int GetYsin(CommonPackage &pkg, Storage &package_data)
		{
			int offset = 1;

            int val = 0x00000000;

			val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0];
			val <<= 8;
			val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 1];
			val <<= 8;
			val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 2];

			// если число отрицательное (старший бит в д.к. равен 1) заполняем пробелы 1
			if (package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0] & (0x80))
				val |= 0xFF000000;

			return val;
		};

		/// Возвращает Ycos для указанной посылки
		template< class Storage>
        static inline unsigned int GetYcos(CommonPackage &pkg, Storage &package_data)
		{
			int offset = 4;
			
            int val = 0x00000000;

			val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0];
			val <<= 8;
			val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 1];
			val <<= 8;
			val |= package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 2];

			// если число отрицательное (старший бит в д.к. равен 1) заполняем пробелы 1
			if (package_data[pkg.first_byte_stream_offset + Header_Data_Size + offset + 0] & (0x80))
				val |= 0xFF000000;

			return val;
		};

		/// Возвращает ChannelNumber для указанной посылки
		template< class Storage>
        static inline unsigned char GetChannelNumber(CommonPackage &pkg, Storage &package_data)
		{
			return package_data[pkg.first_byte_stream_offset + Header_Data_Size];
		};

		/// <summary>Размер посылки данных D_IMPEDANCE_ALTONIX</summary>
		static inline int GetSize()
		{
			return Common_Data_Size + SPECIAL_DATA_SIZE;
		}

		/// <summary>Проверяет, возможен ли такой размер посылки команды</summary>
		static inline bool IsCorrectSize(int size)
		{
			if (size == Common_Data_Size + SPECIAL_DATA_SIZE)
				return true;
			else 
				return false;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке спец-данные для комманды</summary>
		template< class Storage >
        static inline bool IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;	

			return true;
		};
	};

	/****************************************************************************************
	*
	*									ПОСЫЛКИ СООБЩЕНИЙ
	*
	*****************************************************************************************/

	/// <summary>Сообщение готовности устройства</summary>
	class EEG_M_DeviceReady: public EEG_Package
	{
	public:

		/// <summary> Создаёт посылку сообщения DEV_READY. Заполняет указанную структуру pkg и генерирует 
		/// соответствующую последовательность байт</summary>
		template< class Storage >
        static inline void CreatePackage(CommonPackage &pkg, Storage &place_for_package_data, long long &curr_storage_offset)
		{ 
			create_simple_command_pkg<Storage>(pkg, place_for_package_data, curr_storage_offset, M_DEVICE_READY);
		};

		/// <summary>Размер посылки сообщения DEV_READY</summary>
		static inline int GetSize()
		{
			return Common_Data_Size;
		};

		/// <summary>Проверяет, возможен ли такой размер посылки сообщения</summary>
		static inline bool IsCorrectSize(int size)
		{
			return GetSize() == size;
		};

		/// <summary>Проверяет, корректны ли полученные в посылке данные сообщения</summary>
		template< class Storage >
        static inline bool IsCorrectSpecialData(Storage & bytes, long long special_data_start_idx, int special_data_len)
		{
			if (!IsCorrectSize(special_data_len + Common_Data_Size)) 
				return false;

			return true;
		};
	};


	/****************************************************************************************
	*
	*						РЕАЛИЗАЦИЯ МЕТОДОВ EEG_ByteStreamParser
	*
	*****************************************************************************************/

	/// Парсит данные из byte_storage и записывает результатат в packs_storage
	template<class ByteStorage, class PackagesStorage>
	void EEG_ByteStreamParser<ByteStorage, PackagesStorage>::ParseBytes()
	{
		// если находимся в режиме поиска (посылка packs_storage[current_parsing_pack_idx] - битая
		// она расширяется всю битую зону, вплоть до начала первой целой посылки)

        if (CommonByteStreamParser<ByteStorage, PackagesStorage>::Searching)
		{
			// бежим в сторону конца байт-последовательности
            while (CommonByteStreamParser<ByteStorage, PackagesStorage>::current_byte_idx < CommonByteStreamParser<ByteStorage, PackagesStorage>::byte_storage->size())
			{
				// если текущий байт равен стартовому байту посылки
                if ((*(CommonByteStreamParser<ByteStorage, PackagesStorage>::byte_storage))[CommonByteStreamParser<ByteStorage, PackagesStorage>::current_byte_idx] == EEG_Protocol::Starting_Byte)
				{
					// временное хранилище посылки
					CommonPackage pkg;

					// пробуем парсить одну посылку начиная с текущего байта
                    ParseOnePackage(CommonByteStreamParser<ByteStorage, PackagesStorage>::current_byte_idx, pkg);

					// тут возможны три варианта: 
					// *) посылка недообработана - так как просто закончился массив - CommonPackage::UNKNOWN
					// *) посылка обработана и корректна - CommonPackage::CORRECT
					// *) посылка обработана и повреждена - CommonPackage::DAMAGED_NOT_RESTORABLE  - в этом случае, просто идём дальше, как шли
						
					if (pkg.correctness == CommonPackage::CORRECT) // если посылка корректна
					{
						// останавливаем режим поиска
                        CommonByteStreamParser<ByteStorage, PackagesStorage>::Searching = false;
						// добавляем полученную корректную посылку в packs_storage
                        CommonByteStreamParser<ByteStorage, PackagesStorage>::packs_storage->push_back(pkg);
						// добавляем новую посылку, которую будем заполнять на следующей итерации уже в нормальном режиме
						CommonPackage tmp_pkg;
                        CommonByteStreamParser<ByteStorage, PackagesStorage>::packs_storage->push_back(tmp_pkg);
						// смещаем индекс текущей посылки
                        CommonByteStreamParser<ByteStorage, PackagesStorage>::current_parsing_pack_idx += 2;
						// смещаем индекс текущего байта
                        CommonByteStreamParser<ByteStorage, PackagesStorage>::current_byte_idx += pkg.package_data_length;
						// сваливаем
						break;
					}
					else if (pkg.correctness == CommonPackage::UNKNOWN_CORRECTNESS) // если посылка недообработана - так как просто закончился массив
						break; // останавливаемся и продолжим с этого же места в следующий раз
				};

                CommonByteStreamParser<ByteStorage, PackagesStorage>::current_byte_idx++;
                (*(CommonByteStreamParser<ByteStorage, PackagesStorage>::packs_storage))[CommonByteStreamParser<ByteStorage, PackagesStorage>::current_parsing_pack_idx].package_data_length++; // битые байты приписываем последней посылке, которая в режиме поиска является битой
			};
		}
		else // если знаем точно начало очередной посылки
		{
			// бежим в сторону конца байт-последовательности
            while (CommonByteStreamParser<ByteStorage, PackagesStorage>::current_byte_idx < CommonByteStreamParser<ByteStorage, PackagesStorage>::byte_storage->size())
			{
				// пробуем парсить одну посылку начиная с текущего байта
                ParseOnePackage(CommonByteStreamParser<ByteStorage, PackagesStorage>::current_byte_idx, (*(CommonByteStreamParser<ByteStorage, PackagesStorage>::packs_storage))[CommonByteStreamParser<ByteStorage, PackagesStorage>::current_parsing_pack_idx]);

				// если посылка повреждена, переходим к поиску
                if ((*(CommonByteStreamParser<ByteStorage, PackagesStorage>::packs_storage))[CommonByteStreamParser<ByteStorage, PackagesStorage>::current_parsing_pack_idx].correctness == CommonPackage::DAMAGED_NOT_RESTORABLE)
				{
                    CommonByteStreamParser<ByteStorage, PackagesStorage>::Searching = true;
                    CommonByteStreamParser<ByteStorage, PackagesStorage>::current_byte_idx++;
                    ((*(CommonByteStreamParser<ByteStorage, PackagesStorage>::packs_storage))[CommonByteStreamParser<ByteStorage, PackagesStorage>::current_parsing_pack_idx]).package_data_length = 1;
					//break;
					ParseBytes();					
				}
                else if ((*(CommonByteStreamParser<ByteStorage, PackagesStorage>::packs_storage))[CommonByteStreamParser<ByteStorage, PackagesStorage>::current_parsing_pack_idx].correctness == CommonPackage::CORRECT) // если посылка корректна, тогда переходим к следующей
				{
					// смещаем индексы
                    CommonByteStreamParser<ByteStorage, PackagesStorage>::current_byte_idx += ((*(CommonByteStreamParser<ByteStorage, PackagesStorage>::packs_storage))[CommonByteStreamParser<ByteStorage, PackagesStorage>::current_parsing_pack_idx]).package_data_length;
                    CommonByteStreamParser<ByteStorage, PackagesStorage>::current_parsing_pack_idx++;
					// добавляем новую посылку, которую будем заполнять на следующей итерации
					CommonPackage tmp_pkg;
                    CommonByteStreamParser<ByteStorage, PackagesStorage>::packs_storage->push_back(tmp_pkg);
				}
                else if ((*(CommonByteStreamParser<ByteStorage, PackagesStorage>::packs_storage))[CommonByteStreamParser<ByteStorage, PackagesStorage>::current_parsing_pack_idx].correctness == CommonPackage::UNKNOWN_CORRECTNESS) // если неизвестно - значит уткнулись в конец массива байт и прекращаем парсинг, при следующем вызове продолжим
					break;
			};
		};
	};

	/// парсит участок байт-последовательности, начиная с указанной позиции, результаты парсинга пишет в dst_pkg
	template<class ByteStorage, class PackagesStorage>
    inline void EEG_ByteStreamParser<ByteStorage, PackagesStorage>::ParseOnePackage(long long starting_byte_idx, CommonPackage & dst_pkg)
	{
        unsigned short CRC = 0;
		int curr_idx = starting_byte_idx;

		// априори ничего не знаем
		dst_pkg.correctness = ::CommonPackage::UNKNOWN_CORRECTNESS; // любое другое значение является флагом окончания принятия решения по этой посылке
		dst_pkg.type = CommonPackage::UNKNOWN_TYPE;
		dst_pkg.subtype = CommonPackage::UNKNOWN_SUBTYPE;
		// кроме направления
		if (this->computer_side_mode)
			dst_pkg.direction = CommonPackage::IN_DIR;
		else
			dst_pkg.direction = CommonPackage::OUT_DIR;
		// и начала данных посылки
		dst_pkg.first_byte_stream_offset = starting_byte_idx;

		// проверяем стартовый байт
        if ((*(CommonByteStreamParser<ByteStorage, PackagesStorage>::byte_storage))[curr_idx] != EEG_Protocol::Starting_Byte)
		{
			dst_pkg.correctness = ::CommonPackage::DAMAGED_NOT_RESTORABLE;
			return;
		};
		// если наткнулись на конец массива байт
		curr_idx++;
        if (curr_idx >= CommonByteStreamParser<ByteStorage, PackagesStorage>::byte_storage->size()) return;


		// получаем байт размера
        dst_pkg.package_data_length = (*(CommonByteStreamParser<ByteStorage, PackagesStorage>::byte_storage))[curr_idx];
        CRC += (*(CommonByteStreamParser<ByteStorage, PackagesStorage>::byte_storage))[curr_idx];

		// если наткнулись на конец массива байт
		curr_idx++;
        if (curr_idx >= CommonByteStreamParser<ByteStorage, PackagesStorage>::byte_storage->size()) return;
			

		// получаем номер посылки
        dst_pkg.subtype = (*(CommonByteStreamParser<ByteStorage, PackagesStorage>::byte_storage))[curr_idx];
		// проставляем тип посылки
		SetPackageType(dst_pkg, dst_pkg.subtype, this->computer_side_mode);
		// если тип неизвестен
		if (dst_pkg.type == CommonPackage::UNKNOWN_TYPE)
		{
			///
			///
			/// ПОСЛЕ ЭТОГО СРАБАТЫВАЕТ АССЕРТ НА 
			///
			///   assert(pkg.correctness != pkg.UNKNOWN_CORRECTNESS); 
			///
			///  в функции ManagePackageSequenses
			///
			dst_pkg.correctness = CommonPackage::DAMAGED_NOT_RESTORABLE;
			return;
		};

		// проверяем пару (номер посылки-размер данных)
		if (!SubtypeAndSizePairOK(dst_pkg.subtype, dst_pkg.package_data_length))
		{
			dst_pkg.correctness = CommonPackage::DAMAGED_NOT_RESTORABLE;
			return;
		};
        CRC += (*(CommonByteStreamParser<ByteStorage, PackagesStorage>::byte_storage))[curr_idx];
		curr_idx++;
		// если посылка не умещается в массиве, неизвестно корректная посылка или нет на данный момент
        if (starting_byte_idx + dst_pkg.package_data_length - 1 >= CommonByteStreamParser<ByteStorage, PackagesStorage>::byte_storage->size())
			return;

		// проверяем спец-данные посылки
        if (!SpecialDataIsOK(dst_pkg.subtype ,*(CommonByteStreamParser<ByteStorage, PackagesStorage>::byte_storage), curr_idx, dst_pkg.package_data_length - EEG_Protocol::Common_Data_Size))
		{
			dst_pkg.correctness = ::CommonPackage::DAMAGED_NOT_RESTORABLE;
			return;
		};

		// проверяем CRC
		// тут посылка умещается в массиве, тогда считаем её CRC, складываем спец-данные посылки
		for ( ; curr_idx <= starting_byte_idx + dst_pkg.package_data_length - 1 - 2; curr_idx++)
            CRC += (*(CommonByteStreamParser<ByteStorage, PackagesStorage>::byte_storage))[curr_idx];

        unsigned short receivedCRC = 0;
        receivedCRC = (*(CommonByteStreamParser<ByteStorage, PackagesStorage>::byte_storage))[curr_idx + 1]; // справа старший
		receivedCRC <<=8; // сдвигаем его на положенное ему место
        receivedCRC |= (*(CommonByteStreamParser<ByteStorage, PackagesStorage>::byte_storage))[curr_idx]; // слева младший байт
			
		if (receivedCRC != CRC)
		{
			dst_pkg.correctness = ::CommonPackage::DAMAGED_NOT_RESTORABLE;
			return;
		};

		// если дошли до сюдова, то считаем посылку корректной
		dst_pkg.correctness = ::CommonPackage::CORRECT;
	};	

	// проверяет, может ли существовать такая пара из подтипа комманды и её размера
	template<class ByteStorage, class PackagesStorage>
	bool EEG_ByteStreamParser<ByteStorage, PackagesStorage>::SubtypeAndSizePairOK(int subtype, int size)
	{
		// если работаем на стороне хоста
		if (this->computer_side_mode)
			switch(subtype)
			{
			// ответы на комманды
			case EEG_Protocol::A_ERROR: /*assert(0);*/ return false;
			// добавлен ответ на Reset
			case EEG_Protocol::A_RESET: return MBN_EEG::EEG_C_Reset::A_IsCorrectSize(size);
			case EEG_Protocol::A_GET_VERSION: return MBN_EEG::EEG_C_GetVersion::A_IsCorrectSize(size);
			case EEG_Protocol::A_SET_MUX:	return MBN_EEG::EEG_C_SetMux::A_IsCorrectSize(size);
			case EEG_Protocol::A_GET_PROTOCOL_VERSION: return MBN_EEG::EEG_C_GetProtocolVersion::A_IsCorrectSize(size);
			case EEG_Protocol::A_GET_FREQUENCY_TABLE: return MBN_EEG::EEG_C_GetFrequencyTable::A_IsCorrectSize(size);
			case EEG_Protocol::A_SET_SAMPLING_FREQUENCY: return MBN_EEG::EEG_C_SetSamplingFrequency::A_IsCorrectSize(size);
			case EEG_Protocol::A_SET_CHANNELS_MODE: return MBN_EEG::EEG_C_SetChannelsMode::A_IsCorrectSize(size);
			case EEG_Protocol::A_SET_CALIBRATION_SHAPE: return MBN_EEG::EEG_C_SetCalibrationShape::A_IsCorrectSize(size);
			case EEG_Protocol::A_START_CALIBRATION: return MBN_EEG::EEG_C_StartCalibration::A_IsCorrectSize(size);
			case EEG_Protocol::A_STOP_CALIBRATION: return MBN_EEG::EEG_C_StopCalibration::A_IsCorrectSize(size);
			case EEG_Protocol::A_START_TRANSLATION: return MBN_EEG::EEG_C_StartTranslation::A_IsCorrectSize(size);
			case EEG_Protocol::A_STOP_TRANSLATION: return MBN_EEG::EEG_C_StopTranslation::A_IsCorrectSize(size);
			case EEG_Protocol::A_SET_CHANNEL_LEDS: return MBN_EEG::EEG_C_SetChannelLeds::A_IsCorrectSize(size);
			case EEG_Protocol::A_SET_AMPLIFICATION: return MBN_EEG::EEG_C_SetAmplification::A_IsCorrectSize(size);
			case EEG_Protocol::A_SET_ADS_MUX: return MBN_EEG::EEG_C_SetADSMux::A_IsCorrectSize(size);
			// данные
			case EEG_Protocol::D_USUAL_CHANNELS_DATA_WITHOUT_STIMUL: return MBN_EEG::EEG_D_UsualData::IsCorrectSize(size);
			case EEG_Protocol::D_USUAL_CHANNELS_DATA_WITH_STIMUL: return MBN_EEG::EEG_D_UsualData_With_Stimul::IsCorrectSize(size);
			case EEG_Protocol::D_IMPEDANCE_MBN: return MBN_EEG::EEG_D_ImpedanceMBN::IsCorrectSize(size);
			case EEG_Protocol::D_IMPEDANCE_ALTONIX: return MBN_EEG::EEG_D_ImpedanceAltonics::IsCorrectSize(size);
			// сообщения
			case EEG_Protocol::M_DEVICE_READY: return MBN_EEG::EEG_M_DeviceReady::IsCorrectSize(size);
			default:  
				{
					cout<<"WARNING: unknown type";
					return false;
				}
			}
		else
			switch(subtype)
			{
			// комманды
			case EEG_Protocol::C_ERROR: assert(0); return false;
			case EEG_Protocol::C_RESET: return MBN_EEG::EEG_C_Reset::C_IsCorrectSize(size);
			case EEG_Protocol::C_GET_VERSION: return MBN_EEG::EEG_C_GetVersion::C_IsCorrectSize(size);
            case EEG_Protocol::C_SET_MUX:	return MBN_EEG::EEG_C_SetMux::C_IsCorrectSize(size);
			case EEG_Protocol::C_GET_PROTOCOL_VERSION: return MBN_EEG::EEG_C_GetProtocolVersion::C_IsCorrectSize(size);
			case EEG_Protocol::C_GET_FREQUENCY_TABLE: return MBN_EEG::EEG_C_GetFrequencyTable::C_IsCorrectSize(size);
			case EEG_Protocol::C_SET_SAMPLING_FREQUENCY: return MBN_EEG::EEG_C_SetSamplingFrequency::C_IsCorrectSize(size);
			case EEG_Protocol::C_SET_CHANNELS_MODE: return MBN_EEG::EEG_C_SetChannelsMode::C_IsCorrectSize(size);
			case EEG_Protocol::C_SET_CALIBRATION_SHAPE: return MBN_EEG::EEG_C_SetCalibrationShape::C_IsCorrectSize(size);
			case EEG_Protocol::C_START_CALIBRATION: return MBN_EEG::EEG_C_StartCalibration::C_IsCorrectSize(size);
			case EEG_Protocol::C_STOP_CALIBRATION: return MBN_EEG::EEG_C_StopCalibration::C_IsCorrectSize(size);
			case EEG_Protocol::C_START_TRANSLATION: return MBN_EEG::EEG_C_StartTranslation::C_IsCorrectSize(size);
			case EEG_Protocol::C_STOP_TRANSLATION: return MBN_EEG::EEG_C_StopTranslation::C_IsCorrectSize(size);
			case EEG_Protocol::C_SET_CHANNEL_LEDS: return MBN_EEG::EEG_C_SetChannelLeds::C_IsCorrectSize(size);
			case EEG_Protocol::C_SET_AMPLIFICATION: return MBN_EEG::EEG_C_SetAmplification::C_IsCorrectSize(size);
			case EEG_Protocol::C_SET_ADS_MUX: return MBN_EEG::EEG_C_SetADSMux::C_IsCorrectSize(size);

			default: return false;
			};
	};

	// проверяет, корректны ли спец-данные комманды
	template<class ByteStorage, class PackagesStorage>
    bool EEG_ByteStreamParser<ByteStorage, PackagesStorage>::SpecialDataIsOK(int subtype, ByteStorage & bytes, long long special_data_start_idx, int special_data_len)
	{
		// если работаем на стороне хоста
		if (this->computer_side_mode)
			switch(subtype)
			{
			// ответы на комманды
			case EEG_Protocol::A_ERROR: return false;
			// добавлен ответ на Reset
			case EEG_Protocol::A_RESET: return MBN_EEG::EEG_C_Reset::A_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::A_GET_VERSION: return MBN_EEG::EEG_C_GetVersion::A_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::A_SET_MUX:	return MBN_EEG::EEG_C_SetMux::A_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::A_GET_PROTOCOL_VERSION: return MBN_EEG::EEG_C_GetProtocolVersion::A_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::A_GET_FREQUENCY_TABLE: return MBN_EEG::EEG_C_GetFrequencyTable::A_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::A_SET_SAMPLING_FREQUENCY: return MBN_EEG::EEG_C_SetSamplingFrequency::A_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::A_SET_CHANNELS_MODE: return MBN_EEG::EEG_C_SetChannelsMode::A_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::A_SET_CALIBRATION_SHAPE: return MBN_EEG::EEG_C_SetCalibrationShape::A_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::A_START_CALIBRATION: return MBN_EEG::EEG_C_StartCalibration::A_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::A_STOP_CALIBRATION: return MBN_EEG::EEG_C_StopCalibration::A_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::A_START_TRANSLATION: return MBN_EEG::EEG_C_StartTranslation::A_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::A_STOP_TRANSLATION: return MBN_EEG::EEG_C_StopTranslation::A_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::A_SET_CHANNEL_LEDS: return MBN_EEG::EEG_C_SetChannelLeds::A_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::A_SET_AMPLIFICATION: return MBN_EEG::EEG_C_SetAmplification::A_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::A_SET_ADS_MUX: return MBN_EEG::EEG_C_SetADSMux::A_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			// данные
			case EEG_Protocol::D_USUAL_CHANNELS_DATA_WITHOUT_STIMUL: return MBN_EEG::EEG_D_UsualData::IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::D_USUAL_CHANNELS_DATA_WITH_STIMUL: return MBN_EEG::EEG_D_UsualData_With_Stimul::IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::D_IMPEDANCE_MBN: return MBN_EEG::EEG_D_ImpedanceMBN::IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::D_IMPEDANCE_ALTONIX: return MBN_EEG::EEG_D_ImpedanceAltonics::IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			// сообщения
			case EEG_Protocol::M_DEVICE_READY: return MBN_EEG::EEG_M_DeviceReady::IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			default: return false;
			}
		else
			switch(subtype)
			{
			// ответы на комманды
			case EEG_Protocol::C_ERROR: return false;
			case EEG_Protocol::C_RESET: return MBN_EEG::EEG_C_Reset::C_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::C_GET_VERSION: return MBN_EEG::EEG_C_GetVersion::C_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::C_SET_MUX:	return MBN_EEG::EEG_C_SetMux::C_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::C_GET_PROTOCOL_VERSION: return MBN_EEG::EEG_C_GetProtocolVersion::C_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::C_GET_FREQUENCY_TABLE: return MBN_EEG::EEG_C_GetFrequencyTable::C_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::C_SET_SAMPLING_FREQUENCY: return MBN_EEG::EEG_C_SetSamplingFrequency::C_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::C_SET_CHANNELS_MODE: return MBN_EEG::EEG_C_SetChannelsMode::C_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::C_SET_CALIBRATION_SHAPE: return MBN_EEG::EEG_C_SetCalibrationShape::C_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::C_START_CALIBRATION: return MBN_EEG::EEG_C_StartCalibration::C_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::C_STOP_CALIBRATION: return MBN_EEG::EEG_C_StopCalibration::C_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::C_START_TRANSLATION: return MBN_EEG::EEG_C_StartTranslation::C_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::C_STOP_TRANSLATION: return MBN_EEG::EEG_C_StopTranslation::C_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::C_SET_CHANNEL_LEDS: return MBN_EEG::EEG_C_SetChannelLeds::C_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::C_SET_AMPLIFICATION: return MBN_EEG::EEG_C_SetAmplification::C_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);
			case EEG_Protocol::C_SET_ADS_MUX: return MBN_EEG::EEG_C_SetADSMux::C_IsCorrectSpecialData<ByteStorage>(bytes, special_data_start_idx, special_data_len);

			default: return false;
			}
	};

	// в зависимости от направления передачи посылки выставляет тип посылки
	template<class ByteStorage, class PackagesStorage>
	void EEG_ByteStreamParser<ByteStorage, PackagesStorage>::SetPackageType(CommonPackage &pkg, int pkg_number, bool in_direction)
	{
		if (in_direction) // от устройства
		{
			switch(pkg_number)
			{
			// ответы на комманды
			case EEG_Protocol::A_ERROR: pkg.type = EEG_Protocol::ANSWER; return;
			// добавлен ответ на Reset
			case EEG_Protocol::A_RESET: pkg.type = EEG_Protocol::ANSWER; return;
			case EEG_Protocol::A_GET_VERSION: pkg.type = EEG_Protocol::ANSWER; return;
			case EEG_Protocol::A_SET_MUX:	pkg.type = EEG_Protocol::ANSWER; return;
			case EEG_Protocol::A_GET_PROTOCOL_VERSION: pkg.type = EEG_Protocol::ANSWER; return;
			case EEG_Protocol::A_GET_FREQUENCY_TABLE: pkg.type = EEG_Protocol::ANSWER; return;
			case EEG_Protocol::A_SET_SAMPLING_FREQUENCY: pkg.type = EEG_Protocol::ANSWER; return;
			case EEG_Protocol::A_SET_CHANNELS_MODE: pkg.type = EEG_Protocol::ANSWER; return;
			case EEG_Protocol::A_SET_CALIBRATION_SHAPE: pkg.type = EEG_Protocol::ANSWER; return;
			case EEG_Protocol::A_START_CALIBRATION: pkg.type = EEG_Protocol::ANSWER; return;
			case EEG_Protocol::A_STOP_CALIBRATION: pkg.type = EEG_Protocol::ANSWER; return;
			case EEG_Protocol::A_START_TRANSLATION: pkg.type = EEG_Protocol::ANSWER; return;
			case EEG_Protocol::A_STOP_TRANSLATION: pkg.type = EEG_Protocol::ANSWER; return;
			case EEG_Protocol::A_SET_CHANNEL_LEDS: pkg.type = EEG_Protocol::ANSWER; return;
			case EEG_Protocol::A_SET_AMPLIFICATION: pkg.type = EEG_Protocol::ANSWER; return;
			case EEG_Protocol::A_SET_ADS_MUX: pkg.type = EEG_Protocol::ANSWER; return;
			// данные
			case EEG_Protocol::D_USUAL_CHANNELS_DATA_WITHOUT_STIMUL: pkg.type = EEG_Protocol::DATA; return;
			case EEG_Protocol::D_USUAL_CHANNELS_DATA_WITH_STIMUL: pkg.type = EEG_Protocol::DATA; return;
			case EEG_Protocol::D_IMPEDANCE_MBN: pkg.type = EEG_Protocol::DATA; return;
			case EEG_Protocol::D_IMPEDANCE_ALTONIX: pkg.type = EEG_Protocol::DATA; return;
			// сообщения
			case EEG_Protocol::M_DEVICE_READY: pkg.type = EEG_Protocol::MESSAGE; return;

			default: pkg.type = CommonPackage::UNKNOWN_TYPE; return;
			};
		}
		else // от компа идут только комманды
			switch(pkg_number)
			{
			// ответы на комманды
			case EEG_Protocol::C_ERROR: pkg.type = EEG_Protocol::COMMAND; return;
			case EEG_Protocol::C_RESET: pkg.type = EEG_Protocol::COMMAND; return;
			case EEG_Protocol::C_GET_VERSION: pkg.type = EEG_Protocol::COMMAND; return;
			case EEG_Protocol::C_SET_MUX:	pkg.type = EEG_Protocol::COMMAND; return;
			case EEG_Protocol::C_GET_PROTOCOL_VERSION: pkg.type = EEG_Protocol::COMMAND; return;
			case EEG_Protocol::C_GET_FREQUENCY_TABLE: pkg.type = EEG_Protocol::COMMAND; return;
			case EEG_Protocol::C_SET_SAMPLING_FREQUENCY: pkg.type = EEG_Protocol::COMMAND; return;
			case EEG_Protocol::C_SET_CHANNELS_MODE: pkg.type = EEG_Protocol::COMMAND; return;
			case EEG_Protocol::C_SET_CALIBRATION_SHAPE: pkg.type = EEG_Protocol::COMMAND; return;
			case EEG_Protocol::C_START_CALIBRATION: pkg.type = EEG_Protocol::COMMAND; return;
			case EEG_Protocol::C_STOP_CALIBRATION: pkg.type = EEG_Protocol::COMMAND; return;
			case EEG_Protocol::C_START_TRANSLATION: pkg.type = EEG_Protocol::COMMAND; return;
			case EEG_Protocol::C_STOP_TRANSLATION: pkg.type = EEG_Protocol::COMMAND; return;
			case EEG_Protocol::C_SET_CHANNEL_LEDS: pkg.type = EEG_Protocol::COMMAND; return;
			case EEG_Protocol::C_SET_AMPLIFICATION: pkg.type = EEG_Protocol::COMMAND; return;
			case EEG_Protocol::C_SET_ADS_MUX: pkg.type = EEG_Protocol::COMMAND; return;
			default: pkg.type = CommonPackage::UNKNOWN_TYPE; return;
			};
	};














};


#endif
