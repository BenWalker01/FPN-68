#pragma once
#include "EuroScopePlugIn.h"
#include "Constant.h"

#include <string>
#include <vector>
#include "pch.h"
#include <afxpriv.h>
#include <tuple>

class CFPNRadarTarget
{
public:
	CFPNRadarTarget(std::string callsign, EuroScopePlugIn::CPosition pos,int groundsSpeed, int altitude, EuroScopePlugIn::CPosition runwayThreshold, float runwayHeading, int radarRange, float airportElevation,float glideslopeAngle, CRect glideslopeArea, CRect trackArea);  // Threshold alt matters!
	~CFPNRadarTarget();

	void updatePosition(EuroScopePlugIn::CPosition pos, int groundSpeed, int altitude, int radarRange, EuroScopePlugIn::CPosition runwayThreshold, EuroScopePlugIn::CPosition otherThreshold);
	void draw(CDC *pDC);

	std::string callsign;
private:
	EuroScopePlugIn::CPosition pos;
	int groundSpeed;
	int altitude;
	SYSTEMTIME posAltTime;
	EuroScopePlugIn::CPosition runwayThreshold;
	float runwayHeading;
	int radarRange;
	float glideslopeAngle;
	CRect glideslopeArea;
	CRect trackArea;

	EuroScopePlugIn::CPosition previousPos;
	int previousGroundSpeed;
	int previousAltitude = -1;
	SYSTEMTIME previousPosAltTime;

	float airportElevation;
	std::vector<std::tuple<EuroScopePlugIn::CPosition, int, SYSTEMTIME>> pastPositions;
};

