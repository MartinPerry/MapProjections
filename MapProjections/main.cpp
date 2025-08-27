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
#include "./Projections/LambertAzimuthal.h"
#include "./Projections/PolarSteregographic.h"
#include "./Projections/GEOS.h"
#include "./Projections/AEQD.h"


#include "PoleRotationTransform.h"
#include "ProjectionRenderer.h"
#include "MapProjectionUtils.h"
#include "CountriesUtils.h"
#include "lodepng.h"

#include "./simd/avx/ProjectionInfo_avx.h"
#include "./simd/avx/Projections/Miller_avx.h"
#include "./simd/avx/Projections/Mercator_avx.h"
#include "./simd/avx/Projections/GEOS_avx.h"
#include "./simd/avx/Projections/Equirectangular_avx.h"
#include "./simd/avx/Projections/AEQD_avx.h"
#include "./simd/avx/MapProjectionUtils_avx.h"
#include "./simd/avx/Reprojection_avx.h"

#include "./simd/neon/ProjectionInfo_neon.h"
#include "./simd/neon/Projections/Mercator_neon.h"
#include "./simd/neon/Projections/Equirectangular_neon.h"
#include "./simd/neon/MapProjectionUtils_neon.h"
#include "./simd/neon/Reprojection_neon.h"

#include "./tests.h"

using namespace Projections;

namespace nsAvx = Projections::Avx;
namespace nsNeon = Projections::Neon;


void TestLambertAzimuthal()
{
	unsigned w = 600;
	unsigned h = 600;

	//std::vector<uint8_t> imgRawData;
	//std::vector<uint8_t> fileData;
	//lodepng::load_file(fileData, "D://hrrr2.png");
	//lodepng::load_file(fileData, "D://full_disk_ahi_true_color.png");	
	//lodepng::decode(imgRawData, w, h, fileData, LodePNGColorType::LCT_GREY);

	/*
	MinLat = 49.432895
	MinLon = -13.107987
	MaxLat = 61.188428
	MaxLon = 3.646337
	*/


	Projections::Coordinate bbMin, bbMax;

	//======= lambert conic

	bbMin.lat = 49.4329_deg; bbMin.lon = -13.107987_deg;
	bbMax.lat = 61.188428_deg; bbMax.lon = 3.646337_deg;

	//create input projection and set its visible frame
	Projections::LambertAzimuthal* inputImage = new Projections::LambertAzimuthal(-2.5_deg, 54.9_deg);

	//auto kk = inputImage->ProjectInverseInternal(-899.5, -529.5);

	inputImage->SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);


	Projections::Equirectangular* outputImage = new Projections::Equirectangular();
	outputImage->SetFrame(inputImage, false); //same resolution as ipImage frame

	std::cout << outputImage->GetFrame().toString() << std::endl;

	//compute mapping from input -> output projection   	
	Reprojection reprojection = Reprojection<int>::CreateReprojection(inputImage, outputImage);

	CountriesUtils cu;
	cu.Load("D://borders.csv", 5);

	ProjectionRenderer pd(inputImage);
	pd.AddBorders(&cu);	

	pd.Clear();
	pd.DrawParalells(2, 2);
	//pd.DrawImage(&imgRawData[0], ProjectionRenderer::RenderImageType::GRAY, reprojection);
	pd.DrawBorders();
	pd.SaveToFile("D://xxx2.png");

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


void TestVectorization()
{
	Projections::Coordinate bbMin, bbMax;
	bbMin.lat = 47.507_deg; bbMin.lon = -30.850_deg;
	bbMax.lat = 83.507_deg; bbMax.lon = 69.777_deg;

	int w = 800;
	int h = 600;

	Projections::Coordinate gps;
	gps.lat = 60.0_deg;
	gps.lon = 20.0_deg;

	//========================================
	
	nsAvx::Mercator mercAvx;
	nsAvx::Equirectangular eqAvx;

	eqAvx.SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);
	mercAvx.SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);
	
	std::array< Projections::Coordinate, 8> gpsAvx;
	for (int i = 0; i < gpsAvx.size(); i++)
	{
		gpsAvx[i] = gps;
	}

	auto projectedMercSimd = mercAvx.Project(gpsAvx);
	auto projectedEqSimd = eqAvx.Project(gpsAvx);
	
	Reprojection reprojectionAvx = nsAvx::Reprojection<int>::CreateReprojection(&mercAvx, &eqAvx);

	//========================================

	nsNeon::Mercator mercNeon;
	nsNeon::Equirectangular eqNeon;

	eqNeon.SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);
	mercNeon.SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);

	std::array< Projections::Coordinate, 4> tmp4;
	for (int i = 0; i < tmp4.size(); i++)
	{
		tmp4[i] = gps;
	}

	nsNeon::CoordinateNeon gpsNeon = nsNeon::CoordinateNeon::FromArray(tmp4);

	auto projectedMercNeon = mercNeon.Project(gpsNeon);
	auto projectedEqNeon = eqNeon.Project(gpsNeon);

	auto sinCos4 = Coordinate::PrecalcMultipleSinCos(tmp4);

	auto sinCosNeon = nsNeon::CoordinateNeon::PrecalcMultipleSinCos(gpsNeon);

	Reprojection reprojectionNeon = nsNeon::Reprojection<int>::CreateReprojection(&mercNeon, &eqNeon);

	//========================================
}

void drawBorders()
{
	
	unsigned w = 600;
	unsigned h = 600;
	std::vector<uint8_t> imgRawData;
	std::vector<uint8_t> fileData;
	lodepng::load_file(fileData, "D://meteye.png");	
	lodepng::decode(imgRawData, w, h, fileData, LodePNGColorType::LCT_RGBA);

	Projections::Coordinate bbMin, bbMax;
	bbMin.lat = -45.0_deg; bbMin.lon = 111.0_deg;
	bbMax.lat = -9.0_deg; bbMax.lon = 156.0_deg;

	Projections::Equirectangular eq;
	eq.SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false); //same resolution as ipImage frame


	CountriesUtils cu;
	cu.Load("D://borders.csv", 5);

	ProjectionRenderer pd(&eq, ProjectionRenderer::RenderImageType::RGBA);
	pd.AddBorders(&cu);
	pd.Clear();
	//pd.DrawImage(&imgRawData[0], ProjectionRenderer::RenderImageType::GRAY, w, h, &outputImage);
	pd.SetRawDataTarget(&imgRawData[0], ProjectionRenderer::RenderImageType::RGBA);
	pd.DrawBorders();
	pd.SaveToFile("D://xxx2.png");
	
}


void tr07()
{
	//center: 30.4375, 36.266389
	//antppi15.jpg (tr07)

	//center: 29.903333, 40.538333
	//brsppi15.jpg (tr16)


	CountriesUtils cu;
	cu.Load("D://borders.csv", 5);
	auto turBb = cu.GetCountryBoundingBox("TUR");

	//Projections::Equirectangular eq;
	//eq.SetFrame(bbMinTr07, bbMaxTr07, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false); //same resolution as ipImage frame

	Projections::Coordinate bbMin, bbMax;
	bbMin = std::get<0>(turBb);
	bbMax = std::get<1>(turBb);

	Projections::Mercator eq;
	eq.SetFrame(bbMin, bbMax, 1500, 0, Projections::STEP_TYPE::PIXEL_CENTER, true); //same resolution as ipImage frame

	ProjectionRenderer pd(&eq, ProjectionRenderer::RenderImageType::RGB);
	pd.AddBorders(&cu);
	pd.Clear();
	
	unsigned w = 0;
	unsigned h = 0;

	//=====================
	std::vector<uint8_t> imgRawData;
	std::vector<uint8_t> fileData;
	lodepng::load_file(fileData, "D://tr07.png");
	lodepng::decode(imgRawData, w, h, fileData, LodePNGColorType::LCT_RGB);

	Projections::Coordinate bbMinTr07, bbMaxTr07;				
	Projections::AEQD aeqdTr07(30.4375_deg, 36.266389_deg, 370);	
	aeqdTr07.CalcBounds(bbMinTr07, bbMaxTr07);

	std::cout << "Min: " << bbMinTr07 << std::endl;
	std::cout << "Max: " << bbMaxTr07 << std::endl;

	aeqdTr07.SetFrame(bbMinTr07, bbMaxTr07, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);
	auto cc07 = aeqdTr07.Project<int>(Coordinate(Longitude(30.4375_deg), Latitude(36.266389_deg)));
	printf("[%d, %d]\n", cc07.x, cc07.y);
		
	Reprojection reprojectionTr07 = Reprojection<int>::CreateReprojection(&aeqdTr07, &eq);

	pd.DrawImage(imgRawData.data(), Projections::ProjectionRenderer::RenderImageType::RGB, reprojectionTr07);

	//=====================
	imgRawData.clear();
	lodepng::load_file(fileData, "D://tr16.png");
	lodepng::decode(imgRawData, w, h, fileData, LodePNGColorType::LCT_RGB);

	Projections::Coordinate bbMinTr16, bbMaxTr16;
	Projections::AEQD aeqdTr16(29.903333_deg, 40.538333_deg, 370);
	aeqdTr07.CalcBounds(bbMinTr16, bbMaxTr16);

	std::cout << "Min: " << bbMinTr16 << std::endl;
	std::cout << "Max: " << bbMaxTr16 << std::endl;

	aeqdTr16.SetFrame(bbMinTr16, bbMaxTr16, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);
	auto cc16 = aeqdTr16.Project<int>(Coordinate(Longitude(29.903333_deg), Latitude(40.538333_deg)));
	printf("[%d, %d]\n", cc16.x, cc16.y);

	Reprojection reprojectionTr16 = Reprojection<int>::CreateReprojection(&aeqdTr16, &eq);

	pd.DrawImage(imgRawData.data(), Projections::ProjectionRenderer::RenderImageType::RGB, reprojectionTr16);

	//=====================
			
	//pd.SetRawDataTarget(&imgRawData[0], ProjectionRenderer::RenderImageType::RGBA);
	pd.DrawBorders();
	pd.SaveToFile("D://tr07_reproj.png");
}

void testing()
{
	
	//TestGEOS();
	//TestGEOS_AVX();
	//TestGEOS_Neon();

	//TestReprojectEqToMerc();
	//TestReprojectEqToMerc_AVX();
	//TestReprojectEqToMerc_Neon();

	//TestReprojectLambertToEq();

	//TestReprojectionMercToPolar();

	//TestReprojectAEQDToMerc();
	//TestReprojectAEQDToMerc_AVX();

	//TestOblique();

	//TestWrapAround();

	//TestCalculations();
}

int main(int argc, const char* argv[])
{	
	//TestReprojectLambertToEq();
	//tr07();
	//drawBorders();


	return 0;

			
	//TestLambertAzimuthal();

	{
		CountriesUtils cu;
		cu.Load("D://borders.csv", 5);
		auto bb = cu.GetCountryBoundingBox("CZE");


		//create input projection and set its 2D image frame
		Projections::Mercator inputImage = Projections::Mercator();
		inputImage.SetFrame(bb[0], bb[1], 2048, 2048, Projections::STEP_TYPE::PIXEL_CENTER, true);

		Coordinate pragGps;
		pragGps.lat = Latitude::deg(50.071); 
		pragGps.lon = Longitude::deg(14.557);

		ProjectionRenderer pd(&inputImage);
		pd.AddBorders(&cu);
		pd.Clear();
		pd.DrawBorders();
		pd.DrawPoint(pragGps);
		pd.SaveToFile("D://debug_with_border.png");
	}

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

		int n = static_cast<int>(areas.size() / 2);
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

		CountriesUtils cu;
		cu.Load("D://borders.csv", 5);

		ProjectionRenderer pd(eq);
		pd.AddBorders(&cu);
		pd.Clear();
		pd.DrawImage(&imgRawData[0], ProjectionRenderer::RenderImageType::GRAY, w, h, eq);
		pd.DrawBorders();
		pd.SaveToFile("D://xxx2.png");

		printf(".");
	}

	
	return 0;
}
