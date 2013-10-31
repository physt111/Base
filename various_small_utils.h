/*! \file 
	Файл содержит оглавление разнообразных маленьких утилиток-функций,
	которые могут оказаться полезными много где.
*/ 

/*!
|	ID класса в котором описан метод
|	|
|	|	ID метода, вызвавшего исключение
|	|	|
|	|	|	Номер ошибки в методе
01	000	ХХ	ХХ
*/

#ifndef VARIOUS_PHYST_SMALL_UTILS
#define VARIOUS_PHYST_SMALL_UTILS

namespace std
{
	/// строка из двух символов HEX цифр
	struct HEXByteRepresentation;

	/// Преобразует десятиричное значение в символ цифры 16тиричной системы исчисления
	inline unsigned char DecimalNumberToHEXDigit(unsigned char number);

	/// Преобразует символ цифры 16тиричной системы исчисления в соответствующее десятиричное значение
	inline unsigned char HEXDigitToDecimalNumber(unsigned char hex_digit_symbol);

	/// Шаблонные определения
	template<class type_ByteArray, class type_String>
	type_String FromByteArrayToHEXString(type_ByteArray source, int from_index, int to_index, bool with_spaces, char spacer);

	template<class type_String>
	type_String FromByteToHEXString(unsigned char c);

	template<class type_ByteArray, class type_String>
	type_ByteArray FromHEXStringToByteArray(type_String source, bool spaced, int from_index, int to_index);

	// конкретные реализации

};

#include <various_small_utils.cpp>


#endif
