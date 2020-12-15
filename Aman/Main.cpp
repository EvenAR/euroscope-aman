#include "stdafx.h"
#include "AmanPlugIn.h"

AmanPlugIn* gpAmanPlugin = NULL;

//---EuroScopePlugInInit-----------------------------------------------

void __declspec(dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance) {
    *ppPlugInInstance = gpAmanPlugin = new AmanPlugIn;
}

//---EuroScopePlugInExit-----------------------------------------------

void __declspec(dllexport) EuroScopePlugInExit(void) {
    delete gpAmanPlugin;
}
