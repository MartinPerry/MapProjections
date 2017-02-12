#ifndef _PROJECTION_DEBUGGER_H_
#define _PROJECTION_DEBUGGER_H

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

#include "MapProjection.h"

class ProjectionRenderer
{
public:
	ProjectionRenderer(IProjectionInfo * proj);
	~ProjectionRenderer();

	void SetProjection(IProjectionInfo * proj);
	void Clear();

	void AddBorders(const char * fileName, int useEveryNthPoint = 1);
	void DrawBorders();
	void DrawParalells();

	void DrawLine(IProjectionInfo::Coordinate start,
		IProjectionInfo::Coordinate end, int stepCount = 20);

	void DrawPoint(IProjectionInfo::Coordinate p);

	void DrawLines(const std::vector<IProjectionInfo::Coordinate> & points);

	void DrawImage(uint8_t * imData, int w, int h, IProjectionInfo * imProj);

	void DrawImage(uint8_t * imData, const IProjectionInfo::Reprojection & reproj);
	

	void SetPixel(const IProjectionInfo::Pixel & p, uint8_t val);

	void FillData(std::vector<uint8_t> & output);
	void SaveToFile(const char * fileName);

private:
	static const int INSIDE = 0; // 0000
	static const int LEFT = 1;   // 0001
	static const int RIGHT = 2;  // 0010
	static const int BOTTOM = 4; // 0100
	static const int TOP = 8;    // 1000


	uint8_t * rawData;
	IProjectionInfo * proj;
	
	std::unordered_map<std::string, std::vector<IProjectionInfo::Coordinate> > debugBorder;

	int ComputeOutCode(double x, double y);
	void CohenSutherlandLineClipAndDraw(double x0, double y0, double x1, double y1);

	std::vector<std::string> Split(const std::string &s, char delim);
	std::string LoadFromFile(const char * filePath);
	
};

#endif

