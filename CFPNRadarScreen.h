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

    inline void OnAsrContentToBeClosed(void) override {
        delete this;
    }

    void setElevation(float elevation) {
        this->elevation = elevation;
    }

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

