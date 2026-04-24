#pragma once
#include "EuroScopePlugIn.h"
#include "FPN68Plugin.h"
#include "Constant.h"
#include <afxpriv.h>

class CFPNRadarScreen :
    public EuroScopePlugIn::CRadarScreen
{
public:
    CFPNRadarScreen();
    virtual ~CFPNRadarScreen();

    void OnAsrContentLoaded(bool loaded) override;
    void OnAsrContentToBeSaved() override;
    void OnRefresh(HDC hDC, int Phase) override;
    virtual void OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area);
    virtual void OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);

    inline void OnAsrContentToBeClosed(void) override {
        delete this;
    }

    void setElevation(float elevation) {
        this->elevation = elevation;
    }

    typedef void (*Handler) (CFPNRadarScreen*);

    struct Setting {
        std::string text;
        bool selected;
        bool hover;
        bool inop;
        Handler handler;
    };

    static void rangeChangeHandler(int range, CFPNRadarScreen* parent);
    static void glideslopeChangeHandler(float angle, CFPNRadarScreen* parent);
    static void runwayChangeHandler(int i, int j, CFPNRadarScreen* parent);

    std::vector<std::vector<Setting>> mainControlsText = {
        {{"Reset\nDefault", false, false, true, nullptr}, {"Small\nA/C", true, false, true, nullptr}, {"Large\nA/C", false, false, true, nullptr}},
        {{"Runway\nSelect", false, false, true, nullptr}, {"", false, false, false, nullptr}, {"Display\nControl", false, false, true, nullptr}},
        {{"Radar\nControl", false, false, true, nullptr}, {"PPI\nMode", false, false, true, nullptr}, {"WHI Tgt\nCycle", false, false, true, nullptr}},
        {{"Clear\nAlerts", false, false, true, nullptr}, {"Status", false, false, true, nullptr}, {"ACID\nEntry", false, false, true, nullptr}},
    };
    std::vector<std::vector<Setting>> rangeControlsText = {
        {{"\u2190", false, true, true, nullptr}, {"\u2192", false, false, true, nullptr}, {"1", false, false, false, [](CFPNRadarScreen* parent) { rangeChangeHandler(1, parent); }}, {"3", false, false, false, [](CFPNRadarScreen* parent) { rangeChangeHandler(3, parent); }}},
        {{"5", false, false, false, [](CFPNRadarScreen* parent) { rangeChangeHandler(5, parent); }}, {"10", true, false, false, [](CFPNRadarScreen* parent) { rangeChangeHandler(10, parent); }}, {"15", false, false, false, [](CFPNRadarScreen* parent) { rangeChangeHandler(15, parent); }}, {"20", false, false, false, [](CFPNRadarScreen* parent) { rangeChangeHandler(20, parent); }}}
    };
    std::vector<std::vector<Setting>> glideControlsText = {
        {{"3.0", true, false, false, [](CFPNRadarScreen* parent) {glideslopeChangeHandler(3.0, parent); }}, {"2.5", false, false, false, [](CFPNRadarScreen* parent) {glideslopeChangeHandler(2.5, parent); }}, {"3.0", false, false, true, nullptr}, {"GS", false, false, true, nullptr}},
        {{"210", true, false, true, nullptr}, {"200", false, false, true, nullptr}, {"210", false, false, true, nullptr}, {"DH", false, false, true, nullptr}}
    };
    std::vector<std::vector<Setting>> displayControlsText = {
        {{"Wx", true, false, true, nullptr}, {"Obs", false, false, true, nullptr}, {"Map", true, false, true, nullptr}, {"WHI", false, false, true, nullptr}},
        {{"Hist", true, false, true, nullptr}, {"Radar\nCover", true, false, true, nullptr}, {"Syn\nVideo", false, false, true, nullptr}, {"Bird\nAreas", true, false, true, nullptr}},
        {{"Sel\nDBFld", false, false, true, nullptr}, {"Lead\nDir", false, false, true, nullptr}, {"Color\nLegnd", false, false, true, nullptr}, {"", false, false, true, nullptr}},
        {{"Az\nScale", false, false, true, nullptr}, {"El\nScale", false, false, true, nullptr}, {"Az\nOffst", false, false, true, nullptr}, {"Text\nSize", false, false, true, nullptr}},
        {{"Clear\nHist", false, false, true, nullptr}, {"Set #\nHist", false, false, true, nullptr}, {"", false, false, true, nullptr}, {"Shut\nDown", false, false, true, nullptr}},
    };
    std::vector<std::vector<Setting>> radarControlsText = {
        {{"Maint\nMode", false, false, true, nullptr}, {"Ant.\nDrive", true, false, true, nullptr}, {"Rad-\niate", true, false, true, nullptr}, {"", false, false, true, nullptr}},
        {{"", false, false, true, nullptr}, {"", false, false, true, nullptr}, {"", false, false, true, nullptr}, {"", false, false, true, nullptr}},
        {{"Rain\nMode", true, false, true, nullptr}, {"AzAnt\nElev", false, false, true, nullptr}, {"ElAnt\nAzim", false, false, true, nullptr}, {"STC", false, false, true, nullptr}},
    };
    std::vector<std::vector<Setting>> runwayControlsText = {
        {{"Select", false, false, true, [](CFPNRadarScreen* parent) { runwayChangeHandler(0, 0, parent); }}, {"aerodrome", true, false, true, [](CFPNRadarScreen* parent) { runwayChangeHandler(0, 1, parent); }}, {"with", false, false, true, [](CFPNRadarScreen* parent) { runwayChangeHandler(0, 2, parent); }}},
        {{".fpn", false, false, true, [](CFPNRadarScreen* parent) { runwayChangeHandler(1, 0, parent); }}, {"ICAO", false, false, true, [](CFPNRadarScreen* parent) { runwayChangeHandler(1, 1, parent); }}, {" ", false, false, true, [](CFPNRadarScreen* parent) { runwayChangeHandler(1, 2, parent); }}}
    };

private:
    void drawGSAxes(CDC *pDC, CRect area);
    void drawTrackAxes(CDC* pDC, CRect area);
    void drawInfoText(CDC *pDC, int x, int y);
    void drawVerticalScale(CDC *pDC, CRect glideslopeArea, CRect trackArea, int range);
    void drawHorizontalScale(CDC* pDC, CRect glideslopeArea, CRect trackArea, int range);
    void drawGlidepathAndHorizontalTicks(CDC *pDC, CRect glideslopeArea, CRect trackArea, int range, float angle);  // tan(theta) * 6076 * range = alt at that range
    void drawSettingsBox(CDC* pDC, CRect radarArea,CRect axesArea);

    float elevation;
};

