// EGKKHoldingPoints.cpp : Defines the initialization routines for the DLL.
//

#include "pch.h"
//#include "framework.h"
#include "FPN68Plugin.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

CFPNPlugin* pMyPlugIn = NULL;


void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance) {
	// allocate
	*ppPlugInInstance = pMyPlugIn = new CFPNPlugin();
}

void __declspec (dllexport) EuroScopePlugInExit(void) {
	delete pMyPlugIn;
}

