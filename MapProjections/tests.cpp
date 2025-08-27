#include "./tests.h"

#include <vector>
#include <iostream>

//================================================================
// Standard
//================================================================

#include "./Projections/Equirectangular.h"
#include "./Projections/Mercator.h"
#include "./Projections/Miller.h"
#include "./Projections/LambertConic.h"
#include "./Projections/LambertAzimuthal.h"
#include "./Projections/PolarSteregographic.h"
#include "./Projections/GEOS.h"
#include "./Projections/AEQD.h"

//================================================================
// AVX
//================================================================

#include "./simd/avx/ProjectionInfo_avx.h"
#include "./simd/avx/Projections/Miller_avx.h"
#include "./simd/avx/Projections/Mercator_avx.h"
#include "./simd/avx/Projections/GEOS_avx.h"
#include "./simd/avx/Projections/Equirectangular_avx.h"
#include "./simd/avx/Projections/AEQD_avx.h"
#include "./simd/avx/MapProjectionUtils_avx.h"
#include "./simd/avx/Reprojection_avx.h"

//================================================================
// Neon
//================================================================

#include "./simd/neon/ProjectionInfo_neon.h"
#include "./simd/neon/Projections/Mercator_neon.h"
#include "./simd/neon/Projections/Equirectangular_neon.h"
#include "./simd/neon/MapProjectionUtils_neon.h"
#include "./simd/neon/Reprojection_neon.h"

//================================================================

#include "./PoleRotationTransform.h"
#include "./ProjectionRenderer.h"
#include "./MapProjectionUtils.h"
#include "./CountriesUtils.h"
#include "./lodepng.h"

//================================================================

using namespace Projections;

namespace nsAvx = Projections::Avx;
namespace nsNeon = Projections::Neon;

static const std::string TEST_DATA_DIR = "d:\\Martin\\Programming\\test\\MapProjections\\TestData\\";

//================================================================

std::vector<uint8_t> LoadPngAsGray(const std::string& filePath, unsigned& w, unsigned& h)
{
	std::vector<uint8_t> imgRawData;
	std::vector<uint8_t> fileData;
	lodepng::load_file(fileData, filePath);
	lodepng::decode(imgRawData, w, h, fileData, LodePNGColorType::LCT_GREY);

	return imgRawData;
}

std::vector<uint8_t> LoadPngRgb(const std::string& filePath, unsigned& w, unsigned& h)
{
	std::vector<uint8_t> imgRawData;
	std::vector<uint8_t> fileData;
	lodepng::load_file(fileData, filePath);
	lodepng::decode(imgRawData, w, h, fileData, LodePNGColorType::LCT_RGB);

	return imgRawData;
}

template <typename Output>
void Save(Output* outputProj, 
	std::vector<uint8_t>& rawData, 
	ProjectionRenderer::RenderImageType type, 	
	const char* outputFileName,
	uint8_t borderVal = 255)
{
	CountriesUtils cu;
	cu.Load("D://borders.csv", 5);

	ProjectionRenderer pd(outputProj, type);
	pd.SetPixelVal(borderVal);	
	pd.AddBorders(&cu);
	//pd.Clear();	
	pd.SetRawDataTarget(rawData.data(), type);
	pd.DrawBorders();
	pd.SaveToFile(outputFileName);
}

//================================================================

template <typename Input, typename Output, template <class> class Reproj>
void TestGeos(const char* outputFileName)
{	
	unsigned w = 0;
	unsigned h = 0;

	std::vector<uint8_t> imgRawData = LoadPngRgb(TEST_DATA_DIR + "goes16.png", w, h);
	
	Coordinate bbMin, bbMax;	
	bbMin.lat = -90.0_deg; bbMin.lon = -180.0_deg;
	bbMax.lat = 90.0_deg; bbMax.lon = 180.0_deg;

	Input geos(GEOS::SatelliteSettings::Goes16());
	geos.SetFrame(bbMin, bbMax, w, h, STEP_TYPE::PIXEL_CENTER, false);


	
	bbMin.lat = -45.0_deg; bbMin.lon = -135.0_deg;
	bbMax.lat = 45.0_deg; bbMax.lon = -10.0_deg;

	Output mercator;
	mercator.SetFrame(bbMin, bbMax, 4200, 0, STEP_TYPE::PIXEL_CENTER, false);

	
	auto reproj = Reproj<short>::CreateReprojection<Input, Output>(&geos, &mercator);

	std::vector<uint8_t> rawData = reproj.ReprojectDataNerestNeighbor<uint8_t, std::vector<uint8_t>, 3>(imgRawData.data(), 0);

	Save(&mercator, rawData, ProjectionRenderer::RenderImageType::RGB, outputFileName);
}

void TestGEOS()
{
	std::cout << "TestGEOS" << std::endl;

	TestGeos<GEOS, Mercator, Reprojection>("D://goes16_to_mercator_cpu.png");
}

void TestGEOS_AVX()
{
	std::cout << "TestGEOS_AVX" << std::endl;

	TestGeos<nsAvx::GEOS, nsAvx::Mercator, nsAvx::Reprojection>("D://goes16_to_mercator_avx.png");
}

void TestGEOS_Neon()
{
	std::cout << "TestGEOS_Neon" << std::endl;

	TestGeos<GEOS, nsNeon::Mercator, Reprojection>("D://goes16_to_mercator_neon.png");
}

//================================================================

template <typename Input, typename Output, template <class> class Reproj>
void TestReprojectEqToMerc(const char* outputFileName)
{
	unsigned w = 0;
	unsigned h = 0;
	
	std::vector<uint8_t> imgRawData = LoadPngRgb(TEST_DATA_DIR + "8081_earthmap2k.png", w, h);

	Projections::Coordinate bbMin, bbMax;
	bbMin.lat = -90.0_deg; bbMin.lon = -180.0_deg;
	bbMax.lat = 90.0_deg; bbMax.lon = 180.0_deg;

	Input eq;
	eq.SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);

	// Coordinate bbMin, bbMax;	
	bbMin.lat = -70.0_deg; bbMin.lon = -20.0_deg;
	bbMax.lat = 70.0_deg; bbMax.lon = 78.0_deg;

	Output mercator;
	mercator.SetFrame(bbMin, bbMax, 2232, 0, Projections::STEP_TYPE::PIXEL_CENTER, false);


	auto reproj = Reproj<short>::CreateReprojection<Input, Output>(&eq, &mercator);

	//==========================================================

	
	std::vector<uint8_t> rawData = reproj.ReprojectDataNerestNeighbor<uint8_t, std::vector<uint8_t>, 3>(imgRawData.data(), 0);

	Save(&mercator, rawData, ProjectionRenderer::RenderImageType::RGB, outputFileName);
}

void TestReprojectEqToMerc()
{
	std::cout << "TestReprojectEqToMerc" << std::endl;

	TestReprojectEqToMerc<Equirectangular, Mercator, Reprojection>("D://reproj_eq_mercator_cpu.png");
}

void TestReprojectEqToMerc_AVX()
{
	std::cout << "TestReprojectEqToMerc_AVX" << std::endl;

	TestReprojectEqToMerc<nsAvx::Equirectangular, nsAvx::Mercator, nsAvx::Reprojection>("D://reproj_eq_mercator_evx.png");
}

void TestReprojectEqToMerc_Neon()
{
	std::cout << "TestReprojectEqToMerc_Neon" << std::endl;

	TestReprojectEqToMerc<nsNeon::Equirectangular, nsNeon::Mercator, nsNeon::Reprojection>("D://reproj_eq_mercator_neon.png");
}

//================================================================

template <typename Input, typename Output, template <class> class Reproj>
void TestReprojectLambertToEq(const char* outputFileName)
{
	//hrr data projection

	unsigned w = 1799;
	unsigned h = 1059;

	std::vector<uint8_t> imgRawData = std::vector<uint8_t>(w * h, 255);

	Projections::Coordinate bbMin, bbMax;	
	bbMin.lat = 21.140547_deg; bbMin.lon = -134.09548_deg;
	bbMax.lat = 52.6132742_deg; bbMax.lon = -60.9365_deg;

	Input lam(38.5_deg, -97.5_deg, 38.5_deg);
	lam.SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);
	
	Output outputImage;
	outputImage.SetFrame(&lam, false); //same resolution as ipImage frame


	auto reproj = Reproj<short>::CreateReprojection<Input, Output>(&lam, &outputImage);

	//==========================================================


	std::vector<uint8_t> rawData = reproj.ReprojectDataNerestNeighbor<uint8_t, std::vector<uint8_t>, 1>(imgRawData.data(), 0);

	Save(&outputImage, rawData, ProjectionRenderer::RenderImageType::GRAY, outputFileName, 128);
}

void TestReprojectLambertToEq()
{
	std::cout << "TestReprojectLambertToEq" << std::endl;

	TestReprojectLambertToEq<LambertConic, Equirectangular, Reprojection>("D://reproj_lam_eq_cpu.png");
}

//================================================================

template <typename Input, typename Output, template <class> class Reproj>
void TestReprojectAEQDToMerc(const char* outputFileName)
{
	//hrr data projection

	unsigned w = 0;
	unsigned h = 0;

	std::vector<uint8_t> imgRawData = LoadPngRgb(TEST_DATA_DIR + "tr07.png", w, h);

	Projections::Coordinate bbMin, bbMax;
	
	Input aeqd(30.4375_deg, 36.266389_deg, 370);
	aeqd.CalcBounds(bbMin, bbMax);

	aeqd.SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false);
	
	auto cc = aeqd.Project<int>(Coordinate(Longitude(30.4375_deg), Latitude(36.266389_deg)));
	std::cout << "Projected: [" << cc.x << ", " << cc.y << "]" <<std::endl;
	std::cout << "Reference: [360, 360]" << std::endl;

	
	Output outputImage;
	outputImage.SetFrame(&aeqd, false); //same resolution as ipImage frame


	auto reproj = Reproj<short>::CreateReprojection<Input, Output>(&aeqd, &outputImage);

	//==========================================================


	std::vector<uint8_t> rawData = reproj.ReprojectDataNerestNeighbor<uint8_t, std::vector<uint8_t>, 3>(imgRawData.data(), 0);

	Save(&outputImage, rawData, ProjectionRenderer::RenderImageType::RGB, outputFileName);
}


void TestReprojectAEQDToMerc()
{
	std::cout << "TestReprojectAEQDToMerc" << std::endl;

	TestReprojectAEQDToMerc<AEQD, Mercator, Reprojection>("D://reproj_aeqd_merc_cpu.png");
}

void TestReprojectAEQDToMerc_AVX()
{
	std::cout << "TestReprojectAEQDToMerc_AVX" << std::endl;

	TestReprojectAEQDToMerc<nsAvx::AEQD, nsAvx::Mercator, nsAvx::Reprojection>("D://reproj_aeqd_merc_avx.png");
}

//================================================================

void TestReprojectionMercToPolar()
{
	Projections::Coordinate botLeft, topRight;
	botLeft.lat = 50.0_deg; botLeft.lon = 10.0_deg;
	topRight.lat = 53.0_deg; topRight.lon = 15.0_deg;

	
	//create input projection and set its 2D image frame
	Projections::Mercator inputImage;
	inputImage.SetFrame(botLeft, topRight, 2048, 2048, Projections::STEP_TYPE::PIXEL_CENTER, true);


	botLeft.lat = 46.1929_deg;
	botLeft.lon = 4.6759_deg;

	topRight.lat = 55.5342_deg;
	topRight.lon = 17.1126_deg;

	Projections::PolarSteregographic outputImage(10.0_deg, 60.0_deg);
	outputImage.SetFrame(botLeft, topRight, 900, 1100, Projections::STEP_TYPE::PIXEL_CENTER, false); //same resolution as ipImage frame


	auto reproj = Projections::Reprojection<int>::CreateReprojection(&inputImage, &outputImage);

	unsigned w = 2048;
	unsigned h = 2048;
	std::vector<uint8_t> imgRawData = std::vector<uint8_t>(w * h, 255);
	

	auto reprojData = reproj.ReprojectDataNerestNeighbor<uint8_t>(imgRawData.data(), 0);

	std::vector<uint8_t> rawData = reproj.ReprojectDataNerestNeighbor<uint8_t, std::vector<uint8_t>, 1>(imgRawData.data(), 0);

	Save(&outputImage, rawData, ProjectionRenderer::RenderImageType::GRAY, "D://merc_to_polar_cpu.png", 128);
	
}

//================================================================

void TestOblique()
{
	std::cout << "TestOblique" << std::endl;

	//knmi_eu data projection
	
	/*
		:grid_south_pole_latitude = -35.0; // double
		:grid_south_pole_longitude = -8.0; // double
		:grid_south_pole_angle = 0.0; // double
	*/

	
	unsigned int w = 676;
	unsigned int h = 564;

	std::vector<uint8_t> imgRawData = std::vector<uint8_t>(w * h, 255);
	

	Projections::Coordinate bbMin, bbMax;
	Projections::Coordinate newPole;

	//--------
	//proj="ob_tran", o_lat_p=south_pole_lat, o_lon_p=south_pole_lon, lon_0=rotation_angle

	//+proj=ob_tran +o_proj=longlat +lon_0=-40 +o_lat_p=22 +R=6.371e+06 +no_defs
	bbMin.lat = 37.741560_deg; bbMin.lon = -42.075240_deg;
	bbMax.lat = 69.550000_deg; bbMax.lon = 38.757020_deg;

	bbMin.lat = -13.6_deg; bbMin.lon = -13.5_deg;
	bbMax.lat = 14.55_deg; bbMax.lon = 20.25_deg;
	newPole = { Longitude::deg(-8.0), Latitude::deg(90 - 35.0) };


	//+proj=ob_tran +o_proj=longlat +lon_0=26.5 +o_lat_p=40 +o_lon_p=0 +R=6367470 +no_defs	
	//South pole is at 26.5°,40°.
	//you would rotate 50 degrees north (from -90° to -40°)  => Lat: 90 - 40 = 50
	// and 26.5 degrees east (from 0° to 26.5°)	
	// 
	//bbMin.lat = -80.0_deg; bbMin.lon = -170.0_deg;
	//bbMax.lat = 80.0_deg; bbMax.lon = 170.0_deg;
	//newPole = { Longitude::deg(26.5), Latitude::deg(50.0) };

	Projections::Equirectangular* inputImage = new Projections::Equirectangular();
	inputImage->SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_CENTER, false); //same resolution as ipImage frame

	Projections::PoleRotationTransform* transform = new Projections::PoleRotationTransform(newPole);
	inputImage->SetLatLonTransform(transform);


	//--------

	auto a = inputImage->ProjectInverse({ 0, 0 });
	auto b = inputImage->ProjectInverse({ 0, int(h) });
	auto c = inputImage->ProjectInverse({ int(w), 0 });
	auto d = inputImage->ProjectInverse({ int(w), int(h) });

	inputImage->ComputeAABB(0, 0, w - 1, h - 1, bbMin, bbMax);

	std::cout << "Calculated corners:" << std::endl;
	std::cout << a << "\n" << b << "\n" << c << "\n" << d << std::endl;	
	std::cout << "Calculated AABB:" << std::endl;
	std::cout << bbMin << "\n" << bbMax << std::endl;
	
	std::cout << std::endl;

	std::cout << "Reference corners:" << std::endl;
	std::cout << "lat: 66.2157 | lon : -42.0752" << std::endl;
	std::cout << "lat : 39.6922 | lon : -25.1461" << std::endl;
	std::cout << "lat : 62.5895 | lon : 38.8398" << std::endl;
	std::cout << "lat : 37.6775 | lon : 17.2118" << std::endl;

	std::cout << "Reference AABB:" << std::endl;
	std::cout << "lat : 37.7416 | lon : -42.0752" << std::endl;
	std::cout << "lat : 69.55 | lon : 38.757" << std::endl;
	
	

	Projections::Equirectangular* outputImage = new Projections::Equirectangular();
	
	int newW = static_cast<int>(std::sqrt(inputImage->GetFrameWidth() * inputImage->GetFrameWidth() +
		inputImage->GetFrameHeight() * inputImage->GetFrameHeight()));

	outputImage->SetFrame(bbMin, bbMax,
		newW, inputImage->GetFrameHeight(),
		Projections::STEP_TYPE::PIXEL_CENTER, false);


	//--------

	//compute mapping from input -> output projection   
	Reprojection reprojection = Reprojection<int>::CreateReprojection(inputImage, outputImage);

	std::vector<uint8_t> rawData = reprojection.ReprojectDataNerestNeighbor<uint8_t, std::vector<uint8_t>, 1>(imgRawData.data(), 0);


	Save(outputImage, rawData, ProjectionRenderer::RenderImageType::GRAY, "D://oblique_cpu.png", 128);	
}

//================================================================

void TestWrapAround()
{
	std::cout << "TestWrapAround" << std::endl;

	unsigned w = 0;
	unsigned h = 0;
	
	std::vector<uint8_t> imgRawData = LoadPngAsGray(TEST_DATA_DIR + "8081_earthmap2k.png", w, h);



	Projections::Coordinate bbMin, bbMax;

	bbMin.lat = -80.93_deg; bbMin.lon = -650.0_deg;	
	bbMax.lat = 80.06_deg; bbMax.lon = -150.0_deg;
	
	//create input projection and set its visible frame
	auto merc = Projections::Mercator();
	merc.SetFrame(bbMin, bbMax, 2880, 1441, Projections::STEP_TYPE::PIXEL_BORDER, true);

	bbMin.lat = -90.0_deg; bbMin.lon = -180.0_deg;
	bbMax.lat = 90.0_deg; bbMax.lon = 180.0_deg;

	auto eq = Projections::Equirectangular();
	eq.SetFrame(bbMin, bbMax, w, h, Projections::STEP_TYPE::PIXEL_BORDER, false);

	auto reproj = Reprojection<int>::CreateReprojection(&eq, &merc);

	std::vector<uint8_t> rawData = reproj.ReprojectDataNerestNeighbor<uint8_t, std::vector<uint8_t>, 1>(imgRawData.data(), 0);

	Save(&merc, rawData, ProjectionRenderer::RenderImageType::GRAY, "D://wrap_around_cpu.png");
}

//================================================================

void TestCalculations()
{
	std::cout << "TestCalculations" << std::endl;

	std::cout << "=== AABB calculation ===" << std::endl;
	Coordinate min, max;
	Coordinate min2, max2;
	Coordinate c;
	c.lat = -78.0_deg;
	c.lon = -179.0_deg;
	float d = 2000;

	c = Projections::Coordinate(Longitude::deg(14), Latitude::deg(50));

	ProjectionUtils::ComputeAABB_LessAccurateNearPoles(c, d, min, max);
	ProjectionUtils::ComputeAABB(c, d, min2, max2);
	
	std::cout << "Calculated AABB (LessAccurateNearPoles):" << std::endl;
	std::cout << min << "\n" << max << std::endl;
	std::cout << "Calculated AABB:" << std::endl;
	std::cout << min2 << "\n" << max2 << std::endl;

	std::cout << std::endl;

	std::cout << "Reference corners - Calculated AABB (LessAccurateNearPoles):" << std::endl;
	std::cout << "lat: 32.0136 | lon : -14.7112" << std::endl;
	std::cout << "lat : 67.9864 | lon : 42.7112" << std::endl;
	std::cout << "Reference corners - Calculated AABB :" << std::endl;
	std::cout << "lat: 31.9982 | lon : -14.0058" << std::endl;
	std::cout << "lat : 68.0018 | lon : 42.0058" << std::endl;

	std::cout << std::endl;

	//calc distance between AABB corners
	auto dist = ProjectionUtils::Distance(min, max);
	auto dist2 = ProjectionUtils::Distance(min2, max2);

	std::cout << "Distance (less accurate): " << dist << std::endl;
	std::cout << "Reference Distance (less accurate): 5389.7280047610502" << std::endl;
	std::cout << "Distance : " << dist2 << std::endl;
	std::cout << "Reference Distance: 5336.0055339535957" << std::endl;
	

	std::cout << "=== Point projection ===" << std::endl;

	Projections::Coordinate bbMin, bbMax;

	bbMin.lat = 21.1405_deg; bbMin.lon = -134.0955_deg;
	bbMax.lat = 52.6133_deg; bbMax.lon = -60.9365_deg;

	//create input projection and set its visible frame
	Projections::Equirectangular* eq = new Projections::Equirectangular();

	//auto kk = inputImage->ProjectInverseInternal(-899.5, -529.5);

	eq->SetFrame(bbMin, bbMax, 1799, 1059, Projections::STEP_TYPE::PIXEL_BORDER, false);
	auto f = eq->GetFrame();


	Projections::Coordinate gps;
	gps.lat = 39.775_deg;
	gps.lon = -76.641_deg;

	auto px = eq->Project<int>(gps);

	std::cout << "Projected: " << px.x << ", " << px.y << std::endl;
	std::cout << "Reference Projected: 1413, 432" << std::endl;
	
	printf("x");
}