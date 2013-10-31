/*************************************************************************
*
*	Содержит виджет отображения параметров каналов (Только отображения)
*		Все расчёты ведутся в анализаторе сигнала.
*	Виджет содержит:
*		*) Монитор спектра канала
*		*) Минимальное и максимальное значения канала
*		*) Минимальное и максимальное значение выделенной области канала
*		*) Параметры натянутого Синуса
*		*) Параметры натянутого Прямоугольника
*		*) Параметры натянутого Треугольника
*
*************************************************************************/

#define _USE_MATH_DEFINES

#include <vector>
#include <PagedArray.cpp>
#include "Channel_Monitor.h"
#include "MBN_SignalProcessor.h"
#include <assert.h>
#include <math.h>



using namespace std;

/************************************************************
*
*			Безинтерфейсный инструмент измерения импеданса
*
************************************************************/



/// реакция на приход данных импеданса
/// тут мы по сути должны получить данные от устройства и  сложить их в массив для обработки
void MBN_Interfaceless_ImpedanceMeasurer::got_impedance_data_handler(void *u_ptr, unsigned char ch_num, const vector<int> *const chan_data)
{
	// значения, которые интерпретируются, как зашкал
	int MAX_OVERFLOW = 8388607; 
	int MIN_OVERFLOW = -8388607;

	// если данные не равны 0 (нулями пакеты прибор заполняет если не успевает
	// оцифровать весь объём данных, предназначенных для высылки, поэтому,
	// нулевые данные мы фактически вырезаем)
	// массив входных данных, в котором отсутствуют нули
	vector<int> ch_data_without_zeroes;
	
	// это сейчас не используется
		// создаём фильтрованный (без нулей) массив // пока отменено!
	for (int i = 0; i < chan_data->size(); i++)
		//if ((*chan_data)[i] != 0)
			ch_data_without_zeroes.push_back((*chan_data)[i]);


	MBN_Interfaceless_ImpedanceMeasurer *_this = (MBN_Interfaceless_ImpedanceMeasurer*)u_ptr;

	// вычисляем сколько по скольким посылкам (максимум) надо определять импеданс
	int max_samples_count = _this->impedance_sps * _this->averaging_time_interval;

	// индекс нужного массива данных
	int data_index = -1;
		// ищем нужный массив данных
	for (int i = 0; i < _this->channels_data.size(); i++)
		if (_this->channels_data[i].channel_number == ch_num)
		{
			data_index = i;
			break;
		};

	// канал должен найтись
	assert(data_index != -1);

	// блокируем доступ к основному массиву
    //_this->channels_data[data_index].data.StartAccsess();
	//реорганизуем массив данных канала
		// временное хранилище


	vector<float> temp_data;
    temp_data.resize(qMin(max_samples_count, (int)(_this->channels_data[data_index].data.size() + ch_data_without_zeroes.size())));
		// заполняем его			
			// индекс в источнике
	int src_idx = ch_data_without_zeroes.size() - 1;
			// индекс в приёмнике
	int dst_idx = temp_data.size() - 1; 
			// заполняем сначала данными новой посылки
	for (; dst_idx >= 0; dst_idx--) // заполняем сначала данными из посылки				
	{
		if (src_idx >= 0) 
		{
			temp_data[dst_idx] = ch_data_without_zeroes[src_idx]; // пока не уткнулись в начало массива
			//assert( ch_data_without_zeroes[src_idx] );
		}
		else break;

		src_idx--;
	};
			// смещение от старых данных
    src_idx = _this->channels_data[data_index].data.size() - 1;
			// если хранилище заполняется не полностью данными из посылки, заполняем его старыми остатками
	for (; dst_idx >= 0; dst_idx--) // заполняем сначала данными из посылки				
    {
        if (src_idx >= 0)
            temp_data[dst_idx] = _this->channels_data[data_index].data[src_idx]; //
        else break;

		src_idx--;
	};

	//тут dst_idx должен всегда быть == -1
	assert(dst_idx == -1);

	// перемещаем данные из временного хранилища в постоянное
    _this->channels_data[data_index].data = temp_data;

	// высвобождаем доступ к основному массиву
    //_this->channels_data[data_index].data.EndAccsess();


	// проверяем на переполнение
	_this->overflow_flags[data_index] = false;
    for (int i = 0; i < _this->channels_data[data_index].data.size(); i++)
        if ((_this->channels_data[data_index].data[i] >= (float)MAX_OVERFLOW) ||
            (_this->channels_data[data_index].data[i] <= (float)MIN_OVERFLOW))
        {
            _this->overflow_flags[data_index] = true;
            break;
        };

	// рассчитываем импеданс по имеющимся данным
	_this->CalculateImpedanceOnCurrentData();	
};

/// расчёт и отображение импеданса на основе текущих данных
void MBN_Interfaceless_ImpedanceMeasurer::CalculateImpedanceOnCurrentData()
{
	// для каждого канала рассчитываем импеданс
	for (int i = 0; i < CHANNELS_COUNT; i++)
		CalculateImpedanceOnCurrentDataForChannel(i);
};

/// расчёт и отображение импеданса на основе текущих данных для указанного канала
void MBN_Interfaceless_ImpedanceMeasurer::CalculateImpedanceOnCurrentDataForChannel(int ch_index)
{
	// ширина половины максимального окна усреднения (должна быть кратна полупериоду сигнала)
	// полная максимальная ширина окна = 2*half_of_averaging_window_width + 1
		// т.е. это сколько точек надо взять слева (или справа) от центральной
	int half_of_averaging_window_width = 50;

	// число применяемых фильтраций
	int filtrations_count = 2;

	// хранилища для данных, в 1 хранится сигнал, 2 - приемник фильтрованного сигнала, либо наоборот
	vector<float> data1;
	vector<float> data2;

	// оригинальный сигнал (указывает на data1 либо data2)
	vector<float> *source_signal_ptr = &data1;

	// фильтрованный сигнал (указывает на data1 либо data2)
	vector<float> *filtered_signal_ptr = &data2;
	
	// заполняем исходный оригинальный сигнал
    source_signal_ptr->resize(channels_data[ch_index].data.size());
    for (int i = 0; i < channels_data[ch_index].data.size(); i++)
        (*source_signal_ptr)[i] = channels_data[ch_index].data[i];

	// задаём исходный массив фильтрованного сигнала
    filtered_signal_ptr->resize(channels_data[ch_index].data.size());

	// теперь фильтруем его столько раз, сколько надо
	for (int filtering = 1; filtering <= filtrations_count; filtering++)
	{
		// фильтруем
			// значение индексов границ плавающего окна (включительно)
		int fwindow_start_idx = 0;
		int fwindow_end_idx = min((signed int)(source_signal_ptr->size()) - 1, half_of_averaging_window_width);
		int fwindow_center_idx = 0;
			// значение плавающего окна (не нормированного)
		float fwindow_val = 0.0;
			// стартовое значение плавающего окна
		for (int i = fwindow_start_idx; i <= fwindow_end_idx; i++)
			fwindow_val += (*source_signal_ptr)[i];
		//if (fwindow_end_idx - fwindow_start_idx + 1 != 0)
			//fwindow_val /= ((float)fwindow_end_idx) - ((float)fwindow_start_idx) + 1.0f;

			// сама фильтрация
		for (int i = 0; i < source_signal_ptr->size(); i++)
		{
				// обновляем значение окна для текущей итерации
					// старый размер окна
			//float old_fwin_size = (float)fwindow_end_idx - (float)fwindow_start_idx + 1.0f;
					// старый правый индекс
			int old_fwin_end_index = fwindow_end_idx;
					// основные индексы окна
			fwindow_center_idx = i;				
			fwindow_start_idx = max(0, fwindow_center_idx - half_of_averaging_window_width);
            fwindow_end_idx = qMin((int)(source_signal_ptr->size() - 1), fwindow_center_idx + half_of_averaging_window_width);
					// новый размер окна
			float new_fwin_size = (float)fwindow_end_idx - (float)fwindow_start_idx + 1.0f;
					// перемасштабируем значение окна под новый размер окна (если он менялся)
			//fwindow_val = fwindow_val * old_fwin_size / new_fwin_size;
					// модификаторы значения плавающего окна
						// если левый край окна не равен 0, значит на предыдущей итерации он был
						// левее, и надо выкинуть вклад находящейся за левой границей точки
			if (fwindow_start_idx > 0)
				fwindow_val -= (*source_signal_ptr)[fwindow_start_idx - 1];
						// аналогично для правой точки (её надо добавить)
			if (old_fwin_end_index < fwindow_end_idx)
				fwindow_val += (*source_signal_ptr)[fwindow_end_idx];

            assert( new_fwin_size >= qMin((int)(source_signal_ptr->size()), half_of_averaging_window_width));

				// фильтрация
			(*filtered_signal_ptr)[i] = (*source_signal_ptr)[i] - fwindow_val / new_fwin_size;
		};

		// меняем местами исходные и фильтрованный сигналы
		vector<float> *temp_ptr = filtered_signal_ptr;
		filtered_signal_ptr = source_signal_ptr;
		source_signal_ptr = temp_ptr;
	};	

	// тут source_signal_ptr указывает на отфильтрованный  filtrations_count раз сигнал

	// копируем данные сигнала для отображения	
    filtered_channels_data[ch_index].data.resize((*source_signal_ptr).size());
    for (int i = 0; i < (*source_signal_ptr).size(); i++)
        filtered_channels_data[ch_index].data[i] = (*source_signal_ptr)[i];

	// задаём теперь рассчёт на основе дисперсии сигнала
	// D ~ A*A /2, где А - амплитуда синуса
	float D = 0.0;
	if (source_signal_ptr->size()) 
		D = MBN_SignalProcessor1D::CalculateDispersion(*source_signal_ptr, 0, source_signal_ptr->size() - 1);

	// сопротивление
	float R = sqrt(D);
		// с поправочным коэффициентом
	R *= 49.0f/957.0f;

	// оповещаем о том что, расчёт очередного импеданса закончен
	ImpedanceChanged(ch_index, R, this->overflow_flags[ch_index]);
};

// конструктор
MBN_Interfaceless_ImpedanceMeasurer::MBN_Interfaceless_ImpedanceMeasurer(MBN_EEG::EEG *eeg, QObject *parent): QObject(parent)
{
	// стартовые значения
	eeg_ptr = 0;
	CHANNELS_COUNT = 6;
	impedance_sps = 500;
	impedance_current = 0.001;
	averaging_time_interval = 2;

    // флаги перегрузаMBN_Interfaceless_ImpedanceMeasurer
	overflow_flags.resize(CHANNELS_COUNT);
	for (int i = 0; i < overflow_flags.size(); i++)
		overflow_flags[i] = false;


	// подготавливаем хранилища данных
		// оригинальный сигнал
    channels_data.resize(CHANNELS_COUNT);
	for (int i = 0; i < CHANNELS_COUNT; i++)
	{
		channels_data[i].channel_number = i;
        channels_data[i].amp_coeff = 1.0;
    };
		// фильтрованный сигнал
	filtered_channels_data.resize(CHANNELS_COUNT);
	for (int i = 0; i < CHANNELS_COUNT; i++)
	{
		filtered_channels_data[i].channel_number = i;
		filtered_channels_data[i].amp_coeff = 1.0;
	};	
		
	// если указали устройство - подключаемся к нему
	ConnectToEEG(eeg);
};

// прикрепить инструмент у указанному ээг,
// если равен 0, то отключает текущий ээг
void MBN_Interfaceless_ImpedanceMeasurer::ConnectToEEG(MBN_EEG::EEG *eeg)
{
	// если до этого был установлено другое устройство - убираем его
	if (eeg_ptr)
	{		
		//убираем наши обработчики из контроллера прерываний
		eeg_ptr->RemoveEventVector(got_impedance_data_vector);

		// удаляем наши вектора
		delete got_impedance_data_vector;

		// очищаем массивы данных
		for (int i = 0; i < CHANNELS_COUNT; i++)
            channels_data[i].data.resize(0);
	};

	// сохраняем указатель на устройство
	eeg_ptr = eeg;

	// если установили какое-то устройство
	if (eeg_ptr)
	{
		//создаём вектора прерываний
		got_impedance_data_vector = new MBN_EEG::EEG_Event_Vector_D_ImpedanceMBN(&got_impedance_data_handler, this);
		//добавляем их в контроллер прерываний
		eeg_ptr->AddEventVector(got_impedance_data_vector);		

		//включаем прерывания (события)
		eeg_ptr->EnableEvents();
	};
};

// Включить инструмент
void MBN_Interfaceless_ImpedanceMeasurer::SwitchOnImpedanceMode()
{
	// если не задано устройство - выходим
	if (!eeg_ptr) return;

	// запускаем режим измерения импеданса МБН
	eeg_ptr->CS_SetChannelsMode(MBN_EEG::EEG_Device_Architecture::EEG_ChannelsCommutatorArc::CH_MODE_IMPEDANCE_MBN);
};

// Выключить инструмент
void MBN_Interfaceless_ImpedanceMeasurer::SwitchOffImpedanceMode()
{
	// если не задано устройство - выходим
	if (!eeg_ptr) return;

	// запускаем режим измерения импеданса МБН
	eeg_ptr->CS_StopRecording();
};

// реакция на удаление экземпляра класса ээг, к которому подключены
void MBN_Interfaceless_ImpedanceMeasurer::OnEEGAboutToDelete(int index)
{
	ConnectToEEG(0);
};

