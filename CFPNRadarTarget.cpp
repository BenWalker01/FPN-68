#include "pch.h"
#include "CFPNRadarTarget.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace {
	constexpr size_t kMaxTrailPoints = 15;
	constexpr ULONGLONG kMaxExtrapolationMs = 10000;
	constexpr double kMaxAzimuthDeviationDeg = 30.0;
	constexpr double kMinElevationAngleDeg = -1.0;
	constexpr double kMaxElevationAngleDeg = 7.0;

	double normalizeBearingDifference(double angleDeg) {
		double normalized = std::fmod(angleDeg + 180.0, 360.0);
		if (normalized < 0.0) {
			normalized += 360.0;
		}
		return normalized - 180.0;
	}

	enum class ArrowDirection {
		Up,
		Down,
		Left,
		Right
	};

	void drawDirectionArrow(CDC* pDC, int centerX, int centerY, ArrowDirection direction, COLORREF color) {
		CPen arrowPen(PS_SOLID, 2, color);
		CPen* oldPen = pDC->SelectObject(&arrowPen);

		switch (direction) {
		case ArrowDirection::Up:
			pDC->MoveTo(centerX, centerY + 4);
			pDC->LineTo(centerX, centerY - 4);
			pDC->MoveTo(centerX, centerY - 4);
			pDC->LineTo(centerX - 3, centerY - 1);
			pDC->MoveTo(centerX, centerY - 4);
			pDC->LineTo(centerX + 3, centerY - 1);
			break;
		case ArrowDirection::Down:
			pDC->MoveTo(centerX, centerY - 4);
			pDC->LineTo(centerX, centerY + 4);
			pDC->MoveTo(centerX, centerY + 4);
			pDC->LineTo(centerX - 3, centerY + 1);
			pDC->MoveTo(centerX, centerY + 4);
			pDC->LineTo(centerX + 3, centerY + 1);
			break;
		case ArrowDirection::Left:
			pDC->MoveTo(centerX + 4, centerY);
			pDC->LineTo(centerX - 4, centerY);
			pDC->MoveTo(centerX - 4, centerY);
			pDC->LineTo(centerX - 1, centerY - 3);
			pDC->MoveTo(centerX - 4, centerY);
			pDC->LineTo(centerX - 1, centerY + 3);
			break;
		case ArrowDirection::Right:
			pDC->MoveTo(centerX - 4, centerY);
			pDC->LineTo(centerX + 4, centerY);
			pDC->MoveTo(centerX + 4, centerY);
			pDC->LineTo(centerX + 1, centerY - 3);
			pDC->MoveTo(centerX + 4, centerY);
			pDC->LineTo(centerX + 1, centerY + 3);
			break;
		}

		pDC->SelectObject(oldPen);
	}
}

CFPNRadarTarget::CFPNRadarTarget(std::string callsign, EuroScopePlugIn::CPosition pos, int groundSpeed, int altitude, EuroScopePlugIn::CPosition runwayThreshold, float runwayHeading, int radarRange, float airportElevation, float glideslopeAngle, CRect glideslopeArea, CRect trackArea) {
	this->callsign = callsign;
	this->pos = pos;
	this->groundSpeed = groundSpeed;
	this->altitude = altitude;
	this->runwayThreshold = runwayThreshold;
	this->runwayHeading = runwayHeading;
	this->radarRange = (std::max)(1, radarRange);
	this->airportElevation = airportElevation;
	this->glideslopeAngle = glideslopeAngle;
	this->glideslopeArea = glideslopeArea;
	this->trackArea = trackArea;

	const ULONGLONG now = GetTickCount64();

	latestSamplePos = pos;
	latestSampleAltitude = altitude;
	latestSampleTimeMs = now;
	hasLatestSample = true;

	pastPositions.clear();
	pastPositions.emplace_back(pos, altitude, now);
}

CFPNRadarTarget::~CFPNRadarTarget() {
}

bool CFPNRadarTarget::isVisibleToRadarHeads(const EuroScopePlugIn::CPosition& targetPos, int targetAltitude, const EuroScopePlugIn::CPosition& runwayThreshold, float runwayHeading, int radarRangeNm, float airportElevationFt) {
	const int safeRadarRangeNm = (std::max)(1, radarRangeNm);

	const double distanceToRunwayNm = targetPos.DistanceTo(runwayThreshold);
	const double headingToRunwayDeg = targetPos.DirectionTo(runwayThreshold);
	const double azimuthDeviationDeg = normalizeBearingDifference(static_cast<double>(runwayHeading) - headingToRunwayDeg);
	const double azimuthDeviationRad = azimuthDeviationDeg * (M_PI / 180.0);
	const double alongCenterlineNm = std::cos(azimuthDeviationRad) * distanceToRunwayNm;

	if (alongCenterlineNm <= 0.0 || alongCenterlineNm > static_cast<double>(safeRadarRangeNm)) {
		return false;
	}
	if (std::fabs(azimuthDeviationDeg) > kMaxAzimuthDeviationDeg) {
		return false;
	}

	if (airportElevationFt >= 0.0f) {
		const double apparentElevationFt = static_cast<double>(targetAltitude) - static_cast<double>(airportElevationFt);
		const double alongCenterlineFt = (std::max)(alongCenterlineNm * 6076.0, 1.0);
		const double elevationAngleDeg = std::atan2(apparentElevationFt, alongCenterlineFt) * (180.0 / M_PI);

		if (elevationAngleDeg < kMinElevationAngleDeg || elevationAngleDeg > kMaxElevationAngleDeg) {
			return false;
		}
	}

	return true;
}

void CFPNRadarTarget::updatePosition(EuroScopePlugIn::CPosition pos, int groundSpeed, int altitude, int radarRange, EuroScopePlugIn::CPosition runwayThreshold, EuroScopePlugIn::CPosition otherThreshold, float glideslopeAngle, CRect glideslopeArea, CRect trackArea) {
	this->radarRange = (std::max)(1, radarRange);
	this->runwayThreshold = runwayThreshold;
	this->runwayHeading = runwayThreshold.DirectionTo(otherThreshold);
	this->glideslopeAngle = glideslopeAngle;
	this->glideslopeArea = glideslopeArea;
	this->trackArea = trackArea;
	this->groundSpeed = groundSpeed;

	const ULONGLONG now = GetTickCount64();

	const bool hasNewSample = !hasLatestSample ||
		latestSamplePos.m_Latitude != pos.m_Latitude ||
		latestSamplePos.m_Longitude != pos.m_Longitude ||
		latestSampleAltitude != altitude;

	if (hasNewSample) {
		if (hasLatestSample) {
			previousPos = latestSamplePos;
			previousAltitude = latestSampleAltitude;
			previousSampleTimeMs = latestSampleTimeMs;
			hasPreviousSample = true;
		}

		latestSamplePos = pos;
		latestSampleAltitude = altitude;
		latestSampleTimeMs = now;
		hasLatestSample = true;
	}

	EuroScopePlugIn::CPosition projectedPos = latestSamplePos;
	int projectedAltitude = latestSampleAltitude;

	if (hasPreviousSample && latestSampleTimeMs > previousSampleTimeMs) {
		const ULONGLONG sampleDeltaMs = latestSampleTimeMs - previousSampleTimeMs;
		const ULONGLONG sinceLatestMs = now - latestSampleTimeMs;

		const double extrapolatedMs = (std::min)(
			static_cast<double>(sinceLatestMs),
			static_cast<double>(kMaxExtrapolationMs));
		const double extrapolationFactor = extrapolatedMs / static_cast<double>(sampleDeltaMs);

		projectedPos.m_Latitude = latestSamplePos.m_Latitude + (latestSamplePos.m_Latitude - previousPos.m_Latitude) * extrapolationFactor;
		projectedPos.m_Longitude = latestSamplePos.m_Longitude + (latestSamplePos.m_Longitude - previousPos.m_Longitude) * extrapolationFactor;
		projectedAltitude = latestSampleAltitude + static_cast<int>(std::lround((latestSampleAltitude - previousAltitude) * extrapolationFactor));
	}

	this->pos = projectedPos;
	this->altitude = projectedAltitude;

	pastPositions.emplace_back(this->pos, this->altitude, now);
	if (pastPositions.size() > kMaxTrailPoints) {
		pastPositions.erase(pastPositions.begin());
	}
}

void CFPNRadarTarget::draw(CDC* pDC) {
	if (pastPositions.empty()) {
		return;
	}

	if (!isVisibleToRadarHeads(pos, altitude, runwayThreshold, runwayHeading, radarRange, airportElevation)) {
		return;
	}

	const int safeRadarRange = (std::max)(1, radarRange);
	const size_t highlightedIndex = pastPositions.size() - 1;

	for (size_t index = 0; index < pastPositions.size(); index++) {
		const auto& target = pastPositions[index];
		EuroScopePlugIn::CPosition thisPos = std::get<0>(target);
		int thisAlt = std::get<1>(target);

		if (!isVisibleToRadarHeads(thisPos, thisAlt, runwayThreshold, runwayHeading, safeRadarRange, airportElevation)) {
			continue;
		}

		float distanceToRunway = thisPos.DistanceTo(runwayThreshold);
		float hdgToRunway = thisPos.DirectionTo(runwayThreshold);
		float trackDeviationAngle = runwayHeading - hdgToRunway;
		float range = distanceToRunway * cos(trackDeviationAngle * (M_PI / 180));

		// get X location on screen
		int xAxisHeight = glideslopeArea.bottom + (glideslopeArea.top - glideslopeArea.bottom) / 9;
		int xAxisLeft = glideslopeArea.left + X_AXIS_OFFSET;
		int xPos = xAxisLeft + (glideslopeArea.right - xAxisLeft) * range / safeRadarRange;

		// get Y location on screen
		double apparentElevation = static_cast<double>(thisAlt) - airportElevation;
		int yPos = xAxisHeight + (apparentElevation / (safeRadarRange * 200.0)) * (double)((glideslopeArea.top - glideslopeArea.bottom) * 2 / 9);

		// draw radar blip
		CPen primaryPen(0, 1, PRIMARY_COLOUR);
		pDC->SelectObject(&primaryPen);

		pDC->SetDCBrushColor(PRIMARY_COLOUR);

		pDC->MoveTo(xPos, yPos);
		pDC->Ellipse(xPos - 3, yPos - 3, xPos + 3, yPos + 3);

		if (index == highlightedIndex) { // Vertical tag

			COLORREF originalTextColor = pDC->GetTextColor();
			pDC->SetTextColor(TRACK_DEVIATION_COLOUR);
			pDC->TextOutW(xPos - 3, yPos - 18, _T("A"));

			CPen thickPen(PS_SOLID, 3, TRACK_DEVIATION_COLOUR);
			CPen* pOldPen = pDC->SelectObject(&thickPen);
			pDC->MoveTo(xPos - 10, yPos - 10);
			pDC->LineTo(xPos - 27, yPos - 27);

			pDC->SelectObject(pOldPen);


			CSize textSize = pDC->GetTextExtent(_T("A"));
			int textHeight = textSize.cy;
			int totalTextHeight = textHeight * 3;

			pDC->TextOutW(xPos - 35, yPos - 30 - totalTextHeight, _T("S"));

			//GroundSpeed / distance
			std::wstringstream wss;
			wss << std::fixed << std::setprecision(1) << distanceToRunway;
			std::wstring dist = wss.str();

			std::wstring groundSpeedStr = std::to_wstring(groundSpeed) + L" " + dist;
			CString groundSpeedCStr(groundSpeedStr.c_str());

			pDC->TextOutW(xPos - 75, yPos - 30 - totalTextHeight + textHeight + 3, groundSpeedCStr);

			// Vertical Deviation
			int altDiff = static_cast<int>(apparentElevation - (tan(glideslopeAngle * (M_PI) / 180) * (distanceToRunway * 6076.0f)));

			if (altDiff > 0) { // High
				std::wstringstream ss;
				ss << L"+" << std::setw(3) << std::setfill(L'0') << altDiff;
				std::wstring altOffset = ss.str();
				CString altOffsetCStr = altOffset.c_str();
				int altTextX = xPos - 75;
				int altTextY = yPos - 30 - totalTextHeight + textHeight * 2 + 3;
				pDC->TextOutW(altTextX, altTextY, altOffsetCStr);
				CSize altTextSize = pDC->GetTextExtent(altOffsetCStr);
				drawDirectionArrow(pDC, altTextX + altTextSize.cx + 6, altTextY + textHeight / 2, ArrowDirection::Down, TRACK_DEVIATION_COLOUR);
			}
			else if (altDiff < 0) { // Low
				std::wstringstream ss;
				ss << L"-" << std::setw(3) << std::setfill(L'0') << -altDiff;
				std::wstring altOffset = ss.str();
				CString altOffsetCStr = altOffset.c_str();
				int altTextX = xPos - 75;
				int altTextY = yPos - 30 - totalTextHeight + textHeight * 2 + 3;
				pDC->TextOutW(altTextX, altTextY, altOffsetCStr);
				CSize altTextSize = pDC->GetTextExtent(altOffsetCStr);
				drawDirectionArrow(pDC, altTextX + altTextSize.cx + 6, altTextY + textHeight / 2, ArrowDirection::Up, TRACK_DEVIATION_COLOUR);
			}


			pDC->SetTextColor(originalTextColor);
		}

		int trackXAxisHeight = trackArea.CenterPoint().y;
		int trackXAxisLeft = trackArea.left + X_AXIS_OFFSET;

		xPos = trackXAxisLeft + (trackArea.right - trackXAxisLeft) * range / safeRadarRange;
		yPos = trackXAxisHeight + (tan(trackDeviationAngle * (M_PI / 180)) * 6076.0f * (range / 6000.0f) * (double)((trackArea.top - trackArea.bottom) / 8));

		pDC->MoveTo(xPos, yPos);
		pDC->Ellipse(xPos - 3, yPos - 3, xPos + 3, yPos + 3);
		if (index == highlightedIndex) { // Horizontal tag

			COLORREF originalTextColor = pDC->GetTextColor();
			pDC->SetTextColor(TRACK_DEVIATION_COLOUR);
			pDC->TextOutW(xPos - 3, yPos - 18, _T("A"));

			CPen thickPen(PS_SOLID, 3, TRACK_DEVIATION_COLOUR);
			CPen* pOldPen = pDC->SelectObject(&thickPen);
			pDC->MoveTo(xPos - 10, yPos - 10);
			pDC->LineTo(xPos - 27, yPos - 27);

			pDC->SelectObject(pOldPen);


			CSize textSize = pDC->GetTextExtent(_T("A"));
			int textHeight = textSize.cy;
			int totalTextHeight = textHeight * 3;

			pDC->TextOutW(xPos - 35, yPos - 30 - totalTextHeight, _T("S"));
			// GroundSpeed Dist line
			std::wstringstream wss;
			wss << std::fixed << std::setprecision(1) << distanceToRunway;
			std::wstring dist = wss.str();

			std::wstring groundSpeedStr = std::to_wstring(groundSpeed) + L" " + dist;
			CString groundSpeedCStr(groundSpeedStr.c_str());

			pDC->TextOutW(xPos - 75, yPos - 30 - totalTextHeight + textHeight + 3, groundSpeedCStr);

			// Lateral Deviation
			int lateralOffset = static_cast<int>(tan(trackDeviationAngle * (M_PI / 180.0)) * (distanceToRunway * 6076.0f));

			if (lateralOffset > 0) { // Right of centerline
				std::wstringstream ss;
				ss << L"+" << std::setw(3) << std::setfill(L'0') << lateralOffset;
				std::wstring trackOffset = ss.str();
				CString trackOffsetCStr = trackOffset.c_str();
				int trackTextX = xPos - 75;
				int trackTextY = yPos - 30 - totalTextHeight + textHeight * 2 + 3;
				pDC->TextOutW(trackTextX, trackTextY, trackOffsetCStr);
				CSize trackTextSize = pDC->GetTextExtent(trackOffsetCStr);
				drawDirectionArrow(pDC, trackTextX + trackTextSize.cx + 6, trackTextY + textHeight / 2, ArrowDirection::Left, TRACK_DEVIATION_COLOUR);
			}
			else if (lateralOffset < 0) { // Left
				std::wstringstream ss;
				ss << L"-" << std::setw(3) << std::setfill(L'0') << -lateralOffset;
				std::wstring trackOffset = ss.str();
				CString trackOffsetCStr = trackOffset.c_str();
				int trackTextX = xPos - 75;
				int trackTextY = yPos - 30 - totalTextHeight + textHeight * 2 + 3;
				pDC->TextOutW(trackTextX, trackTextY, trackOffsetCStr);
				CSize trackTextSize = pDC->GetTextExtent(trackOffsetCStr);
				drawDirectionArrow(pDC, trackTextX + trackTextSize.cx + 6, trackTextY + textHeight / 2, ArrowDirection::Right, TRACK_DEVIATION_COLOUR);
			}


			pDC->SetTextColor(originalTextColor);
		}
	}
}
