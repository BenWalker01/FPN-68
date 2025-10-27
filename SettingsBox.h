#pragma once
#include <string>
#include <vector>
#include <afxwin.h>
#include <afxpriv.h>
#include "CFPNRadarScreen.h"

class SettingsBox {
public:
    SettingsBox(CFPNRadarScreen *parent, int objectType, CPoint start, const std::string& title, const std::vector<std::vector<CFPNRadarScreen::Setting>>& setting, int maxWidth, bool shortBox = false, bool closeBox = false);
    CPoint Draw(CDC* pDC);

private:
    CFPNRadarScreen *m_parent;
    int m_objectType;
    CPoint m_start;
    
    int m_width;
    int m_maxWidth;
    bool m_shortBox;
    bool m_closeBox;
    std::string m_title;
    std::vector<std::vector<CFPNRadarScreen::Setting>> m_setting;

};