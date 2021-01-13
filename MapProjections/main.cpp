#include <chrono>
#include <iostream>
#include <array>
#include <vector>
#include <algorithm>

#include "./ProjectionInfo.h"
#include "./Reprojection.h"

#include "./Projections/Equirectangular.h"
#include "./Projections/Mercator.h"
#include "./Projections/Miller.h"
#include "./Projections/LambertConic.h"
#include "./Projections/PolarSteregographic.h"
#include "./Projections/GEOS.h"
#include "ProjectionRenderer.h"
#include "lodepng.h"

#include "./simd/ProjectionInfo_simd.h"
#include "./simd/Projections/Miller_simd.h"
#include "./simd/Projections/Mercator_simd.h"
#include "./simd/Projections/GEOS_simd.h"
#include "./simd/Projections/Equirectangular_simd.h"
#include "./simd/MapProjectionUtils_simd.h"
#include "./simd/Reprojection_simd.h"

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
	Reprojection reprojection = Reprojection<int>::CreateReprojection(inputImage, outputImage);
	
	pd.Clear();
	pd.DrawImage(&imgRawData[0], ProjectionRenderer::RenderImageType::GRAY, reprojection);
	pd.DrawBorders();
	pd.SaveToFile("D://xxx2.png");

}


void TestGEOS_Simd()
{
	unsigned w = 600;
	unsigned h = 600;
	std::vector<uint8_t> imgRawData;
	std::vector<uint8_t> fileData;
	lodepng::load_file(fileData, "D://ttt.png");
	lodepng::decode(imgRawData, w, h, fileData, LodePNGColorType::LCT_GREY);


	Projections::Coordinate bbMin, bbMax;
	bbMin.lat = -45.0_deg; bbMin.lon = -135.0_deg;
	bbMax.lat = 45.0_deg; bbMax.lon = -10.0_deg;
	
	ns::Mercator mercator;
	mercator.SetFrame(bbMin, bbMax, 4200, 0, Projections::STEP_TYPE::PIXEL_CENTER, false);

	// Coordinate bbMin, bbMax;	
	bbMin.lat = -90.0_deg; bbMin.lon = -180.0_deg;
	bbMax.lat = 90.0_deg; bbMax.lon = 180.0_deg;

	ns::GEOS geos(GEOS::SatelliteSettings::Goes16());
	geos.SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);
	

	
	Projections::Reprojection<short> reproj = ns::Reprojection<short>::CreateReprojection<ns::GEOS, ns::Mercator>(&geos, &mercator);

	std::vector<uint8_t> rawData = reproj.ReprojectData<uint8_t, std::vector<uint8_t>, 1>(imgRawData.data(), 0);

	
	ProjectionRenderer pd(&mercator, ProjectionRenderer::RenderImageType::GRAY);
	pd.AddBorders("D://borders.csv", 5);
	//pd.Clear();	
	pd.SetRawDataTarget(rawData.data(), ProjectionRenderer::RenderImageType::GRAY);
	pd.DrawBorders();
	pd.SaveToFile("D://yyy_simd.png");
	
}

void TestGEOS()
{

	unsigned w = 600;
	unsigned h = 600;
	std::vector<uint8_t> imgRawData;
	std::vector<uint8_t> fileData;
	lodepng::load_file(fileData, "D://ttt.png");
	lodepng::decode(imgRawData, w, h, fileData, LodePNGColorType::LCT_GREY);


	Projections::Coordinate bbMin, bbMax;
	bbMin.lat = -45.0_deg; bbMin.lon = -135.0_deg;
	bbMax.lat = 45.0_deg; bbMax.lon = -10.0_deg;

	Mercator mercator;
	mercator.SetFrame(bbMin, bbMax, 4200, 0, Projections::STEP_TYPE::PIXEL_CENTER, false);

	// Coordinate bbMin, bbMax;	
	bbMin.lat = -90.0_deg; bbMin.lon = -180.0_deg;
	bbMax.lat = 90.0_deg; bbMax.lon = 180.0_deg;

	GEOS geos(GEOS::SatelliteSettings::Goes16());
	geos.SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);

	
	Projections::Reprojection<short> reproj = Reprojection<short>::CreateReprojection<GEOS, Mercator>(&geos, &mercator);

	std::vector<uint8_t> rawData = reproj.ReprojectData<uint8_t, std::vector<uint8_t>, 1>(imgRawData.data(), 0);


	ProjectionRenderer pd(&mercator, ProjectionRenderer::RenderImageType::GRAY);
	pd.AddBorders("D://borders.csv", 5);
	//pd.Clear();	
	pd.SetRawDataTarget(rawData.data(), ProjectionRenderer::RenderImageType::GRAY);
	pd.DrawBorders();
	pd.SaveToFile("D://yyy.png");
	
}

void ReprojectNightImage() 
{
	unsigned w = 13500;
	unsigned h = 6750;
	

	Projections::Coordinate bbMin, bbMax;
	bbMin.lat = -90.0_deg; bbMin.lon = -180.0_deg;
	bbMax.lat = 90.0_deg; bbMax.lon = 180.0_deg;

	ns::Equirectangular eq;
	eq.SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);

	// Coordinate bbMin, bbMax;	
	bbMin.lat = -70.0_deg; bbMin.lon = -20.0_deg;
	bbMax.lat = 70.0_deg; bbMax.lon = 78.0_deg;

	ns::Mercator mercator;
	mercator.SetFrame(bbMin, bbMax, 2232, 0, Projections::STEP_TYPE::PIXEL_CENTER, false);



	Projections::Reprojection<short> reproj = 
		ns::Reprojection<short>::CreateReprojection<ns::Equirectangular, ns::Mercator>(&eq, &mercator);

	//==========================================================

	std::vector<uint8_t> imgRawData;
	std::vector<uint8_t> fileData;
	lodepng::load_file(fileData, "D://BlackMarble_2016_3km.png");
	lodepng::decode(imgRawData, w, h, fileData, LodePNGColorType::LCT_RGB);


	std::vector<uint8_t> rawData = 
		reproj.ReprojectData<uint8_t, std::vector<uint8_t>, 3>(
			imgRawData.data(), 0
		);


	ProjectionRenderer pd(&mercator, ProjectionRenderer::RenderImageType::RGB);
	pd.AddBorders("D://borders.csv", 5);
	//pd.Clear();	
	pd.SetRawDataTarget(rawData.data(), ProjectionRenderer::RenderImageType::RGB);
	//pd.DrawBorders();
	pd.SaveToFile("D://yyy_simd2.png");

}


void LambertConicPressureEu()
{
	unsigned w = 600;
	unsigned h = 600;

	std::vector<uint8_t> imgRawData;
	std::vector<uint8_t> fileData;
	lodepng::load_file(fileData, "D://BRA_1_2017031700_45.png");
	//lodepng::load_file(fileData, "D://full_disk_ahi_true_color.png");	
	lodepng::decode(imgRawData, w, h, fileData, LodePNGColorType::LCT_GREY);



	Projections::Coordinate bbMin, bbMax;

	//======= lambert conic

	//min: 82.940;-39.023
	//max: 18.229;7.998

	bbMax.lat = 82.940_deg; 
	bbMin.lat = 18.229_deg;

	bbMin.lon = -39.023_deg;		
	bbMax.lon = 7.998_deg;

	//create input projection and set its visible frame
	Projections::LambertConic* inputImage = new Projections::LambertConic(38.5_deg, -97.5_deg, 38.5_deg);

	//auto kk = inputImage->ProjectInverseInternal(-899.5, -529.5);

	inputImage->SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);


	ProjectionRenderer pd(inputImage);
	//compute mapping from input -> output projection   
		
	pd.Clear();
	pd.SetRawDataTarget(&imgRawData[0], ProjectionRenderer::RenderImageType::GRAY);
	pd.DrawBorders();
	pd.SaveToFile("D://xxx2.png");
	return;


	Projections::Equirectangular* outputImage = new Projections::Equirectangular();
	outputImage->SetFrame(inputImage, false); //same resolution as ipImage frame


	ProjectionRenderer pd2(outputImage);
	//compute mapping from input -> output projection   
	Reprojection reprojection = Reprojection<int>::CreateReprojection(inputImage, outputImage);

	pd2.Clear();
	pd2.DrawImage(&imgRawData[0], ProjectionRenderer::RenderImageType::GRAY, reprojection);
	pd.DrawBorders();
	pd2.SaveToFile("D://xxx2.png");
}

void TestWrapAround()
{
	unsigned w = 0;
	unsigned h = 0;

	std::vector<uint8_t> imgRawData;
	std::vector<uint8_t> fileData;
	lodepng::load_file(fileData, "D://8081_earthmap2k.png");
	lodepng::decode(imgRawData, w, h, fileData, LodePNGColorType::LCT_GREY);



	Projections::Coordinate bbMin, bbMax;

	bbMin.lat = -80.93_deg;
	bbMin.lon = -650.0_deg;
	//bbMin.lon = 70.0_deg;

	bbMax.lat = 80.06_deg;
	bbMax.lon = -150.0_deg;
	//bbMax.lon = 610.0_deg;

	//create input projection and set its visible frame
	auto merc = new Projections::Mercator();
	merc->SetFrame(bbMin, bbMax, 2880, 1441, Projections::STEP_TYPE::PIXEL_BORDER, true);

	bbMin.lat = -90.0_deg;
	bbMin.lon = -180.0_deg;

	bbMax.lat = 90.0_deg;
	bbMax.lon = 180.0_deg;
	auto eq = new Projections::Equirectangular();
	eq->SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_BORDER, false);

	auto repro = Reprojection<int>::CreateReprojection(eq, merc);

	ProjectionRenderer pd(merc);
	pd.AddBorders("D://borders.csv", 5);
	pd.Clear();
	pd.DrawImage(imgRawData.data(), ProjectionRenderer::RenderImageType::GRAY, repro);
	pd.DrawBorders();
	pd.SaveToFile("D://wrap.png");
}

int main(int argc, const char * argv[]) 
{

	TestWrapAround();

	
	//LambertConicPressureEu();
	return 0;

	{
		Projections::Coordinate bbMin, bbMax;

		bbMin.lat = -89.93_deg; bbMin.lon = -180.06_deg;
		bbMax.lat = 90.06_deg; bbMax.lon = 179.93_deg;

		//create input projection and set its visible frame
		Projections::Equirectangular * eq = new Projections::Equirectangular();

		//auto kk = inputImage->ProjectInverseInternal(-899.5, -529.5);

		eq->SetFrame(bbMin, bbMax, 2880, 1441, Projections::STEP_TYPE::PIXEL_BORDER, false);
		auto f = eq->GetFrame();
		auto gps = eq->ProjectInverse({ 1437, 330 });

		//{ {1349, 224}, {1337, 341}, {1455, 341}, {1455, 241} }

		std::vector<double> areas;
		double avg = 0;
		for (int y = 0; y < eq->GetFrameHeight() - 10; y += 10)		
		{
			for (int x = 0; x < eq->GetFrameWidth() - 10; x += 10)
			{
				double area = ProjectionUtils::CalcArea(
					std::vector<Pixel<int>>({ {x, y}, {x + 10, y}, {x + 10, y + 10}, {x, y + 10} }),
					eq
				);
				areas.push_back(area / (1000 * 1000));
				avg += areas.back();
			}
		}

		avg /= areas.size();

		int n = areas.size() / 2;
		std::nth_element(areas.begin(), areas.begin() + n, areas.end());
		double medianArea = areas[n];

		double area = ProjectionUtils::CalcArea(
			std::vector<Pixel<int>>({ {1337, 224}, {1337, 341}, {1455, 341}, {1455, 241} }),
			eq
		);

		double area2 = ProjectionUtils::CalcArea(
			std::vector<Pixel<int>>({ {0, 424}, {0, 541}, {118, 541}, {118, 441} }),
			eq
		);

		unsigned w = 600;
		unsigned h = 600;
		std::vector<uint8_t> imgRawData;
		std::vector<uint8_t> fileData;
		lodepng::load_file(fileData, "D://orig_icon_global_single-level_2020102400_012_PMSL.png");
		//lodepng::load_file(fileData, "D://full_disk_ahi_true_color.png");	
		lodepng::decode(imgRawData, w, h, fileData, LodePNGColorType::LCT_GREY);


		ProjectionRenderer pd(eq);
		pd.AddBorders("D://borders.csv", 5);
		pd.Clear();
		pd.DrawImage(&imgRawData[0], ProjectionRenderer::RenderImageType::GRAY, w, h, eq);
		pd.DrawBorders();
		pd.SaveToFile("D://xxx2.png");

		printf(".");
	}

	return 0;

	//ReprojectNightImage();
	//TestGEOS();
	//TestGEOS_Simd();
	//return 0;

	{


		Projections::Coordinate bbMin, bbMax;
		
		bbMin.lat = -70.0_deg; bbMin.lon = 34.0_deg;
		bbMax.lat = 70.0_deg; bbMax.lon = 78.0_deg;

		Projections::Mercator * eq = new Projections::Mercator();

		eq->SetFrame(bbMin, bbMax, 1002, 0, Projections::STEP_TYPE::PIXEL_BORDER, false);
		auto f = eq->GetFrame();
		printf("x");
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
	Reprojection reprojection = Reprojection<int>::CreateReprojection(&g, mercator);
	
		

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
