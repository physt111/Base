/***************************************************************************************
*
*	Общий протокол пакетного канала связи.
*		
*		Обеспечивает функционал для разбора посылок по специализированным хранилищам
*
***************************************************************************************/

#ifndef MBN_COMMON_PROTOCOL
#define MBN_COMMON_PROTOCOL

#include <PagedArray.cpp>
#include <CommonPackage.h>
#include <CommonByteStreamParser.h>
#include <string>


/// Общий коммуникационный протокол
class CommonProtocol
{
public:
	/**************************************************************************************
	*	
	*
	*				Информация о том, что и где лежит, хранилища данных,
	*	Очереди пакетов и байт.
	*
	*
	*
	**************************************************************************************/

	/// возвращает общий класс посылки (по данному протоколу)
	virtual string GetPackageTypeString(int type, int subtype) = 0;
	/// возвращает название посылки (по данному протоколу)
	virtual string GetPackageNameString(int type, int subtype) = 0;


    /// индекс посылки в массиве посылок входной очереди, которую нужно будет обработать на уровне логики протокола при следующем вызове
    int pkg_idx_to_process;

    // Обнулить индекс посылки для обработки на уровне протокола
    void FlushNextPackToManageIndex()
    {
        pkg_idx_to_process = 0;
    }

	/// парсер кода для протокола
    CommonByteStreamParser<PagedArray<unsigned char>, PagedArray<CommonPackage> >* StreamParserPtr;
	
	/// Обрабатывает в логике протокола связи смысла получаемых данных эти самые данные :)
	/// Проще говоря, определяет, логику работы на уровне "между посылок" - т.е. какие комманды были исполнены,
	/// Какие были исполнены верно. А также распределяет полученные от устройства данные в соответствии с их 
	/// смыслом.
	/// Out_pkg_Ptr - адрес выводной очереди пакетов
	/// Out_byte_Ptr - адрес выводной очереди байт
	/// In_pkg_Ptr -  Входная очередь пакетов
	/// In_byte_Ptr - Входная очередь байт
    virtual void ManagePackageSequenses(PagedArray<CommonPackage> *Out_pkg_Ptr, PagedArray<unsigned char> *Out_byte_Ptr,
                                        PagedArray<CommonPackage> *In_pkg_Ptr,  PagedArray<unsigned char> *In_byte_Ptr,
										int next_pack_to_write_idx) = 0;
};

#endif
