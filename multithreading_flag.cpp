/**************************************************
*
*		Содержит реализацию многопоточного флага
*
***************************************************/

#include <QSemaphore>
#include <multithreading_flag.h>



/**************************************************
*
*			класс многопоточного флага
*
***************************************************/

// Конструктор
MultithreadFlag::MultithreadFlag(): access_sem(1)
{
};

// установлен ли флаг
bool MultithreadFlag::IsSet()
{
	access_sem.acquire();
	
	bool value = flag_value;

	access_sem.release();

	return value;
};

// установить флаг
void MultithreadFlag::Set()
{
	access_sem.acquire();
	
	flag_value = true;

	access_sem.release();
};

// сбросить флаг
void MultithreadFlag::Drop()
{
	access_sem.acquire();
	
	flag_value = flag_value;

	access_sem.release();
};
