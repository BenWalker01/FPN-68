#pragma once
#include "pch.h"
#include "SettingsBox.h"
#include "Constant.h"
#include <vector>
#include <string>
#include <sstream>

SettingsBox::SettingsBox(CFPNRadarScreen *parent, int objectType, CPoint start, const std::string& title, const std::vector<std::vector<CFPNRadarScreen::Setting>>& setting, int maxWidth, bool shortBox, bool closeBox)
    : m_parent(parent), m_objectType(objectType), m_start(start),m_title(title), m_setting(setting), m_maxWidth(maxWidth), m_shortBox(shortBox), m_closeBox(closeBox) {}

CPoint SettingsBox::Draw(CDC* pDC) {
    int boxHeight;
    int titleBoxHeight = 30;
    if (m_shortBox) {
        boxHeight = 30;
    }
    else {
        boxHeight = 40;
    }

    int padding = 5;
    int m_height = m_setting.size();
    int m_width;
    if (m_height > 0) {
        m_width = m_setting[0].size();
    }
    else {
        m_width = 0;
    }

    m_start.y -= ((m_height * boxHeight) + titleBoxHeight); // create space for text boxes + title box

    // Define the main rectangle
    CRect boxRect(m_start.x, m_start.y, m_start.x + m_maxWidth, m_start.y + (m_height * boxHeight) + titleBoxHeight);
    boxRect.DeflateRect(10, 10);
    boxRect.OffsetRect(0, 2);

    // Create and select the white pen
    CPen whitePen(PS_SOLID, padding, RGB(244, 244, 244));
    CPen* oldPen = pDC->SelectObject(&whitePen);

    // Select the null brush
    CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

    // Draw the main rectangle
    pDC->Rectangle(boxRect);

    // Restore the old pen and brush
    pDC->SelectObject(oldPen);
    pDC->SelectObject(oldBrush);

    // Define the light blue rectangle across the top, padded from the white border
    CPoint topRectPoint(boxRect.left , boxRect.top);
    CSize topRectSize(boxRect.Width(), titleBoxHeight);
    CRect topRect(topRectPoint,topRectSize);
    topRect.DeflateRect(padding, padding);

    // Create and select the light blue pen and brush
    CPen lightBluePen(PS_SOLID, 1, RGB(28, 171, 239));
    oldPen = pDC->SelectObject(&lightBluePen);
    CBrush lightBlueBrush(RGB(28, 171, 239));
    oldBrush = pDC->SelectObject(&lightBlueBrush);

    // Draw the title rectangle
    pDC->Rectangle(topRect);
    CString text(m_title.c_str());
    pDC->SetTextColor(RGB(246, 246, 220));
    pDC->DrawText(text, topRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // define settings box widths
    auto boxWidth = (boxRect.Width() - 4 - padding) / m_width;
    boxHeight = (boxRect.Height() - titleBoxHeight) / m_height;

    CPen blackPen(PS_SOLID, 1, RGB(0, 0, 0));
    CBrush blackBrush(RGB(0, 0, 0));

    pDC->SelectObject(&blackPen);
    pDC->SelectObject(&blackBrush);
    // Draw boxes
    for (int row = 0; row < m_height; ++row) {
        for (int col = 0; col < m_width; ++col) {
            CFPNRadarScreen::Setting setting = m_setting[row][col];
            if (setting.text.empty()) {
                continue;
            }

            bool flipColors = false;
            if (setting.selected) { // is the box selected
                flipColors = true;
            }

            int left = topRect.left + (col * boxWidth);
            int top = topRect.bottom + (row * boxHeight);
            int right = left + boxWidth;
            int bottom = top + boxHeight;

            CRect subBox(left, top, right, bottom);
            subBox.DeflateRect(2, 2);

            std::string rowcolid = std::to_string(row * 1000 + col);  // just easier than messing around with m_width/m_height
            m_parent->AddScreenObject(m_objectType, rowcolid.c_str(), subBox, false, "");

            // Set colors based on flipColors
            COLORREF boxColor = flipColors ? RGB(246, 246, 220) : RGB(92, 91, 106);
            COLORREF textColor = flipColors ? RGB(92, 91, 106) : RGB(246, 246, 220);

            CBrush brush(boxColor);
            CPen pen(PS_SOLID, 1, boxColor);

            if (setting.hover) {
                if (setting.inop) {
                    CBrush outerBrush(RGB(230, 42, 42));
                    CPen outerPen(PS_SOLID, 1, RGB(230, 42, 42));
                    pDC->SelectObject(&outerBrush);
                    pDC->SelectObject(&outerPen);
                    pDC->Rectangle(subBox);

                    // INOP text
                    pDC->SetTextColor(RGB(246, 246, 220));

                    CString cstrText(L"INOP");
                    CRect lineRect(subBox.left, subBox.top, subBox.right, subBox.top + subBox.Height());
                    pDC->DrawText(cstrText, lineRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                    continue;
                }
                else {
                    CBrush outerBrush(textColor);
                    CPen outerPen(PS_SOLID, 1, textColor);
                    pDC->SelectObject(&outerBrush);
                    pDC->SelectObject(&outerPen);
                    pDC->Rectangle(subBox);

                    subBox.DeflateRect(3, 3);
                    pDC->SelectObject(&pen);
                    pDC->SelectObject(&brush);
                    pDC->Rectangle(subBox);
                    subBox.InflateRect(3, 3);
                }
            }
            else {
                pDC->SelectObject(&pen);
                pDC->SelectObject(&brush);
                pDC->Rectangle(subBox);
            }

            // Split the text on '\n'
            std::istringstream stream(setting.text);
            std::string line;
            int lineCount = std::count(setting.text.begin(), setting.text.end(), '\n') + 1;
            int lineHeight = subBox.Height() / lineCount;
            int currentTop = subBox.top;

            // Set text color
            pDC->SetTextColor(textColor);

            while (std::getline(stream, line)) {
                CString cstrText(line.c_str());
                CRect lineRect(subBox.left, currentTop, subBox.right, currentTop + lineHeight);
                pDC->DrawText(cstrText, lineRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                currentTop += lineHeight;
            }
        }
    }

    // Restore the old pen and brush
    pDC->SelectObject(oldPen);
    pDC->SelectObject(oldBrush);


    // Return the new bottom-left point of the main rectangle
    return CPoint(m_start.x, boxRect.top + padding);
}