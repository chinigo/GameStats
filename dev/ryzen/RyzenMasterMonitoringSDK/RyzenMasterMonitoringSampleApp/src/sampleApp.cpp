//===----------------------------------------------------------------------===//
//====  Copyright (c) 2020 Advanced Micro Devices, Inc.  All rights reserved.
//
//               Developed by: Advanced Micro Devices, Inc.

#pragma once
#include <windows.h>
#include <Shlobj.h>
#include <iostream>
#include <VersionHelpers.h>
#include <intrin.h>
#include "ICPUEx.h"
#include "IPlatform.h"
#include "IDeviceManager.h"
#include "IBIOSEx.h"

#include "Utility.hpp"
#define MAX_LENGTH 50

enum CPU_PackageType
{
	cptFP5 = 0,
	cptAM,
	cptAM4,
	cptSP3r2 = 7,
	cptUnknown = 0xF
};

TCHAR PrintErr[][63] = { _T("Failure"),_T("Success") ,_T("Invalid value"),_T("Method is not implemented by the BIOS"),_T("Cores are already parked. First Enable all the cores"), _T("Unsupported Function") };


inline void printFunc(LPCTSTR func, BOOL bCore, int i = 0)
{
	if (!bCore)
	{
		_tprintf(_T("%s "), func);
		for (size_t i = 0; i < MAX_LENGTH - _tcslen(func); ++i)
		{
			_tprintf(_T("%c"), '.');
		}
	}
	else
	{
		_tprintf(_T("%s Core : %-2d"), func, i);
		for (size_t i = 0; i < MAX_LENGTH - _tcslen(func) - 9; ++i)
		{
			_tprintf(_T("%c"), '.');
		}
	}
}

BOOL IsSupportedProcessor(VOID)
{
	bool retBool = false;
	int CPUInfo[4] = { -1 };
	__cpuid(CPUInfo, 0x80000001);
	unsigned long uCPUID = CPUInfo[0];
	CPU_PackageType pkgType = (CPU_PackageType)(CPUInfo[1] >> 28);
	switch (pkgType)
	{
	case cptFP5:
		switch (uCPUID)
		{
		case 0x00810F80:
		case 0x00810F81:
		case 0x00860F00:
		case 0x00860F01:
		case 0x00A50F00:
		case 0x00A50F01:
			retBool = true;
			break;
		default:
			break;
		}
	case cptAM4:
		switch (uCPUID)
		{
		case 0x00800F00:
		case 0x00800F10:
		case 0x00800F11:
		case 0x00800F12:

		case 0x00810F10:
		case 0x00810F11:

		case 0x00800F82:
		case 0x00800F83:

		case 0x00870F00:
		case 0x00870F10:

		case 0x00810F80:
		case 0x00810F81:

		case 0x00860F00:
		case 0x00860F01:

		case 0x00A20F00:
		case 0x00A20F10:
			retBool = true;
			break;
		default:
			break;
		}
		break;
	case cptSP3r2:
		switch (uCPUID)
		{
		case 0x00800F10:
		case 0x00800F11:
		case 0x00800F12:

		case 0x00800F82:
		case 0x00800F83:
		case 0x00830F00:
		case 0x00830F10:
			retBool = true;
			break;
		default:
			break;
		}
		break;
	}
	return retBool;
}

void apiCall()
{
	bool bRetCode = false;

	IPlatform& rPlatform = GetPlatform();
	bRetCode = rPlatform.Init();
	if (!bRetCode)
	{
		_tprintf(_T("Platform init failed\n"));
		return;
	}
	IDeviceManager& rDeviceManager = rPlatform.GetIDeviceManager();
	ICPUEx* obj = (ICPUEx*)rDeviceManager.GetDevice(dtCPU, 0);
	IBIOSEx* objB = (IBIOSEx*)rDeviceManager.GetDevice(dtBIOS, 0);
	if (obj && objB)
	{
		CACHE_INFO result;
		unsigned long uResult = 0;
		unsigned int uCorePark = 0;
		unsigned int uCoreCount = 0;
		wchar_t package[30] = { 0 };
		PWCHAR wResult = NULL;
		PTCHAR tResult = NULL;
		WCHAR sDate[50] = {};
		WCHAR Year[5] = {};
		WCHAR Month[3] = {};
		WCHAR Day[3] = {};
		WCHAR ChipsetID[256] = { '\0' };
		CPUParameters stData;
		int iRet = 0;
		int i, j = 0;
		unsigned short uVDDIO = 0, uMemClock = 0;
		unsigned char uTcl = 0, uTcdrd = 0, uTras = 0, uTrp = 0;

		wResult = (PWCHAR)obj->GetName();
		printFunc(_T("GetName"), FALSE);
		wprintf(L" %s\n", wResult);
		wResult = (PWCHAR)obj->GetDescription();
		printFunc(_T("GetDescription"), FALSE);
		wprintf(L" %s\n", wResult);
		wResult = (PWCHAR)obj->GetVendor();
		printFunc(_T("GetVendor"), FALSE);
		wprintf(L" %s\n", wResult);
		tResult = (PTCHAR)obj->GetClassName();
		printFunc(_T("GetClassName"), FALSE);
		_tprintf(_T(" %s\n"), tResult);

		iRet = obj->GetCoreCount(uCoreCount);
		printFunc(_T("GetCoreCount"), FALSE);
		if (iRet)
		{
			_tprintf(_T(" %s\n"), PrintErr[iRet + 1]);
		}
		else
		{
			_tprintf(_T(" %u\n"), uCoreCount);
		}

		iRet = obj->GetCorePark(uCorePark);
		printFunc(_T("GetCorePark"), FALSE);
		if (iRet)
		{
			_tprintf(_T(" %s\n"), PrintErr[iRet + 1]);
		}
		else
		{
			_tprintf(_T(" %lu Cores parked\n"), uCorePark);
		}

		iRet = obj->GetL1DataCache(result);
		printFunc(_T("GetL1DataCache"), FALSE);
		if (iRet)
		{
			_tprintf(_T(" %s\n"), PrintErr[iRet + 1]);
		}
		else
		{
			_tprintf(_T(" %d x %.0fKB\n"), (uCoreCount - uCorePark), result.fSize);
		}

		result = { NULL };
		iRet = obj->GetL1InstructionCache(result);
		printFunc(_T("GetL1InstructionCache"), FALSE);
		if (iRet)
		{
			_tprintf(_T(" %s\n"), PrintErr[iRet + 1]);
		}
		else
		{
			_tprintf(_T(" %d x %.0fKB\n"), (uCoreCount - uCorePark), result.fSize);
		}

		result = { NULL };
		iRet = obj->GetL2Cache(result);
		printFunc(_T("GetL2Cache"), FALSE);
		if (iRet)
		{
			_tprintf(_T(" %s\n"), PrintErr[iRet + 1]);
		}
		else
		{
			_tprintf(_T(" %d x %.0fKB\n"), (uCoreCount - uCorePark), result.fSize);
		}

		result = { NULL };
		iRet = obj->GetL3Cache(result);
		printFunc(_T("GetL3Cache"), FALSE);
		if (iRet)
		{
			_tprintf(_T(" %s\n"), PrintErr[iRet + 1]);
		}
		else
		{
			_tprintf(_T(" %0.f KB\n"), result.fSize);
		}

		wcscpy(package, obj->GetPackage());
		printFunc(_T("GetPackage"), FALSE);
		wprintf(L" %s\n", package);

		iRet = obj->GetCPUParameters(stData);
		printFunc(_T("GetCPUParameters"), FALSE);
		if (iRet)
		{
			_tprintf(_T(" %s\n"), PrintErr[iRet + 1]);
		}
		else
		{
			_tprintf(_T(" PPT Current Limit :"));
			LOG_PRINT(stData.fPPTLimit, _T("W"));

			printFunc(_T("GetCPUParameters"), FALSE);
			_tprintf(_T(" PPT Current Value :"));
			LOG_PRINT(stData.fPPTValue, _T("W"));

			printFunc(_T("GetCPUParameters"), FALSE);
			_tprintf(_T(" EDC(VDD) Current Limit :"));
			LOG_PRINT(stData.fEDCLimit_VDD, _T("A"));

			printFunc(_T("GetCPUParameters"), FALSE);
			_tprintf(_T(" EDC(VDD) Current Value :"));
			LOG_PRINT(stData.fEDCValue_VDD, _T("A"));

			printFunc(_T("GetCPUParameters"), FALSE);
			_tprintf(_T(" TDC(VDD) Current Limit :"));
			LOG_PRINT(stData.fTDCLimit_VDD, _T("A"));

			printFunc(_T("GetCPUParameters"), FALSE);
			_tprintf(_T(" TDC(VDD) Current Value :"));
			LOG_PRINT(stData.fTDCValue_VDD, _T("A"));

			printFunc(_T("GetCPUParameters"), FALSE);
			_tprintf(_T(" EDC(SOC) Current Limit :"));
			LOG_PRINT(stData.fEDCLimit_SOC, _T("A"));

			printFunc(_T("GetCPUParameters"), FALSE);
			_tprintf(_T(" EDC(SOC) Average Value :"));
			LOG_PRINT(stData.fEDCValue_SOC, _T("A"));

			printFunc(_T("GetCPUParameters"), FALSE);
			_tprintf(_T(" TDC(SOC) Current Limit :"));
			LOG_PRINT(stData.fTDCLimit_SOC, _T("A"));

			printFunc(_T("GetCPUParameters"), FALSE);
			_tprintf(_T(" TDC(SOC) Current Value :"));
			LOG_PRINT(stData.fTDCValue_SOC, _T("A"));

			printFunc(_T("GetCPUParameters"), FALSE);
			_tprintf(_T(" cHTC Limit :"));
			LOG_PRINT(stData.fcHTCLimit, _T("Celsius"));

			printFunc(_T("GetCPUParameters"), FALSE);
			_tprintf(_T(" Fabric Clock Frequency :"));
			LOG_PRINT(stData.fFCLKP0Freq, _T("MHz"));

			printFunc(_T("GetCPUParameters"), FALSE);
			_tprintf(_T(" VDDCR(VDD) Power :"));
			LOG_PRINT(stData.fVDDCR_VDD_Power, _T("W"));

			printFunc(_T("GetCPUParameters"), FALSE);
			_tprintf(_T(" VDDCR(SOC) Power :"));
			LOG_PRINT(stData.fVDDCR_SOC_Power, _T("W"));

			printFunc(_T("GetCPUParameters"), FALSE);
			_tprintf(_T(" Fmax(CPU Clock) frequency :"));
			LOG_PRINT(stData.fCCLK_Fmax, _T("MHz"));

			printFunc(_T("GetCurrentOCMode"), FALSE);
			if (stData.eMode.uManual)
			{
				_tprintf(_T(" Manual Mode.\n"));
			}
			else if (stData.eMode.uPBOMode)
			{
				_tprintf(_T(" PBO Mode.\n"));
			}
			else if (stData.eMode.uAutoOverclocking)
			{
				_tprintf(_T(" Auto Overclocking Mode.\n"));
			}
			else
			{
				_tprintf(_T(" Default Mode.\n"));
			}

			for (i = 0; i < stData.stFreqData.uLength; i++)
			{
				if (stData.stFreqData.dFreq[i] != 0)
				{
					printFunc(_T("GetEffectiveFrequency"), TRUE, j);
					_tprintf(_T(" %0.1f MHz	C0 Residency...  %0.2f%% \n"), stData.stFreqData.dFreq[i], stData.stFreqData.dState[i]);
					j++;
				}
			}

			printFunc(_T("GetPeakSpeed"), FALSE);
			_tprintf(_T(" %0.1f MHz\n"), stData.dPeakSpeed);
			printFunc(_T("GetPeakCore(s)Voltage"), FALSE);
			_tprintf(_T(" %0.3f V\n"), stData.dPeakCoreVoltage);
			printFunc(_T("GetAverageCoreVoltage"), FALSE);
			_tprintf(_T(" %0.3f V\n"), stData.dAvgCoreVoltage);
			printFunc(_T("GetVDDCR_SOC"), FALSE);
			_tprintf(_T(" %0.3f V\n"), stData.dSocVoltage);;
			printFunc(_T("GetCurrentTemperature"), FALSE);
			_tprintf(_T(" %0.2f Celsius\n"), stData.dTemperature);
		}

		iRet = objB->GetMemVDDIO(uVDDIO);
		printFunc(_T("GetMemVDDIO"), FALSE);
		if (iRet)
		{
			_tprintf(_T(" %s\n"), PrintErr[iRet + 1]);
		}
		else
		{
			_tprintf(_T(" %d mV\n"), uVDDIO);
		}

		iRet = objB->GetCurrentMemClock(uMemClock);
		printFunc(_T("GetCurrentMemClock"), FALSE);
		if (iRet)
		{
			_tprintf(_T(" %s\n"), PrintErr[iRet + 1]);
		}
		else
		{
			_tprintf(_T(" %d MHz\n"), uMemClock);
		}

		iRet = objB->GetMemCtrlTcl(uTcl);
		printFunc(_T("GetMemCtrlTcl"), FALSE);
		if (iRet)
		{
			_tprintf(_T(" %s\n"), PrintErr[iRet + 1]);
		}
		else
		{
			_tprintf(_T(" %d Memory clock cycles\n"), uTcl);
		}

		iRet = objB->GetMemCtrlTrcdrd(uTcdrd);
		printFunc(_T("GetMemCtrlTrcdrd"), FALSE);
		if (iRet)
		{
			_tprintf(_T(" %s\n"), PrintErr[iRet + 1]);
		}
		else
		{
			_tprintf(_T(" %d Memory clock cycles\n"), uTcdrd);
		}

		iRet = objB->GetMemCtrlTras(uTras);
		printFunc(_T("GetMemCtrlTras"), FALSE);
		if (iRet)
		{
			_tprintf(_T(" %s\n"), PrintErr[iRet + 1]);
		}
		else
		{
			_tprintf(_T(" %d Memory clock cycles\n"), uTras);
		}

		iRet = objB->GetMemCtrlTrp(uTrp);
		printFunc(_T("GetMemCtrlTrp"), FALSE);
		if (iRet)
		{
			_tprintf(_T(" %s\n"), PrintErr[iRet + 1]);
		}
		else
		{
			_tprintf(_T(" %d Memory clock cycles\n"), uTrp);
		}

		printFunc(_T("GetBIOSInfo"), FALSE);
		wcscpy(sDate, objB->GetDate());
		wcsncpy(Year, sDate, 4);
		wcsncpy(Month, sDate + 4, 2);
		wcsncpy(Day, sDate + 6, 2);
		wprintf(L" Version : %s , Vendor : %s , Date : %s/%s/%s\n", objB->GetVersion(), objB->GetVendor(), Year, Month, Day);

		printFunc(_T("GetChipsetName"), FALSE);
		iRet = obj->GetChipsetName(ChipsetID);
		if (iRet)
		{
			_tprintf(_T(" %s\n"), PrintErr[iRet + 1]);
		}
		else
		{
			wprintf(L" %s\n", ChipsetID);
		}
	}
	rPlatform.UnInit();
}

int main(int argc, CHAR **argv)
{
	//Check if application running with admin privileged or not
	if (!IsUserAnAdmin())
	{
		MessageBox(NULL, _T("Access Denied : Run application with Admin rights"), TEXT("Error"), MB_OK);
		ShowError(_T("User is not admin..."), FALSE, 1);
	}
	//Check for OS support
	if (!IsSupportedOS())
	{
		ShowError(_T("This Desktop application requires OS version greater than or equals to Windows 10."), FALSE, 1);
	}
	//Check if we have AMD processor
	if (!Authentic_AMD())
		ShowError(_T("No AMD Processor is found!"), FALSE, 1);

	//Check if driver is installed or not
	if (QueryDrvService() < 0)
	{
		if (false == InstallDriver())
			ShowError(_T("Unable to install driver AMDRyzenMasterDriver.sys : Driver not found!"), FALSE, 1);
	}

	//Check if application is running on supported Processor?
	if (!IsSupportedProcessor())
	{
		ShowError(_T("Not Supported Processor!"), FALSE, 1);
	}
	apiCall();
	return 0;
}