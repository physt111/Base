/*************************************************************************
*
*	Файл содержит описание быстрого оповещателя.
*
*	Суть класса в следующем: класс содержит список функций 
*	фиксированного вида (лучше шаблона, да Qt например, шаблоны
*	не поддерживает). Точнее, список структур, каждая из которых 
*	содержит указатель на функцию вида void f(void* ptr), и
*	void* ptr. 
*
*	При вызове метода Execute - последовательно выполняются
*	все вызовы всех указанных функций.
*
*	Идеология использования следующая:
*
*		Когда есть какой-то объект на который по указателю (для скорости,
*	чтобы не копировать) ссылаются другие объекты, о которых этот объкт,
*	по сути, ничего не знает, то при удалении этого объекта все указатели
*	становятся на него становятся невалидными. И об этом надо эти объекты
*	предупредить - собсна, это и делает этот объект.
*
*	При этом, если другие объекты удаляются, они должны сами удалить
*	(например, в деструкторе) все записи оповещения в тех Оповещателях
*	в которые записи были внесены.
*
*************************************************************************/

#ifndef FAST_NOTIFYER
#define FAST_NOTIFYER

#include <vector>
#include <PagedArray.cpp>
//#include <MBN_Signal_Plotter.h> my insertion
#include <qlabel.h>
//#include <QGridLayout.h> my insertion
#include <list>
#include <QSemaphore>
#include <QThread>
using namespace std;

// тип функции обратного вызова (функции оповещения)
// user_ptr - указатель передаваемый клиентским кодом в ФОВ,
// data_ptr - указатель на данные передаваемые клиентскому коду через ФОВ
// last_in_notificators_list - является ли данный вызов последним в списке нотификатора
//		последнее поле нужно для того, чтобы клиентские функции могли удалить данные в 
//		куче (если data_ptr указывает на данные в куче), не боясь, что следующие ФОВ получат
//		указатель уже на невалидные данные
typedef void (*notify_func_ptr_type)(void *user_ptr, void *data_ptr, bool last_in_notificators_list);


/// запись обратного вызова
struct NotifyRecord
{
	notify_func_ptr_type cb_f_ptr;
	void* user_ptr;
	void* data_ptr;
};


/// класс быстрого синхронного оповещателя
class FastNotifyer
{
protected:

	/// Семафор доступа к данному объекту
	QSemaphore accsess_sem;

	// список обратных вызовов
	vector<NotifyRecord>  cb_records;

public:

	// Блокировать доступ (для избежания защелкивания)
	void Block();
	// Разблокировать доступ
	void Release();

	// конструктор
	FastNotifyer();

	// выполнить оповещение всех
	void Notify(void* data_ptr);

	// есть ли хоть одна запись обратного вызова
	bool HasAnyNotifyCBRec();
	// есть ли запись с указанным адресом функции
	bool HasNotifyCBRec(notify_func_ptr_type f_ptr);
	// есть ли запись с указанным пользовательским адресом
	bool HasNotifyCBRec(void* user_ptr);
	// есть ли запись с указанным адресом функции и одновременно с указанным пользовательским адресом
	bool HasNotifyCBRec(notify_func_ptr_type f_ptr, void* user_ptr);

	// если такой записи нет - добавляет запись в список оповещения
	void AddNotifyCB(notify_func_ptr_type f_ptr, void* user_ptr);

	// удалить записи с указанным адресом функции, если нет - ничего не делает
	void DeleteNotifyCBRec(notify_func_ptr_type f_ptr);
	// удалить записи с указанным пользовательским адресом, если нет - ничего не делает
	void DeleteNotifyCBRec(void* user_ptr);
	// удалить записи с указанным адресом функции и одновременно с указанным пользовательским адресом, если нет - ничего не делает
	void DeleteNotifyCBRec(notify_func_ptr_type f_ptr, void* user_ptr);

	// очистить список оповещения
	void Clear();
};



/// класс быстрого асинхронного оповещателя
class FastAsyncNotifyer: protected QThread, public FastNotifyer
{
private:
	
	/// Семафор доступа к данному объекту
	QSemaphore async_accsess_sem;

	/// массив отправленных посылок, о которых надо сообщить
	list<void*> notifications;

	/// флаг останова
	bool exit; 

	/// флаг работы
	bool notify_thread_running;
	
private:

	/// Выполняет всю работу по оповещению о событиях
	/// просматривает очередь оповещений о событиях (notifications_data), если в ней что-то
	/// есть, то вызывает соответствующие нотификаторы и соответственными данными
	virtual void run();

public:
/// работа со списками оповещений

	/// добавить посылку в список уведомлений об отправке на устройство
	void Notify(void *data_ptr);

	/// очистить очередь оповещений об отправке
	void ClearNotificationList();


public:
// управление

	/// работает ли оповещатель
	bool IsNotificatorWorking();

	/// Конструктор
	FastAsyncNotifyer();

	/// Деструктор
	~FastAsyncNotifyer();

	/// запуск оповещателя
	void StartNotificator();

	/// выход (остановка потока)
	void StopNotificator();
};




#endif
