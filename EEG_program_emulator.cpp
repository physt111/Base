#include "EEG_program_emulator.h"

namespace MBN_EEG
{

//  реализация потока устройства
void EEG_Device_Thread::run()
{
	while(1)
	{
		/// создаём и отправляем посылку
		CommonPackage pkg;
        long long offset = 0;
		vector<unsigned char> bytes(MBN_EEG::EEG_D_UsualData::GetSize()); 
        MBN_EEG::EEG_D_UsualData::CreatePackage<vector<unsigned char> >(pkg, bytes, offset,
			10000*sin(emulator_ptr->calibrator_phase),
			10000*cos(emulator_ptr->calibrator_phase),
			10000*sin(emulator_ptr->calibrator_phase),
			10000*cos(emulator_ptr->calibrator_phase)); 

		emulator_ptr->calibrator_phase += emulator_ptr->d_phase;
				
        long long pkg_id = emulator_ptr->PostPackage(pkg, bytes);

		msleep(5);

		if (stop_dev_thread)
			return;
	};
};

};
