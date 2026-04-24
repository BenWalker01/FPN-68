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
	CFPNRadarTarget(std::string callsign, EuroScopePlugIn::CPosition pos,int groundsSpeed, int altitude, EuroScopePlugIn::CPosition runwayThreshold, float runwayHeading, int radarRange, float airportElevation,float glideslopeAngle, CRect glideslopeArea, CRect trackArea);  // Threshold alt matters!!!
	~CFPNRadarTarget();

	static bool isVisibleToRadarHeads(const EuroScopePlugIn::CPosition& targetPos, int targetAltitude, const EuroScopePlugIn::CPosition& runwayThreshold, float runwayHeading, int radarRangeNm, float airportElevationFt);
	void updatePosition(EuroScopePlugIn::CPosition pos, int groundSpeed, int altitude, int radarRange, EuroScopePlugIn::CPosition runwayThreshold, EuroScopePlugIn::CPosition otherThreshold, float glideslopeAngle, CRect glideslopeArea, CRect trackArea);
	void draw(CDC *pDC);

	std::string callsign;
private:
	EuroScopePlugIn::CPosition pos;
	int groundSpeed = 0;
	int altitude = 0;
	EuroScopePlugIn::CPosition runwayThreshold;
	float runwayHeading;
	int radarRange = 1;
	float glideslopeAngle;
	CRect glideslopeArea;
	CRect trackArea;

	EuroScopePlugIn::CPosition previousPos;
	int previousAltitude = 0;
	ULONGLONG previousSampleTimeMs = 0;
	bool hasPreviousSample = false;

	EuroScopePlugIn::CPosition latestSamplePos;
	int latestSampleAltitude = 0;
	ULONGLONG latestSampleTimeMs = 0;
	bool hasLatestSample = false;

	EuroScopePlugIn::CPosition smoothedPos;
	int smoothedAltitude = 0;
	ULONGLONG lastSmoothingTimeMs = 0;
	bool hasSmoothedState = false;

	float airportElevation;
	std::vector<std::tuple<EuroScopePlugIn::CPosition, int, ULONGLONG>> pastPositions;
};

