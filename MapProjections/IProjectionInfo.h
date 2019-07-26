#ifndef IPROJECTION_INFO_H
#define IPROJECTION_INFO_H

#include "MapProjectionStructures.h"

//#define USE_VIRTUAL_INTERFACE

#ifdef USE_VIRTUAL_INTERFACE
#define OVERRIDE override
#else
#define OVERRIDE
#endif

namespace Projections 
{
	class IProjectionInfo
	{
	public:

		const PROJECTION curProjection;

		virtual ~IProjectionInfo() = default;

#ifdef USE_VIRTUAL_INTERFACE
		/*
		template <typename PixelType = int>
		virtual RET_VAL(PixelType, std::is_integral) Project(const Coordinate & c) const = 0;


		template <typename PixelType = float>
		virtual RET_VAL(PixelType, std::is_floating_point) Project(const Coordinate & c) const = 0;


		template <typename PixelType = int, bool Normalize = true>
		virtual Coordinate ProjectInverse(const Pixel<PixelType> & p) const = 0;

		template <typename InputProj>
		virtual void SetFrame(InputProj * proj, bool keepAR = true) = 0;

		template <typename InputProj>
		virtual void SetFrame(InputProj * proj, double w, double h, bool keepAR = true) = 0;
		*/

		virtual void SetFrame(const ProjectionFrame & frame) = 0;
		virtual void SetFrame(std::vector<Coordinate> coord, MyRealType w, MyRealType h, bool keepAR = true) = 0;
		virtual void SetFrame(Coordinate minCoord, Coordinate maxCoord, MyRealType w, MyRealType h, bool keepAR = true) = 0;

		virtual Coordinate GetTopLeftCorner() const = 0;
		virtual Coordinate CalcStep(STEP_TYPE type) const = 0;
		virtual const ProjectionFrame & GetFrame() const = 0;

		virtual Coordinate CalcEndPointShortest(Coordinate start, Angle bearing, MyRealType dist) const = 0;
		virtual Coordinate CalcEndPointDirect(Coordinate start, Angle bearing, MyRealType dist) const = 0;

		virtual void LineBresenham(Pixel<int> start, Pixel<int> end,
			std::function<void(int x, int y)> callback) const = 0;

		virtual void ComputeAABB(Coordinate & min, Coordinate & max) const = 0;
#endif
	protected:
		IProjectionInfo(PROJECTION curProjection) : curProjection(curProjection) {};
	};

}

#endif
