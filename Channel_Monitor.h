/*************************************************************************
*
*	Содержит виджет отображения параметров канала (Только отображения)
*	Только Одного канала
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

#ifndef MBN_CHANNELS_MONITOR
#define MBN_CHANNELS_MONITOR

#include <vector>
#include <PagedArray.cpp>
#include <list>
#include <QString>
#include <EEG.h>
#include <multithreading_flag.h>

//using namespace std;

// инструмент измерения импеданса
class MBN_Interfaceless_ImpedanceMeasurer: public QObject
{
	Q_OBJECT

// внутренние структуры
private:

	/// данные канала, необходимые для измерения импеданса
	struct ChannelData
	{
		/// номер канала на устройстве (как он приходит в посылке)
		unsigned char channel_number;

		/// текущий коэффициент усиления канала
		float amp_coeff;

		/// данные отсчётов
        vector<float> data;
	};

// параметры для вычисления импеданса
private:

	// для первой версии - постоянное число каналова
	int CHANNELS_COUNT;

	// константа - частота оцифровки при импедансе (сэмплах в секунду)
	int impedance_sps;

	// константа - амплитуда подаваемого тока при импедансе (в мА)
	float impedance_current;

private:	

	// указатель на экземпляр устройства
	MBN_EEG::EEG *eeg_ptr;

	// время (в секундах) за которое мы усредняем данные импеданса
	float averaging_time_interval;

	// текущая частота дискретизации по всем каналам
	int current_sps;
	
	// интересующие нас отсчёты массивы данных в вольтах по каналам
	vector<ChannelData> channels_data;
	// интересующие нас отсчёты массивы данных в вольтах по каналам
	vector<ChannelData> filtered_channels_data;
	// флаги зашкала каналов
	vector<bool> overflow_flags;


	// обработчик прихода данных в режиме импеданса
	MBN_EEG::EEG_Event_Vector_D_ImpedanceMBN *got_impedance_data_vector;


	/// расчёт и отображение импеданса на основе текущих данных
	void CalculateImpedanceOnCurrentData();

	/// расчёт и отображение импеданса на основе текущих данных для указанного канала
	/// ch_index - индекс канала в массиве channels_data
	void CalculateImpedanceOnCurrentDataForChannel(int ch_index);

/// функциональность
public:

	// конструктор
	MBN_Interfaceless_ImpedanceMeasurer(MBN_EEG::EEG *eeg = 0, QObject *parent = 0);

	// прикрепить инструмент у указанному ээг,
	// если равен 0, то отключает текущий ээг
	void ConnectToEEG(MBN_EEG::EEG *eeg = 0);

	// Включить режим импеданса
	void SwitchOnImpedanceMode();

	// Выключить  режим импеданса
	void SwitchOffImpedanceMode();

private:

	/// реакция на приход данных импеданса
	static void got_impedance_data_handler(void *u_ptr, unsigned char ch_num, const vector<int> *const ch_data);
	/// реакция на приход ответа по команде
    static void got_command_handler(long long command_pkg_id, bool acked, void* u_ptr, MBN_EEG::EEG_Event::EEG_EVENT_SUBTYPE e_stype);

public slots:

	// реакция на удаление экземпляра класса ээг, к которому подключены
	void OnEEGAboutToDelete(int index);

signals:

	// Вызывается, когда закончен очередной расчёт импеданса для канала
	void ImpedanceChanged(int channel_index, float resistance, bool overflow);
};



#endif MBN_CHANNEL_MONITOR
