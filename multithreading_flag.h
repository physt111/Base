/*************************************************************************
*
*	Содержит класс многопоточного флага
*
*************************************************************************/

#ifndef PHYST_MULTITHREADING_FLAG
#define PHYST_MULTITHREADING_FLAG

#include <QSemaphore>

// класс многопоточного флага
class MultithreadFlag
{
private:

	QSemaphore access_sem;

	// сам флаг
	bool flag_value;

public:

	// Конструктор
	MultithreadFlag();

	// установлен ли флаг
	bool IsSet();

	// установить флаг
	void Set();

	// сбросить флаг
	void Drop();
};


#endif
