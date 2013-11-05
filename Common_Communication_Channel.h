/***************************************************************************************
*
*	Активный модуль связи с прибором, работающим по пакетному протоколу. 
*		Общается с прибором, предоставляя верхним
*	программным уровням абстракцию очереди сообщений на передачу и очереди принятых 
*	сообщений. 
*
*	Знает только о существовании посылок. 
*		*) Поставленные в очередь на отправку посылки
*	отправляются на устройство сразу, как появляется свободное от обработки приходящих 
*	сообщений время. 
*		*) Основное время постоянно проверяет, не пришли ли какие-либо данные
*	от устройства. 
*		*) Пришедшие данные парсит с помощью подключённого парсера байт кода, 
*	соответствующего конкретному устройству. 
*		*) Результаты парсинга помешаются в специализорованные структуры, уникальные 
*	для каждого устройства, о них знает парсер.
*		
*
***************************************************************************************/

#ifndef MBN_COMMON_COMMUNICATION_CHANNEL_CLASS
#define MBN_COMMON_COMMUNICATION_CHANNEL_CLASS

#include <assert.h>
#include <exception>
#include <Basic_CommunicationChannel_Base.h>
#include <PagedArray.cpp>
#include <CommonPackage.h>
#include <CommonByteStreamParser.h>
#include <Common_Protocol.h>
#include <QThread>
#include <QSemaphore>
#include <FastNotifyer.h>
#include <list>
#include <iostream>

extern int joo;

/// структура, передаваемая с извещением об отправке-приёме посылки
struct PackageIONotifyData
{
	/// поля структуры
    CommonPackage const *pack_ptr;
    vector<unsigned char> const * pack_data_ptr;

	/// конструктор
    PackageIONotifyData(CommonPackage *package_ptr = 0, vector<unsigned char> const *package_data_ptr = 0):
		pack_ptr(package_ptr), pack_data_ptr(package_data_ptr)
	{	};	
};

/// Низкоуровневый поток оповещения о принятых - отправленных посылках
class PackageIOAsyncNotifyer: protected QThread
{
private:

	/// Семафор доступа к данному объекту
	QSemaphore accsess_sem;

    /// массив отправленных посылок, о которых надо PackageIOAsyncNotifyerсообщить
	list<PackageIONotifyData> posted_packs_notifications;
	/// массив принятых посылок, о которых надо сообщить
	list<PackageIONotifyData> got_packs_notifications;

	/// флаг работы
	bool notify_thread_running;

	/// флаг останова
	bool exit; 

public:

	/// оповещатель об отправке посылки (потокобезопасен)
	FastNotifyer post_package_notifyer;

	/// оповещатель о приёме посылки (потокобезопасен)
	FastNotifyer got_package_notifyer;
	
private:

	/// Выполняет всю работу по оповещению об отправке-приёме посылок,
	/// просматривает очереди оповещений об отправке-приёме посылок, если в них что-то
	/// есть, то вызывает соответствующие нотификаторы и соответственными данными
	virtual void run()
	{
		notify_thread_running = true;

		// флаг, были ли посылки на предыдущей итерации
		bool had_packs_to_process = true;

		while (!exit)
		{
			// если на предыдущей итерации не было посылок, то можно поспать
			if (!had_packs_to_process)
				this->msleep(250);
			// по умолчанию, можно спать ))
			had_packs_to_process = false;

			// обрабатываем вызовы по исходящим посылкам
				// вытаскиваем данные оповещения из разделяемой памяти
				// чтобы не задерживать основной поток ввода-вывода канала
				// вытаскиваем по принципу FIFO
					// данные оповещения о выводе
			PackageIONotifyData current_post_notify_data(0, 0);
					// флаг, было ли выбрано оповещения из очереди отсылки
			bool have_post_notification = false;
				// эксклюзивный доступ к очереди
			accsess_sem.acquire();

			if (posted_packs_notifications.size())
			{				
					// вытаскиваем информацию оповещения по-быстрому
				current_post_notify_data.pack_data_ptr = (*(posted_packs_notifications.begin())).pack_data_ptr;
				current_post_notify_data.pack_ptr = (*(posted_packs_notifications.begin())).pack_ptr;
					// убираем вытащенное оповещение из очереди
				posted_packs_notifications.pop_front();
					// устанавливаем флаги
				have_post_notification = true;
				had_packs_to_process = true;
			}
					// и сразу отпускаем семафор, а то задержимся
			accsess_sem.release();
				// всё, очередь обмена освободили, теперь можно не торопясь вызвать оповещение, если было что-то выбрано, конечно
			if (have_post_notification)
				post_package_notifyer.Notify(&current_post_notify_data);

			// теперь проделываем всё то же самое с оповещениями о приёме

			// обрабатываем вызовы по входящим посылкам
			PackageIONotifyData current_got_notify_data(0, 0);
			bool have_got_notification = false;

			accsess_sem.acquire();

			if (got_packs_notifications.size())
			{				
					// вытаскиваем информацию оповещения по-быстрому
				current_got_notify_data.pack_data_ptr = (*(got_packs_notifications.begin())).pack_data_ptr;
				current_got_notify_data.pack_ptr = (*(got_packs_notifications.begin())).pack_ptr;
					// убираем вытащенное оповещение из очереди
				got_packs_notifications.pop_front();
					// устанавливаем флаги
				have_got_notification = true;
				had_packs_to_process = true;
			};
					// и сразу отпускаем семафор, а то задержимся
			accsess_sem.release();
				// всё, очередь обмена освободили, теперь можно не торопясь вызвать оповещение, если было что-то выбрано, конечно
			if (have_got_notification)
				got_package_notifyer.Notify(&current_got_notify_data);
		};
        std::cout<<"exit from run Common_Communication_Channel"<<std::endl;

		notify_thread_running = false;
	};

public:
/// работа со списками оповещений

	/// добавить посылку в список уведомлений об отправке на устройство
	void AddPostNotification(const PackageIONotifyData &notification)
	{
		accsess_sem.acquire();

		posted_packs_notifications.push_back(notification);

		accsess_sem.release();
	};

	/// добавить посылку в список уведомлений о приёме с устройства
	void AddGotNotification(const PackageIONotifyData &notification)
	{
		accsess_sem.acquire();

		got_packs_notifications.push_back(notification);

		accsess_sem.release();
	};

	/// очистить очередь оповещений об отправке
	void ClearPostNotificationList()
	{
		accsess_sem.acquire();

		posted_packs_notifications.clear();

		accsess_sem.release();
	};

	/// очистить очередь оповещений о приёме
	void ClearGotNotificationList()
	{
		accsess_sem.acquire();

		got_packs_notifications.clear();

		accsess_sem.release();
	};

public:
// управление

	/// работает ли оповещатель
	bool IsNotificatorWorking()
	{
		return notify_thread_running;
	};

	/// Конструктор
	PackageIOAsyncNotifyer(): accsess_sem(1)
	{
		exit = false;
		notify_thread_running = false;
	};

	/// Деструктор
	~PackageIOAsyncNotifyer()
	{
		StopNotificator();
	};

	/// запуск оповещателя
	void StartNotificator()
	{
		accsess_sem.acquire();

		if (!notify_thread_running)
			this->start();

		accsess_sem.release();

        std::cout<<"PackageIOAsyncNotifyer::StartNotificator";
	};

	/// выход (остановка потока)
	void StopNotificator()
	{
		accsess_sem.acquire();

		exit = true;

		accsess_sem.release();

		this->wait();
	};
};


/// Общий коммуникационный модуль
class Common_Communication_Channel: protected QThread
{
public:

	/// оповещатель об удалении канала
	FastNotifyer delete_notifyer;

	/// оповещатель об отправке-приёме посылки 
	/// СОГЛАШЕНИЕ: (данные передаваемые вовне - удаляются клиентом!)
	/// указатель на данные указывает на созданный в стеке PackageIONotifyData элементы 
	/// которого указывают на созданные в КУЧЕ CommonPackage и vector<unsigned char>
	PackageIOAsyncNotifyer package_io_notifyer;

	/// оповещатель о приёме байт  
	/// СОГЛАШЕНИЕ: (данные передаваемые вовне - удаляются клиентом!)
	/// указатель на данные указывает на созданный в куче vector<unsigned char>
	FastAsyncNotifyer got_bytes_notifyer;

	/// оповещатель о передаче байт  
	/// СОГЛАШЕНИЕ: (данные передаваемые вовне - удаляются клиентом!)
	/// указатель на данные указывает на созданный в куче vector<unsigned char>
	FastAsyncNotifyer sent_bytes_notifyer;

private:
	/**************************************************************
	*
	*					Семафор доступа к данному объекту
	*	
	**************************************************************/
	QSemaphore accsess_sem;

	/**************************************************************
	*
	*		Базовый объект, обеспечиваюший элементарные синхронные 
	*	операции ввода-вывода.
	*	
	**************************************************************/
	BaseCommCh *base_ch;

	/****************************************************************************************************
	*====================================================================================================
	*					Выводная очередь
	*====================================================================================================	
	****************************************************************************************************/
	/// Выводная очередь пакетов
	PagedArray<CommonPackage> Out_pkg;
	/// Выводная очередь байт
    PagedArray<unsigned char> Out_byte;
	/// Индекс байта который должен быть записан
	int byte_to_write_idx;
	/// Индекс посылки, поторая должна быть отправлена
	int pack_to_write_idx;
	/// семафор выводной очереди
	QSemaphore out_sem;
	/// ID следующего пакета для отправки (НЕЧЁТНЫЙ)
    long long out_id;

	/****************************************************************************************************
	*====================================================================================================
	*					Входная очередь
	*====================================================================================================	
	****************************************************************************************************/
	/// Входная очередь пакетов
	PagedArray<CommonPackage> In_pkg;
	/// Входная очередь байт
    PagedArray<unsigned char> In_byte;
	/// семафор входной очереди
	QSemaphore in_sem;
	/// ID следующего принимаемого пакета (ЧЁТНЫЙ)
    long long in_id;


	/**************************************************************
	*
	*					Парсер входного потока байт
	*	
	**************************************************************/
    CommonByteStreamParser<PagedArray<unsigned char>, PagedArray<CommonPackage> > *BSP;

	/**************************************************************
	*
	*					     Протокол связи
	*	
	**************************************************************/
	CommonProtocol *Protocol;

	/**************************************************************
	*
	*					Различные флаги и тех. переменные
	*	
	**************************************************************/
	/// Номер последней ошибки
	int LastError;
	/// Флаг останова. True - останавливаемся и завершаем поток
	bool stop;

	/// Оповещать о приёме байт
	bool got_bytes_notification_enabled;
	/// Оповещать об отправке байт
	bool sent_bytes_notification_enabled;

	/// Оповещать об отправке посылки
	bool post_package_notification_enabled;
	/// Оповещать о получении посылки
	bool got_package_notification_enabled;


	/**************************************************************
	*
	*							Тайминг
	*	
	**************************************************************/

	/// Время ожижания прихода байт
	int Read_WaitingTime_us;
    /// Время ожижания освобоobjectName : QStrinждения данных для парсинга
	int Parse_WaitingTime_us;
	/// Время ожижания освобождения данных для распределения
	int Manage_WaitingTime_us;
	/// Время ожижания освобождения вызодной очереди для записи в порт
	int Write_WaitingTime_us;

	/// дифференциальные коррекционные коэффициенты для расчётного времени сна. [last] => текущий такт, [last-1] => предыдущий такт...
	static const int sliding_window_size = 2;
	float corr_coeff[sliding_window_size];



	/**************************************************************
	*
	*			Перегруженная функция потока приёма-передачи
	*	
	**************************************************************/
protected:
	/// Выполняет всю работу по приёму данных, парсингу принятых данных, распределению принятых данных, пересылке выходных данных
	virtual void run()
	{
		// так, внимание, походу тут будут операторы goto, сильно не 
		// серчайте, ибо логичны здесь они......
		// хотя.... может и не нужно :)

        joo = 10;
        std::cout<<std::endl<<"joo = "<<joo<<std::endl;
		
        std::cout<<"Runing";

		// если мы не подключены, то выходим               
		if (!base_ch->isOpened())
        {
            joo = 20;
            std::cout<<std::endl<<"joo = "<<joo<<std::endl;
            std::cout<<"Exiting from the thread - reason - channel is not opened"<<std::endl;
            return;
        }

		// вечно :)
		while(true)
		{
			int last_red_size = 0;

			// первое, что надо сделать, это проверить, есть ли данные 
			if (base_ch->bytesAvailable())
			{
				// если прямо сейчас входная очередь не занята
				if (in_sem.tryAcquire(1, Read_WaitingTime_us))
				{
					// принимаем всё, что есть в приёмном буффере
					last_red_size = base_ch->readAllSyncAndPushBack(In_byte);

					// если надо - копируем передаём данные перехватчику события приёма байт
					if (got_bytes_notification_enabled && (last_red_size > 0))
						// если у оповещателя есть хоть один клиент и оповещатель включён
						if (got_bytes_notifyer.HasAnyNotifyCBRec() && got_bytes_notifyer.IsNotificatorWorking())
						{
							//подготавливаем данные, для этого 
							vector<unsigned char> *new_data = new vector<unsigned char>();

							new_data->resize(last_red_size);

							for (int i = 0; i < new_data->size(); i++)
								(*new_data)[i] = In_byte[In_byte.size() - last_red_size + i];

							// оповещаем
							got_bytes_notifyer.Notify((void*)new_data);

							// удалять массив должен клиент
						};

					in_sem.release();
				};
			};

			// парсим входные данные
			if (in_sem.tryAcquire(1, Parse_WaitingTime_us))
			{
				// индекс посылки с которого (включительно) надо проставить нужную дату и время
                long long idx = In_pkg.size() - 1;

				BSP->ParseBytes();

				// теперь проставляем штампы времени и ID на ПОЛНОСТЬЮ принятые посылки (т.е. за которыми уже идёт следующая)
				// на все, кроме последней, парсинг которой по определению не закончен, а парсинг всех предыдущих - закончен 
				for ( ; idx <= In_pkg.size() - 2; idx++)
				{
					In_pkg[idx].date_time = QDateTime::currentDateTime();
					In_pkg[idx].id = in_id;
					in_id += 2;

					/// оповещаем клиентов оповещателя, если надо (лучше использовать только в отладочном режиме работы канала)
					/// т.к. package_io_notifyer асинхронен, то сильных задержек во временной диагнамме не будет
					/// (AddPostNotification - имеет вполне фиксированное, малое время срабатывания)
                    if (got_package_notification_enabled)
						// если у оповещателя есть хоть один клиент и оповещатель включён
						if (package_io_notifyer.got_package_notifyer.HasAnyNotifyCBRec() && package_io_notifyer.IsNotificatorWorking())
						{
                            // эти данные из кучи удаляются по соглашению последним обработчиком
							// вызываемого оповещения
							CommonPackage *pack_copy_ptr = new CommonPackage(In_pkg[idx]);
                            vector<unsigned char> *data_copy_ptr = new vector<unsigned char>(In_pkg[idx].package_data_length);

							// копируем данные в новый массив
							for (int i = 0; i < In_pkg[idx].package_data_length; i++)
								(*data_copy_ptr)[i] = In_byte[In_pkg[idx].first_byte_stream_offset + i];

							// отправляем уведомление
							PackageIONotifyData notification(pack_copy_ptr, data_copy_ptr);

							// помещаем уведомление в очередь уведомлений
							package_io_notifyer.AddGotNotification(notification);
						};
				};

				in_sem.release();
			};


			// обрабатываем очереди посылок, распределяем даныне и сохраняем текущее состояние девайса
			if (in_sem.tryAcquire(1, Manage_WaitingTime_us))
			{
				if (out_sem.tryAcquire(1, Manage_WaitingTime_us))
				{
					Protocol->ManagePackageSequenses(&Out_pkg, &Out_byte, &In_pkg, &In_byte, pack_to_write_idx);

                    // тут мы можем удалять пропарсенные (и уже отнотифайенные данные =) Слава русскому языку =)
                    // если объём пропарсенных входных данных превысил лимит, то стираем всё наф!
                    BSP->MaxShiftLeft();

                    // смещаем индекс следующей обрабатываемой на уровне протокола посылки
                    Protocol->FlushNextPackToManageIndex();

					out_sem.release();                    
				};

				in_sem.release();
			};


			// доступ к выходной очереди для посылки
			if (out_sem.tryAcquire(1, Manage_WaitingTime_us))
			{
				// если есть, что отсылать - шлём
				if (byte_to_write_idx < Out_byte.size())
				{
					base_ch->writeSync(Out_byte, byte_to_write_idx, Out_byte.size() - 1);

					// если надо оповестить о высылке байт, оповещаем
					if (sent_bytes_notification_enabled)
						// если у оповещателя есть хоть один клиент и оповещиатель включён
						if (sent_bytes_notifyer.HasAnyNotifyCBRec() && sent_bytes_notifyer.IsNotificatorWorking())
						{
							//подготавливаем данные, для этого 
							vector<unsigned char> *new_data = new vector<unsigned char>();

							new_data->resize(Out_byte.size() - byte_to_write_idx);

							for (int i = 0; i < new_data->size(); i++)
								(*new_data)[i] = Out_byte[In_byte.size() - last_red_size + i];

							// оповещаем
							sent_bytes_notifyer.Notify((void*)new_data);

							// удалять массив должен клиент
						};

					byte_to_write_idx = Out_byte.size();

					// теперь проставляем штампы времени и маркер отправки на высланные посылки
					for (;pack_to_write_idx < Out_pkg.size(); pack_to_write_idx++)
					{
						Out_pkg[pack_to_write_idx].date_time = QDateTime::currentDateTime();
						Out_pkg[pack_to_write_idx].parsed_or_sended = true;
					};

				};

				out_sem.release();
			};

			// регулируемый блок задержки, спит столько, сколько можно, только чтобы не терять данные
			float sleep_time = CalculateSleepTime(base_ch->GetInBufferSize() , 0.2, last_red_size, base_ch->GetReadingSpeed(), base_ch->GetWritingSpeed(), corr_coeff, 0.025);
			// спим
			this->msleep(sleep_time);
			
			//if (sleep_time)
			//{
			//	cout<<last_red_size<<"        - >    "<<sleep_time<<endl;
			//}


			// если выставлен флаг останова - выходим
			accsess_sem.acquire();
            std::cout<<"Checking exit flag (common communication channel)  ====== "<<this->stop<<std::endl;
			if (this->stop)
			{
				accsess_sem.release();
                std::cout<<"THIS F*CKING COMMON COMM CHANNEL HAD BEEN STOPPED!"<<std::endl;
				return;
			};
			accsess_sem.release();
		};
	};

	/// Вычисляет, сколько можно поспать без потерь данных (чтобы приёмный буфер не забился) и торможения
	/// buffer_size - размер приёмного буфера канала связи в байтах
	/// allowed_buffer_part - [0; 1] - часть буфера устройства, которую максимум (при максимальном загрузе канала) хотим загружать 
	/// last_data_red - количество прочитанных за последнее чтение 
	/// read_speed - скорость канала при чтении (байт/c)
	/// write_speed - скорость канала на запись (байт/c)
	/// corr_coeff - массив поправочных коэффициентов 
	/// max_sleep_time - максимальное разрешённое время простоя (в секундах)
	inline int CalculateSleepTime(int buffer_size, float allowed_buffer_part, int last_data_red_size, int read_speed, int write_speed,
								  float (&corr_coeff)[sliding_window_size], float max_sleep_time)
	{
		// время, затрачиваемое на
			// получение одного байта из буфера порта (c/байт)
		const float tg = 0.0000010;	
			// парсинг одного байта (c/байт)
		const float tp = 0.0000010;
			// передачу одного байта по каналу при чтении (c/байт)
		const float tr = 1/float(read_speed);
			// передачу одного байта по каналу при записи (c/байт)
		const float tw = 1/float(write_speed);

		///смещаем коррекционные коэффициенты
		for (int i = 1; i <= sliding_window_size - 1; i++)
			corr_coeff[i-1] = corr_coeff[i];

		// вычисляем последний (т.е. текущий) коррекционный коэффициент
			// теоретическое время сна
		float theor_sleep_time = (float(buffer_size)*allowed_buffer_part) * (tr - tg - tp);
			// вычисляем коррекционный коэффициент, который уменьшает время сна, пропорционально выбросу количества данных. 
			// данных в два раза больше, чем расчётное - увеличиваем время вдвое, например
		float diff_correction_coeff = (float(buffer_size)*allowed_buffer_part) / float(last_data_red_size);

		// записываем коррекционный коэффициент в конец массива коррекционных коэффов
		corr_coeff[sliding_window_size - 1] = diff_correction_coeff;

		//вычисляем текущую коррекцию (как среднее массива коррекций)
		float current_correction = corr_coeff[0];
		for (int i = 1; i <= sliding_window_size - 1; i++)
			current_correction += corr_coeff[0];
		current_correction /= sliding_window_size;

		// время простоя после этой итерации
		float sleep_time = 0.0;

		// если поток данных мал или буфер велик настолько, что мы получили возможное
		// очень большое время сна - например секунд 10 - то как ограничение выступает 
		// максимальное время отклика, которое задаётся как параметр
		if (theor_sleep_time * current_correction > max_sleep_time)
			sleep_time = max_sleep_time;
		else
			sleep_time = theor_sleep_time * current_correction;

		// переводим секунды в миллисекунды и возвращаемся
		return int(sleep_time*1000.0);
	};

public:
	/// конструктор канала (в основном ему нужно принять только указатель на парсер кода)
	/// Также получает параметры входных-выходных массивов, чтобы была возможность регулировать производительность
	/// и расход памяти
	Common_Communication_Channel(BaseCommCh *communication_channel,
								 CommonProtocol * CommunicationProtocol,
								 int in_bytes_page_size, int out_bytes_page_size,
								 int in_packs_page_size, int out_packs_page_size,
								 int in_bytes_reserved_size = 0, int out_bytes_reserved_size = 0, 
								 int in_packs_reserved_size = 0, int out_packs_reserved_size = 0, 
								 int in_bytes_header_step = 0, int out_bytes_header_step = 0,
								 int in_packs_header_step = 0, int out_packs_header_step = 0)
								 : 
						In_byte(0, in_bytes_page_size, in_bytes_reserved_size, in_bytes_header_step),
						Out_byte(0, out_bytes_page_size, out_bytes_reserved_size, out_bytes_header_step),
						In_pkg(0, in_packs_page_size, in_packs_reserved_size, in_packs_header_step),
						Out_pkg(0, out_packs_page_size, out_packs_reserved_size, out_packs_header_step), 
						accsess_sem(1),  
						in_sem(1), 
						out_sem(1),
						stop(false)
	{
        CommonByteStreamParser<PagedArray<unsigned char>, PagedArray<CommonPackage> > *bsp = CommunicationProtocol->StreamParserPtr;
		
		assert(bsp != 0);

		base_ch = communication_channel;

		BSP = bsp;

		BSP->SetQueues(&In_byte, &In_pkg);

		Protocol = CommunicationProtocol;

		// какой байт следующим мы пишем в канал
		byte_to_write_idx = 0;
		pack_to_write_idx = 0;

		// устанавливаем таймауты
		Read_WaitingTime_us = 1;
		Parse_WaitingTime_us = 1;
		Manage_WaitingTime_us = 1;
		Write_WaitingTime_us = 1;

		// стартовые ID
		out_id = 1;
		in_id = 2;

		// устанавливаем коррекционные коэффициенты расчёта времени сна
		for (int i = 0; i <= sliding_window_size - 1; i++)
			corr_coeff[i] = 1.0;

		// устанавливаем таймеры
		post_package_notification_enabled = false;
		got_package_notification_enabled = false;
	};

	// Деструктор (завершает поток активного канала)
	~Common_Communication_Channel()
	{
		// оповещаем о нашем удалении
		delete_notifyer.Notify(0);
		// останавливаем канал
		StopChannel();
	};


	/// Установить режим оповещения о приёме байт
	void EnableGotBytesNotify()
	{
		accsess_sem.acquire();

		got_bytes_notification_enabled = true;

		accsess_sem.release();
	};

	/// Сбросить режим оповещения о приёме байт
	void DisableGotBytesNotify()
	{
		accsess_sem.acquire();

		got_bytes_notification_enabled = false;

		accsess_sem.release();
	};

	/// Установить режим оповещения о передаче байт
	void EnableSentBytesNotify()
	{
		accsess_sem.acquire();

		sent_bytes_notification_enabled = true;

		accsess_sem.release();
	};

	/// Сбросить режим оповещения о передаче байт
	void DisableSentBytesNotify()
	{
		accsess_sem.acquire();

		sent_bytes_notification_enabled = false;

		accsess_sem.release();
	};


	/// Установить режим оповещения об отправке посылки
	void EnablePostPackageNotify()
	{
		accsess_sem.acquire();

		post_package_notification_enabled = true;

		accsess_sem.release();
	};

	/// Сбросить режим оповещения об отправке посылки
	void DisablePostPackageNotify()
	{
		accsess_sem.acquire();

		post_package_notification_enabled = false;

		accsess_sem.release();
	};

	/// Установить режим оповещения о приёме посылки
	void EnableGotPackageNotify()
	{
		accsess_sem.acquire();

		got_package_notification_enabled = true;

		accsess_sem.release();
	};

	/// Сбросить режим оповещения о приёме посылки
	void DisableGotPackageNotify()
	{
		accsess_sem.acquire();

		got_package_notification_enabled = false;

		accsess_sem.release();
	};


	/// Добавляет копию данной посылки в очередь на отправку на устройство
	/// pack - посылка. pack.first_byte_offset здесь не используется, а вычисляется свой
	/// data - массив, содержащий только данные этой посылки
	/// возвращает ID, присвоенный посылке, по нему можно будет отслеживать её состояние. 0 - если отправка не удалась.
    long long PostPackage(const CommonPackage &pack, const vector<unsigned char> &data, int timeout_ms = 0x0FFFFFFF)
	{
        long long ID = 0;

		// получаем эксклюзивный доступ к выходной очереди
		if (out_sem.tryAcquire(1,timeout_ms))
		{
			// добавляем посылку в выходную очередь
			Out_pkg.push_back(pack);
			// устанавливаем правильно её ID
			Out_pkg[Out_pkg.size() - 1].id = out_id;
			ID = out_id;
			out_id += 2;
			// устанавливаем правильно индекс её первого байта
			Out_pkg[Out_pkg.size() - 1].first_byte_stream_offset = Out_byte.size();
			// добавляем байт-код посылки в байт-очередь
				// ресайзим массив, чтобы вместил данные посылки
			Out_byte.resize(Out_byte.size() + data.size());
				// копируем данные
			for (int i = Out_byte.size() - data.size(); i <= Out_byte.size() - 1; i++)
				Out_byte[i] = data[i - Out_byte.size() + data.size()];

			// оповещаем клиентов оповещателя, если надо (лучше использовать только в отладочном режиме работы канала)
			if (post_package_notification_enabled)
				// если у оповещателя есть хоть один клиент и оповещатель включён
				if (package_io_notifyer.post_package_notifyer.HasAnyNotifyCBRec() && package_io_notifyer.IsNotificatorWorking())
				{
					// эти данные из кучи удаляются по соглашению последним обработчиком
					// вызываемого оповещения
					CommonPackage *pack_copy_ptr = new CommonPackage(Out_pkg[Out_pkg.size() - 1]);
                    vector<unsigned char> *data_copy_ptr = new vector<unsigned char>(data);

					PackageIONotifyData notification(pack_copy_ptr, data_copy_ptr);

					// помещаем уведомление в очередь уведомлений
					package_io_notifyer.AddPostNotification(notification);
				};

			out_sem.release();
		}		

		return ID;
	};

	/// Получает посылку из приёмной очереди.
	/// После отработки pack содержит посылку, а data - данные соответствующей посылки
	/// idx - индекс посылки
	/// возвращает успешность получения посылки
    void GetPackageFromInQueue(int idx, CommonPackage &pack, vector<unsigned char> &data)
	{
		// ждём доступа
		in_sem.acquire();

		assert((idx <= In_pkg.size() - 1) && (idx >= 0));

		// получаем структуру посылки
		pack = In_pkg[idx];

		// только если посылка корректна
		if (pack.correctness == pack.CORRECT)
		{
			// копируем байт-последовательность посылки в data
			data.resize(pack.package_data_length);
			for (int i = 0; i < pack.package_data_length; i++)
				data[i] = In_byte[i + pack.first_byte_stream_offset];
		}
		else
			data.resize(0);

		in_sem.release();
	};

	/// Получает посылку из приёмной очереди.
	/// После отработки pack содержит посылку, а data - данные соответствующей посылки
	/// idx - индекс посылки
	/// возвращает успешность получения посылки
    void GetPackageFromOutQueue(int idx, CommonPackage &pack, vector<unsigned char> &data)
	{
		// ждём доступа
		out_sem.acquire();

		assert((idx <= Out_pkg.size() - 1) && (idx >= 0));

		// получаем структуру посылки
		pack = Out_pkg[idx];

		// только если посылка корректна
		if (pack.correctness == pack.CORRECT)
		{
			// копируем байт-последовательность посылки в data
			data.resize(pack.package_data_length);
			for (int i = 0; i < pack.package_data_length; i++)
				data[i] = Out_byte[i + pack.first_byte_stream_offset];
		}
		else
			data.resize(0);

		out_sem.release();
	};

	/// Получает посылку из приёмной очереди.
	/// После отработки pack содержит посылку
	/// idx - индекс посылки
	/// возвращает успешность получения посылки
	void GetPackageFromInQueue(int idx, CommonPackage &pack)
	{
		// ждём доступа
		in_sem.acquire();

		assert((idx <= In_pkg.size() - 1) && (idx >= 0));

		// получаем структуру посылки
		pack = In_pkg[idx];

		in_sem.release();
	};

	/// Получает посылку из приёмной очереди.
	/// После отработки pack содержит посылку
	/// idx - индекс посылки
	/// возвращает успешность получения посылки
	void GetPackageFromOutQueue(int idx, CommonPackage &pack)
	{
		// ждём доступа
		out_sem.acquire();

		assert((idx <= Out_pkg.size() - 1) && (idx >= 0));

		// получаем структуру посылки
		pack = Out_pkg[idx];

		out_sem.release();
	};

	/// Длина входной очереди посылок
	int LengthInPacksQueue(int timeout_us = 0)
	{
		// ждём доступа
		if (in_sem.tryAcquire(1, timeout_us))
		{
			// получаем структуру посылки
			int out = In_pkg.size();

			in_sem.release(1);
			return out;
		};

		return -1;
	};

	/// Длина входной очереди байт
	int LengthInBytesQueue(int timeout_us = 0)
	{
		// ждём доступа
		if (in_sem.tryAcquire(1, timeout_us))
		{
			// получаем структуру посылки
			int out = In_byte.size();

			in_sem.release(1);
			return out;
		};

		return -1;
	};

	/// Длина выходной очереди посылок
	int LengthOutPacksQueue(int timeout_us = 0)
	{
		// ждём доступа
		if (out_sem.tryAcquire(1, timeout_us))
		{
			// получаем структуру посылки
			int out = Out_pkg.size();

			out_sem.release(1);
			return out;
		};

		return -1;
	};

	/// Длина выходной очереди байт
	int LengthOutBytesQueue(int timeout_us = 0)
	{
		// ждём доступа
		if (out_sem.tryAcquire(1, timeout_us))
		{
			// получаем структуру посылки
			int out = Out_byte.size();

			out_sem.release(1);
			return out;
		};

		return -1;
	};

	/// получаем байт по указанному индексу из входной очереди байт
    unsigned char GetByteFromInQueue(int idx, int timeout_us = 0)
	{
		// ждём доступа
		if (in_sem.tryAcquire(1, timeout_us))
		{
			// при неправильном индексе
			if ((idx >= In_pkg.size())||(idx < 0)) 
			{
				in_sem.release(1);
				return 0;
			};

            unsigned char res = In_byte[idx];

			in_sem.release(1);
			return res;
		};

		return 0;
	};

	/// получаем байт по указанному индексу из входной очереди байт
    unsigned char GetByteFromOutQueue(int idx, int timeout_us = 0)
	{
		// ждём доступа
		if (out_sem.tryAcquire(1, timeout_us))
		{
			// при неправильном индексе
			if ((idx >= Out_pkg.size())||(idx < 0)) 
			{
				out_sem.release(1);
				return 0;
			};

            unsigned char res = Out_byte[idx];

			out_sem.release(1);
			return res;
		};

		return 0;
	};

	/// получаем байты по указанному индексу из входной очереди байт
	vector<unsigned char> GetBytesFromInQueue(int from_idx, int to_idx)
	{
		// ждём доступа
		in_sem.acquire();

		// индексы должны быть правильными
		assert((from_idx >= 0)&&(from_idx < In_byte.size())&&
			   (to_idx >= 0)&&(to_idx < In_byte.size()) &&
			   (to_idx <= from_idx));

		// выходной массив байт
		vector<unsigned char> res(to_idx - from_idx + 1);

		for (int i = from_idx; i <= to_idx; i++)
			res[i - from_idx] = In_byte[i];

		in_sem.release();

		return res;
	};

	/// получаем байты по указанному индексу из входной очереди байт
	vector<unsigned char> GetBytesFromOutQueue(int from_idx, int to_idx)
	{
		// ждём доступа
		out_sem.acquire();

		// индексы должны быть правильными
		assert((from_idx >= 0)&&(from_idx < Out_byte.size())&&
			   (to_idx >= 0)&&(to_idx < Out_byte.size()) &&
			   (to_idx >= from_idx));

		// выходной массив байт
		vector<unsigned char> res(to_idx - from_idx + 1);

		for (int i = from_idx; i <= to_idx; i++)
			res[i - from_idx] = Out_byte[i];

		out_sem.release();

		return res;
	};


	/// Возвращает указатель на протокол связи
	CommonProtocol * GetProtocolPtr()
	{
		return Protocol;
	};


	/// Выставляет флаг останова канала и ждёт, завершения исполнения потока активного канала
	inline void StopChannel()
	{
        std::cout<<"Common_Comm_Ch::StopChannel:::: <BEFORE stop request>  managed thread running = "<<this->isRunning()<<std::endl;

		accsess_sem.acquire();

		this->stop = true;

		accsess_sem.release();

        this->wait();

        std::cout<<"Common_Comm_Ch::StopChannel:::: <AFTER stop request>  managed thread running = "<<this->isRunning()<<std::endl;
	};

	/// Запускает канал
	inline void StartChannel()
	{
        joo++;
        std::cout<<std::endl<<"joo = "<<joo<<std::endl;
		this->start();
	};
};


#endif
