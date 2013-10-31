/*! \file 
	Файл содержит.... а хз что он содержит на данный момент))
*/ 

#include <EEG_device_architecture.h>
#include <iostream>
#include <stdio.h>
#include <vector>

namespace MBN_EEG
{
//---------------------------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------КЛАССЫ АРХИТЕКТУРЫ---------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------------------------------------------//


/**********************************************************************************
*									РЕАЛИЗАЦИЯ
*
*					EEG_Device_Architecture::EEG_CP_BridgeArc						
*
***********************************************************************************/

const std::string MBN_EEG::EEG_Device_Architecture::EEG_CP_BridgeArc::DEF_SerialNumber = "";
const std::string MBN_EEG::EEG_Device_Architecture::EEG_CP_BridgeArc::DEF_ProductDescriptionString = "";

/**********************************************************************************
*									РЕАЛИЗАЦИЯ
*
*					EEG_Device_Architecture::EEG_ChannelsLedsArc						
*
***********************************************************************************/

std::string EEG_Device_Architecture::EEG_ChannelsLedsArc::print_to_str(EEG_Led_State led_st)
{
	switch (led_st)
	{
	case EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_OFF: return "Off\n";
	case EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_GREEN: return "Green\n";
	case EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_ORANGE: return "Orange\n";
	case EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_RED: return "Red\n";
	};
};


void EEG_Device_Architecture::EEG_ChannelsLedsArc::print(EEG_Led_State led_st)
{
	switch (led_st)
	{
	case EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_OFF: std::cout<<"Off"; break;
	case EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_GREEN: std::cout<<"Green"; break;
	case EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_ORANGE: std::cout<<"Orange"; break;
	case EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_RED: std::cout<<"Red"; break;
	};

	std::cout<<std::endl;
};

/**********************************************************************************
*									РЕАЛИЗАЦИЯ
*
*					EEG_Device_Architecture::EEG_ADC_Arc						
*
***********************************************************************************/

const unsigned short EEG_Device_Architecture::EEG_ADC_Arc::DEF_FrequencyTable[EEG_Device_Architecture::EEG_ADC_Arc::DEF_FrequencyTable_Length]  = {250, 500, 1000, 2000};

// распечатка в строку
std::string EEG_Device_Architecture::EEG_ADC_Arc::print_to_str(EEG_ADS_Mux_Mode mux_mode)
{
	switch (mux_mode)
	{
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__NORMAL: return "Normal\n";
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__INTERNAL_TEST_SOURCE: return "Internal Test Source\n";
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__MVDD: return "MVdd\n"; 
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__REF_and_RLD_IN: return "Ref And RLD IN\n"; 
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__RLD_DRN: return "RLD DRN\n"; 
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__RLD_DRP: return "RLD DRP\n";
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__SHORT_CIRCUT: return "Short circuit\n";
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__TEMPERATURE: return "Temperature\n"; 
	};
};

// распечатка состояния в строку
std::string EEG_Device_Architecture::EEG_ADC_Arc::print_to_str(EEG_ADS_Amplification ampl_mode)
{
	switch (ampl_mode)
	{
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_1x: return "1x\n";
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_2x: return "2x\n";
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_3x: return "3x\n";
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_4x: return "4x\n";
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_6x: return "6x\n";
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_8x: return "8x\n";
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_12x: return "12x\n";
	};
};

// распечатка состояния
void EEG_Device_Architecture::EEG_ADC_Arc::print(EEG_ADS_Mux_Mode mux_mode)
{
	switch (mux_mode)
	{
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__NORMAL: std::cout<<"Normal";  break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__INTERNAL_TEST_SOURCE: std::cout<<"Internal Test Source";  break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__MVDD: std::cout<<"MVdd";  break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__REF_and_RLD_IN: std::cout<<"Ref And RLD IN";  break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__RLD_DRN: std::cout<<"RLD DRN";  break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__RLD_DRP: std::cout<<"RLD DRP";  break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__SHORT_CIRCUT: std::cout<<"Short circuit";  break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_CH_ADS_MUX_MODE__TEMPERATURE: std::cout<<"Temperature";  break;
	};

	std::cout<<std::endl;
};

// распечатка состояния
void EEG_Device_Architecture::EEG_ADC_Arc::print(EEG_ADS_Amplification ampl_mode)
{
	switch (ampl_mode)
	{
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_1x: std::cout<<"1x"; break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_2x: std::cout<<"2x"; break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_3x: std::cout<<"3x"; break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_4x: std::cout<<"4x"; break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_6x: std::cout<<"6x"; break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_8x: std::cout<<"8x"; break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_12x: std::cout<<"12x"; break;
	};

	std::cout<<std::endl;
};

/**********************************************************************************
*									РЕАЛИЗАЦИЯ
*
*					EEG_Device_Architecture::EEG_CalibratorArc						
*
***********************************************************************************/

const float EEG_Device_Architecture::EEG_CalibratorArc::DEF_Frequency = 10.0;


const float MBN_EEG::EEG_Device_Architecture::EEG_CalibratorArc::EEG_CALIBRATION_SIGNAL_FREQUENCY_STEP_HZ = 0.1;

std::string EEG_Device_Architecture::EEG_CalibratorArc::print_to_str(EEG_Calibrator_Shape cal_shape)
{
	switch (cal_shape)
	{
	case EEG_Device_Architecture::EEG_CalibratorArc::EEG_CAL_SHAPE_SINE: return "Sine\n";
	case EEG_Device_Architecture::EEG_CalibratorArc::EEG_CAL_SHAPE_RECTANGLE: return "Rectangle\n";
	case EEG_Device_Architecture::EEG_CalibratorArc::EEG_CAL_SHAPE_TRIANGLE: return "Triangle\n";
	};
};

void EEG_Device_Architecture::EEG_CalibratorArc::print(EEG_Calibrator_Shape cal_shape)
{
	switch (cal_shape)
	{
	case EEG_Device_Architecture::EEG_CalibratorArc::EEG_CAL_SHAPE_SINE: std::cout<<"Sine"; break;
	case EEG_Device_Architecture::EEG_CalibratorArc::EEG_CAL_SHAPE_RECTANGLE: std::cout<<"Rectangle"; break;
	case EEG_Device_Architecture::EEG_CalibratorArc::EEG_CAL_SHAPE_TRIANGLE: std::cout<<"Triangle"; break;
	};

	std::cout<<std::endl;
};


/**********************************************************************************
*									РЕАЛИЗАЦИЯ
*
*					EEG_Device_Architecture::EEG_CommonArc						
*
***********************************************************************************/

void EEG_Device_Architecture::EEG_CommonArc::print(EEG_CHANNEL channel)
{
	switch (channel)
	{
	case EEG_Device_Architecture::EEG_CommonArc::EEG_CHANNEL_A1: std::cout<<"A1"; break;
	case EEG_Device_Architecture::EEG_CommonArc::EEG_CHANNEL_A2: std::cout<<"A2"; break;
	case EEG_Device_Architecture::EEG_CommonArc::EEG_CHANNEL_C3: std::cout<<"C3"; break;
	case EEG_Device_Architecture::EEG_CommonArc::EEG_CHANNEL_C4: std::cout<<"C4"; break;
	case EEG_Device_Architecture::EEG_CommonArc::EEG_CHANNEL_Fp1: std::cout<<"Fp1"; break;
	case EEG_Device_Architecture::EEG_CommonArc::EEG_CHANNEL_Fp2: std::cout<<"Fp2"; break;
	};

	std::cout<<std::endl;
};

//---------------------------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------КЛАССЫ СОСТОЯНИЯ-----------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------------------------------------------//


/**********************************************************************************
*									РЕАЛИЗАЦИЯ
*
*					EEG_Device_Architecture::EEG_CP_BridgeState						
*
***********************************************************************************/

const unsigned short EEG_Device_Architecture::EEG_CP_BridgeArc::DEF_VID = 1000;
const unsigned short EEG_Device_Architecture::EEG_CP_BridgeArc::DEF_PID = 1000;
const unsigned char EEG_Device_Architecture::EEG_CP_BridgeArc::DEF_PowerDescriptorAttributes = 0;
const unsigned char EEG_Device_Architecture::EEG_CP_BridgeArc::DEF_PowerDescriptorMaxPower = 0;
const unsigned short EEG_Device_Architecture::EEG_CP_BridgeArc::DEF_ReleaseNumber = 0;

EEG_Device_Architecture::EEG_CP_BridgeState::EEG_CP_BridgeState(bool reset_state)
{
	if (reset_state)
	{
		// записать в узел значения по-умолчанию
        this->PID = EEG_CP_BridgeArc::DEF_PID;
		this->VID = EEG_CP_BridgeArc::DEF_VID;
		this->PowerDescriptorAttributes = EEG_CP_BridgeArc::DEF_PowerDescriptorAttributes;
		this->PowerDescriptorMaxPower = EEG_CP_BridgeArc::DEF_PowerDescriptorMaxPower;
		this->ProductDescriptionString = EEG_CP_BridgeArc::DEF_ProductDescriptionString;
		this->ReleaseNumber = EEG_CP_BridgeArc::DEF_ReleaseNumber;
		this->SerialNumber = EEG_CP_BridgeArc::DEF_SerialNumber;
	};
};

// распечатка состояния в строку
std::string EEG_Device_Architecture::EEG_CP_BridgeState::print_to_str()
{
	// временный буффер для sprintf
	const int BUFF_SIZE = 256;
	char buff[BUFF_SIZE];

	std::string out = "";

	out += "CP Bridge State:\n";

	out += "\tPID = ";
	if (this->PID.Defined()) 
	{
        printf(buff, BUFF_SIZE, "%hu\n", this->PID.Get());
		out += buff;
	}
	else out += "<UNKNOWN>\n";

	out += "\tVID = ";
    if (this->VID.Defined()) printf(buff, BUFF_SIZE, "%hu\n", this->VID.Get());
	else out += "<UNKNOWN>\n";

	out += "\tPowerDA = ";
	if (this->PowerDescriptorAttributes.Defined()) 
	{
        printf(buff, BUFF_SIZE, "%hhX\n", this->PowerDescriptorAttributes.Get());
		out += buff;
	}
	else out += "<UNKNOWN>\n";

	out += "\tPowerMax = ";
	if (this->PowerDescriptorMaxPower.Defined()) 
	{
        printf(buff, BUFF_SIZE, "%hhX\n", this->PowerDescriptorMaxPower.Get());
		out += buff;
	}
	else out += "<UNKNOWN>\n";

	out += "\tDescStr = ";
	if (this->ProductDescriptionString.Defined()) out += this->ProductDescriptionString.Get();
	else out += "<UNKNOWN>\n";

	out += "\tReleaseNum = ";
	if (this->ReleaseNumber.Defined()) 
	{
        printf(buff, BUFF_SIZE, "%hX\n", this->ReleaseNumber.Get());
		out += buff;
	}
	else out += "<UNKNOWN>\n";

	out += "\tSN = ";
	if (this->SerialNumber.Defined()) out += this->SerialNumber.Get();
	else out += "<UNKNOWN>\n";

	return out;
};

#ifdef _DEBUG
void EEG_Device_Architecture::EEG_CP_BridgeState::print()
{
	std::cout<<"\t\tCP Bridge State:"<<std::endl;

	std::cout<<"\t\t\tPID = ";
	if (this->PID.Defined()) printf("%hu", this->PID.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tVID = ";
	if (this->VID.Defined()) printf("%hu", this->VID.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tPowerDA = ";
	if (this->PowerDescriptorAttributes.Defined()) printf("%hhX", this->PowerDescriptorAttributes.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tPowerMax = ";
	if (this->PowerDescriptorMaxPower.Defined()) printf("%hhX", this->PowerDescriptorMaxPower.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tDescStr = ";
	if (this->ProductDescriptionString.Defined()) std::cout<<this->ProductDescriptionString.Get();
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tReleaseNum = ";
	if (this->ReleaseNumber.Defined())printf("%hX", this->ReleaseNumber.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tSN = ";
	if (this->SerialNumber.Defined()) std::cout<<this->SerialNumber.Get();
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;
};
#endif

/**********************************************************************************
*									РЕАЛИЗАЦИЯ
*
*					EEG_Device_Architecture::EEG_ChannelsCommutatorState						
*
***********************************************************************************/
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_NORMAL = 0x08;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_NORMAL = 0x09;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_NORMAL = 0x08;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_NORMAL = 0x09;

const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_DEF_U34_State = EEG_U34_State_NORMAL;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_DEF_U6_State =  EEG_U6_State_NORMAL;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_DEF_U24_State = EEG_U24_State_NORMAL;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_DEF_U2_State =  EEG_U2_State_NORMAL;

EEG_Device_Architecture::EEG_ChannelsCommutatorState::EEG_ChannelsCommutatorState(bool reset_state)
{
	if (reset_state)
	{
		// записать в узел значения по-умолчанию
        this->U34_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_DEF_U34_State;
		this->U6_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_DEF_U6_State;
		this->U24_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_DEF_U24_State;
		this->U2_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_DEF_U2_State;
	};
};

// печать себя в строку
std::string EEG_Device_Architecture::EEG_ChannelsCommutatorState::print_to_str()
{
	// временный буффер для sprintf
	const int BUFF_SIZE = 256;
	char buff[BUFF_SIZE];

	std::string out = "";

	out += "Channels Commutator State:\n";

	out += "\tU2_State: ";
	if (this->U2_State.Defined())
	{
        printf(buff, BUFF_SIZE, "%hhX\n", this->U2_State.Get());
		out += buff;
	}
	else out += "<UNKNOWN>\n";

	out += "\tU6_State: ";
	if (this->U6_State.Defined())
	{
        printf(buff, BUFF_SIZE, "%hhX\n", this->U6_State.Get());
		out += buff;
	}
	else out += "<UNKNOWN>\n";

	out += "\tU24_State: ";
	if (this->U24_State.Defined())
	{
        printf(buff, BUFF_SIZE, "%hhX\n", this->U24_State.Get());
		out += buff;
	}
	else out += "<UNKNOWN>\n";

	out += "\tU34_State: ";
	if (this->U34_State.Defined())
	{
        printf(buff, BUFF_SIZE, "%hhX\n", this->U34_State.Get());
		out += buff;
	}
	else out += "<UNKNOWN>\n";

	return out;
};

#ifdef _DEBUG
void EEG_Device_Architecture::EEG_ChannelsCommutatorState::print()
{
	std::cout<<"\t\tChannels Commutator State:"<<std::endl;

	std::cout<<"\t\t\tU2_State: ";
	if (this->U2_State.Defined())printf("%hhX", this->U2_State.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tU6_State: ";
	if (this->U6_State.Defined())printf("%hhX", this->U6_State.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tU24_State: ";
	if (this->U24_State.Defined())printf("%hhX", this->U24_State.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tU34_State: ";
	if (this->U34_State.Defined())printf("%hhX", this->U34_State.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;
};
#endif


	/*******************************					
	*		СТАНДАРТНЫЕ РЕЖИМЫ
	********************************/
	
/// Установка режима обычного измерения
void EEG_Device_Architecture::EEG_ChannelsCommutatorState::SetNormal()
{
	U34_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_NORMAL;
	U6_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_NORMAL;
	U24_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_NORMAL;
	U2_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_NORMAL;
};

const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_CALIBRATION = 0x10;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_CALIBRATION = 0x60;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_CALIBRATION = 0x10;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_CALIBRATION = 0x60;

/// Установка режима калибровки
void EEG_Device_Architecture::EEG_ChannelsCommutatorState::SetCalibration()
{
	U34_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_CALIBRATION;
	U6_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_CALIBRATION;
	U24_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_CALIBRATION;
	U2_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_CALIBRATION;
};
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_A1 = 0x10;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_A1 = 0x90;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_A1 = 0x0C;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_A1 = 0x90;

const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_A2 = 0x90;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_A2 = 0x0C;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_A2 = 0x10;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_A2 = 0x90;

const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_C3 = 0x10;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_C3 = 0x90;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_C3 = 0x10;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_C3 = 0x13;

const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_C4 = 0x10;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_C4 = 0x13;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_C4 = 0x10;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_C4 = 0x90;

const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_Fp1 = 0x10;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_Fp1 = 0x90;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_Fp1 = 0x10;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_Fp1 = 0x8C;

const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_Fp2 = 0x10;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_Fp2 = 0x8C;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_Fp2 = 0x10;
const EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_OneChipState   EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_Fp2 = 0x90;
/// Установка режима измерения импеданса МБН по указанному каналу
void EEG_Device_Architecture::EEG_ChannelsCommutatorState::SetImpedanceMBN(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Channel channel)
{
	// полагаем, что ничего другого мы устанавливать не будем
	assert( (channel == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_A1) ||
			(channel == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_A2) ||
			(channel == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_C3) ||
			(channel == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_C4) ||
			(channel == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_Fp1) ||
			(channel == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_Fp2));

	// устанавливаем
	switch (channel)
	{
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_A1:
		U34_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_A1;
		U6_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_A1;
		U24_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_A1;
		U2_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_A1;
		break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_A2:
		U34_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_A2;
		U6_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_A2;
		U24_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_A2;
		U2_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_A2;
		break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_C3:
		U34_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_C3;
		U6_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_C3;
		U24_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_C3;
		U2_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_C3;
		break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_C4:
		U34_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_C4;
		U6_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_C4;
		U24_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_C4;
		U2_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_C4;
		break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_Fp1:
		U34_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_Fp1;
		U6_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_Fp1;
		U24_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_Fp1;
		U2_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_Fp1;
		break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_Fp2:
		U34_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_Fp2;
		U6_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_Fp2;
		U24_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_Fp2;
		U2_State = EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_Fp2;
		break;
	}
};

/// Является ли установленный режим нормальным
bool EEG_Device_Architecture::EEG_ChannelsCommutatorState::IsNormal()
{
	return  (U34_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_NORMAL) &&
			(U6_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_NORMAL) &&
			(U24_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_NORMAL) &&
			(U2_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_NORMAL);
};

/// Является ли установленный режим калибровочным
bool EEG_Device_Architecture::EEG_ChannelsCommutatorState::IsCalibration()
{
	return  (U34_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_CALIBRATION) &&
			(U6_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_CALIBRATION) &&
			(U24_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_CALIBRATION) &&
			(U2_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_CALIBRATION);
};

/// Является ли установленный режим измерением импеданса МБН
bool EEG_Device_Architecture::EEG_ChannelsCommutatorState::IsImpedanceMBN(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Channel channel)
{
	// полагаем, что ничего другого мы проверять не будем
	assert( (channel == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_A1) ||
			(channel == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_A2) ||
			(channel == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_C3) ||
			(channel == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_C4) ||
			(channel == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_Fp1) ||
			(channel == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_Fp2));

	// проверяем
	switch (channel)
	{
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_A1:
		return  (U34_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_A1) &&
				(U6_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_A1) &&
				(U24_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_A1) &&
				(U2_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_A1);
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_A2:
		return  (U34_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_A2) &&
				(U6_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_A2) &&
				(U24_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_A2) &&
				(U2_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_A2);
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_C3:
		return  (U34_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_C3) &&
				(U6_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_C3) &&
				(U24_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_C3) &&
				(U2_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_C3);
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_C4:
		return  (U34_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_C4) &&
				(U6_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_C4) &&
				(U24_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_C4) &&
				(U2_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_C4);
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_Fp1:
		return  (U34_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_Fp1) &&
				(U6_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_Fp1) &&
				(U24_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_Fp1) &&
				(U2_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_Fp1);
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_Ch_Fp2:
		return  (U34_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U34_State_IMPEDANCE_MBN_Fp2) &&
				(U6_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U6_State_IMPEDANCE_MBN_Fp2) &&
				(U24_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U24_State_IMPEDANCE_MBN_Fp2) &&
				(U2_State == EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_U2_State_IMPEDANCE_MBN_Fp2);
	default: 
			assert(false);
			return false;
	}

				
};

	/*****************************************
	*		ПРОИЗВОЛЬНЫЕ  РЕЖИМЫ
	*****************************************/

/// Разъединить всё пары пинов на всех чипах
void EEG_Device_Architecture::EEG_ChannelsCommutatorState::SetDisconnectAll()
{
	U34_State = 0;
	U6_State = 0;
	U24_State = 0;
	U2_State = 0;
};

/// установить соединёнными указанную пару пинов на указанном коммутаторе
void EEG_Device_Architecture::EEG_ChannelsCommutatorState::SetConnect(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_Chip chip, EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_PinPair pin_pair)
{
    def_wrap<unsigned char> *ptr;

	switch (chip)
	{
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U2: ptr = & U2_State;	break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U6: ptr = & U6_State;	break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U24: ptr = & U24_State;	break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U34: ptr = & U34_State;	break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_ALL:
		SetConnect(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U2, pin_pair);
		SetConnect(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U6, pin_pair);
		SetConnect(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U24, pin_pair);
		SetConnect(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U34, pin_pair);
		return;
	};

    unsigned char set = pin_pair;

	(*ptr).Set((*ptr).Get() | set);
};

/// установить разъединить указанную пару пинов на указанном коммутаторе
void EEG_Device_Architecture::EEG_ChannelsCommutatorState::SetDisconnect(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_Chip chip, EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_PinPair pin_pair)
{
    def_wrap<unsigned char> *ptr;

	switch (chip)
	{
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U2: ptr = & U2_State;	break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U6: ptr = & U6_State;	break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U24: ptr = & U24_State;	break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U34: ptr = & U34_State;	break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_ALL:
		SetDisconnect(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U2, pin_pair);
		SetDisconnect(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U6, pin_pair);
		SetDisconnect(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U24, pin_pair);
		SetDisconnect(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U34, pin_pair);
		return;
	};

    unsigned char set = pin_pair;

	set = ~set;

	(*ptr).Set((*ptr).Get() & set);
};

/// возвращает, соедининена ли выбранная пара пинов. Если выбран параметр ALL, то true вернёт только в случае 
/// если все попадающие под условие выборки пары пинов соединены
bool EEG_Device_Architecture::EEG_ChannelsCommutatorState::IsConnected(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_Chip chip, EEG_Device_Architecture::EEG_ChannelsCommutatorArc::EEG_MainComm_PinPair pin_pair)
{
    def_wrap<unsigned char> *ptr;

	switch (chip)
	{
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U2: ptr = & U2_State;	break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U6: ptr = & U6_State;	break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U24: ptr = & U24_State;	break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U34: ptr = & U34_State;	break;
	case EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_ALL:
		return  IsConnected(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U2, pin_pair) &&
				IsConnected(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U6, pin_pair) &&
				IsConnected(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U24, pin_pair) &&
				IsConnected(EEG_Device_Architecture::EEG_ChannelsCommutatorArc::chip_U34, pin_pair);
	};

	bool result = true;

    unsigned char mask = pin_pair;

	if ((mask & (*ptr).Get()) != mask) return false;
	else return true;
};



/**********************************************************************************
*									РЕАЛИЗАЦИЯ
*
*					EEG_Device_Architecture::EEG_ChannelsLedsState						
*
***********************************************************************************/

const EEG_Device_Architecture::EEG_ChannelsLedsArc::EEG_Led_State EEG_Device_Architecture::EEG_ChannelsLedsArc::DEF_connector_Fp2_state = EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_OFF;
const EEG_Device_Architecture::EEG_ChannelsLedsArc::EEG_Led_State EEG_Device_Architecture::EEG_ChannelsLedsArc::DEF_connector_C4_state = EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_OFF;
const EEG_Device_Architecture::EEG_ChannelsLedsArc::EEG_Led_State EEG_Device_Architecture::EEG_ChannelsLedsArc::DEF_connector_Fp1_state = EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_OFF;
const EEG_Device_Architecture::EEG_ChannelsLedsArc::EEG_Led_State EEG_Device_Architecture::EEG_ChannelsLedsArc::DEF_connector_C3_state = EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_OFF;
const EEG_Device_Architecture::EEG_ChannelsLedsArc::EEG_Led_State EEG_Device_Architecture::EEG_ChannelsLedsArc::DEF_connector_A1_state = EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_OFF;
const EEG_Device_Architecture::EEG_ChannelsLedsArc::EEG_Led_State EEG_Device_Architecture::EEG_ChannelsLedsArc::DEF_connector_A2_state = EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_OFF;
const EEG_Device_Architecture::EEG_ChannelsLedsArc::EEG_Led_State EEG_Device_Architecture::EEG_ChannelsLedsArc::DEF_connector_GND_state = EEG_Device_Architecture::EEG_ChannelsLedsArc::LED_OFF;

EEG_Device_Architecture::EEG_ChannelsLedsState::EEG_ChannelsLedsState(bool reset_state)
{
	if (reset_state)
	{
		// записать в узел значения по-умолчанию
		this->Fp2_state = EEG_ChannelsLedsArc::DEF_connector_Fp2_state;
		this->C4_state = EEG_ChannelsLedsArc::DEF_connector_C4_state;
		this->Fp1_state = EEG_ChannelsLedsArc::DEF_connector_Fp1_state;
		this->C3_state = EEG_ChannelsLedsArc::DEF_connector_C3_state;
		this->A1_state = EEG_ChannelsLedsArc::DEF_connector_A1_state;
		this->A2_state = EEG_ChannelsLedsArc::DEF_connector_A2_state;
		this->GND_state = EEG_ChannelsLedsArc::DEF_connector_GND_state;
	};
};

// печать себя в строку
std::string EEG_Device_Architecture::EEG_ChannelsLedsState::print_to_str()
{
	// временный буффер для sprintf
	const int BUFF_SIZE = 256;
	char buff[BUFF_SIZE];

	std::string out = "";

	out += "Channels Leds State:\n";

	out += "\tA1 state: ";
	if (this->A1_state.Defined()) out += EEG_Device_Architecture::EEG_ChannelsLedsArc::print_to_str(this->A1_state.Get());
	else out += "<UNKNOWN>\n";

	out += "\tA2 state: ";
	if (this->A2_state.Defined()) out += EEG_Device_Architecture::EEG_ChannelsLedsArc::print_to_str(this->A2_state.Get());
	else out += "<UNKNOWN>\n";

	out += "\tC3 state: ";
	if (this->C3_state.Defined()) out += EEG_Device_Architecture::EEG_ChannelsLedsArc::print_to_str(this->C3_state.Get());
	else out += "<UNKNOWN>\n";

	out += "\tC4 state: ";
	if (this->C4_state.Defined()) out += EEG_Device_Architecture::EEG_ChannelsLedsArc::print_to_str(this->C4_state.Get());
	else out += "<UNKNOWN>\n";

	out += "\tFp1 state: ";
	if (this->Fp1_state.Defined()) out += EEG_Device_Architecture::EEG_ChannelsLedsArc::print_to_str(this->Fp1_state.Get());
	else out += "<UNKNOWN>\n";

	out += "\tFp2 state: ";
	if (this->Fp2_state.Defined()) out += EEG_Device_Architecture::EEG_ChannelsLedsArc::print_to_str(this->Fp2_state.Get());
	else out += "<UNKNOWN>\n";

	out += "\tGND state: ";
	if (this->GND_state.Defined()) out += EEG_Device_Architecture::EEG_ChannelsLedsArc::print_to_str(this->GND_state.Get());
	else out += "<UNKNOWN>\n";

	return out;
};

#ifdef _DEBUG
void EEG_Device_Architecture::EEG_ChannelsLedsState::print()
{
	std::cout<<"\t\tChannels Leds State:"<<std::endl;

	std::cout<<"\t\t\tA1 state: ";
	if (this->A1_state.Defined())EEG_Device_Architecture::EEG_ChannelsLedsArc::print(this->A1_state.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tA2 state: ";
	if (this->A2_state.Defined())EEG_Device_Architecture::EEG_ChannelsLedsArc::print(this->A2_state.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tC3 state: ";
	if (this->C3_state.Defined())EEG_Device_Architecture::EEG_ChannelsLedsArc::print(this->C3_state.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tC4 state: ";
	if (this->C4_state.Defined())EEG_Device_Architecture::EEG_ChannelsLedsArc::print(this->C4_state.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tFp1 state: ";
	if (this->Fp1_state.Defined())EEG_Device_Architecture::EEG_ChannelsLedsArc::print(this->Fp1_state.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tFp2 state: ";
	if (this->Fp2_state.Defined())EEG_Device_Architecture::EEG_ChannelsLedsArc::print(this->Fp2_state.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tGND state: ";
	if (this->GND_state.Defined())EEG_Device_Architecture::EEG_ChannelsLedsArc::print(this->GND_state.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	
};
#endif

/**********************************************************************************
*									РЕАЛИЗАЦИЯ
*
*					EEG_Device_Architecture::EEG_FunctionalLedsState						
*
***********************************************************************************/

const bool EEG_Device_Architecture::EEG_FunctionalLedsArc::DEF_DS1_state = false;
const bool EEG_Device_Architecture::EEG_FunctionalLedsArc::DEF_DS2_state = false;

EEG_Device_Architecture::EEG_FunctionalLedsState::EEG_FunctionalLedsState(bool reset_state)
{
	if (reset_state)
	{
		this->DS1_on = EEG_Device_Architecture::EEG_FunctionalLedsArc::DEF_DS1_state;
		this->DS2_on = EEG_Device_Architecture::EEG_FunctionalLedsArc::DEF_DS2_state;
	};
};

// печать себя в строку
std::string EEG_Device_Architecture::EEG_FunctionalLedsState::print_to_str()
{
	// временный буффер для sprintf
	const int BUFF_SIZE = 256;
	char buff[BUFF_SIZE];

	std::string out = "";

	out += "Functional Leds State:\n";
	
	out += "\tDS1:  ";
	if (this->DS1_on.Defined())
		{
			if (DS1_on.Get()) out += "ON";
			else out += "OFF";
		}
	else out += "<UNKNOWN>\n";

	out += "\tDS2:  ";
	if (this->DS2_on.Defined())
		{
			if (DS2_on.Get()) out += "ON";
			else out += "OFF";
		}
	else out += "<UNKNOWN>\n";

	return out;
};

#ifdef _DEBUG
void EEG_Device_Architecture::EEG_FunctionalLedsState::print()
{
	std::cout<<"\t\tFunctional Leds State:"<<std::endl;
	
	std::cout<<"\t\t\tDS1:  ";
	if (this->DS1_on.Defined())
		{
			if (DS1_on.Get()) std::cout<<"ON";
			else std::cout<<"OFF";
		}
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tDS2:  ";
	if (this->DS2_on.Defined())
		{
			if (DS2_on.Get()) std::cout<<"ON";
			else std::cout<<"OFF";
		}
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;
};
#endif

/**********************************************************************************
*									РЕАЛИЗАЦИЯ
*
*					EEG_Device_Architecture::EEG_ADC_State						
*
***********************************************************************************/

const EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification EEG_Device_Architecture::EEG_ADC_Arc::DEF_Ch0_Ampl_STATE = EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_6x;
const EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification EEG_Device_Architecture::EEG_ADC_Arc::DEF_Ch1_Ampl_STATE = EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_6x;
const EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification EEG_Device_Architecture::EEG_ADC_Arc::DEF_Ch2_Ampl_STATE = EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_6x;
const EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification EEG_Device_Architecture::EEG_ADC_Arc::DEF_Ch3_Ampl_STATE = EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_6x;

const char EEG_Device_Architecture::EEG_ADC_Arc::DEF_FrequencyIndex = 1;
const bool EEG_Device_Architecture::EEG_ADC_Arc::DEF_TurnedOn_STATE = false;

EEG_Device_Architecture::EEG_ADC_State::EEG_ADC_State(bool reset_state)
{
	if (reset_state)
	{
		// записать в узел значения по-умолчанию
		this->Ch0_Ampl = EEG_Device_Architecture::EEG_ADC_Arc::DEF_Ch0_Ampl_STATE;
		this->Ch1_Ampl = EEG_Device_Architecture::EEG_ADC_Arc::DEF_Ch1_Ampl_STATE;
		this->Ch2_Ampl = EEG_Device_Architecture::EEG_ADC_Arc::DEF_Ch2_Ampl_STATE;
		this->Ch3_Ampl = EEG_Device_Architecture::EEG_ADC_Arc::DEF_Ch3_Ampl_STATE;

		//this->Ch0_MUX = EEG_Device_Architecture::EEG_ADC_Arc::DEF_Ch0_MUX_STATE;
		//this->Ch1_MUX = EEG_Device_Architecture::EEG_ADC_Arc::DEF_Ch1_MUX_STATE;
		//this->Ch2_MUX = EEG_Device_Architecture::EEG_ADC_Arc::DEF_Ch2_MUX_STATE;
		//this->Ch3_MUX = EEG_Device_Architecture::EEG_ADC_Arc::DEF_Ch3_MUX_STATE;
		// при включении неопределены
		this->Ch0_MUX.SetUndefined();
		this->Ch1_MUX.SetUndefined();
		this->Ch2_MUX.SetUndefined();
		this->Ch3_MUX.SetUndefined();

		this->CurrentFrequencyIndex = EEG_Device_Architecture::EEG_ADC_Arc::DEF_FrequencyIndex;
		this->TurnedOn = EEG_Device_Architecture::EEG_ADC_Arc::DEF_TurnedOn_STATE;
        this->FrequencyTable = std::vector<unsigned short>(EEG_Device_Architecture::EEG_ADC_Arc::DEF_FrequencyTable, EEG_Device_Architecture::EEG_ADC_Arc::DEF_FrequencyTable + EEG_Device_Architecture::EEG_ADC_Arc::DEF_FrequencyTable_Length);
	};
};

/// вывод себя в строку
std::string EEG_Device_Architecture::EEG_ADC_State::print_to_str()
{
	// временный буффер для sprintf
	const int BUFF_SIZE = 256;
	char buff[BUFF_SIZE];

	std::string out = "";


	out += "ADC State:\n";
	
	out += "\tOn/Off state: ";
	if (this->TurnedOn.Defined())
		{
			if (TurnedOn.Get()) out += "ON\n";
			else out += "OFF\n";
		}
	else out += "<UNKNOWN>\n";

	out += "\tFreq. tbl.: ";
	if (this->FrequencyTable.Defined())
		{
			out += "\n";

            std::vector<unsigned short> tbl = this->FrequencyTable.Get();

			for (int i = 0; i <= tbl.size() - 1; i++)
			{
                printf(buff, BUFF_SIZE, "%d", tbl[i]);

				out += "\t\t";
				out += buff;
				out += " sps\n";
			};
		}
	else out += "<UNKNOWN>\n";

	out += "\tCurr. F idx = ";
	if (this->CurrentFrequencyIndex.Defined()) 
	{
        printf(buff, BUFF_SIZE, "%hhu\n", this->CurrentFrequencyIndex.Get());
		out += buff;
	}
	else out += "<UNKNOWN>\n";



	out += "\tCh0: \n";
	out += "\t\tAmpl = ";
	if (this->Ch0_Ampl.Defined()) out += EEG_Device_Architecture::EEG_ADC_Arc::print_to_str(this->Ch0_Ampl.Get());
	else out += "<UNKNOWN>\n";

	out += "\t\tMux:  ";
	if (this->Ch0_MUX.Defined()) out += EEG_Device_Architecture::EEG_ADC_Arc::print_to_str(this->Ch0_MUX.Get());
	else out += "<UNKNOWN>\n";

	out += "\tCh1: \n";
	out += "\t\tAmpl = ";
	if (this->Ch1_Ampl.Defined()) out += EEG_Device_Architecture::EEG_ADC_Arc::print_to_str(this->Ch1_Ampl.Get());
	else out += "<UNKNOWN>\n";

	out += "\t\tMux:  ";
	if (this->Ch1_MUX.Defined()) out += EEG_Device_Architecture::EEG_ADC_Arc::print_to_str(this->Ch1_MUX.Get());
	else out += "<UNKNOWN>\n";

	out += "\tCh2: \n";
	out += "\t\tAmpl = ";
	if (this->Ch2_Ampl.Defined()) out += EEG_Device_Architecture::EEG_ADC_Arc::print_to_str(this->Ch2_Ampl.Get());
	else out += "<UNKNOWN>\n";

	out += "\t\tMux:  ";
	if (this->Ch2_MUX.Defined()) out += EEG_Device_Architecture::EEG_ADC_Arc::print_to_str(this->Ch2_MUX.Get());
	else out += "<UNKNOWN>\n";

	out += "\tCh3: \n";
	out += "\t\tAmpl = ";
	if (this->Ch3_Ampl.Defined()) out += EEG_Device_Architecture::EEG_ADC_Arc::print_to_str(this->Ch3_Ampl.Get());
	else out += "<UNKNOWN>\n";

	out += "\t\tMux:  ";
	if (this->Ch3_MUX.Defined()) out += EEG_Device_Architecture::EEG_ADC_Arc::print_to_str(this->Ch3_MUX.Get());
	else out += "<UNKNOWN>\n";

	out += "\n";

	return out;
};

#ifdef _DEBUG
void EEG_Device_Architecture::EEG_ADC_State::print()
{
	std::cout<<"\t\tADC State:"<<std::endl;
	
	std::cout<<"\t\t\tOn/Off state: ";
	if (this->TurnedOn.Defined())
		{
			if (TurnedOn.Get()) std::cout<<"ON";
			else std::cout<<"OFF";
		}
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tFreq. tbl.: ";
	if (this->FrequencyTable.Defined())
		{
			std::cout<<std::endl;

			std::vector<unsigned __int16> tbl = this->FrequencyTable.Get();

			for (int i = 0; i <= tbl.size() - 1; i++)
				std::cout<<"\t\t\t\t"<<tbl[i]<<" sps"<<std::endl;
		}
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tCurr. F idx = ";
	if (this->CurrentFrequencyIndex.Defined()) printf("%hhu", this->CurrentFrequencyIndex.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;



	std::cout<<"\t\t\tCh0: "<<std::endl;
	std::cout<<"\t\t\t\tAmpl = ";
	if (this->Ch0_Ampl.Defined()) EEG_Device_Architecture::EEG_ADC_Arc::print(this->Ch0_Ampl.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;
	std::cout<<"\t\t\t\tMux:  ";
	if (this->Ch0_MUX.Defined()) EEG_Device_Architecture::EEG_ADC_Arc::print(this->Ch0_MUX.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tCh1: "<<std::endl;
	std::cout<<"\t\t\t\tAmpl = ";
	if (this->Ch1_Ampl.Defined()) EEG_Device_Architecture::EEG_ADC_Arc::print(this->Ch1_Ampl.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;
	std::cout<<"\t\t\t\tMux:  ";
	if (this->Ch1_MUX.Defined()) EEG_Device_Architecture::EEG_ADC_Arc::print(this->Ch1_MUX.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tCh2: "<<std::endl;
	std::cout<<"\t\t\t\tAmpl = ";
	if (this->Ch2_Ampl.Defined()) EEG_Device_Architecture::EEG_ADC_Arc::print(this->Ch2_Ampl.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;
	std::cout<<"\t\t\t\tMux:  ";
	if (this->Ch2_MUX.Defined()) EEG_Device_Architecture::EEG_ADC_Arc::print(this->Ch2_MUX.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tCh3: "<<std::endl;
	std::cout<<"\t\t\t\tAmpl = ";
	if (this->Ch3_Ampl.Defined()) EEG_Device_Architecture::EEG_ADC_Arc::print(this->Ch3_Ampl.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;
	std::cout<<"\t\t\t\tMux:  ";
	if (this->Ch3_MUX.Defined()) EEG_Device_Architecture::EEG_ADC_Arc::print(this->Ch3_MUX.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;



	std::cout<<std::endl;
};
#endif

/**********************************************************************************
*									РЕАЛИЗАЦИЯ
*
*					EEG_Device_Architecture::EEG_Calibrator_State						
*
***********************************************************************************/
const EEG_Device_Architecture::EEG_CalibratorArc::EEG_Calibrator_Shape EEG_Device_Architecture::EEG_CalibratorArc::DEF_Shape = EEG_Device_Architecture::EEG_CalibratorArc::EEG_CAL_SHAPE_SINE;
const bool EEG_Device_Architecture::EEG_CalibratorArc::DEF_TurnedOn_STATE = false;

EEG_Device_Architecture::EEG_Calibrator_State::EEG_Calibrator_State(bool reset_state)
{
	if (reset_state)
	{
		// записать в узел значения по-умолчанию
		this->Shape = EEG_Device_Architecture::EEG_CalibratorArc::DEF_Shape;
		this->Frequency = EEG_Device_Architecture::EEG_CalibratorArc::DEF_Frequency;
		this->TurnedOn = EEG_Device_Architecture::EEG_CalibratorArc::DEF_TurnedOn_STATE;
	};
};

/// вывод себя в строку
std::string EEG_Device_Architecture::EEG_Calibrator_State::print_to_str()
{
	// временный буффер для sprintf
	const int BUFF_SIZE = 256;
	char buff[BUFF_SIZE];

	std::string out = "";


	out += "Calibrator State:\n";

	out += "\tFreq. = ";
	if (this->Frequency.Defined()) out += this->Frequency.Get();
	else out += "<UNKNOWN>\n";

	out += "\tOn/Off state: ";
	if (this->TurnedOn.Defined())
		{
			if (TurnedOn.Get()) out += "ON\n";
			else out += "OFF\n";
		}
	else out += "<UNKNOWN>\n";

	out += "\tShape. = ";
	if (this->Shape.Defined()) out += EEG_Device_Architecture::EEG_CalibratorArc::print_to_str(this->Shape.Get());
	else out += "<UNKNOWN>\n";


	return out;
};

#ifdef _DEBUG
void EEG_Device_Architecture::EEG_Calibrator_State::print()
{
	std::cout<<"\t\tCalibrator State:"<<std::endl;

	std::cout<<"\t\t\tFreq. = ";
	if (this->Frequency.Defined()) std::cout<<this->Frequency.Get();
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tOn/Off state: ";
	if (this->TurnedOn.Defined())
		{
			if (TurnedOn.Get()) std::cout<<"ON";
			else std::cout<<"OFF";
		}
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tShape. = ";
	if (this->Shape.Defined()) EEG_Device_Architecture::EEG_CalibratorArc::print(this->Shape.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;
};
#endif

/**********************************************************************************
*									РЕАЛИЗАЦИЯ
*
*					EEG_Device_Architecture::EEG_Common_State						
*
***********************************************************************************/

const bool EEG_Device_Architecture::EEG_CommonArc::DEF_Booting = true;
const int EEG_Device_Architecture::EEG_CommonArc::DEF_LastErrorID = 0;
const bool EEG_Device_Architecture::EEG_CommonArc::DEF_Recording = false;


EEG_Device_Architecture::EEG_Common_State::EEG_Common_State(bool reset_state)
{
	if (reset_state)
	{
		// записать в узел значения по-умолчанию
		this->Booting = EEG_Device_Architecture::EEG_CommonArc::DEF_Booting;
		this->CircuityVersion.SetUndefined();
		this->FirmwareVersion.SetUndefined();
		this->LastErrorID = EEG_Device_Architecture::EEG_CommonArc::DEF_LastErrorID;
		this->ProtocolVersion.SetUndefined();
		this->Recording = EEG_Device_Architecture::EEG_CommonArc::DEF_Recording;
		this->SerialNumber.SetUndefined();
	};
};

// вывод себя в строку
std::string EEG_Device_Architecture::EEG_Common_State::print_to_str()
{
	// временный буффер для sprintf
	const int BUFF_SIZE = 256;
	char buff[BUFF_SIZE];

	std::string out = "";

	out += "Common State:\n";

	out += "\tBooting: ";
	if (this->Booting.Defined())
		{
			if (Booting.Get()) out += "Yeap\n";
			else out += "No\n";
		}
	else out += "<UNKNOWN>\n";

	out += "\tCircuity Version: ";
	if (this->CircuityVersion.Defined()) 
	{
        printf(buff, BUFF_SIZE, "%hhu\n", this->CircuityVersion.Get());
		out += buff;
	}
	else out += "<UNKNOWN>\n";

	out += "\tFirmware Version: ";
	if (this->FirmwareVersion.Defined()) 
	{
        printf(buff, BUFF_SIZE, "%hhu\n", this->FirmwareVersion.Get());
		out += buff;
	}
	else out += "<UNKNOWN>\n";

	out += "\tProtocol Version: ";
	if (this->ProtocolVersion.Defined())
	{
        printf(buff, BUFF_SIZE, "%hhu\n", this->ProtocolVersion.Get());
		out += buff;
	}
	else out += "<UNKNOWN>\n";

	out += "\tSN: ";
	if (this->SerialNumber.Defined()) 
	{
        printf(buff, BUFF_SIZE, "%hd\n", this->SerialNumber.Get());
		out += buff;
	}
	else out += "<UNKNOWN>\n";

	out += "\tLast Device error: ";
	if (this->LastErrorID.Defined()) 
	{
        printf(buff, BUFF_SIZE, "%d\n", this->LastErrorID.Get());
		out += buff;
	}
	else out += "<UNKNOWN>\n";

	out += "\tRecording: ";
	if (this->Recording.Defined())
		{
			if (Recording.Get()) out += "Yeap\n";
			else out += "No\n";
		}
	else out += "<UNKNOWN>\n";
	
	return out;
};


#ifdef _DEBUG
void EEG_Device_Architecture::EEG_Common_State::print()
{
	std::cout<<"\t\tCommon State:"<<std::endl;

	std::cout<<"\t\t\tBooting: ";
	if (this->Booting.Defined())
		{
			if (Booting.Get()) std::cout<<"Yeap";
			else std::cout<<"No";
		}
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tCircuity Version: ";
	if (this->CircuityVersion.Defined()) printf("%hhu", this->CircuityVersion.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tFirmware Version: ";
	if (this->FirmwareVersion.Defined()) printf("%hhu", this->FirmwareVersion.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tProtocol Version: ";
	if (this->ProtocolVersion.Defined()) printf("%hhu", this->ProtocolVersion.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tSN: ";
	if (this->SerialNumber.Defined()) printf("%hd", this->SerialNumber.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tLast Device error: ";
	if (this->LastErrorID.Defined()) printf("%d", this->LastErrorID.Get());
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;

	std::cout<<"\t\t\tRecording: ";
	if (this->Recording.Defined())
		{
			if (Recording.Get()) std::cout<<"Yeap";
			else std::cout<<"No";
		}
	else std::cout<<"<UNKNOWN>";
	std::cout<<std::endl;
};
#endif






/**********************************************************************************
*									РЕАЛИЗАЦИЯ
*
*						EEG_Device_Architecture::EEG_Device_State
*					
*								
*
***********************************************************************************/

std::string EEG_Device_Architecture::EEG_Device_State::print_to_str()
{
	return	
		BridgeData_State.print_to_str() + '\n' +
		Common_State.print_to_str() + '\n' +
		MainCommutator_State.print_to_str() + '\n' + 
		FunctionalLeds_State.print_to_str() + '\n' + 
		DAC_State.print_to_str() + '\n' + 
		ChannelsLeds_State.print_to_str() + '\n' + 
		ADC_State.print_to_str() + '\n';
};


#ifdef _DEBUG

void EEG_Device_Architecture::EEG_Device_State::print()
{
	this->BridgeData_State.print();
	std::cout<<std::endl;

	this->Common_State.print();
	std::cout<<std::endl;

	this->MainCommutator_State.print();
	std::cout<<std::endl;

	this->FunctionalLeds_State.print();
	std::cout<<std::endl;

	this->DAC_State.print();
	std::cout<<std::endl;

	this->ChannelsLeds_State.print();
	std::cout<<std::endl;

	this->ADC_State.print();
};

#endif





/**********************************************************************************
*									РЕАЛИЗАЦИЯ
*
*							EEG_Device_Architecture
*					
*								
*
***********************************************************************************/


// исходя из настроек, записанных в текущем объекте возвращает величину напряжения  в микровольтах по данным канала
float EEG_Device_Architecture::GetRealVoltage__uV(int ch_data, char channel_idx, EEG_Device_Architecture::EEG_ADC_State* ADC_state_ptr)
{
	//// коэффициент усиления в канале
	//float ampl_coeff = 1.0;

	//int ampl = 0;

	//// получаем, какая цепь усиления используется
	//switch (channel_idx)
	//{		
	//case EEG_CommonArc::EEG_CHANNEL_Fp2: ampl = ADC_state_ptr->Ch0_Ampl.Get(); break;	// Fp2
	//case EEG_CommonArc::EEG_CHANNEL_C4: ampl = ADC_state_ptr->Ch1_Ampl.Get(); break; // C4
	//case EEG_CommonArc::EEG_CHANNEL_Fp1: ampl = ADC_state_ptr->Ch2_Ampl.Get(); break;	// Fp1
	//case EEG_CommonArc::EEG_CHANNEL_C3: ampl = ADC_state_ptr->Ch3_Ampl.Get(); break; // C3
	//case EEG_CommonArc::EEG_CHANNEL_A1: ampl = ADC_state_ptr->Ch3_Ampl.Get(); break;	// A1 - используется цепь С3
	//case EEG_CommonArc::EEG_CHANNEL_A2: ampl = ADC_state_ptr->Ch1_Ampl.Get(); break;	// A2 - используется цепь С4
	//};

	return  0.0;
};

/// Статическая функция, возвращает вольтаж в микровольтах на основе указанного коэффициента усиления канала
/// и значения отсчёта АЦП.
float EEG_Device_Architecture::GetRealVoltage__uV(int ch_data, EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch_amplification)
{
	return ((float)ch_data) * GetMeasurementToVoltageCoeff(ch_amplification);
};

/// Возвразает коэффициент преобразования из отсчёта АЦПа в микровольты
float EEG_Device_Architecture::GetMeasurementToVoltageCoeff(EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_Amplification ch_amplification)
{
	// коэффициент линии усиления
	float base_amp_coeff = 0.1f/20504576.0f;

	// дополнительный коэффициент усиления на АЦПе
	float adc_amp_coeff = 1.0;

	// выбираем коефф усиления на АЦПе
	switch (ch_amplification)
	{
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_1x: adc_amp_coeff = 1.0f; break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_2x: adc_amp_coeff = 2.0f; break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_3x: adc_amp_coeff = 3.0f; break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_4x: adc_amp_coeff = 4.0f; break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_6x: adc_amp_coeff = 6.0f; break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_8x: adc_amp_coeff = 8.0f; break;
	case EEG_Device_Architecture::EEG_ADC_Arc::EEG_ADS_AMPLIFICATION_12x: adc_amp_coeff = 12.0f; break;
	default: assert(0); break;
	};

	return base_amp_coeff * adc_amp_coeff;
};


};
