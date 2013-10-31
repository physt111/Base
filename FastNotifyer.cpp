/*************************************************************************
*
*
*	Файл содержит реализацию класса быстрого оповещателя.
*
*
*************************************************************************/

#include <vector>
#include <PagedArray.cpp>
//#include <MBN_Signal_Plotter.h>
#include <qlabel.h>
//#include <QGridLayout.h>
#include <list>
#include <FastNotifyer.h>
#include <assert.h>
#include <QSemaphore>


/****************************************************************
*
*		Класс синхронного быстрого оповещателя (РЕАЛИЗАЦИЯ)
*
****************************************************************/


// Блокировать доступ (для избежания защелкивания)
void FastNotifyer::Block()
{
	accsess_sem.acquire();
};

// Разблокировать доступ
void FastNotifyer::Release()
{
	accsess_sem.release();
};

// выполнить оповещение всех
void FastNotifyer::Notify(void* data_ptr)
{
	accsess_sem.acquire();
	// вызываем функции по-очерёдно
	for (int i = 0; i < cb_records.size(); i++)
		(*(cb_records[i].cb_f_ptr))(cb_records[i].user_ptr, data_ptr, i == cb_records.size() - 1);
	accsess_sem.release();
};

// есть ли хоть одна запись обратного вызова
bool FastNotifyer::HasAnyNotifyCBRec()
{
	accsess_sem.acquire();

	int count = cb_records.size();

	accsess_sem.release();

	return (count > 0);
};

// есть ли запись с указанным адресом функции
bool FastNotifyer::HasNotifyCBRec(notify_func_ptr_type f_ptr)
{
	accsess_sem.acquire();
	for (int i = 0; i < cb_records.size(); i++)
		if (cb_records[i].cb_f_ptr == f_ptr)
		{
			accsess_sem.release();
			return true;
		};

	accsess_sem.release();
	return false;
};

// есть ли запись с указанным пользовательским адресом
bool FastNotifyer::HasNotifyCBRec(void* user_ptr)
{
	accsess_sem.acquire();
	for (int i = 0; i < cb_records.size(); i++)
		if (cb_records[i].user_ptr == user_ptr)
		{
			accsess_sem.release();
			return true;
		};

	accsess_sem.release();
	return false;
};

// есть ли запись с указанным адресом функции и одновременно с указанным пользовательским адресом
bool FastNotifyer::HasNotifyCBRec(notify_func_ptr_type f_ptr, void* user_ptr)
{
	accsess_sem.acquire();
	for (int i = 0; i < cb_records.size(); i++)
		if (cb_records[i].user_ptr == user_ptr)
			if (cb_records[i].cb_f_ptr == f_ptr)
			{
				accsess_sem.release();
				return true;
			};

	accsess_sem.release();
	return false;
};

// если такой записи нет - добавляет запись в список оповещения
void FastNotifyer::AddNotifyCB(notify_func_ptr_type f_ptr, void* user_ptr)
{
	assert(!HasNotifyCBRec(f_ptr, user_ptr));

	accsess_sem.acquire();

	NotifyRecord new_rec;

	new_rec.cb_f_ptr = f_ptr;
	new_rec.user_ptr = user_ptr;

	cb_records.push_back(new_rec);

	accsess_sem.release();
};

// удалить записи с указанным адресом функции, если нет - ничего не делает
void FastNotifyer::DeleteNotifyCBRec(notify_func_ptr_type f_ptr)
{
	assert(!HasNotifyCBRec(f_ptr));

	accsess_sem.acquire();

	vector<NotifyRecord> new_arr;

	for (int i = 0; i < cb_records.size(); i++)
		if (cb_records[i].cb_f_ptr != f_ptr)
			new_arr.push_back(cb_records[i]);

	cb_records = new_arr;

	accsess_sem.release();
};

// удалить записи с указанным пользовательским адресом, если нет - ничего не делает
void FastNotifyer::DeleteNotifyCBRec(void* user_ptr)
{
	assert(!HasNotifyCBRec(user_ptr));

	accsess_sem.acquire();	

	vector<NotifyRecord> new_arr;

	for (int i = 0; i < cb_records.size(); i++)
		if (cb_records[i].user_ptr != user_ptr)
			new_arr.push_back(cb_records[i]);

	cb_records = new_arr;

	accsess_sem.release();
};

// удалить записи с указанным адресом функции и одновременно с указанным пользовательским адресом, если нет - ничего не делает
void FastNotifyer::DeleteNotifyCBRec(notify_func_ptr_type f_ptr, void* user_ptr)
{
	assert(HasNotifyCBRec(f_ptr, user_ptr));

	accsess_sem.acquire();	

	vector<NotifyRecord> new_arr;

	for (int i = 0; i < cb_records.size(); i++)
		if ((cb_records[i].user_ptr != user_ptr) || (cb_records[i].user_ptr != user_ptr))
			new_arr.push_back(cb_records[i]);

	cb_records = new_arr;

	accsess_sem.release();
};

// очистить список оповещения
void FastNotifyer::Clear()
{
	accsess_sem.acquire();

	cb_records.clear();

	accsess_sem.release();
};

// конструктор
FastNotifyer::FastNotifyer(): accsess_sem(1)
{
	cb_records.clear();
};





/****************************************************************
*
*		Класс асинхронного быстрого оповещателя (РЕАЛИЗАЦИЯ)
*
****************************************************************/


/// Выполняет всю работу по оповещению о событиях
/// просматривает очередь оповещений о событиях (notifications_data), если в ней что-то
/// есть, то вызывает соответствующие нотификаторы и соответственными данными
void FastAsyncNotifyer::run()
{
	notify_thread_running = true;

	// флаг, были ли оповещения на предыдущей итерации
	bool had_notifications_to_process = true;

	while (!exit)
	{
		// если на предыдущей итерации не было событий
		if (!had_notifications_to_process)
			this->msleep(250);
		// по умолчанию, можно спать ))
		had_notifications_to_process = false;

		// обрабатываем вызовы
			// вытаскиваем данные оповещения из разделяемой памяти
			// чтобы не задерживать основной поток ввода-вывода канала
			// вытаскиваем по принципу FIFO
				// данные оповещения о выводе
		void * notification_data_ptr = 0;
				// флаг, было ли выбрано оповещения из очереди
		bool have_notification = false;
			// эксклюзивный доступ к очереди
		async_accsess_sem.acquire();

		if (notifications.size())
		{				
				// вытаскиваем информацию оповещения по-быстрому
			notification_data_ptr = (*(notifications.begin()));
				// убираем вытащенное оповещение из очереди
			notifications.pop_front();
				// устанавливаем флаги
			have_notification = true;
			had_notifications_to_process = true;
		}
				// и сразу отпускаем семафор, а то задержим кого-нибудь
		async_accsess_sem.release();

			// всё, очередь обмена освободили, теперь можно не торопясь вызвать оповещение, если было что-то выбрано, конечно
		if (have_notification)
			FastNotifyer::Notify(notification_data_ptr);			
	};

	notify_thread_running = false;
};

/// добавить посылку в список уведомлений об отправке на устройство
void FastAsyncNotifyer::Notify(void *data_ptr)
{
	accsess_sem.acquire();

	notifications.push_back(data_ptr);

	accsess_sem.release();
};

/// очистить очередь оповещений об отправке
void FastAsyncNotifyer::ClearNotificationList()
{
	accsess_sem.acquire();

	notifications.clear();

	accsess_sem.release();
};

/// работает ли оповещатель
bool FastAsyncNotifyer::IsNotificatorWorking()
{
	return notify_thread_running;
};

/// Конструктор
FastAsyncNotifyer::FastAsyncNotifyer(): async_accsess_sem(1)
{
	exit = false;
	notify_thread_running = false;
};

/// Деструктор
FastAsyncNotifyer::~FastAsyncNotifyer()
{
	StopNotificator();
};

/// запуск оповещателя
void FastAsyncNotifyer::StartNotificator()
{
	async_accsess_sem.acquire();

	if (!notify_thread_running)
	{
		notify_thread_running = true;

		this->start();
	};

	async_accsess_sem.release();
};

/// выход (остановка потока)
void FastAsyncNotifyer::StopNotificator()
{
	accsess_sem.acquire();

	exit = true;

	accsess_sem.release();

	this->wait();
};
