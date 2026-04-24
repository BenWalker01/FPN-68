#pragma once
#include "EuroScopePlugIn.h"
#include "CFPNRadarTarget.h"
#include <string>
#include "pch.h"
#include <vector>

class CFPNPlugin : public EuroScopePlugIn::CPlugIn {

public:
    CFPNPlugin();
    virtual ~CFPNPlugin();

    void LoadCustomFont();

    virtual EuroScopePlugIn::CRadarScreen* OnRadarScreenCreated(const char* sDisplayName,
        bool NeedRadarContent,
        bool GeoReferenced,
        bool CanBeSaved,
        bool CanBeCreated
    );

    virtual bool OnCompileCommand(const char* sCommandLine);

    virtual void sendMessage(std::string message);
    inline std::vector<CFPNRadarTarget>* getPreviousTargets() {
        return &previousTargets;
    }

    float getElevation() const {
        return elevation;
    }

    std::vector<CFPNRadarTarget> previousTargets = std::vector<CFPNRadarTarget>();

    int range = 15;
    float angle = 3;
    float elevation = -1;
    EuroScopePlugIn::CPosition runwayThreshold;
    EuroScopePlugIn::CPosition otherThreshold;

    std::string icao;
    void loadNewAerodrome(const char* icao, const char* runway);
private:
    std::vector<void *> radarScreens;  // CFPNRadarScreen *

    
    std::vector<std::string> getAerodromeRunways(const char* icao);
};
