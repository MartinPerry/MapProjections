#ifndef _PROJECTION_DEBUGGER_H_
#define _PROJECTION_DEBUGGER_H_

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

#include "Projections.h"
#include "MapProjection.h"

namespace Projections
{
	
	class ProjectionRenderer
	{
	public:

		static void ReprojectImage(uint8_t * fromData, uint8_t * toData, const Reprojection & reproj);

		template <typename Proj>
		ProjectionRenderer(Proj * proj);
		~ProjectionRenderer();

		template <typename Proj>
		void SetProjection(Proj * proj);
		void Clear();

		void SetRawDataTarget(uint8_t * target);

		void AddBorders(const char * fileName, int useEveryNthPoint = 1);
		void DrawBorders();
		void DrawParalells();

		void DrawLine(Coordinate start, Coordinate end, int stepCount = 20);

		void DrawPoint(Coordinate p);

		void DrawLines(const std::vector<Coordinate> & points);

		template <typename Proj>
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

		ProjectionFrame frame;
		std::function<Pixel<int>(const Coordinate & c)> projectCallback;
		std::function<Coordinate(const Pixel<int> & p)> projectInverseCallback;
		std::function<void(const Pixel<int> & start, const Pixel<int> & end, std::function<void(int x, int y)> callback)> lineBresenhamCallback;
	
		std::unordered_map<std::string, std::vector<Coordinate> > debugBorder;

		int ComputeOutCode(double x, double y);
		void CohenSutherlandLineClipAndDraw(double x0, double y0, double x1, double y1);

		std::vector<std::string> Split(const std::string &s, char delim);
		std::string LoadFromFile(const char * filePath);

	};


	/// <summary>
	/// ctor
	/// </summary>
	/// <param name="proj"></param>
	template <typename Proj>
	ProjectionRenderer::ProjectionRenderer(Proj * proj)
		: rawData(nullptr), externalData(false)
	{
		this->SetProjection(proj);
	};

	/// <summary>
	/// Set current projection - rewrites projection from ctor
	/// It also clears current data, because new projection can have
	/// different frame size
	/// </summary>
	/// <param name="proj"></param>
	template <typename Proj>
	void ProjectionRenderer::SetProjection(Proj * proj)
	{
		frame = proj->GetFrame();
		projectCallback = [proj](const Coordinate & c) -> Pixel<int> {
            return proj->template Project<int>(c);
		};
		projectInverseCallback = [proj](const Pixel<int> & p) -> Coordinate {
			return proj->ProjectInverse(p);
		};

		lineBresenhamCallback = [proj](const Pixel<int> & start,
			const Pixel<int> & end,
			std::function<void(int x, int y)> callback) -> void {
			proj->LineBresenham(start, end, callback);
		};


		if (!externalData)
		{
			delete[] rawData;
			rawData = new uint8_t[static_cast<int>(frame.w) * static_cast<int>(frame.h)];
		}
		this->Clear();
	};

	/// <summary>
	/// Draw image based on input projection
	/// For each pixel [x, y] -> calculate inverse based on this projection -> [lat, lon]
	/// Use [lat, lon] on imProj to get pixel coordinates [xx, yy]
	/// Map imData[xx, yy] to currentData[x, y]
	/// </summary>
	/// <param name="imData"></param>
	/// <param name="w"></param>
	/// <param name="h"></param>
	/// <param name="imProj"></param>
	template <typename Proj>
	void ProjectionRenderer::DrawImage(uint8_t * imData, int w, int h, Proj * imProj)
	{
		for (int y = 0; y < static_cast<int>(frame.h); y++)
		{
			for (int x = 0; x < static_cast<int>(frame.w); x++)
			{

				Coordinate cc = this->projectInverseCallback({ x,y });
                Pixel<int> p = imProj->template Project<int>(cc);

				if (p.x < 0) continue;
				if (p.y < 0) continue;
				if (p.x >= w) continue;
				if (p.y >= h) continue;

				this->rawData[x + y * static_cast<int>(frame.w)] = imData[p.x + p.y * w];

			}
		}

	};

};

#endif

