#include <chrono>
#include <iostream>
#include <array>
#include <vector>

#include "ProjectionInfo.h"
#include "./Projections/Equirectangular.h"
#include "./Projections/Mercator.h"
#include "./Projections/Miller.h"
#include "./Projections/LambertConic.h"
#include "./Projections/GEOS.h"
#include "ProjectionRenderer.h"
#include "lodepng.h"

#include "./simd/ProjectionInfo_simd.h"
#include "./simd/Projections/Miller_simd.h"
#include "./simd/Projections/Mercator_simd.h"
#include "./simd/MapProjectionUtils_simd.h"

using namespace Projections;

namespace ns = Projections::Simd;


void TestLambertConic()
{
	unsigned w = 600;
	unsigned h = 600;

	std::vector<uint8_t> imgRawData;
	std::vector<uint8_t> fileData;
	lodepng::load_file(fileData, "D://hrrr2.png");
	//lodepng::load_file(fileData, "D://full_disk_ahi_true_color.png");	
	lodepng::decode(imgRawData, w, h, fileData, LodePNGColorType::LCT_GREY);



	Projections::Coordinate bbMin, bbMax;

	//======= lambert conic

	bbMin.lat = 21.1381_deg; bbMin.lon = -122.72_deg;
	bbMax.lat = 47.84364_deg; bbMax.lon = -60.90137_deg;

	bbMin.lat = 21.140547_deg; bbMin.lon = -134.09548_deg;
	bbMax.lat = 52.6132742_deg; bbMax.lon = -60.9365_deg;

	//create input projection and set its visible frame
	Projections::LambertConic * inputImage = new Projections::LambertConic(38.5_deg, -97.5_deg, 38.5_deg);

	//auto kk = inputImage->ProjectInverseInternal(-899.5, -529.5);

	inputImage->SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);


	Projections::Equirectangular * outputImage = new Projections::Equirectangular();
	outputImage->SetFrame(inputImage, false); //same resolution as ipImage frame


	ProjectionRenderer pd(outputImage);
	//compute mapping from input -> output projection   
	Reprojection reprojection = Projections::ProjectionUtils::CreateReprojection(inputImage, outputImage);
	
	pd.Clear();
	pd.DrawImage(&imgRawData[0], ProjectionRenderer::RenderImageType::GRAY, reprojection);
	pd.DrawBorders();
	pd.SaveToFile("D://xxx2.png");

}



int main(int argc, const char * argv[]) 
{

	{


		Projections::Coordinate bbMin, bbMax;
	
		bbMin.lat = 10.0_deg; bbMin.lon = -134.0_deg;
		bbMax.lat = 50.0_deg; bbMax.lon = -61.0_deg;

		Projections::Mercator * eq = new Projections::Mercator();

		eq->SetFrame(bbMin, bbMax, 5400, 0, Projections::STEP_TYPE::PIXEL_BORDER, false);
		auto f = eq->GetFrame();
		printf("x");
	}

	{
		Projections::Coordinate bbMin, bbMax;
	
		bbMin.lat = -89.93_deg; bbMin.lon = -180.06_deg;
		bbMax.lat = 90.06_deg; bbMax.lon = 179.93_deg;

		//create input projection and set its visible frame
		Projections::Equirectangular * eq = new Projections::Equirectangular();

		//auto kk = inputImage->ProjectInverseInternal(-899.5, -529.5);

		eq->SetFrame(bbMin, bbMax, 720, 360, Projections::STEP_TYPE::PIXEL_BORDER, false);
		auto f = eq->GetFrame();
		printf(".");
	}

	//TestLambertConic();

	return 0;
	//=======

    unsigned w = 600;
    unsigned h = 600;
    
		
    /*
    std::array<Projections::Pixel<int>, 8> p;
    ns::Mercator mercSimd;
    ns::Miller millerSimd;
    mercSimd.ProjectInverse(p);
    mercSimd.GetFrameWidth();
    
    Reprojection reprojectionSimd = Projections::Simd::ProjectionUtils::CreateReprojection(&millerSimd, &mercSimd);
    */
    
   
    //===================================================
    //Build input projection
    //===================================================
	Projections::Coordinate bbMin, bbMax;

   // Coordinate bbMin, bbMax;
    bbMin.lat = -90.0_deg; bbMin.lon = -180.0_deg;
    bbMax.lat = 90.0_deg; bbMax.lon = 180.0_deg;
    

	GEOS g(GEOS::SatelliteSettings::Himawari8());
	g.SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);

	Coordinate c;

	//new zeeland
	c.lat = -47.5_deg;
	c.lon = 167.6_deg;
	//[400,536] +-
	//[3662,4925] +-
	auto pp1 = g.Project<double>(c);
	auto pp1i = g.ProjectInverse(pp1);

	//hawai
	c.lat = 19.0_deg; //lat je preklopena znamenkem
	c.lon = -155.6_deg;
	//[563,193] +-
	//[5187,1808] +-
	auto pp2 = g.Project<double>(c);

	auto pp2i = g.ProjectInverse(pp2);

	//auto xx = g.ProjectInternal(c);

	//auto x = 5005.5 + xx.x * std::pow(2, -16) * 40932513;
	//auto y = 5005.5 + xx.y * std::pow(2, -16) * 40932513;

	//Mercator latitude in <-85, 85>

	bbMin.lat = -70.0_deg; bbMin.lon = 77.0_deg;
	bbMax.lat = 74.0_deg; bbMax.lon = 179.9_deg;

	if (bbMin.lat.deg() < -85) bbMin.lat = -85.0_deg;
	if (bbMax.lat.deg() > 85) bbMax.lat = 85.0_deg;

	Mercator * mercator = new Mercator();
	mercator->SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, true);

	ProjectionRenderer pd(mercator);
	//compute mapping from input -> output projection   
	Reprojection reprojection = Projections::ProjectionUtils::CreateReprojection(&g, mercator);
	
		

	std::vector<uint8_t> imgRawData;	
	std::vector<uint8_t> fileData;
	lodepng::load_file(fileData, "D://h8fulldisk.png");
	//lodepng::load_file(fileData, "D://full_disk_ahi_true_color.png");	
	lodepng::decode(imgRawData, w, h, fileData, LodePNGColorType::LCT_GREY);


	pd.Clear();
	pd.DrawImage(&imgRawData[0], ProjectionRenderer::RenderImageType::GRAY, reprojection);
	pd.DrawBorders();
	pd.SaveToFile("D://xxx.png");
	

	return 0;
}
