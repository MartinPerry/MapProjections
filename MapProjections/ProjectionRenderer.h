#ifndef _PROJECTION_DEBUGGER_H_
#define _PROJECTION_DEBUGGER_H_

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

#include "MapProjection.h"

namespace Projections
{
	template <typename Proj>
	class ProjectionRenderer
	{
	public:

		static void ReprojectImage(uint8_t * fromData, uint8_t * toData, const Reprojection & reproj);

		ProjectionRenderer(Proj * proj);
		~ProjectionRenderer();

		void SetProjection(Proj * proj);
		void Clear();

		void SetRawDataTarget(uint8_t * target);

		void AddBorders(const char * fileName, int useEveryNthPoint = 1);
		void DrawBorders();
		void DrawParalells();

		void DrawLine(Coordinate start, Coordinate end, int stepCount = 20);

		void DrawPoint(Coordinate p);

		void DrawLines(const std::vector<Coordinate> & points);

		void DrawImage(uint8_t * imData, int w, int h, Proj * imProj);
		void DrawImage(uint8_t * imData, const Reprojection & reproj);

		void SetPixel(const Pixel<int> & p, uint8_t val);

		void FillData(std::vector<uint8_t> & output);
		void SaveToFile(const char * fileName);

	private:
		static const int INSIDE = 0; // 0000
		static const int LEFT = 1;   // 0001
		static const int RIGHT = 2;  // 0010
		static const int BOTTOM = 4; // 0100
		static const int TOP = 8;    // 1000


		uint8_t * rawData;
		bool externalData;
		Proj * proj;

		std::unordered_map<std::string, std::vector<Coordinate> > debugBorder;

		int ComputeOutCode(double x, double y);
		void CohenSutherlandLineClipAndDraw(double x0, double y0, double x1, double y1);

		std::vector<std::string> Split(const std::string &s, char delim);
		std::string LoadFromFile(const char * filePath);

	};

};

#endif

