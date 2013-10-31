/*! \file 
	Файл содержит.... а хз что он содержит на данный момент))
*/ 

#include <vector>
#include <string>
#include <PagedArray.cpp>
#include <assert.h>
#include <QSemaphore>
#include <EEG_device_architecture.h>
#include <EEG_Device_States_Storage.h>


namespace MBN_EEG
{

/*****************************************************************************************************************************
*
*						РЕАЛИЗАЦИЯ
*		EEG_Device_States_Storage::EEG_Device_State_Handle
*
*****************************************************************************************************************************/

// конструктор класса EEG_Device_State_Handle, просто заполняет поля структуры
EEG_Device_States_Storage::EEG_Device_State_Handle::EEG_Device_State_Handle(int bridgeData_St_idx,
						int mainCommutator_St_idx,	int channelsLeds_St_idx, int functionalLeds_St_idx,
						int _ADC_St_idx, int _DAC_St_idx, int common_St_idx, qint64 ack_id)
{
	this->BridgeData_St_idx = bridgeData_St_idx;
	this->MainCommutator_St_idx = mainCommutator_St_idx;
	this->ChannelsLeds_St_idx = channelsLeds_St_idx;
	this->FunctionalLeds_St_idx = functionalLeds_St_idx;
	this->ADC_St_idx = _ADC_St_idx;
	this->DAC_St_idx = _DAC_St_idx;
	this->Common_St_idx = common_St_idx;
	this->created_by_ack_id = ack_id;
};


/***************************************************************************************
*
*									РЕАЛИЗАЦИЯ
*							EEG_Device_States_Storage
*
***************************************************************************************/

/// конструктор хранилища состояний устройства по-умолчанию
/// создаём первое неизвестное состояние устройства
EEG_Device_States_Storage::EEG_Device_States_Storage(): accsess_sem(1)
{
	// создаём состояния узлов
	BridgeData_Seq.push_back(EEG_Device_Architecture::EEG_CP_BridgeState(false));
	MainCommutator_Seq.push_back(EEG_Device_Architecture::EEG_ChannelsCommutatorState(false));
	ChannelsLeds_Seq.push_back(EEG_Device_Architecture::EEG_ChannelsLedsState(false));
	FunctionalLeds_Seq.push_back(EEG_Device_Architecture::EEG_FunctionalLedsState(false));
	ADC_Seq.push_back(EEG_Device_Architecture::EEG_ADC_State(false));
	DAC_Seq.push_back(EEG_Device_Architecture::EEG_Calibrator_State(false));
	Common_Seq.push_back(EEG_Device_Architecture::EEG_Common_State(false));

	// добавляем состояние устройства
	Device_States_Sequence.push_back(EEG_Device_States_Storage::EEG_Device_State_Handle(0, 0, 0, 0, 0, 0, 0, 0));
};

/// добавляет новое состояние устройства в последовательность состояний.
/// указатели - на описатели новых состояний узлов, если == NULL, то состояния соответствующих узлов не меняются
/// ack_id - id подтверждения, которое определило это состояние
void EEG_Device_States_Storage::AddNextDevState(EEG_Device_Architecture::EEG_CP_BridgeState*  newBridgeDataPtr, 
						EEG_Device_Architecture::EEG_ChannelsCommutatorState* newChannelsCommutatorStatePtr,
						EEG_Device_Architecture::EEG_ChannelsLedsState* newChannelsLedsStatePtr, 
						EEG_Device_Architecture::EEG_FunctionalLedsState* newFunctionalLedsStatePtr,
						EEG_Device_Architecture::EEG_ADC_State* newADC_StatePtr, 
						EEG_Device_Architecture::EEG_Calibrator_State* newCalibrator_StatePtr,
						EEG_Device_Architecture::EEG_Common_State* newCommon_StatePtr, 
						qint64 ack_id)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	// добавляем новые состояния узлов в соответствующие массивы
	if (newBridgeDataPtr)	BridgeData_Seq.push_back(*newBridgeDataPtr);
	if (newChannelsCommutatorStatePtr)	MainCommutator_Seq.push_back(*newChannelsCommutatorStatePtr);
	if (newChannelsLedsStatePtr)	ChannelsLeds_Seq.push_back(*newChannelsLedsStatePtr);
	if (newFunctionalLedsStatePtr)	FunctionalLeds_Seq.push_back(*newFunctionalLedsStatePtr);
	if (newADC_StatePtr)	ADC_Seq.push_back(*newADC_StatePtr);
	if (newCalibrator_StatePtr)		DAC_Seq.push_back(*newCalibrator_StatePtr);
	if (newCommon_StatePtr)		Common_Seq.push_back(*newCommon_StatePtr);

	// добавляем состояние устройства
	Device_States_Sequence.push_back(EEG_Device_State_Handle(BridgeData_Seq.size() - 1,
														MainCommutator_Seq.size() - 1,
														ChannelsLeds_Seq.size() - 1,
														FunctionalLeds_Seq.size() - 1,
														ADC_Seq.size() - 1,
														DAC_Seq.size() - 1,
														Common_Seq.size() - 1,
														ack_id)
									);

	// высвобождаем объект
	accsess_sem.release();
};

/// заменяет последнее состояние прибора, чтобы не расширять хранилище
void EEG_Device_States_Storage::ReplaceLasrDevState(EEG_Device_Architecture::EEG_CP_BridgeState*  newBridgeDataPtr, 
							EEG_Device_Architecture::EEG_ChannelsCommutatorState* newChannelsCommutatorStatePtr,
							EEG_Device_Architecture::EEG_ChannelsLedsState* newChannelsLedsStatePtr, 
							EEG_Device_Architecture::EEG_FunctionalLedsState* newFunctionalLedsStatePtr,
							EEG_Device_Architecture::EEG_ADC_State* newADC_StatePtr, 
							EEG_Device_Architecture::EEG_Calibrator_State* newCalibrator_StatePtr,
							EEG_Device_Architecture::EEG_Common_State* newCommon_StatePtr, 
							qint64 ack_id)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	// перезаписываем последние состояния узлов, если надо
	if (newBridgeDataPtr)	BridgeData_Seq[BridgeData_Seq.size() - 1] = (*newBridgeDataPtr);
	if (newChannelsCommutatorStatePtr)	MainCommutator_Seq[MainCommutator_Seq.size() - 1] = (*newChannelsCommutatorStatePtr);
	if (newChannelsLedsStatePtr)	ChannelsLeds_Seq[ChannelsLeds_Seq.size() - 1] = (*newChannelsLedsStatePtr);
	if (newFunctionalLedsStatePtr)	FunctionalLeds_Seq[FunctionalLeds_Seq.size() - 1] = (*newFunctionalLedsStatePtr);
	if (newADC_StatePtr)	ADC_Seq[ADC_Seq.size() - 1] = (*newADC_StatePtr);
	if (newCalibrator_StatePtr)		DAC_Seq[DAC_Seq.size() - 1] = (*newCalibrator_StatePtr);
	if (newCommon_StatePtr)		Common_Seq[Common_Seq.size() - 1] = (*newCommon_StatePtr);

	// добавляем состояние устройства
	Device_States_Sequence[Device_States_Sequence.size() - 1].created_by_ack_id = ack_id;

	// высвобождаем объект
	accsess_sem.release();
};

/// возвращает количество записей в последовательности состояний устройства
int EEG_Device_States_Storage::GetStateSequenceLength()
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	int result = Device_States_Sequence.size();

	// высвобождаем объект
	accsess_sem.release();

	return result;
};



/// возвращает состояние моста прибора в указанном индексом состоянии прибора
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
EEG_Device_Architecture::EEG_CP_BridgeState  EEG_Device_States_Storage::GetDevState_BridgeState(int dev_state_idx)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	EEG_Device_Architecture::EEG_CP_BridgeState bs(BridgeData_Seq[Device_States_Sequence[dev_state_idx].BridgeData_St_idx]);
			
	// высвобождаем объект
	accsess_sem.release();

	return bs;
};

/// возвращает состояние коммутатора прибора в указанном индексом состоянии прибора
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
EEG_Device_Architecture::EEG_ChannelsCommutatorState  EEG_Device_States_Storage::GetDevState_ChannelsCommutatorState(int dev_state_idx)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();


    EEG_Device_Architecture::EEG_ChannelsCommutatorState cs(MainCommutator_Seq[Device_States_Sequence[dev_state_idx].MainCommutator_St_idx]);
			
	// высвобождаем объект
    accsess_sem.release();

    return cs;
};

/// возвращает состояние индикации каналов прибора в указанном индексом состоянии прибора
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
EEG_Device_Architecture::EEG_ChannelsLedsState  EEG_Device_States_Storage::GetDevState_ChannelsLedsState(int dev_state_idx)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

    EEG_Device_Architecture::EEG_ChannelsLedsState cls(ChannelsLeds_Seq[Device_States_Sequence[dev_state_idx].ChannelsLeds_St_idx]);
			
	// высвобождаем объект
	accsess_sem.release();

    return cls;
};

/// возвращает состояние технической индикации прибора в указанном индексом состоянии прибора
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
EEG_Device_Architecture::EEG_FunctionalLedsState  EEG_Device_States_Storage::GetDevState_FunctionalLedsState(int dev_state_idx)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	EEG_Device_Architecture::EEG_FunctionalLedsState fls(FunctionalLeds_Seq[Device_States_Sequence[dev_state_idx].FunctionalLeds_St_idx]);
			
	// высвобождаем объект
	accsess_sem.release();

	return fls;
};

/// возвращает состояние АЦП прибора в указанном индексом состоянии прибора
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
EEG_Device_Architecture::EEG_ADC_State  EEG_Device_States_Storage::GetDevState_ADCState(int dev_state_idx)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	EEG_Device_Architecture::EEG_ADC_State ADCs(ADC_Seq[Device_States_Sequence[dev_state_idx].ADC_St_idx]);
			
	// высвобождаем объект
	accsess_sem.release();

	return ADCs;
};

/// возвращает состояние технической индикации прибора в указанном индексом состоянии прибора
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
EEG_Device_Architecture::EEG_Calibrator_State  EEG_Device_States_Storage::GetDevState_Calibrator_State(int dev_state_idx)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	EEG_Device_Architecture::EEG_Calibrator_State CALs(DAC_Seq[Device_States_Sequence[dev_state_idx].DAC_St_idx]);
			
	// высвобождаем объект
	accsess_sem.release();

	return CALs;
};

/// возвращает состояние общей части прибора в указанном индексом состоянии прибора
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
EEG_Device_Architecture::EEG_Common_State  EEG_Device_States_Storage::GetDevState_Common_State(int dev_state_idx)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	EEG_Device_Architecture::EEG_Common_State cs(Common_Seq[Device_States_Sequence[dev_state_idx].Common_St_idx]);
			
	// высвобождаем объект
	accsess_sem.release();

	return cs;
};

/// возвращает ID подтверждения, подтвердившего данное состояние
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
qint64  EEG_Device_States_Storage::GetDevState_AckId(int dev_state_idx)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	qint64 result = Device_States_Sequence[dev_state_idx].created_by_ack_id;
			
	// высвобождаем объект
	accsess_sem.release();

	return result;
};

/// возвращает полное состояние прибора
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
EEG_Device_Architecture::EEG_Device_State  EEG_Device_States_Storage::GetDevState(int dev_state_idx)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	EEG_Device_Architecture::EEG_Device_State ds;

	ds.ADC_State = ADC_Seq[Device_States_Sequence[dev_state_idx].ADC_St_idx];
	ds.BridgeData_State = BridgeData_Seq[Device_States_Sequence[dev_state_idx].BridgeData_St_idx];
	ds.ChannelsLeds_State = ChannelsLeds_Seq[Device_States_Sequence[dev_state_idx].ChannelsLeds_St_idx];
	ds.Common_State = Common_Seq[Device_States_Sequence[dev_state_idx].Common_St_idx];
	ds.DAC_State = DAC_Seq[Device_States_Sequence[dev_state_idx].DAC_St_idx];
	ds.FunctionalLeds_State = FunctionalLeds_Seq[Device_States_Sequence[dev_state_idx].FunctionalLeds_St_idx];
	ds.MainCommutator_State = MainCommutator_Seq[Device_States_Sequence[dev_state_idx].MainCommutator_St_idx];
			
	// высвобождаем объект
	accsess_sem.release();

	return ds;
};




/// возвращает состояние моста прибора в указанном индексом состоянии прибора
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
EEG_Device_Architecture::EEG_CP_BridgeState  EEG_Device_States_Storage::GetCurrDevState_BridgeState(int *DevState_IDX)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	if (DevState_IDX) (*DevState_IDX) = Device_States_Sequence.size() - 1;

	EEG_Device_Architecture::EEG_CP_BridgeState bs(BridgeData_Seq[Device_States_Sequence[Device_States_Sequence.size() - 1].BridgeData_St_idx]);
			
	// высвобождаем объект
	accsess_sem.release();

	return bs;
};

/// возвращает состояние коммутатора прибора в указанном индексом состоянии прибора
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
EEG_Device_Architecture::EEG_ChannelsCommutatorState  EEG_Device_States_Storage::GetCurrDevState_ChannelsCommutatorState(int *DevState_IDX)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	if (DevState_IDX) (*DevState_IDX) = Device_States_Sequence.size() - 1;

    EEG_Device_Architecture::EEG_ChannelsCommutatorState cs(MainCommutator_Seq[Device_States_Sequence[Device_States_Sequence.size() - 1].MainCommutator_St_idx]);
			
	// высвобождаем объект
	accsess_sem.release();

    return cs;
};

/// возвращает состояние индикации каналов прибора в указанном индексом состоянии прибора
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
EEG_Device_Architecture::EEG_ChannelsLedsState  EEG_Device_States_Storage::GetCurrDevState_ChannelsLedsState(int *DevState_IDX)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	if (DevState_IDX) (*DevState_IDX) = Device_States_Sequence.size() - 1;

    EEG_Device_Architecture::EEG_ChannelsLedsState cls(ChannelsLeds_Seq[Device_States_Sequence[Device_States_Sequence.size() - 1].ChannelsLeds_St_idx]);
			
	// высвобождаем объект
	accsess_sem.release();

    return cls;
};

/// возвращает состояние технической индикации прибора в указанном индексом состоянии прибора
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
EEG_Device_Architecture::EEG_FunctionalLedsState  EEG_Device_States_Storage::GetCurrDevState_FunctionalLedsState(int *DevState_IDX)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	if (DevState_IDX) (*DevState_IDX) = Device_States_Sequence.size() - 1;

	EEG_Device_Architecture::EEG_FunctionalLedsState fls(FunctionalLeds_Seq[Device_States_Sequence[Device_States_Sequence.size() - 1].FunctionalLeds_St_idx]);
			
	// высвобождаем объект
	accsess_sem.release();

	return fls;
};

/// возвращает состояние АЦП прибора в указанном индексом состоянии прибора
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
EEG_Device_Architecture::EEG_ADC_State  EEG_Device_States_Storage::GetCurrDevState_ADCState(int *DevState_IDX)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	if (DevState_IDX) (*DevState_IDX) = Device_States_Sequence.size() - 1;

	EEG_Device_Architecture::EEG_ADC_State ADCs(ADC_Seq[Device_States_Sequence[Device_States_Sequence.size() - 1].ADC_St_idx]);
			
	// высвобождаем объект
	accsess_sem.release();

	return ADCs;
};

/// возвращает состояние технической индикации прибора в указанном индексом состоянии прибора
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
EEG_Device_Architecture::EEG_Calibrator_State  EEG_Device_States_Storage::GetCurrDevState_Calibrator_State(int *DevState_IDX)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	if (DevState_IDX) (*DevState_IDX) = Device_States_Sequence.size() - 1;

	EEG_Device_Architecture::EEG_Calibrator_State CALs(DAC_Seq[Device_States_Sequence[Device_States_Sequence.size() - 1].DAC_St_idx]);
			
	// высвобождаем объект
	accsess_sem.release();

	return CALs;
};

/// возвращает состояние общей части прибора в указанном индексом состоянии прибора
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
EEG_Device_Architecture::EEG_Common_State  EEG_Device_States_Storage::GetCurrDevState_Common_State(int *DevState_IDX)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	if (DevState_IDX) (*DevState_IDX) = Device_States_Sequence.size() - 1;

	EEG_Device_Architecture::EEG_Common_State cs(Common_Seq[Device_States_Sequence[Device_States_Sequence.size() - 1].Common_St_idx]);
			
	// высвобождаем объект
	accsess_sem.release();

	return cs;
};

/// возвращает ID подтверждения, подтвердившего данное состояние
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
qint64  EEG_Device_States_Storage::GetCurrDevState_AckId(int *DevState_IDX)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	if (DevState_IDX) (*DevState_IDX) = Device_States_Sequence.size() - 1;

	qint64 result = Device_States_Sequence[Device_States_Sequence.size() - 1].created_by_ack_id;
			
	// высвобождаем объект
	accsess_sem.release();

	return result;
};

/// возвращает полное состояние прибора
/// dev_state_idx - индекс состояния прибора в массиве состояний прибора [0; Device_States_Sequence.size() - 1]
EEG_Device_Architecture::EEG_Device_State  EEG_Device_States_Storage::GetCurrDevState(int *DevState_IDX)
{
	// получаем монопольный доступ к объекту
	accsess_sem.acquire();

	if (DevState_IDX) (*DevState_IDX) = Device_States_Sequence.size() - 1;

	EEG_Device_Architecture::EEG_Device_State ds;

	ds.ADC_State = ADC_Seq[Device_States_Sequence[Device_States_Sequence.size() - 1].ADC_St_idx];
	ds.BridgeData_State = BridgeData_Seq[Device_States_Sequence[Device_States_Sequence.size() - 1].BridgeData_St_idx];
	ds.ChannelsLeds_State = ChannelsLeds_Seq[Device_States_Sequence[Device_States_Sequence.size() - 1].ChannelsLeds_St_idx];
	ds.Common_State = Common_Seq[Device_States_Sequence[Device_States_Sequence.size() - 1].Common_St_idx];
	ds.DAC_State = DAC_Seq[Device_States_Sequence[Device_States_Sequence.size() - 1].DAC_St_idx];
	ds.FunctionalLeds_State = FunctionalLeds_Seq[Device_States_Sequence[Device_States_Sequence.size() - 1].FunctionalLeds_St_idx];
	ds.MainCommutator_State = MainCommutator_Seq[Device_States_Sequence[Device_States_Sequence.size() - 1].MainCommutator_St_idx];
			
	// высвобождаем объект
	accsess_sem.release();

	return ds;
};




/// убирает из хранилища все состояния устройства кроме последнего ( все массивы непустые)
void EEG_Device_States_Storage::CleanButLast()
{
	// для массивов состояний узлов
		// ставим на первое место данные из последних ячеек
	BridgeData_Seq[0] = BridgeData_Seq[BridgeData_Seq.size() - 1];
	MainCommutator_Seq[0] = MainCommutator_Seq[MainCommutator_Seq.size() - 1];
	ChannelsLeds_Seq[0] = ChannelsLeds_Seq[ChannelsLeds_Seq.size() - 1];
	FunctionalLeds_Seq[0] = FunctionalLeds_Seq[FunctionalLeds_Seq.size() - 1];
	ADC_Seq[0] = ADC_Seq[ADC_Seq.size() - 1];
	DAC_Seq[0] = DAC_Seq[DAC_Seq.size() - 1];
	Common_Seq[0] = Common_Seq[Common_Seq.size() - 1];

		// ресайзим массивы
	BridgeData_Seq.resize(1);
	MainCommutator_Seq.resize(1);
	ChannelsLeds_Seq.resize(1);
	FunctionalLeds_Seq.resize(1);
	ADC_Seq.resize(1);
	DAC_Seq.resize(1);
	Common_Seq.resize(1);

	// массив состояний
	Device_States_Sequence[0].created_by_ack_id = Device_States_Sequence[Device_States_Sequence.size() - 1].created_by_ack_id;
	Device_States_Sequence[0].ADC_St_idx = 0;
	Device_States_Sequence[0].BridgeData_St_idx = 0;
	Device_States_Sequence[0].ChannelsLeds_St_idx = 0;
	Device_States_Sequence[0].Common_St_idx = 0;
	Device_States_Sequence[0].DAC_St_idx = 0;
	Device_States_Sequence[0].FunctionalLeds_St_idx = 0;
	Device_States_Sequence[0].MainCommutator_St_idx = 0;
	Device_States_Sequence.resize(1);
};




};
