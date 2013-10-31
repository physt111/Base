/****************************************************************************************
*
*							РЕАЛИЗАЦИЯ МЕТОДОВ EEG_Protocol
*
*****************************************************************************************/

#ifndef MBN_EEG_PROTOCOL
#define MBN_EEG_PROTOCOL

#include <EEG_Protocol.h>
#include <EEG_device_architecture.h>
#include <EEG_Package.h>
#include <qdatetime.h>
#include <math.h>

namespace MBN_EEG
{


/// возвращает общий класс посылки (по данному протоколу)
string EEG_Protocol::GetPackageTypeString(int type, int subtype)
{
    switch (type)
    {
    case ANSWER: return "answer";
    case COMMAND: return "command";
    case DATA: return "data";
    case MESSAGE: return "message";
    default: return "<unknown type>";
    };
};

/// возвращает название посылки (по данному протоколу)
string EEG_Protocol::GetPackageNameString(int type, int subtype)
{
	switch (type)
	{
    case ANSWER:
		{
			switch (subtype)
			{
			case A_ERROR: return "Error";
			case A_RESET: return "Reset";
			case A_GET_VERSION: return "GetVersion";
			case A_SET_MUX: return "SetMux";
			case A_GET_PROTOCOL_VERSION: return "GetProtocolVersion";
			case A_GET_FREQUENCY_TABLE: return "GetFrequencyTable";
			case A_SET_SAMPLING_FREQUENCY: return "SetSamplingFrequency";
			case A_SET_CHANNELS_MODE: return "SetChannelsMode";
			case A_SET_CALIBRATION_SHAPE: return "SetCalibrationShape";
			case A_START_CALIBRATION: return "StartCalibration";
			case A_STOP_CALIBRATION: return "StopCalibration";
			case A_START_TRANSLATION: return "StartTranslation";
			case A_STOP_TRANSLATION: return "StopTranslation";
			case A_SET_CHANNEL_LEDS: return "SetChannelLeds";
			case A_SET_AMPLIFICATION: return "SetAmplification";
			case A_SET_ADS_MUX: return "SetADSMux";
			default: return "<unknown answer>";
			};			
		};
    case COMMAND:
		{
			switch (subtype)
			{
			case C_ERROR: return "Error";
			case C_RESET: return "Reset";
			case C_GET_VERSION: return "GetVersion";
			case C_SET_MUX: return "SetMux";
			case C_GET_PROTOCOL_VERSION: return "GetProtocolVersion";
			case C_GET_FREQUENCY_TABLE: return "GetFrequencyTable";
			case C_SET_SAMPLING_FREQUENCY: return "SetSamplingFrequency";
			case C_SET_CHANNELS_MODE: return "SetChannelsMode";
			case C_SET_CALIBRATION_SHAPE: return "SetCalibrationShape";
			case C_START_CALIBRATION: return "StartCalibration";
			case C_STOP_CALIBRATION: return "StopCalibration";
			case C_START_TRANSLATION: return "StartTranslation";
			case C_STOP_TRANSLATION: return "StopTranslation";
			case C_SET_CHANNEL_LEDS: return "SetChannelLeds";
			case C_SET_AMPLIFICATION: return "SetAmplification";
			case C_SET_ADS_MUX: return "SetADSMux";
			default: return "<unknown command>";
			};
		};
    case DATA:
		{
			switch (subtype)
			{
			case D_USUAL_CHANNELS_DATA_WITHOUT_STIMUL: return "Data without stimul";
			case D_USUAL_CHANNELS_DATA_WITH_STIMUL: return "Data with stimul";
			case D_IMPEDANCE_MBN: return "Data in MBN impedance mode";
			case D_IMPEDANCE_ALTONIX: return "Data in Altonix impedance mode";
			default: return "<unknown data>";
			};
		};
    case MESSAGE:
		{
			switch (subtype)
			{
			case M_DEVICE_READY: return "device ready";
			default: return "<unknown message>";
			};
		};
	default: return "<unknown message type>";
	};	
};


/// вспомогательные функции для получения информации
/// переводит из EEG_Command_Subtype в строку с названием команды
QString EEG_Protocol::GetCommandNameByNumber(int command_number)
{
	switch (command_number)
	{
	case C_ERROR: return "Error";
	case C_RESET: return "Reset";
	case C_GET_VERSION: return "GetVersion";
	case C_SET_MUX: return "SetMux";
	case C_GET_PROTOCOL_VERSION: return "GetProtocolVersion";
	case C_GET_FREQUENCY_TABLE: return "GetFrequencyTable";
	case C_SET_SAMPLING_FREQUENCY: return "SetSamplingFrequency";
	case C_SET_CHANNELS_MODE: return "SetChannelsMode";
	case C_SET_CALIBRATION_SHAPE: return "SetCalibrationShape";
	case C_START_CALIBRATION: return "StartCalibration";
	case C_STOP_CALIBRATION: return "StopCalibration";
	case C_START_TRANSLATION: return "StartTranslation";
	case C_STOP_TRANSLATION: return "StopTranslation";
	case C_SET_CHANNEL_LEDS: return "SetChannelLeds";
	case C_SET_AMPLIFICATION: return "SetAmplification";
	case C_SET_ADS_MUX: return "SetADSMux";
	default: return "<unknown command>";
	};

};

/// определение констант
	/// Ошибки
const simple_error EEG_Protocol::EEG_PROTOCOL_ERR_GOT_DAMAGED_PACK = simple_error("Got the damaged pack.",120);
const simple_error EEG_Protocol::EEG_PROTOCOL_ERR_ANSWER_ON_NOT_EXISTING_COMMAND = simple_error("Answer to the command which does not exist.", 121);
const simple_error EEG_Protocol::EEG_PROTOCOL_ERR_COMMAND_HAS_BEEN_SKIPPED = simple_error("Command has been skipped.",122);
const simple_error EEG_Protocol::EEG_PROTOCOL_ERR_COMMAND_REPLY_TIMEOUT = simple_error("Command reply timeout.",123);




/// Обрабатывает в логике протокола связи смысла получаемых данных эти самые данные :)
/// Проще говоря, определяет, логику работы на уровне "между посылок" - т.е. какие комманды были исполнены,
/// Какие были исполнены верно. А также распределяет полученные от устройства данные в соответствии с их 
/// смыслом.
/// next_pack_to_write_idx - иднекс посылки в выходной очереди, которая следующей будет записана в порт
void EEG_Protocol::ManagePackageSequenses(PagedArray<CommonPackage> *Out_pkg_Ptr, PagedArray<unsigned char> *Out_byte_Ptr,
                                    PagedArray<CommonPackage> *In_pkg_Ptr,  PagedArray<unsigned char> *In_byte_Ptr,
									int next_pack_to_write_idx)
{
	// если работаем на стороне компьютера
	if (computer_side_mode)
	{
		// бежим по необработанным ещё посылкам приёмной очереди (кроме последней, которая по определению всегда не завершена)
		for (; pkg_idx_to_process < In_pkg_Ptr->size() - 1; pkg_idx_to_process++)
		{
			// текущая обрабатываемая входная посылка
			CommonPackage &pkg = (*(In_pkg_Ptr))[pkg_idx_to_process];

			// не должно быть неизвестных посылок тут
			assert(pkg.correctness != pkg.UNKNOWN_CORRECTNESS);

			// если посылка битая - пропускаем
			if ((pkg.correctness == pkg.DAMAGED_NOT_RESTORABLE)||(pkg.correctness == pkg.DAMAGED_RESTORABLE))
			{
				errors_count++;
				errors_list.push_back(EEG_PROTOCOL_ERR_GOT_DAMAGED_PACK);

				// еслит задана ф-ция обратного вызова по ошибке - вызываем её
				if (ErrorCallback)
					(*ErrorCallback)(EEG_PROTOCOL_ERR_GOT_DAMAGED_PACK, this->error_callback_object_ptr);

				continue;
			};

			// если это данные - вытаскиваем их и сохраняем в хранилище данных
			if (pkg.type == DATA)
			{
				__process_data_pkg(pkg, In_byte_Ptr, Out_byte_Ptr);

				// если нужно - вызываем callback-функцию приёма данных и передаём ей данные и пользовательский указатель
				if (DataCallback)
				{
					// сохраняем ответную посылку для высоких уровней анализа
					CommonPackage dataPkg(pkg);
					dataPkg.data_location = CommonPackage::INTERNAL;
					dataPkg.first_byte_stream_offset = 0;
					// вставляем в посылку встроенные данные
					dataPkg.direct_data = vector<unsigned char>(pkg.package_data_length);
					for (int i = 0; i < pkg.package_data_length; i++)
						dataPkg.direct_data[i] = (*In_byte_Ptr)[pkg.first_byte_stream_offset + i];

					(*DataCallback)(dataPkg, this->data_callback_object_ptr);
				};
			};

			// если это сообщение - обрабатываем его и вызываем, если надо функцию обратного вызова
			if (pkg.type == MESSAGE)
			{
				__process_message_pkg(pkg, In_byte_Ptr);

				// если нужно - вызываем callback-функцию приёма сообщения и передаём ей данные и пользовательский указатель
				if (MessageCallback)
					(*MessageCallback)(pkg, this->data_callback_object_ptr);
			};

			// если это ответ на команду
			if (pkg.type == ANSWER)
			{				
				// устанавливаем состояние устройства в соответствии с полученным ответом
				__process_comm_pkg(pkg, In_byte_Ptr, Out_byte_Ptr);

				// ищем, какой команде из выходной очереди соответствует полученный ответ
				int corr_out_pkg = __apply_ack(pkg, this->unsubmitted_pkg_idx, In_pkg_Ptr, In_byte_Ptr, Out_pkg_Ptr, Out_byte_Ptr, next_pack_to_write_idx);

				// если не нашли в выходной очереди комманды на которую получили ответ, то плюсуем количество ошибок
				if (corr_out_pkg == -1)
				{
					errors_count++;
					errors_list.push_back(EEG_PROTOCOL_ERR_ANSWER_ON_NOT_EXISTING_COMMAND);

					// еслит задана ф-ция обратного вызова по ошибке - вызываем её
					if (ErrorCallback)
						(*ErrorCallback)(EEG_PROTOCOL_ERR_ANSWER_ON_NOT_EXISTING_COMMAND, this->error_callback_object_ptr);

				}
				else if (corr_out_pkg != unsubmitted_pkg_idx) //  если пришёл ответ не на первую неподтверждённую команду, значит мы пропустили комманду
				{
					// выставляем флаг о том, что эта посылка подтвердила команду
					pkg.ack = true;
					// выставляем флаг о том, что эта посылка была подтверждена ответом
					(*Out_pkg_Ptr)[corr_out_pkg].ack = true;
					(*Out_pkg_Ptr)[corr_out_pkg].can_be_erased = true;

					errors_count++;
					errors_list.push_back(EEG_PROTOCOL_ERR_COMMAND_HAS_BEEN_SKIPPED);

					// еслит задана ф-ция обратного вызова по ошибке - вызываем её
					if (ErrorCallback)
						(*ErrorCallback)(EEG_PROTOCOL_ERR_COMMAND_HAS_BEEN_SKIPPED, this->error_callback_object_ptr);

					// если задана ф-ция обратного вызова по подтверждению выполнения команды
					if (AckCallback)
					{
						// сохраняем ответную посылку для высоких уровней анализа
						CommonPackage answerPkg(pkg);
						answerPkg.data_location = CommonPackage::INTERNAL;
						answerPkg.first_byte_stream_offset = 0;
						// вставляем в посылку встроенные данные
						answerPkg.direct_data = vector<unsigned char>(pkg.package_data_length);
						for (int i = 0; i < pkg.package_data_length; i++)
							answerPkg.direct_data[i] = (*In_byte_Ptr)[pkg.first_byte_stream_offset + i];

						// отправляем встроенную послылку в callback функцию
						(*AckCallback)((*Out_pkg_Ptr)[corr_out_pkg].id, answerPkg, this->acknowledge_callback_object_ptr);
					};
				}
				else // если подтвердили выполнение первой неподтверждённой команды
				{
					// выставляем флаг о том, что эта посылка подтвердила команду
					pkg.ack = true;
					// выставляем флаг о том, что эта посылка была подтверждена ответом
					(*Out_pkg_Ptr)[unsubmitted_pkg_idx].ack = true;
					(*Out_pkg_Ptr)[unsubmitted_pkg_idx].can_be_erased = true;

					// смещаем указатель на первую неподтверждённую команду
					unsubmitted_pkg_idx++;
					
					// еслит задана ф-ция обратного вызова по подтверждению выполнения команды
					if (AckCallback)
					{
						// сохраняем ответную посылку для высоких уровней анализа
						CommonPackage answerPkg(pkg);
						answerPkg.data_location = CommonPackage::INTERNAL;
						answerPkg.first_byte_stream_offset = 0;
						// вставляем в посылку встроенные данные
						answerPkg.direct_data = vector<unsigned char>(pkg.package_data_length);
						for (int i = 0; i < pkg.package_data_length; i++)
							answerPkg.direct_data[i] = (*In_byte_Ptr)[pkg.first_byte_stream_offset + i];
						
						// отправляем встроенную послылку в callback функцию
						(*AckCallback)((*Out_pkg_Ptr)[corr_out_pkg].id, answerPkg, this->acknowledge_callback_object_ptr);
					};
				};
			};

			// выставляем флаг о том, что эта входная посылка была послностью обработана и может быть удалена вместе со своими данными
			pkg.can_be_erased = true;
		};

		// теперь проверяем по временнОй диаграмме выходную очередь - сколько там осталось неподтверждённых посылок 
		if (unsubmitted_pkg_idx < Out_pkg_Ptr->size())
			for (; unsubmitted_pkg_idx < next_pack_to_write_idx; unsubmitted_pkg_idx++)
			{
				// текущая первая неподтверждённая посылка
				CommonPackage &out_pkg = (*(Out_pkg_Ptr))[unsubmitted_pkg_idx];

				// если она не подтверждена
				if (!out_pkg.ack)
				{
					// проверяем, не просрочена ли посылка
					int ch_speed = 115000; // байт в с

					// полное время передачи по каналу (туда + обратно)
					float total_transmit_time = float(out_pkg.package_data_length) / (float(ch_speed));
					// полное время от передачи до получения подтверждения
					float total_time = total_transmit_time;

					// добавляем время выполнения комманды
					switch (out_pkg.subtype)
					{
					case MBN_EEG::EEG_Protocol::C_ERROR: total_time += 0.0;
					case MBN_EEG::EEG_Protocol::C_GET_FREQUENCY_TABLE: total_time += 0.020;
					case MBN_EEG::EEG_Protocol::C_GET_PROTOCOL_VERSION: total_time += 0.020;
					case MBN_EEG::EEG_Protocol::C_GET_VERSION: total_time += 0.020;
					case MBN_EEG::EEG_Protocol::C_RESET: total_time += 0.020;
					case MBN_EEG::EEG_Protocol::C_SET_ADS_MUX: total_time += 0.100;
					case MBN_EEG::EEG_Protocol::C_SET_AMPLIFICATION: total_time += 0.060;
					case MBN_EEG::EEG_Protocol::C_SET_CALIBRATION_SHAPE: total_time += 0.020;
					case MBN_EEG::EEG_Protocol::C_SET_CHANNELS_MODE: total_time += 0.020;
					case MBN_EEG::EEG_Protocol::C_SET_CHANNEL_LEDS: total_time += 0.010;
					case MBN_EEG::EEG_Protocol::C_SET_MUX: total_time += 0.060;
					case MBN_EEG::EEG_Protocol::C_SET_SAMPLING_FREQUENCY: total_time += 0.050;
					case MBN_EEG::EEG_Protocol::C_START_CALIBRATION: total_time += 0.005;
					case MBN_EEG::EEG_Protocol::C_START_TRANSLATION: total_time += 0.005;
					case MBN_EEG::EEG_Protocol::C_STOP_CALIBRATION: total_time += 0.005;
					case MBN_EEG::EEG_Protocol::C_STOP_TRANSLATION: total_time += 0.005;
					};

					// если с момента отправки пакета прошло больше чем максимально необходимо на его обработку
					if (QDateTime::currentDateTime().toMSecsSinceEpoch() - out_pkg.date_time.toMSecsSinceEpoch() > 1000.0 * total_time)
					{
						errors_count++;
						errors_list.push_back(EEG_PROTOCOL_ERR_COMMAND_REPLY_TIMEOUT);

						// еслит задана ф-ция обратного вызова по ошибке - вызываем её
						if (ErrorCallback)
							(*ErrorCallback)(EEG_PROTOCOL_ERR_COMMAND_REPLY_TIMEOUT, this->error_callback_object_ptr);

						// если задана ф-ция обратного вызова по таймауту выполнения команды
						if (AckTimeoutCallback)
							(*AckTimeoutCallback)(out_pkg.id, out_pkg.type, out_pkg.subtype, this->acknowledge_timeout_callback_object_ptr);

						//  и далее идём на увеличение unsubmitted_pkg_idx
					}
					else // если по времени ещё можем ждать ответа на команду - выходим, так как других уж точно можем ждать
						break;
				};
			};
	}
	else // если имитируем устройство
	{
		//////// тут просто отвечаем на комманды от компа
		// бежим по необработанным ещё посылкам приёмной очереди (кроме последней, которая по определению всегда не завершена)
		for (; pkg_idx_to_process < In_pkg_Ptr->size() - 1; pkg_idx_to_process++)
		{
			// текущая обрабатываемая входная посылка
			CommonPackage &pkg = (*(In_pkg_Ptr))[pkg_idx_to_process];

			// не должно быть неизвестных посылок тут
			assert(pkg.correctness != pkg.UNKNOWN_CORRECTNESS);

			// если посылка битая - пропускаем
			if ((pkg.correctness == pkg.DAMAGED_NOT_RESTORABLE)||(pkg.correctness == pkg.DAMAGED_RESTORABLE))
			{
				errors_count++;
				errors_list.push_back(EEG_PROTOCOL_ERR_GOT_DAMAGED_PACK);

				// еслит задана ф-ция обратного вызова по ошибке - вызываем её
				if (ErrorCallback)
					(*ErrorCallback)(EEG_PROTOCOL_ERR_GOT_DAMAGED_PACK, this->error_callback_object_ptr);

				continue;
			};

			// если это команда
			if (pkg.type == COMMAND)
			{
				// отвечаем на команду
				__dev_answer(pkg, Out_pkg_Ptr, Out_byte_Ptr, In_byte_Ptr);

				// если надо - вызываем callback
				if (AckCallback)
				{
					// сохраняем ответную посылку для высоких уровней анализа
					CommonPackage answerPkg(pkg);
					answerPkg.data_location = CommonPackage::INTERNAL;
					answerPkg.first_byte_stream_offset = 0;

					// вставляем в посылку встроенные данные
					answerPkg.direct_data = vector<unsigned char>(pkg.package_data_length);
					for (int i = 0; i < pkg.package_data_length; i++)
						answerPkg.direct_data[i] = (*In_byte_Ptr)[pkg.first_byte_stream_offset + i];

					// отправляем встроенную послылку в callback функцию
					(*AckCallback)(0, answerPkg, this->acknowledge_callback_object_ptr);
				};
			};

			// выставляем флаг о том, что эта входная посылка была послностью обработана и может быть удалена вместе со своими данными
			pkg.can_be_erased = true;
		};
	};

};




/// обработать пришедшую посылку с данными
///		*) Преобразовать данные в вольтаж
///		*) Записать данные вольтажа в хранилище
/// pgk -  предполагается тут корректной посылкой с данными, тип = DATA
void EEG_Protocol::__process_data_pkg(CommonPackage &pkg, PagedArray<unsigned char> *In_byte_Ptr, PagedArray<unsigned char> *Out_byte_Ptr)
{
	// если внутреннее хранение отключено - выходим
	if (!this->internalDataStorageEnabled) return;

	// при направлении вовнутрь
	if (pkg.direction == CommonPackage::IN_DIR)
	{
		/// в зависимости от данных
		switch(pkg.subtype)
		{
		case D_USUAL_CHANNELS_DATA_WITHOUT_STIMUL: // обычные данные в обычном режиме
			{
				int ch0 = 0;
				int ch1 = 0;
				int ch2 = 0; 
				int ch3 = 0;

				// получаем данные
                MBN_EEG::EEG_D_UsualData::GetChannelsData<PagedArray<unsigned char> >(pkg, *In_byte_Ptr, ch0, ch1, ch2, ch3);
							
				// получаем состояние ADC
				int dev_state_idx = 0;
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_State adc_st = this->device_states_ptr->GetCurrDevState_ADCState(&dev_state_idx);

				// помещаем данные в хранилище
				(*channels_data_ptr)[0].Add(MBN_EEG::EEG_Device_Architecture::GetRealVoltage__uV(ch0, 0, &adc_st), QDateTime::currentDateTime(), dev_state_idx);
				(*channels_data_ptr)[1].Add(MBN_EEG::EEG_Device_Architecture::GetRealVoltage__uV(ch1, 1, &adc_st), QDateTime::currentDateTime(), dev_state_idx);
				(*channels_data_ptr)[2].Add(MBN_EEG::EEG_Device_Architecture::GetRealVoltage__uV(ch2, 2, &adc_st), QDateTime::currentDateTime(), dev_state_idx);
				(*channels_data_ptr)[3].Add(MBN_EEG::EEG_Device_Architecture::GetRealVoltage__uV(ch3, 3, &adc_st), QDateTime::currentDateTime(), dev_state_idx);

				break;
			};
		case D_USUAL_CHANNELS_DATA_WITH_STIMUL: // обычные данные со стимулом
			{
				int ch0 = 0;
				int ch1 = 0;
				int ch2 = 0; 
				int ch3 = 0;

				// получаем данные
                MBN_EEG::EEG_D_UsualData_With_Stimul::GetChannelsData<PagedArray<unsigned char> >(pkg, *In_byte_Ptr, ch0, ch1, ch2, ch3);
							
				// получаем состояние ADC
				int dev_state_idx = 0;
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_State adc_st = this->device_states_ptr->GetCurrDevState_ADCState(&dev_state_idx);
				
				// помещаем данные в хранилище
				(*channels_data_ptr)[0].Add(MBN_EEG::EEG_Device_Architecture::GetRealVoltage__uV(ch0, 0, &adc_st), QDateTime::currentDateTime(), dev_state_idx);
				(*channels_data_ptr)[1].Add(MBN_EEG::EEG_Device_Architecture::GetRealVoltage__uV(ch1, 1, &adc_st), QDateTime::currentDateTime(), dev_state_idx);
				(*channels_data_ptr)[2].Add(MBN_EEG::EEG_Device_Architecture::GetRealVoltage__uV(ch2, 2, &adc_st), QDateTime::currentDateTime(), dev_state_idx);
				(*channels_data_ptr)[3].Add(MBN_EEG::EEG_Device_Architecture::GetRealVoltage__uV(ch3, 3, &adc_st), QDateTime::currentDateTime(), dev_state_idx);

				break;
			};
		case D_IMPEDANCE_MBN: // обычные данные со стимулом
			{
				unsigned char ch_num = -1;
				vector<int> ch_data(MBN_EEG::EEG_D_ImpedanceMBN::GetSamplesCount(pkg));

				// получаем данные
                MBN_EEG::EEG_D_ImpedanceMBN::GetChannelData<PagedArray<unsigned char>, vector<int> >(pkg, *In_byte_Ptr, ch_data, 0);
				ch_num = MBN_EEG::EEG_D_ImpedanceMBN::GetChannelNumber<PagedArray<unsigned char> >(pkg, *In_byte_Ptr);

				assert((ch_num >= 0)&&(ch_num <= 5));
							
				// получаем и обновляем состояние главного коммутатора каналов
				int dev_state_idx = 0;
				MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorState ch_mux_st = this->device_states_ptr->GetCurrDevState_ChannelsCommutatorState(&dev_state_idx);
				ch_mux_st.SetImpedanceMBN(ch_num);
				this->device_states_ptr->AddNextDevState(0, &ch_mux_st, 0,0,0,0,0, pkg.id);

				// получаем состояние АЦПа
				dev_state_idx = 0;
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_State adc_st = this->device_states_ptr->GetCurrDevState_ADCState(&dev_state_idx);
							
				// помещаем данные в хранилище
				for (int i = 0; i < ch_data.size(); i++)
					(*channels_data_ptr)[ch_num].Add(MBN_EEG::EEG_Device_Architecture::GetRealVoltage__uV(ch_data[i], ch_num, &adc_st), QDateTime::currentDateTime(), dev_state_idx);

				break;
			};
		};


	};


};

/// ФУНКЦИЯ ИЗМЕНЯЮЩАЯ (ТОЧНЕЕ СОХРАНЯЮЩАЯ) СОСТОЯНИЕ УСТРОЙСТВА ПРИ ПОЛУЧЕНИИ ПОДТВЕРЖДЕНИЯ О ВЫПОЛНЕНИИ КОММАНДЫ
/// pkg - посылка с типом ANSWER
void EEG_Protocol::__process_comm_pkg(CommonPackage &pkg, PagedArray<unsigned char> *In_byte_Ptr, PagedArray<unsigned char> *Out_byte_Ptr)
{
	// если внутреннее хранение состояния устройства отключено - выходим
	if (!this->internalDeviceStatesStorageEnabled) return;

	// при направлении вовнутрь
	if (pkg.direction == CommonPackage::IN_DIR)
	{
		/// в зависимости от данных
		switch(pkg.subtype)
		{
		case A_ERROR: // ответ на запрос последней ошибки  (ПОКА НЕ ИСПОЛЬЗУЕТСЯ)
			{
				break;
			};
		case A_RESET: // подтверждение приёма команды reset
			{
				// текущее состояние устройства
				MBN_EEG::EEG_Device_Architecture::EEG_Common_State new_c_st = this->device_states_ptr->GetCurrDevState_Common_State();
				new_c_st.Booting = true;
				// обновляем поля состояния устройства
				this->device_states_ptr->AddNextDevState(0, 0, 0, 0, 0, 0, &new_c_st, pkg.id);

				break;
			};
		case A_GET_FREQUENCY_TABLE: // ответ на запрос таблицы частот
			{
				// получаем число частот в полученной таблице
				unsigned char freq_count = MBN_EEG::EEG_C_GetFrequencyTable::A_GetFrequenciesCount(pkg);
				
				//массив частот
                vector<unsigned short> f_arr = MBN_EEG::EEG_C_GetFrequencyTable::A_GetFrequenciesList<PagedArray<unsigned char> >(pkg, *In_byte_Ptr);

				//новое состояние АЦПа
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_State new_ADC_state = this->device_states_ptr->GetCurrDevState_ADCState();
				new_ADC_state.FrequencyTable.Set(f_arr);
				// обновляем поля состояния устройства
				this->device_states_ptr->AddNextDevState(0, 0, 0, 0, &new_ADC_state, 0, 0, pkg.id);

				break;
			};
		case A_GET_PROTOCOL_VERSION: // ответ на запрос протокола
			{
				// получаем версию протокола
				unsigned char prot_v = MBN_EEG::EEG_C_GetProtocolVersion::GetProtocolVersion(pkg, *In_byte_Ptr);

				// текущее состояние устройства
				MBN_EEG::EEG_Device_Architecture::EEG_Common_State new_c_st = this->device_states_ptr->GetCurrDevState_Common_State();
				new_c_st.ProtocolVersion = prot_v;
				// обновляем поля состояния устройства
				this->device_states_ptr->AddNextDevState(0, 0, 0, 0, 0, 0, &new_c_st, pkg.id);

				break;
			};
		case A_GET_VERSION: // ответ на запрос версии
			{
				// получаем версию схемотехники
				unsigned char circuity_v = MBN_EEG::EEG_C_GetVersion::GetCircuityVersion(pkg, *In_byte_Ptr);

				// получаем версию прошивки
				unsigned char fimrware_v = MBN_EEG::EEG_C_GetVersion::GetFirmwareVersion(pkg, *In_byte_Ptr);

				// получаем серийник прибора
				unsigned char SN = MBN_EEG::EEG_C_GetVersion::GetSerialNumber(pkg, *In_byte_Ptr);

				// текущее состояние устройства
				MBN_EEG::EEG_Device_Architecture::EEG_Common_State new_c_st = this->device_states_ptr->GetCurrDevState_Common_State();
				new_c_st.CircuityVersion = circuity_v;
				new_c_st.FirmwareVersion = fimrware_v;
				new_c_st.SerialNumber = SN;
				// обновляем поля состояния устройства
				this->device_states_ptr->AddNextDevState(0, 0, 0, 0, 0, 0, &new_c_st, pkg.id);

				break;
			};
		case A_SET_ADS_MUX: // ответ на запрос установки коммутатора каналов АЦП
			{
				// получаем состояние коммутаторов каналов АЦП
				EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode ch0_mux_m =  MBN_EEG::EEG_C_SetADSMux::A_GetChannel_0_MUX_Set(pkg, *In_byte_Ptr);
				EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode ch1_mux_m =  MBN_EEG::EEG_C_SetADSMux::A_GetChannel_1_MUX_Set(pkg, *In_byte_Ptr);
				EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode ch2_mux_m =  MBN_EEG::EEG_C_SetADSMux::A_GetChannel_2_MUX_Set(pkg, *In_byte_Ptr);
				EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Mux_Mode ch3_mux_m =  MBN_EEG::EEG_C_SetADSMux::A_GetChannel_3_MUX_Set(pkg, *In_byte_Ptr);
				
				// текущее состояние устройства
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_State new_ADC_st = this->device_states_ptr->GetCurrDevState_ADCState();
				new_ADC_st.Ch0_MUX = ch0_mux_m;
				new_ADC_st.Ch1_MUX = ch1_mux_m;
				new_ADC_st.Ch2_MUX = ch2_mux_m;
				new_ADC_st.Ch3_MUX = ch3_mux_m;

				// обновляем поля состояния устройства
				this->device_states_ptr->AddNextDevState(0, 0, 0, 0, &new_ADC_st, 0, 0, pkg.id);

				break;
			};
		case A_SET_AMPLIFICATION: // ответ на запрос установки усиления каналов АЦП
			{
				// получаем состояние коммутаторов каналов АЦП
				EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch0_ampl =  MBN_EEG::EEG_C_SetAmplification::A_GetChannel0_Amplification_Set(pkg, *In_byte_Ptr);
				EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch1_ampl =  MBN_EEG::EEG_C_SetAmplification::A_GetChannel1_Amplification_Set(pkg, *In_byte_Ptr);
				EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch2_ampl =  MBN_EEG::EEG_C_SetAmplification::A_GetChannel2_Amplification_Set(pkg, *In_byte_Ptr);
				EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch3_ampl =  MBN_EEG::EEG_C_SetAmplification::A_GetChannel3_Amplification_Set(pkg, *In_byte_Ptr);
				
				// текущее состояние устройства
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_State new_ADC_st = this->device_states_ptr->GetCurrDevState_ADCState();
				new_ADC_st.Ch0_Ampl = ch0_ampl;
				new_ADC_st.Ch1_Ampl = ch1_ampl;
				new_ADC_st.Ch2_Ampl = ch2_ampl;
				new_ADC_st.Ch3_Ampl = ch3_ampl;

				// обновляем поля состояния устройства
				this->device_states_ptr->AddNextDevState(0, 0, 0, 0, &new_ADC_st, 0, 0, pkg.id);

				break;
			};
		case A_SET_CALIBRATION_SHAPE: // ответ на запрос установки формы калибровочного сигнала
			{
				// получаем установленные частоту и форму сигнала
				float cal_freq = MBN_EEG::EEG_C_SetCalibrationShape::A_GetCalibrationFreq_Set(pkg, *In_byte_Ptr);
				MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_Calibrator_Shape cal_shape = MBN_EEG::EEG_C_SetCalibrationShape::A_GetCalibrationSignalShape_Set(pkg, *In_byte_Ptr);
				
				// текущее состояние устройства
				MBN_EEG::EEG_Device_Architecture::EEG_Calibrator_State new_Cal_st = this->device_states_ptr->GetCurrDevState_Calibrator_State();
				new_Cal_st.Frequency = cal_freq;
				new_Cal_st.Shape = cal_shape;

				// обновляем поля состояния устройства
				this->device_states_ptr->AddNextDevState(0, 0, 0, 0, 0, &new_Cal_st, 0, pkg.id);

				break;
			};
		case A_SET_CHANNELS_MODE: // ответ на запрос установки шаблонного режима каналов
			{
				// получаем установленный режим работы каналов
				MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_ChannelsMode ch_mode = 
					MBN_EEG::EEG_C_SetChannelsMode::A_GetChannelsModeSet(pkg, *In_byte_Ptr);

				// текущее состояние устройства
				MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorState new_mcomm_st = this->device_states_ptr->GetCurrDevState_ChannelsCommutatorState();
				
				switch (ch_mode)
				{
				case MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::CH_MODE_CALIBRATION: new_mcomm_st.SetCalibration(); break;
				case MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::CH_MODE_EEG: new_mcomm_st.SetNormal(); break;
				case MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::CH_MODE_IMPEDANCE_ALTONICS: break;
				case MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::CH_MODE_IMPEDANCE_MBN: new_mcomm_st.SetImpedanceMBN(0); break;
				};

				// обновляем поля состояния устройства
				this->device_states_ptr->AddNextDevState(0,  &new_mcomm_st, 0, 0, 0, 0, 0, pkg.id);

				break;
			};
		case A_SET_CHANNEL_LEDS: // ответ на запрос установки подсветки каналов
			{
				// получаем установленный тип подсветки
				MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsArc::EEG_Led_State ch_led_st = 
					MBN_EEG::EEG_C_SetChannelLeds::A_GetChannelLedColor_Set(pkg, *In_byte_Ptr);

				// получаем канал
				unsigned char ch = MBN_EEG::EEG_C_SetChannelLeds::A_GetChannelNumber_Set(pkg, *In_byte_Ptr);

				// текущее состояние устройства
				MBN_EEG::EEG_Device_Architecture::EEG_ChannelsLedsState new_ch_leds_st = this->device_states_ptr->GetCurrDevState_ChannelsLedsState();
				
				// обновляем
				switch (ch)
				{
				case MBN_EEG::EEG_Device_Architecture::EEG_CommonArc::EEG_CHANNEL_A1: new_ch_leds_st.A1_state = ch_led_st; break;
				case MBN_EEG::EEG_Device_Architecture::EEG_CommonArc::EEG_CHANNEL_A2: new_ch_leds_st.A2_state = ch_led_st; break;
				case MBN_EEG::EEG_Device_Architecture::EEG_CommonArc::EEG_CHANNEL_C3: new_ch_leds_st.C3_state = ch_led_st; break;
				case MBN_EEG::EEG_Device_Architecture::EEG_CommonArc::EEG_CHANNEL_C4: new_ch_leds_st.C4_state = ch_led_st; break;
				case MBN_EEG::EEG_Device_Architecture::EEG_CommonArc::EEG_CHANNEL_Fp1: new_ch_leds_st.Fp1_state = ch_led_st; break;
				case MBN_EEG::EEG_Device_Architecture::EEG_CommonArc::EEG_CHANNEL_Fp2: new_ch_leds_st.Fp2_state = ch_led_st; break;
				};
				
				// обновляем поля состояния устройства
				this->device_states_ptr->AddNextDevState(0, 0, &new_ch_leds_st, 0, 0, 0, 0, pkg.id);

				break;
			};
		case A_SET_MUX: // ответ на запрос ручной установки коммутатора каналов
			{
				// получаем установленное состояние коммутатора
				unsigned char u34 = MBN_EEG::EEG_C_SetMux::GetMUX_U34_State(pkg, *In_byte_Ptr);
				unsigned char u24 = MBN_EEG::EEG_C_SetMux::GetMUX_U24_State(pkg, *In_byte_Ptr);
				unsigned char u6 = MBN_EEG::EEG_C_SetMux::GetMUX_U6_State(pkg, *In_byte_Ptr);
				unsigned char u2 = MBN_EEG::EEG_C_SetMux::GetMUX_U2_State(pkg, *In_byte_Ptr);

				// текущее состояние устройства
				MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorState new_ch_comm_st = this->device_states_ptr->GetCurrDevState_ChannelsCommutatorState();
				
				// обновляем
				new_ch_comm_st.U34_State = u34;
				new_ch_comm_st.U24_State = u24;
				new_ch_comm_st.U6_State = u6;
				new_ch_comm_st.U2_State = u2;
				
				// обновляем поля состояния устройства
				this->device_states_ptr->AddNextDevState(0, &new_ch_comm_st, 0, 0, 0, 0, 0, pkg.id);

				break;
			};
		case A_SET_SAMPLING_FREQUENCY: // ответ на запрос ручной установки коммутатора каналов
			{
				// получаем нидекс установленной частоты
				short f_idx = MBN_EEG::EEG_C_SetSamplingFrequency::A_GetFrequencySetIndex(pkg, *In_byte_Ptr);
				
				// текущее состояние устройства
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_State new_ADC_st = this->device_states_ptr->GetCurrDevState_ADCState();
				
				// обновляем
				new_ADC_st.CurrentFrequencyIndex = f_idx;
				
				// обновляем поля состояния устройства
				this->device_states_ptr->AddNextDevState(0, 0, 0, 0, &new_ADC_st, 0, 0, pkg.id);

				break;
			};
		case A_START_CALIBRATION: // ответ на запрос запуска калибровщика
			{
				// текущее состояние устройства
				MBN_EEG::EEG_Device_Architecture::EEG_Calibrator_State new_cal_st = this->device_states_ptr->GetCurrDevState_Calibrator_State();
				
				// обновляем
				new_cal_st.TurnedOn = true;
				
				// обновляем поля состояния устройства
				this->device_states_ptr->AddNextDevState(0, 0, 0, 0, 0, &new_cal_st, 0, pkg.id);

				break;
			};
		case A_START_TRANSLATION: // ответ на запрос запуска семплинга
			{
				// текущее состояние устройства
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_State new_ADC_st = this->device_states_ptr->GetCurrDevState_ADCState();
				MBN_EEG::EEG_Device_Architecture::EEG_Common_State new_comm_st = this->device_states_ptr->GetCurrDevState_Common_State();
				
				// обновляем
				new_ADC_st.TurnedOn = true;
				new_comm_st.Recording = true;
				
				// обновляем поля состояния устройства
				this->device_states_ptr->AddNextDevState(0, 0, 0, 0, &new_ADC_st, 0, &new_comm_st, pkg.id);

				break;
			};
		case A_STOP_CALIBRATION: // ответ на запрос остановки калибровщика
			{
				// текущее состояние устройства
				MBN_EEG::EEG_Device_Architecture::EEG_Calibrator_State new_cal_st = this->device_states_ptr->GetCurrDevState_Calibrator_State();
				
				// обновляем
				new_cal_st.TurnedOn = false;
				
				// обновляем поля состояния устройства
				this->device_states_ptr->AddNextDevState(0, 0, 0, 0, 0, &new_cal_st, 0, pkg.id);

				break;
			};
		case A_STOP_TRANSLATION: // ответ на запрос остановки семплинга
			{
				// текущее состояние устройства
				MBN_EEG::EEG_Device_Architecture::EEG_ADC_State new_ADC_st = this->device_states_ptr->GetCurrDevState_ADCState();
				MBN_EEG::EEG_Device_Architecture::EEG_Common_State new_comm_st = this->device_states_ptr->GetCurrDevState_Common_State();
				
				// обновляем
				new_ADC_st.TurnedOn = false;
				new_comm_st.Recording = false;
				
				// обновляем поля состояния устройства
				this->device_states_ptr->AddNextDevState(0, 0, 0, 0, &new_ADC_st, 0, &new_comm_st, pkg.id);

				break;
			};
		};


	};


};
	
/// ФУНКЦИЯ ИЗМЕНЯЮЩАЯ (ТОЧНЕЕ СОХРАНЯЮЩАЯ) СОСТОЯНИЕ УСТРОЙСТВА ПРИ ПОЛУЧЕНИИ СООБЩЕНИЯ ОТ УСТРОЙСТВА
/// pkg - посылка с типом MESSAGE
void EEG_Protocol::__process_message_pkg(CommonPackage &pkg, PagedArray<unsigned char> *In_byte_Ptr)
{
	// при направлении вовнутрь
	if (pkg.direction == CommonPackage::IN_DIR)
	{
		/// в зависимости от данных
		switch(pkg.subtype)
		{
		case M_DEVICE_READY: // сообщение о готовности устройства
			{
				// текущее состояние устройства
				MBN_EEG::EEG_Device_Architecture::EEG_Common_State new_c_st = this->device_states_ptr->GetCurrDevState_Common_State();
				new_c_st.Booting = false;
				// обновляем поля состояния устройства
				this->device_states_ptr->AddNextDevState(0, 0, 0, 0, 0, 0, &new_c_st, pkg.id);

				break;
			};
		};
	};


};

/// Пробует применить указанную посылку для подтверждения выполнения комманды из выходной очереди.
/// ТОЛЬКО ДЛЯ ПОСЫЛОК, КОТОРЫЕ ЯВЛЯЮТСЯ ОТВЕТАМИ
/// Сначала пробует применить её, конечно, к Unsubmitted_pkg_idx посылке, но если это не удаётся,
/// то применяет её к ближайшей более поздней посылке.
/// Возвращает индекс посылки в выходной очереди, к которой указанная посылка была применёна как подтверждение
/// Если найти соответствующую посылку не удалось - возвращает -1
int EEG_Protocol::__apply_ack(CommonPackage &in_pkg, int Unsubmitted_pkg_idx,
                              PagedArray<CommonPackage> *In_pkg_Ptr,  PagedArray<unsigned char> *In_byte_Ptr,
                              PagedArray<CommonPackage> *Out_pkg_Ptr, PagedArray<unsigned char> *Out_byte_Ptr,
							  int next_pack_to_write_idx)
{
	//бежим по всем посылкам начиная с первой неподтверждённой и до первой неотправленной посылки выходной очереди
	for (int i = Unsubmitted_pkg_idx; i < next_pack_to_write_idx; i++)
	{
		CommonPackage out_pkg = (*Out_pkg_Ptr)[i];
		
		// проверяем, может ли быть данная посылка подтверждением посылки out_pkg
			// подтип посылок должен совпадать и выходная посылка не была подтверждена ещё
		if ((in_pkg.subtype == out_pkg.subtype) && (!out_pkg.ack))
		{
			// данные посылок должны соответствовать друг-другу
			// в некоторых подтипах нету данных для сравнения, поэтому сразу
			// возвращаем значение индекса
			switch(in_pkg.subtype)
			{
			case MBN_EEG::EEG_Protocol::A_ERROR: return i;
			case MBN_EEG::EEG_Protocol::A_RESET: return i;
			case MBN_EEG::EEG_Protocol::A_GET_FREQUENCY_TABLE: return i;
			case MBN_EEG::EEG_Protocol::A_GET_PROTOCOL_VERSION: return i;
			case MBN_EEG::EEG_Protocol::A_GET_VERSION: return i;
			case MBN_EEG::EEG_Protocol::A_SET_ADS_MUX: 
				{
					// данные в ответе и в команде должны быть одинакомыми
					if (MBN_EEG::EEG_C_SetADSMux::A_GetChannel_0_MUX_Set(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetADSMux::A_GetChannel_0_MUX_Set(out_pkg, *Out_byte_Ptr))
						 continue;
					if (MBN_EEG::EEG_C_SetADSMux::A_GetChannel_1_MUX_Set(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetADSMux::A_GetChannel_1_MUX_Set(out_pkg, *Out_byte_Ptr))
						 continue;
					if (MBN_EEG::EEG_C_SetADSMux::A_GetChannel_2_MUX_Set(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetADSMux::A_GetChannel_2_MUX_Set(out_pkg, *Out_byte_Ptr))
						 continue;
					if (MBN_EEG::EEG_C_SetADSMux::A_GetChannel_3_MUX_Set(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetADSMux::A_GetChannel_3_MUX_Set(out_pkg, *Out_byte_Ptr))
						 continue;

					return i;
				}
			case MBN_EEG::EEG_Protocol::A_SET_AMPLIFICATION: 
				{
					// данные в ответе и в команде должны быть одинакомыми
					if (MBN_EEG::EEG_C_SetAmplification::A_GetChannel0_Amplification_Set(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetAmplification::A_GetChannel0_Amplification_Set(out_pkg, *Out_byte_Ptr))
						 continue;
					if (MBN_EEG::EEG_C_SetAmplification::A_GetChannel1_Amplification_Set(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetAmplification::A_GetChannel1_Amplification_Set(out_pkg, *Out_byte_Ptr))
						 continue;
					if (MBN_EEG::EEG_C_SetAmplification::A_GetChannel2_Amplification_Set(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetAmplification::A_GetChannel2_Amplification_Set(out_pkg, *Out_byte_Ptr))
						 continue;
					if (MBN_EEG::EEG_C_SetAmplification::A_GetChannel3_Amplification_Set(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetAmplification::A_GetChannel3_Amplification_Set(out_pkg, *Out_byte_Ptr))
						 continue;

					return i;
				}
			case MBN_EEG::EEG_Protocol::A_SET_CALIBRATION_SHAPE: 
				{
					// данные в ответе и в команде должны быть одинакомыми
                    if (fabsf(MBN_EEG::EEG_C_SetCalibrationShape::A_GetCalibrationFreq_Set(in_pkg, *In_byte_Ptr) -
						    MBN_EEG::EEG_C_SetCalibrationShape::A_GetCalibrationFreq_Set(out_pkg, *Out_byte_Ptr)) 
								> 2.0 * MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_CALIBRATION_SIGNAL_FREQUENCY_STEP_HZ)
						 continue;

					if (MBN_EEG::EEG_C_SetCalibrationShape::A_GetCalibrationSignalShape_Set(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetCalibrationShape::A_GetCalibrationSignalShape_Set(out_pkg, *Out_byte_Ptr))
						 continue;

					return i;
				}
			case MBN_EEG::EEG_Protocol::A_SET_CHANNELS_MODE: 
				{
					// данные в ответе и в команде должны быть одинакомыми
					if (MBN_EEG::EEG_C_SetChannelsMode::A_GetChannelsModeSet(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetChannelsMode::A_GetChannelsModeSet(out_pkg, *Out_byte_Ptr))
						 continue;

					return i;
				}
			case MBN_EEG::EEG_Protocol::A_SET_CHANNEL_LEDS: 
				{
					// данные в ответе и в команде должны быть одинакомыми
					if (MBN_EEG::EEG_C_SetChannelLeds::A_GetChannelNumber_Set(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetChannelLeds::A_GetChannelNumber_Set(out_pkg, *Out_byte_Ptr))
						 continue;
					if (MBN_EEG::EEG_C_SetChannelLeds::A_GetChannelLedColor_Set(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetChannelLeds::A_GetChannelLedColor_Set(out_pkg, *Out_byte_Ptr))
						 continue;

					return i;
				}
			case MBN_EEG::EEG_Protocol::A_SET_MUX: 
				{
					// данные в ответе и в команде должны быть одинакомыми
					if (MBN_EEG::EEG_C_SetMux::GetMUX_U34_State(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetMux::GetMUX_U34_State(out_pkg, *Out_byte_Ptr))
						 continue;
					if (MBN_EEG::EEG_C_SetMux::GetMUX_U24_State(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetMux::GetMUX_U24_State(out_pkg, *Out_byte_Ptr))
						 continue;
					if (MBN_EEG::EEG_C_SetMux::GetMUX_U6_State(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetMux::GetMUX_U6_State(out_pkg, *Out_byte_Ptr))
						 continue;
					if (MBN_EEG::EEG_C_SetMux::GetMUX_U2_State(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetMux::GetMUX_U2_State(out_pkg, *Out_byte_Ptr))
						 continue;

					return i;
				}
			case MBN_EEG::EEG_Protocol::A_SET_SAMPLING_FREQUENCY:
				{
					// данные в ответе и в команде должны быть одинакомыми
					if (MBN_EEG::EEG_C_SetSamplingFrequency::A_GetFrequencySetIndex(in_pkg, *In_byte_Ptr) !=
						MBN_EEG::EEG_C_SetSamplingFrequency::A_GetFrequencySetIndex(out_pkg, *Out_byte_Ptr))
						 continue;

					return i;
				}
			case MBN_EEG::EEG_Protocol::A_START_CALIBRATION: return i;
			case MBN_EEG::EEG_Protocol::A_START_TRANSLATION: return i;
			case MBN_EEG::EEG_Protocol::A_STOP_CALIBRATION: return i;	
			case MBN_EEG::EEG_Protocol::A_STOP_TRANSLATION: return i;		
			};
		};
	};

	return -1;
};

/// Функция применяет политику протокола к очередям сообщений в целом,
/// определяет на какие команды был получен ответ,
/// на какие был получен изменённый ответ,
/// на какие ответ не был получен,
/// решает вопрос о состоянии канала связи и устройства (сбой - норма)
void EEG_Protocol::__apply_protocol_policy(PagedArray<CommonPackage> *In_pkg_Ptr, PagedArray<CommonPackage> *Out_pkg_Ptr, PagedArray<unsigned char> *In_byte_Ptr, PagedArray<unsigned char> *Out_byte_Ptr)
{
	// текущая обрабатываемая посылка
	CommonPackage &pkg = (*(In_pkg_Ptr))[pkg_idx_to_process];


};


/// записывает в очередь на вывод посылку и её байт код
void EEG_Protocol::__post_package(const CommonPackage &pack, PagedArray<CommonPackage> *Out_pkg_Ptr, PagedArray<unsigned char> *Out_byte_Ptr)
{
	// добавляем посылку в выходную очередь
	Out_pkg_Ptr->push_back(pack);

	// устанавливаем правильно индекс её первого байта
	(*Out_pkg_Ptr)[Out_pkg_Ptr->size() - 1].first_byte_stream_offset = Out_byte_Ptr->size();

	// добавляем байт-код посылки в байт-очередь
		// ресайзим массив, чтобы вместил данные посылки
	Out_byte_Ptr->resize(Out_byte_Ptr->size() + pack.direct_data.size());
		// копируем данные
	for (int i = Out_byte_Ptr->size() - pack.direct_data.size(); i <= Out_byte_Ptr->size() - 1; i++)
		(*Out_byte_Ptr)[i] = pack.direct_data[i - Out_byte_Ptr->size() + pack.direct_data.size()];
};

/// для режима эмуляции устройства, - отвечает на пришедшую с хоста команду
void EEG_Protocol::__dev_answer(CommonPackage &c_pkg, PagedArray<CommonPackage> *Out_pkg_Ptr, PagedArray<unsigned char> *Out_byte_Ptr,
                                 PagedArray<unsigned char> *In_byte_Ptr)
{
	assert(!computer_side_mode);

	switch(c_pkg.subtype)
	{
	case C_RESET: 
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			pkg.direct_data.resize(MBN_EEG::EEG_C_Reset::A_GetSize()); 
            MBN_EEG::EEG_C_Reset::CreatePackage<vector<unsigned char> >(pkg, pkg.direct_data, offset);
			
			__post_package(pkg, Out_pkg_Ptr, Out_byte_Ptr);

			break;
		};
	case C_GET_VERSION: 
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			pkg.direct_data.resize(MBN_EEG::EEG_C_GetVersion::A_GetSize()); 
            MBN_EEG::EEG_C_GetVersion::CreateDevicePackage<vector<unsigned char> >(pkg, pkg.direct_data, offset, 1, 2, 3456);
			
			__post_package(pkg, Out_pkg_Ptr, Out_byte_Ptr);

			break;
		};
	case C_SET_MUX: 
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			pkg.direct_data.resize(MBN_EEG::EEG_C_SetMux::A_GetSize()); 
            MBN_EEG::EEG_C_SetMux::CreateDevicePackage<vector<unsigned char> >(pkg, pkg.direct_data, offset,
                MBN_EEG::EEG_C_SetMux::GetMUX_U34_State< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)),
                MBN_EEG::EEG_C_SetMux::GetMUX_U6_State< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)),
                MBN_EEG::EEG_C_SetMux::GetMUX_U24_State< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)),
                MBN_EEG::EEG_C_SetMux::GetMUX_U2_State< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr))  );
			
			__post_package(pkg, Out_pkg_Ptr, Out_byte_Ptr);

			break;
		};
	case C_GET_PROTOCOL_VERSION: 
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			pkg.direct_data.resize(MBN_EEG::EEG_C_GetProtocolVersion::A_GetSize()); 
            MBN_EEG::EEG_C_GetProtocolVersion::CreateDevicePackage<vector<unsigned char> >(pkg, pkg.direct_data, offset, 56);
			
			__post_package(pkg, Out_pkg_Ptr, Out_byte_Ptr);

			break;
		};
	case C_GET_FREQUENCY_TABLE: 
		{
			// таблица частот
			vector<unsigned short> table;
			table.push_back(250);
			table.push_back(500);
			table.push_back(1000);
			table.push_back(2000);
			table.push_back(4000);

			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			pkg.direct_data.resize(5 + table.size()*2); 
            MBN_EEG::EEG_C_GetFrequencyTable::CreateDevicePackage<vector<unsigned char> >(pkg, pkg.direct_data, offset, table);
			
			__post_package(pkg, Out_pkg_Ptr, Out_byte_Ptr);

			break;
		};
	case C_SET_SAMPLING_FREQUENCY: 
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			pkg.direct_data.resize(EEG_C_SetSamplingFrequency::A_GetSize()); 
            MBN_EEG::EEG_C_SetSamplingFrequency::CreateDevicePackage<vector<unsigned char> >(pkg, pkg.direct_data, offset,
                MBN_EEG::EEG_C_SetSamplingFrequency::A_GetFrequencySetIndex< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)));
			
			__post_package(pkg, Out_pkg_Ptr, Out_byte_Ptr);

			break;
		};
	case C_SET_CHANNELS_MODE: 
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			pkg.direct_data.resize(EEG_C_SetChannelsMode::A_GetSize()); 
            MBN_EEG::EEG_C_SetChannelsMode::CreateDevicePackage<vector<unsigned char> >(pkg, pkg.direct_data, offset,
                MBN_EEG::EEG_C_SetChannelsMode::A_GetChannelsModeSet< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)),
                MBN_EEG::EEG_C_SetChannelsMode::A_IsCh0_activeSet< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)),
                MBN_EEG::EEG_C_SetChannelsMode::A_IsCh1_activeSet< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)),
                MBN_EEG::EEG_C_SetChannelsMode::A_IsCh2_activeSet< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)),
                MBN_EEG::EEG_C_SetChannelsMode::A_IsCh3_activeSet< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)));
			
			__post_package(pkg, Out_pkg_Ptr, Out_byte_Ptr);

			break;
		};
	case C_SET_CALIBRATION_SHAPE: 
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			pkg.direct_data.resize(EEG_C_SetCalibrationShape::A_GetSize()); 
            MBN_EEG::EEG_C_SetCalibrationShape::CreateDevicePackage<vector<unsigned char> >(pkg, pkg.direct_data, offset,
                MBN_EEG::EEG_C_SetCalibrationShape::A_GetCalibrationSignalShape_Set< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)),
                MBN_EEG::EEG_C_SetCalibrationShape::A_GetCalibrationFreq_Set< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)));
			
			__post_package(pkg, Out_pkg_Ptr, Out_byte_Ptr);

			break;
		};
	case C_START_CALIBRATION: 
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			pkg.direct_data.resize(MBN_EEG::EEG_C_StartCalibration::A_GetSize()); 
            MBN_EEG::EEG_C_StartCalibration::CreateDevicePackage<vector<unsigned char> >(pkg, pkg.direct_data, offset);
			
			__post_package(pkg, Out_pkg_Ptr, Out_byte_Ptr);

			break;
		};
	case C_STOP_CALIBRATION: 
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			pkg.direct_data.resize(MBN_EEG::EEG_C_StopCalibration::A_GetSize()); 
            MBN_EEG::EEG_C_StopCalibration::CreateDevicePackage<vector<unsigned char> >(pkg, pkg.direct_data, offset);
			
			__post_package(pkg, Out_pkg_Ptr, Out_byte_Ptr);

			break;
		};
	case C_START_TRANSLATION: 
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			pkg.direct_data.resize(MBN_EEG::EEG_C_StartTranslation::A_GetSize()); 
            MBN_EEG::EEG_C_StartTranslation::CreateDevicePackage<vector<unsigned char> >(pkg, pkg.direct_data, offset);
			
			__post_package(pkg, Out_pkg_Ptr, Out_byte_Ptr);

			break;
		};
	case C_STOP_TRANSLATION: 
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			pkg.direct_data.resize(MBN_EEG::EEG_C_StopTranslation::A_GetSize()); 
            MBN_EEG::EEG_C_StopTranslation::CreateDevicePackage<vector<unsigned char> >(pkg, pkg.direct_data, offset);
			
			__post_package(pkg, Out_pkg_Ptr, Out_byte_Ptr);

			break;
		};
	case C_SET_CHANNEL_LEDS: 
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			pkg.direct_data.resize(MBN_EEG::EEG_C_SetChannelLeds::A_GetSize()); 
            MBN_EEG::EEG_C_SetChannelLeds::CreateDevicePackage<vector<unsigned char> >(pkg, pkg.direct_data, offset,
                MBN_EEG::EEG_C_SetChannelLeds::A_GetChannelNumber_Set< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)),
                MBN_EEG::EEG_C_SetChannelLeds::A_GetChannelLedColor_Set< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)));
			
			__post_package(pkg, Out_pkg_Ptr, Out_byte_Ptr);

			break;
		};
	case C_SET_AMPLIFICATION: 
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			pkg.direct_data.resize(MBN_EEG::EEG_C_SetAmplification::A_GetSize()); 
            MBN_EEG::EEG_C_SetAmplification::CreateDevicePackage<vector<unsigned char> >(pkg, pkg.direct_data, offset,
                MBN_EEG::EEG_C_SetAmplification::A_GetChannel0_Amplification_Set< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)),
                MBN_EEG::EEG_C_SetAmplification::A_GetChannel1_Amplification_Set< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)),
                MBN_EEG::EEG_C_SetAmplification::A_GetChannel2_Amplification_Set< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)),
                MBN_EEG::EEG_C_SetAmplification::A_GetChannel3_Amplification_Set< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr))
				); 
			
			__post_package(pkg, Out_pkg_Ptr, Out_byte_Ptr);

			break;
		};
	case C_SET_ADS_MUX: 
		{
			/// создаём и отправляем посылку
			CommonPackage pkg;
            long long offset = 0;
			pkg.direct_data.resize(MBN_EEG::EEG_C_SetADSMux::A_GetSize()); 
            MBN_EEG::EEG_C_SetADSMux::CreateDevicePackage<vector<unsigned char> >(pkg, pkg.direct_data, offset,
                MBN_EEG::EEG_C_SetADSMux::A_GetChannel_0_MUX_Set< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)),
                MBN_EEG::EEG_C_SetADSMux::A_GetChannel_1_MUX_Set< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)),
                MBN_EEG::EEG_C_SetADSMux::A_GetChannel_2_MUX_Set< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr)),
                MBN_EEG::EEG_C_SetADSMux::A_GetChannel_3_MUX_Set< PagedArray<unsigned char> >(c_pkg, (*In_byte_Ptr))
				); 
			
			__post_package(pkg, Out_pkg_Ptr, Out_byte_Ptr);

			break;
		};
	};
};


};

#endif
