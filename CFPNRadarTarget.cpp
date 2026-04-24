#include "pch.h"
#include "CFPNRadarTarget.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace {
	constexpr size_t kMaxTrailPoints = 5;
	constexpr double kMaxExtrapolationRatio = 0.8;
	constexpr ULONGLONG kMaxExtrapolationMs = 1200;
	constexpr double kSmoothingTimeConstantMs = 140.0;
	constexpr double kMinSmoothingAlpha = 0.12;
	constexpr double kMaxSmoothingAlpha = 0.9;
	constexpr double kMaxAzimuthDeviationDeg = 8.0;
	constexpr double kMaxElevationAngleDeg = 7.0;

	double clampValue(double value, double minValue, double maxValue) {
		return (std::max)(minValue, (std::min)(value, maxValue));
	}

	EuroScopePlugIn::CPosition lerpPosition(const EuroScopePlugIn::CPosition& from, const EuroScopePlugIn::CPosition& to, double alpha) {
		EuroScopePlugIn::CPosition result;
		result.m_Latitude = from.m_Latitude + (to.m_Latitude - from.m_Latitude) * alpha;
		result.m_Longitude = from.m_Longitude + (to.m_Longitude - from.m_Longitude) * alpha;
		return result;
	}

	int lerpInt(int from, int to, double alpha) {
		return static_cast<int>(std::lround(from + (to - from) * alpha));
	}

	double normalizeBearingDifference(double angleDeg) {
		double normalized = std::fmod(angleDeg + 180.0, 360.0);
		if (normalized < 0.0) {
			normalized += 360.0;
		}
		return normalized - 180.0;
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

	smoothedPos = pos;
	smoothedAltitude = altitude;
	lastSmoothingTimeMs = now;
	hasSmoothedState = true;

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

		if (elevationAngleDeg > kMaxElevationAngleDeg) {
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

		const double cappedLeadMs = (std::min)(
			static_cast<double>(kMaxExtrapolationMs),
			static_cast<double>(sampleDeltaMs) * kMaxExtrapolationRatio);
		const double extrapolatedMs = clampValue(static_cast<double>(sinceLatestMs), 0.0, cappedLeadMs);
		const double extrapolationFactor = extrapolatedMs / static_cast<double>(sampleDeltaMs);

		projectedPos.m_Latitude = latestSamplePos.m_Latitude + (latestSamplePos.m_Latitude - previousPos.m_Latitude) * extrapolationFactor;
		projectedPos.m_Longitude = latestSamplePos.m_Longitude + (latestSamplePos.m_Longitude - previousPos.m_Longitude) * extrapolationFactor;
		projectedAltitude = latestSampleAltitude + static_cast<int>(std::lround((latestSampleAltitude - previousAltitude) * extrapolationFactor));
	}

	if (!hasSmoothedState) {
		smoothedPos = projectedPos;
		smoothedAltitude = projectedAltitude;
		hasSmoothedState = true;
		lastSmoothingTimeMs = now;
	}
	else {
		const ULONGLONG frameDeltaMs = now - lastSmoothingTimeMs;
		const double rawAlpha = 1.0 - std::exp(-(static_cast<double>(frameDeltaMs) / kSmoothingTimeConstantMs));
		const double alpha = clampValue(rawAlpha, kMinSmoothingAlpha, kMaxSmoothingAlpha);

		smoothedPos = lerpPosition(smoothedPos, projectedPos, alpha);
		smoothedAltitude = lerpInt(smoothedAltitude, projectedAltitude, alpha);
		lastSmoothingTimeMs = now;
	}

	this->pos = smoothedPos;
	this->altitude = smoothedAltitude;

	pastPositions.emplace_back(this->pos, this->altitude, now);
	if (pastPositions.size() > kMaxTrailPoints) {
		pastPositions.erase(pastPositions.begin());
	}
}

void CFPNRadarTarget::draw(CDC* pDC) {
	if (pastPositions.empty()) {
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
				ss << L"+" << std::setw(3) << std::setfill(L'0') << altDiff << L"\u2193";
				std::wstring altOffset = ss.str();
				CString altOffsetCStr = altOffset.c_str();
				pDC->TextOutW(xPos - 75, yPos - 30 - totalTextHeight + textHeight * 2 + 3, altOffsetCStr);
			}
			else if (altDiff < 0) { // Low
				std::wstringstream ss;
				ss << L"-" << std::setw(3) << std::setfill(L'0') << -altDiff << L"\u2191";
				std::wstring altOffset = ss.str();
				CString altOffsetCStr = altOffset.c_str();
				pDC->TextOutW(xPos - 75, yPos - 30 - totalTextHeight + textHeight * 2 + 3, altOffsetCStr);
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
				ss << L"+" << std::setw(3) << std::setfill(L'0') << lateralOffset << L"\u2190";
				std::wstring trackOffset = ss.str();
				CString trackOffsetCStr = trackOffset.c_str();
				pDC->TextOutW(xPos - 75, yPos - 30 - totalTextHeight + textHeight * 2 + 3, trackOffsetCStr);
			}
			else if (lateralOffset < 0) { // Left
				std::wstringstream ss;
				ss << L"-" << std::setw(3) << std::setfill(L'0') << -lateralOffset << L"\u2192";
				std::wstring trackOffset = ss.str();
				CString trackOffsetCStr = trackOffset.c_str();
				pDC->TextOutW(xPos - 75, yPos - 30 - totalTextHeight + textHeight * 2 + 3, trackOffsetCStr);
			}


			pDC->SetTextColor(originalTextColor);
		}
	}
}
