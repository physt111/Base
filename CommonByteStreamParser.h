/*! \file 
	Файл содержит класс общего пакета данных CommonByteStreamParser
		CommonPackage - общий класс пакета для всех пакетных протоколов, который хранит данные о принятом/отосланном пакете данных
*/ 

#include <string>
#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <iostream>
#include <vector>

#ifndef MBN_COMMON_BYTE_STREAM_PARSER_INCLUDE_GUARD
#define MBN_COMMON_BYTE_STREAM_PARSER_INCLUDE_GUARD

using namespace std;

/*!
|	ID класса в котором описан метод
|	|
|	|	ID метода, вызвавшего исключение
|	|	|
|	|	|	Номер ошибки в методе
03	003	ХХ	ХХ
*/

/// <summary> Значит так,... Класс содержит 
///		*) данные, хранящие состояние парсинга очереди данных в данный момент
///		*) в том числе ссылки на очереди посылок и данных
/// А также - чисто виртуальную функцию - собсна - сам парсер, которая должна быть определена в производном классе.
template<class ByteStorage, class PackagesStorage>
class CommonByteStreamParser
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

protected:
	/// <summary> Указатель на хранилище байт, которое, собсна, должно поддерживать
	/// операцию [], как минимум...остальное допишу, как ясно будет</summary>
	ByteStorage * byte_storage;

	/// <summary> Указатель на хранилище посылок, которое, собсна, должно поддерживать
	/// операции [] и, возможно, увеличение размера...остальное тоже допишу, как ясно будет</summary>
	PackagesStorage * packs_storage;

	/// <summary> Байт, с которого (включительно) надо продолжить парсинг при седующем вызове ParseBytes() </summary>
    long long current_byte_idx;

	/// <summary> Индекс посылки, которую будем заполнять при следующем вызове ParseBytes() </summary>
	int current_parsing_pack_idx;

	/// <summary> Находимся в режиме поиска, предыдущая или текущая посылка битая или мы только что начали парсинг</summary>
	bool Searching;

	/// <summary> Указывает направление данных, для которого успользуется парсер</summary>
	const bool computer_side_mode;

public:

	/// Умолчательный конструктор
	CommonByteStreamParser(bool __computer_side_mode): computer_side_mode(__computer_side_mode)
	{
		byte_storage = 0;
		packs_storage = 0;
		current_byte_idx = 0;
		current_parsing_pack_idx = 0;
		Searching = false;
	};

	/// Конструктор принимающий указатель на очередь байт для парсинга и приёмник посылок, а также флаг направления потока данных
	CommonByteStreamParser(ByteStorage *bytes, PackagesStorage *packs, bool __computer_side_mode = true): 
		computer_side_mode(__computer_side_mode)
	{
		byte_storage = bytes;
		packs_storage = packs;
		current_byte_idx = 0;
		current_parsing_pack_idx = 0;
		Searching = false;

		// добавляем новую посылку, которую будем заполнять на следующей итерации
		CommonPackage tmp_pkg;
		packs_storage->push_back(tmp_pkg);
	};

	/// Задаёт указатели на хранилища байт и пакетов
	void SetQueues(ByteStorage *bytes, PackagesStorage *packs)
	{
		byte_storage = bytes;
		packs_storage = packs;
		current_byte_idx = 0;
		current_parsing_pack_idx = 0;
		Searching = false;

		// добавляем новую посылку, которую будем заполнять на следующей итерации
		CommonPackage tmp_pkg;
		packs_storage->push_back(tmp_pkg);
	};

	/// Парсит последовательность байт byte_storage. Результаты добавляет в packs_storage
	virtual void ParseBytes() = 0;
};



#endif
