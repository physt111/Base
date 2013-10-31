/*! \file 
	Файл содержит разнообразные маленькие утилитки-функции,
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

namespace std
{

	struct HEXByteRepresentation
	{
		unsigned char hi_digit;
		unsigned char low_digit;
	};

	/// <summary> 
	/// Превращает число [0;15] в HEX-цифру.
	/// </summary>
	/// <param name = "number">	Беззнаковое число от [0; 15] </param>
	/// <returns>	Символ соответствующей HEX-цифры. Если передано число не в диапазоне [0; 15], возвращает 0; </returns>
	inline unsigned char DecimalNumberToHEXDigit(unsigned char number)
	{
		switch (number)
		{
		case 0: return '0'; 
		case 1: return '1'; 
		case 2: return '2'; 
		case 3: return '3'; 
		case 4: return '4'; 
		case 5: return '5'; 
		case 6: return '6'; 
		case 7: return '7'; 
		case 8: return '8'; 
		case 9: return '9'; 
		case 10: return 'A'; 
		case 11: return 'B'; 
		case 12: return 'C'; 
		case 13: return 'D'; 
		case 14: return 'E'; 
		case 15: return 'F'; 
		default: return 0;
		};
	};
	 
	///<summary> 
	/// Превращает HEX-цифру в число.
	/// </summary>
	/// </remarks>
	/// Переводит cимвол HEX-цифры ['0','1'...'F'] в беззнаковое целое число из диапазона [0; 15].
	/// <remarks>
	/// <param name = "hex_digit_symbol">		Cимвол HEX-цифры ['0'; 'F'] </param>
	/// <returns>	Соответствующее беззнаковое целое число из диапазона [0; 15]. Если передано число не в диапазоне [0; 15], возвращает 0xFF. </returns>
	inline unsigned char HEXDigitToDecimalNumber(unsigned char hex_digit_symbol)	
	{
		switch (hex_digit_symbol)
		{
		case '0': return 0; 
		case '1': return 1; 
		case '2': return 2; 
		case '3': return 3; 
		case '4': return 4; 
		case '5': return 5; 
		case '6': return 6; 
		case '7': return 7; 
		case '8': return 8; 
		case '9': return 9; 
		case 'A': return 10; 
		case 'B': return 11; 
		case 'C': return 12; 
		case 'D': return 13; 
		case 'E': return 14; 
		case 'F': return 15; 
		default: return 0xFF;
		};
	};

	/// <summary>  
	/// Преобразование массива байт (или его части) в HEX-строку.
	/// </summary>
	/// <remarks>
	/// Переводит указанную индексами [from_index; to_index] (включительно) часть байт-массива в соответствующую строку HEX символов,
	/// расставляя spacer между парами HEX-символов, если указан with_spaces. 
	/// \attention	Никаких проверок не делает! Индексы должны быть правильными. 
	/// </remarks>
	/// <typeparam name = "type_ByteArray">	Класс входного массива байтов. Должна быть определена операция [].  </typeparam>
	/// <typeparam name = "type_String">		Класс выходной строки HEX символов. Должны быть определены операции resize(...) и []. </typeparam>
	/// <param name = "source">			входной байт-массив для конвертации в HEX-строку </param>
	/// <param name = "from_index">		индекс в source, с которого (включительно) начинать преобразование </param>
	/// <param name = "to_index">		индекс в source, до которого (включительно) преобразовывать </param>
	/// <param name = "with_spaces">		устанавливать ли разделитель между парами HEX-символов </param>
	/// <param name = "spacer">			разделитель, если with_spaces == false, игнорируется </param>
	/// <returns>		соответствующую байт-массиву строку HEX символов </returns>
	template<class type_ByteArray, class type_String>
	type_String FromByteArrayToHEXString(type_ByteArray source, int from_index, int to_index, bool with_spaces, char spacer)
	{
		// на каждый байт по два символа
		type_String tmps;

		if (!with_spaces)
		{
			tmps.resize((to_index - from_index + 1) * 2);

			for (int i = 0; i <= to_index - from_index; i++)
			{
				tmps[2*i + 0] = DecimalNumberToHEXDigit(source[from_index + i]>>4);
				tmps[2*i + 1] = DecimalNumberToHEXDigit(source[from_index + i] & 0x0F);
			};
		}
		else if (to_index > from_index)
		{
			tmps.resize((to_index - from_index + 1) * 3 - 1);

			for (int i = 0; i <= to_index - from_index; i++)
			{
				tmps[3*i + 0] = DecimalNumberToHEXDigit(source[from_index + i]>>4);
				tmps[3*i + 1] = DecimalNumberToHEXDigit(source[from_index + i] & 0x0F);
				if (i != to_index - from_index)
					tmps[3*i + 2] = spacer;
			};
		};

		return tmps;
	};

	/// <summary>  
	/// Переводит указанное беззнаковое число в соответствующую HEX строку (например, 255 -> "FF"). 
	/// </summary>
	/// <tparam name = "type_String">	Класс выходной строки HEX символов. Должны быть определены операции resize(...) и []. </typeparam>
	/// <param name = "c">	Беззнаковое число для конвертации в HEX-строку. </param>
	/// <returns>	соответствующую беззнаковому числу строку из двух HEX символов </returns>
	template<class type_String>
	type_String FromByteToHEXString(unsigned char c)
	{
		// на каждый байт по два символа
		type_String tmps;
		tmps.resize(2);

		tmps[0] = DecimalNumberToHEXDigit(c>>4);
		tmps[1] = DecimalNumberToHEXDigit(c & 0x0F);
		
		return tmps;
	};

	/// <summary>  
	/// Переводит строку HEX символов (в виде "FF0F3DE3"  или "FF 0F 3D E3", где разделителем может быть любой символ) в беззнаковый байт массив.
	/// Cтрока парсится в [from_index; to_index], результат (то, что смогли пропарсить) возвращается.
	/// </summary>
	/// <remarks>  \attention	Никаких проверок не делает! Индексы должны быть правильными. </remarks> 
	/// <typeparam name = "type_ByteArray">	Класс выходного массива беззнаковых байт. Должна быть определена операция push_back(...). </typeparam>
	/// <typeparam name = "type_String"> Класс входной строки HEX символов. Должна быть определена операция []. </typeparam>
	/// <param name = "from_index">	индекс в source, с которого (включительно) начинать преобразование </param>
	/// <param name = "to_index">	индекс в source, до которого (включительно)  </param>
	/// <param name = "spaced">	есть ли разделитель между парами HEX-цифр </param>
	/// <returns>		соответствующий HEX-строке байт-массив (то, что смогли пропарсить) </returns>
	template<class type_ByteArray, class type_String>
	type_ByteArray FromHEXStringToByteArray(type_String source, bool spaced, int from_index, int to_index)
	{
		// временный массив
		type_ByteArray tmpa;

		// инкремент индекса до следующей пары HEX-символов
		int di = 2;
		if (spaced) di = 3;

		for (int i = from_index; i <= to_index; )
		{
			// если хотя бы один из символов из пары не является HEX символом, сдвигаемся на 1
			if ((HEXDigitToDecimalNumber(source[i]) == 0xFF) || (HEXDigitToDecimalNumber(source[i+1]) == 0xFF)) i++;
			else /// иначе, парсим текущую пару символов
			{
				tmpa.push_back((HEXDigitToDecimalNumber(source[i])<<4) | HEXDigitToDecimalNumber(source[i+1]));
				i += di;
			};
		};

		return tmpa;
	};
};
