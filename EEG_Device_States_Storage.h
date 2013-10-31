/*! \file 
	Файл содержит.... а хз что он содержит на данный момент))
*/ 

#ifndef MBN_EEG_DEVICE_STATES_STORAGE
#define MBN_EEG_DEVICE_STATES_STORAGE

#include <vector>
#include <string>
#include <PagedArray.cpp>
#include <assert.h>
#include <QSemaphore>
#include <EEG_device_architecture.h>


namespace MBN_EEG
{

	/**************************************************************************
	*
	*			Класс - хранилище последовательности состояний устройства
	*
	**************************************************************************/

	/// Хранит последовательность состояний узлов прибора
	class EEG_Device_States_Storage
	{	

	public:
		/**************************************************************
		*
		*			Класс - ручка для описания устройства
		*
		**************************************************************/

		/// Описывает текущее состояние прибора
		///  просто хранит индексы состояний узлов в массивах состояний устройств
		class EEG_Device_State_Handle
		{
		public:

			/// индексы состояний узлов в массивах класса EEG_Device_States
			int BridgeData_St_idx;
			int MainCommutator_St_idx;
			int ChannelsLeds_St_idx;
			int FunctionalLeds_St_idx;
			int ADC_St_idx;
			int DAC_St_idx;
			int Common_St_idx;

			/// id подтверждения, которое определило это состояние
			qint64 created_by_ack_id;

		public:

			// конструктор класса
			EEG_Device_State_Handle(int bridgeData_St_idx,
								    int mainCommutator_St_idx,
									int channelsLeds_St_idx,
									int functionalLeds_St_idx,
									int _ADC_St_idx,
									int _DAC_St_idx,
									int common_St_idx,
									qint64 ack_id);

			EEG_Device_State_Handle ()
			{
				BridgeData_St_idx = 0;
				MainCommutator_St_idx = 0;
				ChannelsLeds_St_idx = 0;
				FunctionalLeds_St_idx = 0;
				ADC_St_idx = 0;
				DAC_St_idx = 0;
				Common_St_idx = 0;

				
				qint64 created_by_ack_id = 0;
			};

		};

		
	private:

		/// семафор доступа ко всему объекту
		QSemaphore accsess_sem;
		
		/// последовательности состояний узлов:
		PagedArray<EEG_Device_Architecture::EEG_CP_BridgeState> BridgeData_Seq;
		PagedArray<EEG_Device_Architecture::EEG_ChannelsCommutatorState> MainCommutator_Seq;
		PagedArray<EEG_Device_Architecture::EEG_ChannelsLedsState> ChannelsLeds_Seq;
		PagedArray<EEG_Device_Architecture::EEG_FunctionalLedsState> FunctionalLeds_Seq;
		PagedArray<EEG_Device_Architecture::EEG_ADC_State> ADC_Seq;
		PagedArray<EEG_Device_Architecture::EEG_Calibrator_State> DAC_Seq;
		PagedArray<EEG_Device_Architecture::EEG_Common_State> Common_Seq;

		/// последовательность состояний устройства
		PagedArray<EEG_Device_State_Handle> Device_States_Sequence;

	public:

		/// конструктор по-умолчанию создаёт первое состояние устройства
		/// оно по-умолчанию находится в состоянии Unknown
		EEG_Device_States_Storage();

		/// добавляет новое состояние устройства в последовательность состояний.
		/// указатели - на описатели новых состояний узлов, если == NULL, то состояния соответствующих узлов не меняются
		/// ack_id - id подтверждения, которое определило это состояние
		void AddNextDevState(EEG_Device_Architecture::EEG_CP_BridgeState*  newBridgeDataPtr, 
							 EEG_Device_Architecture::EEG_ChannelsCommutatorState* newChannelsCommutatorStatePtr,
							 EEG_Device_Architecture::EEG_ChannelsLedsState* newChannelsLedsStatePtr, 
							 EEG_Device_Architecture::EEG_FunctionalLedsState* newFunctionalLedsStatePtr,
							 EEG_Device_Architecture::EEG_ADC_State* newADC_StatePtr, 
							 EEG_Device_Architecture::EEG_Calibrator_State* newCalibrator_StatePtr,
							 EEG_Device_Architecture::EEG_Common_State* newCommon_StatePtr, 
							 qint64 ack_id);

		/// заменяет последнее состояние прибора, чтобы не расширять хранилище
		void ReplaceLasrDevState(EEG_Device_Architecture::EEG_CP_BridgeState*  newBridgeDataPtr, 
								 EEG_Device_Architecture::EEG_ChannelsCommutatorState* newChannelsCommutatorStatePtr,
								 EEG_Device_Architecture::EEG_ChannelsLedsState* newChannelsLedsStatePtr, 
								 EEG_Device_Architecture::EEG_FunctionalLedsState* newFunctionalLedsStatePtr,
								 EEG_Device_Architecture::EEG_ADC_State* newADC_StatePtr, 
								 EEG_Device_Architecture::EEG_Calibrator_State* newCalibrator_StatePtr,
								 EEG_Device_Architecture::EEG_Common_State* newCommon_StatePtr, 
								 qint64 ack_id);

		/// возвращает количество записей в последовательности состояний устройства
		int GetStateSequenceLength();



		/// возвращает состояние моста прибора в указанном индексом состоянии прибора
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		EEG_Device_Architecture::EEG_CP_BridgeState  GetDevState_BridgeState(int dev_state_idx);

		/// возвращает состояние коммутатора прибора в указанном индексом состоянии прибора
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		EEG_Device_Architecture::EEG_ChannelsCommutatorState  GetDevState_ChannelsCommutatorState(int dev_state_idx);

		/// возвращает состояние индикации каналов прибора в указанном индексом состоянии прибора
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		EEG_Device_Architecture::EEG_ChannelsLedsState  GetDevState_ChannelsLedsState(int dev_state_idx);

		/// возвращает состояние технической индикации прибора в указанном индексом состоянии прибора
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		EEG_Device_Architecture::EEG_FunctionalLedsState  GetDevState_FunctionalLedsState(int dev_state_idx);

		/// возвращает состояние АЦП прибора в указанном индексом состоянии прибора
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		EEG_Device_Architecture::EEG_ADC_State  GetDevState_ADCState(int dev_state_idx);

		/// возвращает состояние технической индикации прибора в указанном индексом состоянии прибора
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		EEG_Device_Architecture::EEG_Calibrator_State  GetDevState_Calibrator_State(int dev_state_idx);

		/// возвращает состояние общей части прибора в указанном индексом состоянии прибора
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		EEG_Device_Architecture::EEG_Common_State  GetDevState_Common_State(int dev_state_idx);

		/// возвращает ID подтверждения, подтвердившего данное состояние
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		qint64  GetDevState_AckId(int dev_state_idx);

		/// возвращает полное состояние прибора
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		EEG_Device_Architecture::EEG_Device_State  GetDevState(int dev_state_idx);




		/// возвращает текущее (последнее)  состояние моста прибора в указанном индексом состоянии прибора
		/// в DevState_IDX - записывается индекс состояния устройства, для которого было получено состояние узла
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		EEG_Device_Architecture::EEG_CP_BridgeState  GetCurrDevState_BridgeState(int *DevState_IDX = 0);

		/// возвращает текущее (последнее)  состояние коммутатора прибора в указанном индексом состоянии прибора
		/// в DevState_IDX - записывается индекс состояния устройства, для которого было получено состояние узла
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		EEG_Device_Architecture::EEG_ChannelsCommutatorState  GetCurrDevState_ChannelsCommutatorState(int *DevState_IDX = 0);

		/// возвращает текущее (последнее)  состояние индикации каналов прибора в указанном индексом состоянии прибора
		/// в DevState_IDX - записывается индекс состояния устройства, для которого было получено состояние узла
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		EEG_Device_Architecture::EEG_ChannelsLedsState  GetCurrDevState_ChannelsLedsState(int *DevState_IDX = 0);

		/// возвращает текущее (последнее)  состояние технической индикации прибора в указанном индексом состоянии прибора
		/// в DevState_IDX - записывается индекс состояния устройства, для которого было получено состояние узла
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		EEG_Device_Architecture::EEG_FunctionalLedsState  GetCurrDevState_FunctionalLedsState(int *DevState_IDX = 0);

		/// возвращает текущее (последнее)  состояние АЦП прибора в указанном индексом состоянии прибора
		/// в DevState_IDX - записывается индекс состояния устройства, для которого было получено состояние узла
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		EEG_Device_Architecture::EEG_ADC_State  GetCurrDevState_ADCState(int *DevState_IDX = 0);

		/// возвращает текущее (последнее)  состояние технической индикации прибора в указанном индексом состоянии прибора
		/// в DevState_IDX - записывается индекс состояния устройства, для которого было получено состояние узла
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		EEG_Device_Architecture::EEG_Calibrator_State  GetCurrDevState_Calibrator_State(int *DevState_IDX = 0);

		/// возвращает текущее (последнее)  состояние общей части прибора в указанном индексом состоянии прибора
		/// в DevState_IDX - записывается индекс состояния устройства, для которого было получено состояние узла
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		EEG_Device_Architecture::EEG_Common_State  GetCurrDevState_Common_State(int *DevState_IDX = 0);

		/// возвращает ID подтверждения, подтвердившего  текущее (последнее)  состояние
		/// в DevState_IDX - записывается индекс состояния устройства, для которого было получено состояние узла
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		qint64  GetCurrDevState_AckId(int *DevState_IDX = 0);

		/// возвращает текущее (последнее) полное состояние прибора
		/// в DevState_IDX - записывается индекс состояния устройства, для которого было получено состояние узла
		/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
		EEG_Device_Architecture::EEG_Device_State  GetCurrDevState(int *DevState_IDX = 0);


		

		

		/// убирает из хранилища все состояния устройства кроме последнего ( все массивы непустые)
		void CleanButLast();

	};


};

#endif
