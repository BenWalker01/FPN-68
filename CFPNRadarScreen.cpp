#include "pch.h"
#include "CFPNRadarScreen.h"
#include "CFPNRadarTarget.h"
#include "SettingsBox.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <afxwin.h>
#include <map>

CFPNRadarScreen::CFPNRadarScreen() {

}

CFPNRadarScreen::~CFPNRadarScreen() = default;

void CFPNRadarScreen::OnAsrContentLoaded(bool loaded) {
	// TODO: load settings
}

void CFPNRadarScreen::OnAsrContentToBeSaved() {
	// TODO: save settings
}

/*
* EuroScope has requested a screen refresh
*/
void CFPNRadarScreen::OnRefresh(HDC hDC, int Phase) {
	if (Phase != EuroScopePlugIn::REFRESH_PHASE_BEFORE_TAGS) return;

	CDC dc;
	dc.Attach(hDC);

	CRect radarArea = GetRadarArea();

	CBrush brush(BACKGROUND_COLOUR);
	dc.FillRect(radarArea, &brush);

	radarArea.DeflateRect(70, 50);  // margins

	// Settings box takes up 1/8 of the screen on the right
	radarArea.right -= radarArea.right / 8;
	
	// Split screen vertically in 2
	CRect glideslopeArea = CRect(radarArea.left, radarArea.top, radarArea.right, radarArea.CenterPoint().y);
	CRect trackArea = CRect(radarArea.left, radarArea.CenterPoint().y, radarArea.right, radarArea.bottom);
	
	// Margins
	glideslopeArea.bottom -= 30;
	trackArea.top += 30;

	// get range from main plugin
	int range = ((CFPNPlugin*)GetPlugIn())->range;

	drawVerticalScale(&dc, glideslopeArea, trackArea, range);
	drawGlidepathAndHorizontalTicks(&dc, glideslopeArea, trackArea, range, 3.0f);
	drawGSAxes(&dc, glideslopeArea);
	drawTrackAxes(&dc, trackArea);

	drawInfoText(&dc, glideslopeArea.left + 80, glideslopeArea.bottom - 10);

	radarArea = GetRadarArea();
	drawSettingsBox(&dc, radarArea , trackArea);

	EuroScopePlugIn::CPosition runwayThreshold = ((CFPNPlugin*)GetPlugIn())->runwayThreshold;
	EuroScopePlugIn::CPosition otherThreshold = ((CFPNPlugin*)GetPlugIn())->otherThreshold;
	float runwayHeading = runwayThreshold.DirectionTo(otherThreshold);

	std::vector<std::string> foundCallsigns = std::vector<std::string>();

	// Draw targets
	for (EuroScopePlugIn::CRadarTarget target = GetPlugIn()->RadarTargetSelectFirst(); target.IsValid(); target = GetPlugIn()->RadarTargetSelectNext(target)) {
		// decide if we should try to draw target

		EuroScopePlugIn::CPosition pos = target.GetPosition().GetPosition();
		int groundSpeed = target.GetGS();
		float hdgToRunway = pos.DirectionTo(runwayThreshold);
		float angleDifference = fmod((hdgToRunway - runwayHeading + 360), 360);
		if (angleDifference > 180) {
			angleDifference = 360 - angleDifference;
		}
		float angleDifferenceRadians = angleDifference * M_PI / 180.0;
		float distAlongCenterline = cos(angleDifferenceRadians) * pos.DistanceTo(runwayThreshold);
		// if behind radar or outside range, don't draw target
		if (distAlongCenterline < 0 || distAlongCenterline > range) continue;

		int altitude = target.GetPosition().GetPressureAltitude();
		float trackDeviationAngle = runwayThreshold.DirectionTo(otherThreshold) - hdgToRunway;

		// if too far outside track or altitude too large, don't draw target
		if (abs(trackDeviationAngle) > 8 || altitude > 6000) continue;

		std::string callsign = target.GetCallsign();
		foundCallsigns.push_back(callsign);

		// if we've already seen them, update their position. Else, add them to our vector of targets.
		bool found = false;
		std::vector<CFPNRadarTarget> *prevTargets = ((CFPNPlugin*)GetPlugIn())->getPreviousTargets();
		for (int i = 0; i < prevTargets->size(); i++) {
			if ((prevTargets->at(i)).callsign == callsign) {
				(prevTargets->at(i)).updatePosition(pos,groundSpeed, altitude, range, runwayThreshold, otherThreshold);
				(prevTargets->at(i)).draw(&dc);

				found = true;
				break;
			}
		}
		if (!found) {
			CFPNRadarTarget targetPlot = CFPNRadarTarget(callsign, pos,groundSpeed, altitude, runwayThreshold, runwayThreshold.DirectionTo(otherThreshold), range, elevation, 3.0f, glideslopeArea, trackArea);
			targetPlot.draw(&dc);
			prevTargets->push_back(targetPlot);
		}
	}

	// Set hover status to false

	for (auto item : { &mainControlsText, &rangeControlsText, &glideControlsText, &displayControlsText, &radarControlsText, &runwayControlsText }) {
		for (int i = 0; i < item->size(); i++) {
			for (int j = 0; j < item->at(i).size(); j++) {
				item->at(i)[j].hover = false;
			}
		}
	}

	dc.Detach();
}

/*
* EuroScope has told us that we're over one of our objects
*/
void CFPNRadarScreen::OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area) {
	std::vector<std::vector<Setting>>* currentControls;
	switch (ObjectType) {
	case SETTINGS_OBJECT_MAIN_CONTROLS:
		currentControls = &mainControlsText;
		break;
	case SETTINGS_OBJECT_RANGE_CONTROLS: 
		currentControls = &rangeControlsText;
		break;
	case SETTINGS_OBJECT_GLIDE_CONTROLS: 
		currentControls = &glideControlsText;
		break;
	case SETTINGS_OBJECT_DISPLAY_CONTROLS: 
		currentControls = &displayControlsText;
		break;
	case SETTINGS_OBJECT_RADAR_CONTROLS: 
		currentControls = &radarControlsText;
		break;
	case SETTINGS_OBJECT_RUNWAY_CONTROLS: 
		currentControls = &runwayControlsText;
		break;
	default: return;  // Some object we don't control / care about
	}

	int rowColId = std::atoi(sObjectId);
	int row = rowColId / 1000;
	int col = rowColId % 1000;

	currentControls->at(row).at(col).hover = true;

	// Redraw now so that we don't wait a second to draw
	RequestRefresh();
}

/*
* When we try to change the range, find that selected range in our list and update it on the display
*/
void CFPNRadarScreen::rangeChangeHandler(int range, CFPNRadarScreen* parent) {
	((CFPNPlugin*)(parent->GetPlugIn()))->range = range;
	std::map<int, int> m{ {1,2}, {3,3}, {5, 1000}, {10, 1001}, {15, 1002}, {20, 1003} };
	int newlyActive = m[range];
	for (int i = 0; i < parent->rangeControlsText.size(); i++) {
		for (int j = 0; j < parent->rangeControlsText[i].size(); j++) {
			if (i != newlyActive / 1000 || j != newlyActive % 1000) {
				parent->rangeControlsText[i][j].selected = false;
			}
		}
	}
}

/*
* When we change the runway, update it
*/
void CFPNRadarScreen::runwayChangeHandler(int r, int c, CFPNRadarScreen* parent) {
	((CFPNPlugin*)(parent->GetPlugIn()))->loadNewAerodrome(((CFPNPlugin*)(parent->GetPlugIn()))->icao.c_str(), parent->runwayControlsText[r][c].text.c_str());

	for (int i = 0; i < parent->runwayControlsText.size(); i++) {
		for (int j = 0; j < parent->runwayControlsText[i].size(); j++) {
			if (r == i && c == j) parent->runwayControlsText[i][j].selected = true;
			else parent->runwayControlsText[i][j].selected = false;
		}
	}
}

/*
* When we click something, we want to select it
*/
void CFPNRadarScreen::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button) {
	std::vector<std::vector<Setting>>* currentControls;
	switch (ObjectType) {
	case SETTINGS_OBJECT_MAIN_CONTROLS:
		currentControls = &mainControlsText;
		break;
	case SETTINGS_OBJECT_RANGE_CONTROLS:
		currentControls = &rangeControlsText;
		break;
	case SETTINGS_OBJECT_GLIDE_CONTROLS:
		currentControls = &glideControlsText;
		break;
	case SETTINGS_OBJECT_DISPLAY_CONTROLS:
		currentControls = &displayControlsText;
		break;
	case SETTINGS_OBJECT_RADAR_CONTROLS:
		currentControls = &radarControlsText;
		break;
	case SETTINGS_OBJECT_RUNWAY_CONTROLS:
		currentControls = &runwayControlsText;
		break;
	default: return;  // Some object we don't control / care about
	}

	int rowColId = std::atoi(sObjectId);
	int row = rowColId / 1000;
	int col = rowColId % 1000;

	currentControls->at(row).at(col).selected = !currentControls->at(row).at(col).selected;

	if (currentControls->at(row).at(col).selected) {
		if (currentControls->at(row).at(col).handler != nullptr) {
			currentControls->at(row).at(col).handler(this);
		}
	}

	RequestRefresh();
}

/*
* Draw X and Y axis for glideslope
*/
void CFPNRadarScreen::drawGSAxes(CDC *pDC, CRect area) {
	CPen axesPen(0, 2, AXES_COLOUR);
	pDC->SelectObject(&axesPen);

	// Y Axis
	pDC->MoveTo(area.left, area.top);
	pDC->LineTo(area.left, area.bottom);
	
	// X Axis
	int xAxisHeight = area.bottom + (area.top - area.bottom) / 9;
	int xAxisLeft = area.left + X_AXIS_OFFSET;
	pDC->MoveTo(xAxisLeft, xAxisHeight);
	pDC->LineTo(area.right, xAxisHeight);
}

/*
* Draw X and Y axes for track
*/
void CFPNRadarScreen::drawTrackAxes(CDC* pDC, CRect area) {
	CPen axesPen(0, 2, AXES_COLOUR);
	pDC->SelectObject(&axesPen);

	// Y Axis
	pDC->MoveTo(area.left, area.top);
	pDC->LineTo(area.left, area.bottom);

	// X Axis
	int xAxisHeight = area.CenterPoint().y;
	int xAxisLeft = area.left + X_AXIS_OFFSET;
	pDC->MoveTo(xAxisLeft, xAxisHeight);
	pDC->LineTo(area.right, xAxisHeight);
}

/*
* Draw vertical scale (ticks) for glideslope and tracks
*/
void CFPNRadarScreen::drawVerticalScale(CDC* pDC, CRect glideslopeArea, CRect trackArea, int range) {
	CPen axesPen(0, 2, AXES_COLOUR);
	pDC->SelectObject(&axesPen);

	CFont font;
	font.CreatePointFont(100, L"VCR OSD Mono", pDC);
	pDC->SetTextColor(AXES_COLOUR);
	pDC->SetTextAlign(TA_RIGHT);
	auto* defFont = pDC->SelectObject(&font);

	// Glideslope

	// Bottom tick
	pDC->MoveTo(glideslopeArea.left - 8, glideslopeArea.bottom);
	pDC->LineTo(glideslopeArea.left + 8, glideslopeArea.bottom);
	pDC->TextOutW(glideslopeArea.left - 13, glideslopeArea.bottom - 9, ("-" + std::to_string(range * 100)).c_str());

	// Top ticks
	for (int i = 1; i <= 4; i++) {
		int yPos = glideslopeArea.bottom + (glideslopeArea.top - glideslopeArea.bottom) * (2 * i + 1) / 9;
		pDC->MoveTo(glideslopeArea.left - 8, yPos);
		pDC->LineTo(glideslopeArea.left + 8, yPos);
		pDC->TextOutW(glideslopeArea.left - 13, yPos - 7, std::to_string(i * 200 * range).c_str());
	}

	// Track
	for (int i = 0; i <= 8; i++) {
		if (i == 4) continue; // Don't draw a tick in the middle

		int yPos = trackArea.bottom + (trackArea.top - trackArea.bottom) * i / 8;
		pDC->MoveTo(trackArea.left - 8, yPos);
		pDC->LineTo(trackArea.left + 8, yPos);
		pDC->TextOutW(trackArea.left - 13, yPos - 7, std::to_string((i - 4) * 400 * range).c_str());
	}

	pDC->SelectObject(defFont);
	font.DeleteObject();
}

/*
* Draw the glidepath, and all the horizontal ticks
*/
void CFPNRadarScreen::drawGlidepathAndHorizontalTicks(CDC* pDC, CRect glideslopeArea, CRect trackArea, int range, float angle) {
	// Glideslope
	CPen glideslopePen(0, 2, GLIDESLOPE_COLOUR);
	pDC->SelectObject(&glideslopePen);

	int xAxisHeight = glideslopeArea.bottom + (glideslopeArea.top - glideslopeArea.bottom) / 9;
	int xAxisLeft = glideslopeArea.left + X_AXIS_OFFSET;

	int topOfGS = xAxisHeight // base X axis height
				+ tan(angle * (M_PI / 180))  // trig to get height
				  * 6076.0f // feet -> miles
				  * 0.005f // height scaling factor
				  * ((double)(glideslopeArea.top - glideslopeArea.bottom) * 2 / 9);  // gap between two ticks

	pDC->MoveTo(xAxisLeft, xAxisHeight);
	pDC->LineTo(glideslopeArea.right, topOfGS);

	// Draw track paths @ 1.25 and 3 degrees
	CPen trackDeviationPen(0, 1, TRACK_DEVIATION_COLOUR);
	pDC->SelectObject(&trackDeviationPen);

	int trackXAxisHeight = trackArea.CenterPoint().y;
	int trackXAxisLeft = trackArea.left + X_AXIS_OFFSET;

	for (int i = 0; i < 4; i++) {
		float angle;
		if (i == 0) angle = 3.0f;
		else if (i == 1) angle = 1.25f;
		else if (i == 2) angle = -1.25f;
		else if (i == 3) angle = -3.0f;

		int trackOffset = trackXAxisHeight + tan(angle * (M_PI / 180)) * 6076.0f * 0.0025f * ((double)(trackArea.top - trackArea.bottom) / 8);

		pDC->MoveTo(trackXAxisLeft, trackXAxisHeight);
		pDC->LineTo(trackArea.right, trackOffset);
	}

	// Both axis ticks and text
	CPen axesPen(0, 2, AXES_COLOUR);
	pDC->SelectObject(&axesPen);

	CFont font;
	font.CreatePointFont(120, L"VCR OSD Mono", pDC);
	pDC->SetTextColor(AXES_COLOUR);
	pDC->SetTextAlign(TA_CENTER);
	auto* defFont = pDC->SelectObject(&font);

	for (int i = 0; i <= range * 2; i++) { // each "i" is equivalent to half a mile
		int xPos = xAxisLeft + (glideslopeArea.right - xAxisLeft) * i / (range * 2);
		int displacement = i % 2 == 0 ? 8 : 4;  // Larger ticks on every second tick
		
		// Glideslope
		pDC->MoveTo(xPos, xAxisHeight - displacement);
		pDC->LineTo(xPos, xAxisHeight + displacement);
		if (i <= 6) {  // 0-3 miles, draw text every half a tick
			if (i % 2 == 0) pDC->TextOutW(xPos, xAxisHeight + 9, std::to_string(i / 2).c_str());
			else {
				std::ostringstream oss;
				oss << std::fixed << std::setprecision(1) << ((i / 2) + 0.5);
				pDC->TextOutW(xPos, xAxisHeight + 9, oss.str().c_str());
			}				
		} // otherwise draw text every tick
		else if (i % 2 == 0) pDC->TextOutW(xPos, xAxisHeight + 9, std::to_string(i / 2).c_str());

		// draw gear down (4 mile) and coordination (8 mile) lines
		if (i == 8 || i == 16){
			CPen redPen(PS_SOLID, 2, RGB(255, 0, 0));
			CPen* pOldPen = pDC->SelectObject(&redPen);
			
			pDC->MoveTo(xPos, trackXAxisHeight - displacement); 
			pDC->LineTo(xPos, trackXAxisHeight - displacement * 5);

			pDC->SelectObject(pOldPen);

		}

		// Now the same for track
		pDC->MoveTo(xPos, trackXAxisHeight - displacement);
		pDC->LineTo(xPos, trackXAxisHeight + displacement);
		if (i <= 6) {
			if (i % 2 == 0) pDC->TextOutW(xPos, trackXAxisHeight + 9, std::to_string(i / 2).c_str());
			else {
				std::ostringstream oss;
				oss << std::fixed << std::setprecision(1) << ((i / 2) + 0.5);
				pDC->TextOutW(xPos, trackXAxisHeight + 9, oss.str().c_str());
			}			
		}
		else if (i % 2 == 0) pDC->TextOutW(xPos, trackXAxisHeight + 9, std::to_string(i / 2).c_str());
	}

	// GS Ticks
	pDC->SelectObject(&glideslopePen);

	for (int i = 0; i <= range * 2; i++) {
		int xPos = xAxisLeft + (glideslopeArea.right - xAxisLeft) * i / (range * 2);
		int displacement = i % 2 == 0 ? 8 : 4;
		int yPos = xAxisHeight + (tan(angle * (M_PI / 180)) * 6076.0f * (((float)i/2.0f) / (range * 200)) * (double)((glideslopeArea.top - glideslopeArea.bottom) * 2 / 9));
		pDC->MoveTo(xPos, yPos - displacement);  // GS
		pDC->LineTo(xPos, yPos + displacement);
	}

	pDC->SelectObject(defFont);
	font.DeleteObject();
}

/*
* Draw green info text on screen
*/
void CFPNRadarScreen::drawInfoText(CDC* pDC, int x, int y) {
	CFont font;
	font.CreatePointFont(130, L"VCR OSD Mono", pDC);
	pDC->SetTextColor(RGB(200, 255, 211));
	pDC->SetTextAlign(TA_LEFT);
	auto* defFont = pDC->SelectObject(&font);

	std::string text = "GS: 3.0  RWY: ";

	std::stringstream stream;
	stream << std::fixed << std::setprecision(1) << ((CFPNPlugin*)GetPlugIn())->runwayThreshold.DirectionTo(((CFPNPlugin*)GetPlugIn())->otherThreshold);

	text += stream.str();
	text += "  DH: 200FT     TIME : ";

	SYSTEMTIME st;
	GetSystemTime(&st);

	text += (st.wHour < 10 ? "0" : "") + std::to_string(st.wHour) + ":"
		 + (st.wMinute < 10 ? "0" : "") + std::to_string(st.wMinute) + ":"
		 + (st.wSecond < 10 ? "0" : "") + std::to_string(st.wSecond);

	pDC->TextOutW(x, y, text.c_str());

	pDC->SelectObject(defFont);
	font.DeleteObject();
}
/*
* Draw settings box on right hand side of screen
*/
void CFPNRadarScreen::drawSettingsBox(CDC* pDC, CRect radarArea, CRect axesArea) {
	// Calculate the area for the main settings box
	CRect settingsBoxArea;
	settingsBoxArea.left = axesArea.right + 10;
	settingsBoxArea.top = radarArea.top;
	settingsBoxArea.right = radarArea.right;
	settingsBoxArea.bottom = radarArea.bottom;

	settingsBoxArea.DeflateRect(5, 5);
	CBrush whiteBrush(RGB(255, 255, 255));
	CBrush* pOldBrush = pDC->SelectObject(&whiteBrush);
	pDC->FillSolidRect(settingsBoxArea, RGB(255, 255, 255));
	pDC->SelectObject(pOldBrush);

	CRect blueBoxArea = settingsBoxArea;
	blueBoxArea.DeflateRect(2,2,2,2);

	CBrush blueBrush(RGB(0, 0, 255));
	pOldBrush = pDC->SelectObject(&blueBrush);
	pDC->FillSolidRect(blueBoxArea, RGB(30, 56, 247));
	pDC->SelectObject(pOldBrush);

	// Draw windows

	CPoint freeSpace(settingsBoxArea.left - 3, settingsBoxArea.bottom);
	int width = settingsBoxArea.right - settingsBoxArea.left;

	// Main controls
	SettingsBox mainControls(this, SETTINGS_OBJECT_MAIN_CONTROLS, freeSpace, "Main Controls                        RPS", mainControlsText, width);
	freeSpace = mainControls.Draw(pDC);

	// Range Controls
	SettingsBox rangeScale(this, SETTINGS_OBJECT_RANGE_CONTROLS, freeSpace, "Range Scale (nmi)", rangeControlsText, width, true);
	freeSpace = rangeScale.Draw(pDC);

	// GlideSlope / DescHight
	SettingsBox glide(this, SETTINGS_OBJECT_GLIDE_CONTROLS, freeSpace, "Glide Slope / Decsn Hgt", glideControlsText, width, true);
	freeSpace = glide.Draw(pDC);

	// Display Controls
	SettingsBox display(this, SETTINGS_OBJECT_DISPLAY_CONTROLS, freeSpace, "Display Controls", displayControlsText, width, false,true);
	freeSpace = display.Draw(pDC);

	// Radar Controls
	SettingsBox radar(this, SETTINGS_OBJECT_RADAR_CONTROLS, freeSpace, "Radar Controls", radarControlsText, width, false, true);
	freeSpace = radar.Draw(pDC);

	// Runway Controls
	SettingsBox runway(this, SETTINGS_OBJECT_RUNWAY_CONTROLS, freeSpace, "Select Runway", runwayControlsText, width, true, true);
	freeSpace = runway.Draw(pDC);
}