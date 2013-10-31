/*! \file 
	Файл содержит класс страничного массива.
*/ 

/*!
|	ID класса в котором описан метод
|	|
|	|	ID метода, вызвавшего исключение
|	|	|
|	|	|	Номер ошибки в методе
01	001	ХХ	ХХ
*/

#ifndef GLOBAL_PAGED_ARRAY
#define GLOBAL_PAGED_ARRAY

//#include <StdAfx.h>
#include <vector>

/// Класс (ID = 002), Блок данных в страничном массиве PagedArray
template <class T> 
struct DataBlock 
{
	T* data;	 /// данные блока
	const unsigned int capacity; /// ёмкость блока

	unsigned int size;  /// используемый размер блока

    long long ID;  /// идентефикатор блока данных

	bool pushed; /// вытолкнут ли этот блок данных на диск (или другое хранилище)


    DataBlock(long long Id = 0, unsigned int Capacity = 1, unsigned int Size = 0): capacity(Capacity)
	{
		//if (!Capacity) throw std::exception("Нулевой размер страницы" __FILE__);
		//if (Size > Capacity) throw std::exception("Размер больше ёмкости страницы" __FILE__);

		ID = Id;

		data = new T[capacity];

		size = Size;

		pushed = false;
	};

	DataBlock(const DataBlock &Orig): capacity(Orig.capacity)
	{
		data = new T[capacity];
		size = Orig.size;

		//if (size > capacity) throw std::exception("Размер больше ёмкости страницы" __FILE__);

		for (unsigned int i = 0; i < size; i++) 
			data[i] = Orig.data[i];

		ID = Orig.ID;
		pushed = Orig.pushed;
	};

	DataBlock operator=(const DataBlock& Orig)
	{
		return DataBlock(Orig);
	};

	~DataBlock()
	{
		delete[] data;
		data = 0;

		size = 0;
	};

	void resize(unsigned int new_size)
	{
		if (new_size > capacity) 
            throw;// std::exception("Устанавливаемый размер больше чем ёмкость страницы" __FILE__);
		size = new_size;
	};
};

/// Класс (ID = 001), служащий для хранения данных в страничном массиве, которые по мере необходимости могут кешироваться или 
/// совсем сохраняться в хранилище. Структура массива: массив массивов, каждый вторичный массив - это страница,
/// которая не зависит от остальных. Выделение памяти идёт по странично, без копирования остальных страниц масива,
/// что особенно важно про больших размерах массива.
template <class T>
class PagedArray
{
#define DEF_PAGE_SIZE 1024	/// Размер страницы по-умолчанию
#define DEF_HEADER_CAPACITY_STEP 1024	/// Инкремент размера массива-оглавления по-умолчанию

private:
	std::vector< DataBlock<T> > header; /// массив к которому подключаются блоки данных в виде массивов
								   /// размер изменяется инкрементально, порциями по header_capacity_step

	int page_capacity;   /// ёмкость страницы (в элементах типа T) (страницы в памяти выделяются целиком, по page_capacity элементов).

	int header_capacity_step; // Инкремент ёмкости массива-оглавления. Т.е. какими порциями увеличиваем или уменьшаем массив оглавления

	int size_; /// текущий размер массива (т.е. количество элементов T в нём)

    long long ID_for_new_el; /// ID для следующей страницы

private:
	/// Вычисляет большее, но ближайшее к частному A/Divider, целое значение (ID = 16). Например: 5.4 -> 6;	5.9 -> 6;	6.0 -> 6; 
	/// \param  A			делимое
	/// \param  Divider		делитель
	/// \return				Вычисляет большее, но ближайшее к частному A/Divider, целое значение
	inline int IntFractionCeil(int A, int Divider)
	{
		int result = A/Divider;
		if (A % Divider) result++;
		return result;
	};

	/// Какое количество страниц текущего размера понадобится для ElementsCount элементов (ID = 17).
	/// \param ElementsCount		количество элементов для которого нужно вычислить соответствующее число необходимых страниц
	/// \return						Какое количество страниц текущего размера понадобится для ElementsCount элементов
	inline int NecessaryPagesCount(int ElementsCount)
	{
		return IntFractionCeil(ElementsCount, page_capacity);
	};

	/// Вычисляет квантованный размер оглавления (в структурах страниц) на основании текущего шага изменения размера оглавления (ID = 18).
	/// \param NecessaryPagesCount		количество страниц для которого нужно вычислить соответствующий квантованный размер оглавления
	/// \return						Квантованный размер оглавления соответствующий указанному количеству страниц
	inline int HeaderQuantizizedRecordsCount(int NecessaryPagesCount)
	{
		return header_capacity_step * IntFractionCeil(NecessaryPagesCount, header_capacity_step);
	};

public:
	/// Конструктор (ID = 01).
	/// \param _size				Размер создаваемого массива (в элементах T)			(Должен быть >=0)
	/// \param _page_size			Размер одной страницы массива (в элементах T)		(Должен быть >=0. Если == 0, то размер по-умолчанию)
	/// \param _min_reserved_size	Размер, зарезервированный в памяти (в элементах T) (!) реальный размер округляется до страницы (!)	(Должен быть >=_size или 0. Если == 0 - автоматически)
	/// \param _header_capacity_step	Инкремент размера массива-оглавления. Т.е. какими порциями увеличиваем или уменьшаем массив оглавления	(Должен быть >=0. Если == 0, то размер по-умолчанию)
	PagedArray(int _size = 0, int _page_size = DEF_PAGE_SIZE, int _min_reserved_size = 0, int _header_capacity_step = DEF_HEADER_CAPACITY_STEP)
	{
		// корректность параметров
        if (_size < 0) throw;// std::exception("Неверный размер страничного массива", 01);
        if (_page_size < 0) throw;// std::exception("Неверный размер страницы памяти", 02);
        if ((_min_reserved_size < _size) && (_min_reserved_size != 0)) throw;// std::exception("Неверный объём резервирования памяти", 03);
        if (_header_capacity_step < 0) throw;// std::exception("Неверный размер инкремента размера массива-оглавления", 04);
		
		// установка умолчаний, где надо
		if (_page_size == 0) _page_size = DEF_PAGE_SIZE;
		if (_min_reserved_size == 0) _min_reserved_size = _size;
		if (_header_capacity_step == 0) _header_capacity_step = DEF_HEADER_CAPACITY_STEP;

		// обновляем управляюшие поля
		page_capacity = _page_size;
		header_capacity_step = _header_capacity_step;
		ID_for_new_el = 0;
		size_ = 0;
		
		// создаёмся %:)
		resize(_size);
	};
			
	/// Изменение размера массива (выделяет дополнительные страницы, если надо, но не удаляет освободившиеся) (ID = 02).
	/// \param new_size				Новый размер массива (в элементах T)
	void resize(int new_size)
	{
		// проверка параметров на глупость
        if (new_size < 0) throw;// std::exception("Неверный размер страничного массива", 01);

		// если нужна дополнительная память, выделяем её
		reserve(new_size);
		
		int new_pages_count = NecessaryPagesCount(new_size); // число страниц, требуемых для нового размера
		int orig_not_empty_pages_count = NecessaryPagesCount(size_); // начальное число непустых страниц

		// Варианты изменения размера
		if (new_pages_count > orig_not_empty_pages_count) // если число непустых страниц увеличилось
			{
				// отдельный случай, для первой страницы, которая была раньше, но могла быть не до конца использована.
				if (orig_not_empty_pages_count) header[orig_not_empty_pages_count - 1].resize(page_capacity);

				// теперь расширяем страницы (всем, кроме последней заполняемой ставим размер на максимум)
				for (int i = orig_not_empty_pages_count; i < new_pages_count - 1; i++) 
					header[i].resize(page_capacity);
			}
		else if (new_pages_count < orig_not_empty_pages_count) // число страниц уменьшилось
			for (int i = orig_not_empty_pages_count - 1; i > new_pages_count - 1; i--) // теперь ужимаем страницы
				header[i].resize(0);
		// для последней страницы - смотрим по кратности размеру страницы
		if (new_pages_count)
			if (new_size % page_capacity) header[new_pages_count - 1].resize(new_size % page_capacity); // полупустая страница
			else header[new_pages_count - 1].resize(page_capacity); // полная страница

		// обновляяем поле размера
		size_ = new_size;

		// всё, вроде :)
	};

	/// Резервирует необходимое для указанного размера массива (в элементах Т) число страниц (ID = 03).
	/// Если текущая ёмкость больше запрашиваемой - ничего не делает.
	/// \param reserve_size				Размер массива (в элементах T) под который нужно выделить место
	void reserve(int reserve_size)
	{
		// проверка параметров на глупость
        if (reserve_size < 0) throw;// std::exception("Неверный размер для резервирования страничного массива", 01);

		int orig_pages_count = header.size(); // начальное число страниц (в т.ч. зарезервированных)
		int new_pages_count = NecessaryPagesCount(reserve_size); // число страниц, требуемых для нового размера
		
		// если в заголовке недостаточно ёмкости, чтобы вместить записи о новых страницах, изменяем его ёмкость
		if ((unsigned int)(new_pages_count) > header.capacity()) 
			header.reserve(HeaderQuantizizedRecordsCount(new_pages_count)); // квантуем ёмкость заголовка

		// если выделено недостаточно страниц 
		if (new_pages_count > orig_pages_count)
		{
			DataBlock<T> orig(0, page_capacity, 0); // оригинал, с которого делаем массив
			header.resize(new_pages_count, orig); // задаём требуемый размер заголовка
			
			// обновляем ID
			for (int i = orig_pages_count; i < new_pages_count; i++)
				header[i].ID = ID_for_new_el++;
		};
	};

	/// Уменьшает размер массива до минимально возможного для помещённых в него данных (ID = 04).
	void shrink_to_fit()
	{
		int needed_page_count = NecessaryPagesCount(size_); // индекс последней нужной страницы

		// ресайзим оглавление, до минимального размера, что автоматически вызывает деструкторы страниц и освобождение их памяти.
		header.resize(needed_page_count);
		// ужимаем оглавление до минимального
		header.shrink_to_fit();
		// квантуем размер
		header.reserve(HeaderQuantizizedRecordsCount(needed_page_count));
	};

	/// Возвращает ссылку на указанную страницу массива, рекомендуется использовать только для быстрого чтения-записи данных без изменения размеров массивов (ID = 05).
	/// \param i		индекс страницы в массиве (может бежать от начала массива до последней зарезервированной страницы)
	/// \return		Описатель блока данных страничного массива (описатель страницы)
	inline DataBlock<T>& operator()(int i)
	{
		return header[i];
	};

	/// получение элемента по индексу (ID = 06).
	/// \param i			индекс элемента в массиве
	/// \return		ссылку на указанный элемент
	inline T& operator[] (int i)
	{
		return header[i / page_capacity].data[i % page_capacity];
	};

	///// Получение адреса начала страницы на которой находится элемент (ID = 07).
	///// \param element_index		индекс элемента в массиве
	///// \return		адреса начала страницы на которой находится элемент
	//inline T* PageBegin(int element_index)
	//{
	//	return &(header[i / page_capacity].data[0]);
	//};

	///// получение адреса конца страницы на которой находится элемент (ID = 08).
	///// \param element_index		индекс элемента в массиве
	///// \return		адреса конца страницы на которой находится элемент
	//inline T* PageEnd(int element_index)
	//{
	//	return &(header[i / page_capacity].data[page_capacity - 1]);
	//};

	/// получение ёмкости массива (ID = 09).
	/// \return		Текущую ёмкость массива
	inline int capacity() const 
	{
		return page_capacity * header.size();
	};

	/// получение размера массива (ID = 10).
	/// \return		Размер массива
	inline int size() const
	{
		return size_;
	};

	/// получение размера страницы (ID = 11).
	/// \return		размер страницы
	inline int get_page_size() const 
	{
		return page_capacity;
	};

	/// получение полного количества страниц (включая зарезервированные) (ID = 12).
	/// \return		полное количество страниц (включая зарезервированные)
	inline int get_all_pages_count()
	{
		return header.size();
	};

	/// получение количества используемых страниц (включая полу-заполненную страницу) (ID = 13).
	/// \return		количество используемых страниц
	inline int get_used_pages_count()
	{
		return IntFractionCeil(size_, page_capacity);
	};

	/// добавляет элемент в конец массива, при необходимости выделяет новую страницу (ID = 14).
	/// \param new_el	Новый элемент для добавления в массив
	void push_back(const T& new_el) 
	{
		resize(size_ + 1);
		this->operator[](size_ - 1) = T(new_el);
	};

	/// Удаляет последний элемент, размер массива уменьшается на 1 (ID = 15).
	void pop_back()
	{
		if (size_ > 0)
			resize(size_ - 1);
	};
};

#endif
