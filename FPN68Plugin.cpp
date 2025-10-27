#include "pch.h"
#include "FPN68Plugin.h"
#include "Constant.h"
#include "CFPNRadarScreen.h"
#include "Resource.h"

#include <sstream>
#include <cstring>
#include <windows.h>
#include <tchar.h>

// =================== Ben's code starts here ===================
void CFPNPlugin::LoadCustomFont() {
	HMODULE m;
	GetModuleHandleExA(NULL, "FPN68.dll", &m);
	std::ostringstream es;
	es << GetLastError();
	sendMessage(es.str());

    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_VCRFONT), RT_FONT);
	std::ostringstream os;
	os << GetLastError();
	sendMessage(os.str());

	hRes = FindResource(m, MAKEINTRESOURCE(IDR_VCRFONT), RT_FONT);
	std::ostringstream as;
	as << GetLastError();
	sendMessage(as.str());
    if (hRes) {
        HGLOBAL hMem = LoadResource(AfxGetInstanceHandle(), hRes);
        if (hMem) {
			
            DWORD dwSize = SizeofResource(NULL, hRes);
            void* pFontData = LockResource(hMem);
            if (pFontData) {
                DWORD nFonts;
                HANDLE hFont = AddFontMemResourceEx(pFontData, dwSize, NULL, &nFonts);
            }
        }
    }
}
// =================== Ben's code ends here ===================

CFPNPlugin::CFPNPlugin(void) : CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, "FPN-68 PAR", "1.0.0", "Alice Ford, Ben Walker","GPL v3") {
	LoadCustomFont();
	
	loadNewAerodrome("EGKK", "26L");
	
	sendMessage("FPN-68 PAR Plugin Loaded");

	RegisterDisplayType(FPN_NAME, false, false, true, true);
}

CFPNPlugin::~CFPNPlugin() {
	sendMessage("FPN-68 PAR Plugin Unloaded");
}

EuroScopePlugIn::CRadarScreen* CFPNPlugin::OnRadarScreenCreated(const char* sDisplayName,
	bool NeedRadarContent,
	bool GeoReferenced,
	bool CanBeSaved,
	bool CanBeCreated
) {
	if (strcmp(sDisplayName, FPN_NAME) != 0) {
		return nullptr;
	}
	else {
		CFPNRadarScreen* radarScreen = new CFPNRadarScreen;
		radarScreen->setElevation(this->getElevation());
		radarScreens.push_back(radarScreen);
		return radarScreen;
	}
}

void CFPNPlugin::sendMessage(std::string message) {
	try {
		DisplayUserMessage("FPN-68", "System", message.c_str(), true, true, true, false, false);
	}
	catch (...) {
		DisplayUserMessage("FPN-68", "System", "ERROR", true, true, true, true, false);
	}
}

bool CFPNPlugin::OnCompileCommand(const char* sCommandLine) {
	if (sCommandLine == nullptr || strlen(sCommandLine) < 4) {
		return false;
	}

	if (strncmp(sCommandLine, ".fpn", 4) == 0) {
		std::string command(sCommandLine);

		// Ensure the command length is greater than 4 before extracting the payload
		if (command.length() > 4) {
			std::string payload = command.substr(5);

			if (payload.empty()) {
				sendMessage("Command was empty");
				return false;
			}

			icao = payload.c_str();
			std::vector<std::string> runways = getAerodromeRunways(payload.c_str());
			while (runways.size() < 6) {
				runways.push_back(" ");
			}

			for (int i = 0; i < radarScreens.size(); i++) {
				for (int j = 0; j < 6; j++) {
					((CFPNRadarScreen *) radarScreens[i])->runwayControlsText[j / 3][j % 3].text = runways[j];
					if (runways[j] == " ") {
						((CFPNRadarScreen*)radarScreens[i])->runwayControlsText[j / 3][j % 3].inop = true;
					}
					else {
						((CFPNRadarScreen*)radarScreens[i])->runwayControlsText[j / 3][j % 3].inop = false;
					}
				}
			}

			return true;
		}
		else {
			sendMessage("Command was empty");
			return false;
		}
	}

	return false;
}

void CFPNPlugin::loadNewAerodrome(const char* icao, const char* runway) {
	EuroScopePlugIn::CSectorElement elem = SectorFileElementSelectFirst(EuroScopePlugIn::SECTOR_ELEMENT_RUNWAY);

	bool foundRunway = false;
	
	do {
		if (strncmp(elem.GetAirportName(), icao, 4) == 0) {
			int runwayIndex = -1;
			if (strcmp(elem.GetRunwayName(0), runway) == 0) {
				runwayIndex = 0;
			}
			else if (strcmp(elem.GetRunwayName(1), runway) == 0) {
				runwayIndex = 1;
			}
			if (runwayIndex >= 0) {
				elem.GetPosition(&runwayThreshold, runwayIndex);
				elem.GetPosition(&otherThreshold, runwayIndex == 0 ? 1 : 0);
				auto airportElevation = AIRPORT_ELEVATION;
				std::string airportName(elem.GetAirportName(), 4);
				if (airportElevation.find(airportName) != airportElevation.end() &&
					airportElevation[airportName].find(elem.GetRunwayName(runwayIndex)) != airportElevation[airportName].end()) {
					elevation = airportElevation[airportName][elem.GetRunwayName(runwayIndex)];
				}
				sendMessage(("Loaded: " + std::string(elem.GetAirportName()) + ", runway " + std::string(elem.GetRunwayName(runwayIndex)) + ", threshold elevation " + std::to_string(elevation) + " ft").c_str());
				foundRunway = true;
				break;
			}
		}

		elem = SectorFileElementSelectNext(elem, EuroScopePlugIn::SECTOR_ELEMENT_RUNWAY);
	} while (elem.IsValid());

	if (!foundRunway) {
		sendMessage(("Runway " + std::string(runway) + " at " + std::string(icao) + " not found!").c_str());
	}
}

std::vector<std::string> CFPNPlugin::getAerodromeRunways(const char* icao) {
	EuroScopePlugIn::CSectorElement elem = SectorFileElementSelectFirst(EuroScopePlugIn::SECTOR_ELEMENT_RUNWAY);

	std::vector<std::string> runways;

	do {
		if (strncmp(elem.GetAirportName(), icao, 4) == 0) {
			runways.push_back(elem.GetRunwayName(0));
			runways.push_back(elem.GetRunwayName(1));
		}

		elem = SectorFileElementSelectNext(elem, EuroScopePlugIn::SECTOR_ELEMENT_RUNWAY);
	} while (elem.IsValid());

	return runways;
}
