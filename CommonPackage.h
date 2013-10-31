/*! \file 
	Файл содержит класс общего пакета данных CommonPackage
		CommonPackage - общий класс пакета для всех пакетных протоколов, который хранит данные о принятом/отосланном пакете данных
*/ 

#include <string>
#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <iostream>
#include <vector>

#ifndef MBN_COMMON_PACKAGE_INCLUDE_GUARD
#define MBN_COMMON_PACKAGE_INCLUDE_GUARD

using namespace std;

/*!
|	ID класса в котором описан метод
|	|
|	|	ID метода, вызвавшего исключение
|	|	|
|	|	|	Номер ошибки в методе
02	003	ХХ	ХХ
*/

/// <summary> Общий класс пакета для всех пакетных протоколов, который хранит данные о принятом/отосланном пакете данных </summary>
struct CommonPackage
{
public:
	/****************************************
	 *		ПЕРЕЧИСЛЕНИЯ
	 ****************************************/
	/// <summary> Направление посылки </summary>
	enum PackageDirection {IN_DIR, OUT_DIR};
	/// <summary> Уровень целостности посылки </summary>
	enum PackageCorrectnessLevel {UNKNOWN_CORRECTNESS, CORRECT, DAMAGED_RESTORABLE, DAMAGED_NOT_RESTORABLE};
	/// <summary> Размещение данных посылки </summary>
	enum PackageDataLocation {INTERNAL, EXTERNAL};

	/// <summary> Возможные типы посылок </summary>
	enum Common_PackageType {UNKNOWN_TYPE = -1};
	/// <summary> Возможные подтипы посылок </summary>
	enum Common_PackageSubType {UNKNOWN_SUBTYPE = -1};

public:

	/****************************************
	 *	КОНСТАНСТНЫЕ ДАННЫЕ ПОСЫЛКИ
	 ****************************************/
	/// <summary> Направление посылки </summary>
	PackageDirection direction; 
	/// <summary> Тип посылки (назачение определяется пользователем), это может быть тип комманды или тип данных или что-то ещё.</summary>
	int type;
	/// <summary> Подтип посылки (назачение определяется пользователем), это может быть подтип комманды или подтип данных или что-то ещё.</summary>
	int subtype;

	/***************************
	 *	ОБЩИЕ ДАННЫЕ ПОСЫЛКИ
	 ***************************/
	/// <summary> Уровень целостности посылки </summary>
	PackageCorrectnessLevel correctness;
	/// <summary> ID посылки, как он задаётся - определяется пользователем.</summary>
    unsigned long long id;
	/// <summary> ID посылки, как он задаётся - определяется пользователем.</summary>
	::QDateTime date_time;

	/******************************
	 *	ЛОКАЛИЗАЦИЯ ДАННЫХ ПОСЫЛКИ
	 ******************************/
	/// <summary> Подтип посылки (назачение определяется пользователем), это может быть подтип комманды или подтип данных или что-то ещё.</summary>
	PackageDataLocation data_location;
	/// <summary> Полное количество байт в данных посылки.</summary>
	int package_data_length;
	/// <summary> Смещение (в байтах) первого байта данных посылки от начала потока данных.</summary>
    long long first_byte_stream_offset;
	/// <summary> Если используется встроенное хранение данных то размер этого массива не нуль и он хранит данные посылки</summary>
	vector<unsigned char> direct_data;

	/**************************************
	 *	ПОЛЯ ДЛЯ ОРГАНИЗАЦИИ ВВОДА-ВЫВОДА
	 **************************************/
	/// <summary> Флаг, можно ли удалить эту посылку.</summary>
	bool can_be_erased;
	/// <summary> Флаг, была ли эта посылка, в зависимости от направления, пропарсена или отправлена.</summary>
	bool parsed_or_sended;
	/// <summary> Флаг, была ли эта посылка, в зависимости от направления, применена, как подтверждение, или была подтверждена.</summary>
	bool ack; 

	CommonPackage()
	{
		type = UNKNOWN_TYPE;
		subtype = UNKNOWN_SUBTYPE;
		correctness = UNKNOWN_CORRECTNESS;
		parsed_or_sended = false;
		can_be_erased = false;
		ack = false;
	};

//public:
//	/// <summary> Обычный конструктор </summary>
//	CommonPackage(PackageDirection pack_dir, int pack_type, int pack_subtype, PackageDataLocation d_location): 
//	  direction(pack_dir),  type(pack_type), subtype(pack_subtype), data_location(d_location)
//	{
//
//	};
//
//	/// <summary> Возвращает метод хранения данных посылки</summary>
//	/// <returns> Метод хранения данных посылки</returns>
//	PackageDataLocation GetDataLocation()
//	{
//		return data_location;
//	};
//
//	/// <summary> Возвращает направление посылки </summary>
//	/// <returns> Направление посылки </returns>
//	PackageDirection GetDirection()
//	{
//		return direction;
//	};
//
//	/// <summary> Устанавливает направление посылки </summary>
//	/// <param name="dir"> Устанавливаемое направление посылки</param> 
//	virtual void SetDirection(PackageDirection dir)
//	{
//		direction = dir;
//	};
//
//	/// <summary> Возвращает коррекность посылки </summary>
//	/// <returns> Коррекность посылки </returns>
//	PackageCorrectnessLevel GetCorrectness()
//	{
//		return correctness;
//	};
//
//	/// <summary> Устанавливает направление посылки </summary>
//	/// <param name="dir"> Устанавливаемое направление посылки</param> 
//	virtual void SetCorrectness(PackageCorrectnessLevel corr_lev)
//	{
//		correctness = corr_lev;
//	};
//	
//	/// <summary> Возвращает тип посылки </summary>
//	/// <returns> Тип посылки </returns>
//	virtual int GetType()
//	{
//		return type;
//	};
//
//	/// <summary> Возвращает подтип посылки </summary>
//	/// <returns> Подтип посылки </returns>
//	virtual int GetSubtype()
//	{
//		return subtype;
//	};
//
//	/// <summary> Возвращает ID посылки </summary>
//	/// <returns> ID посылки </returns>
//	long long GetID()
//	{
//		return id;
//	};
//
//	/// <summary> Устанавливает ID посылки </summary>
//	/// <param name="dir"> Устанавливаемый ID посылки</param> 
//	virtual void SetID(long long n_id)
//	{
//		id = n_id;
//	};
//
//	/// <summary> Возвращает время операции приёма-передачи посылки </summary>
//	/// <returns> Время операции приёма-передачи посылки </returns>
//	QDateTime GetDateTime()
//	{
//		return date_time;
//	};
//
//	/// <summary> Устанавливает время операции приёма-передачи посылки </summary>
//	/// <param name="dir"> Устанавливаемое время операции приёма-передачи посылки</param> 
//	virtual void SetDateTime(QDateTime n_date_time)
//	{
//		date_time = n_date_time;
//	};
//
//	/// <summary> Возвращает полный размер данных посылки в байтах </summary>
//	/// <returns> Полный размер данных посылки в байтах  </returns>
//	int GetTotalDataLength()
//	{
//		return package_data_length;
//	};
//
//	/// <summary> Устанавливает полный размер данных посылки в байтах  </summary>
//	/// <param name="dir"> Устанавливаемый полный размер данных посылки в байтах </param> 
//	virtual void SetDateTime(int n_package_data_length)
//	{
//		package_data_length = n_package_data_length;
//	};
//
//	/// <summary> Возвращает смещение первого байта данных посылки относительно начала потока </summary>
//	/// <returns> Смещение первого байта данных посылки относительно начала потока  </returns>
//	long long GetFirstByteOffset()
//	{
//		return first_byte_stream_offset;
//	};
//
//	/// <summary> Устанавливает смещение первого байта данных посылки относительно начала потока  </summary>
//	/// <param name="dir"> Устанавливаемое смещение первого байта данных посылки относительно начала потока </param> 
//	virtual void SetFirstByteOffset(int n_first_byte_stream_offset)
//	{
//		first_byte_stream_offset = n_first_byte_stream_offset;
//	};
//
//	/// <summary> Возвращает флаг, можно ли удалить эту посылку </summary>
//	/// <returns> флаг, можно ли удалить эту посылку </returns>
//	bool GetCanBeErasedFlag()
//	{
//		return can_be_erased;
//	};
//
//	/// <summary> Устанавливает флаг, можно ли удалить эту посылку  </summary>
//	/// <param name="dir"> Устанавливаемый флаг, можно ли удалить эту посылку </param> 
//	virtual void SetCanBeErasedFlag(bool n_can_be_erased)
//	{
//		can_be_erased = n_can_be_erased;
//	};
//
//	/// <summary> Возвращает флаг, была ли эта посылка, в зависимости от направления, пропарсена или отправлена </summary>
//	/// <returns> флаг, была ли эта посылка, в зависимости от направления, пропарсена или отправлена </returns>
//	bool GetParsedOrSendedFlag()
//	{
//		return parsed_or_sended;
//	};
//
//	/// <summary> Устанавливает флаг, была ли эта посылка, в зависимости от направления, пропарсена или отправлена </summary>
//	/// <param name="dir"> Устанавливаемый флаг, была ли эта посылка, в зависимости от направления, пропарсена или отправлена </param> 
//	virtual void SetParsedOrSendedFlag(bool n_parsed_or_sended)
//	{
//		parsed_or_sended = n_parsed_or_sended;
//	};
//
//	/// <summary> 
//	///		Возвращает данные посылки
//	/// </summary>
//	/// <returns> Возвращает встроенные данные посылки. Они могут быть пустыми, если используется не встроенное хранение данных</returns>
//	vector<unsigned char> GetEmbeddedData()
//	{
//		return this->direct_data;
//	};
};

#endif
