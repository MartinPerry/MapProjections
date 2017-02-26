
#include "./lodepng.h"
#include "./MapProjection.h"
#include "./Projections.h"
#include "./ProjectionRenderer.h"


void test1()
{
	//convert image from one to another projection

	// Load data in input projection
	unsigned int ww, hh;
	std::vector<uint8_t> rawPngData;
	lodepng::decode(rawPngData, ww, hh, "D:\\hrrr2.png", LodePNGColorType::LCT_GREY);

	//===================================================
	//Build input projection
	//===================================================

	IProjectionInfo::Coordinate bbMin, bbA, bbB, bbMax;
	bbMin.lat = 21.1381_deg; bbMin.lon = -122.72_deg;	
	bbMax.lat = 47.84364_deg; bbMax.lon = -60.90137_deg;

	//create input projection and set its visible frame
	IProjectionInfo * lambert = new LambertConic(38.5_deg, -97.5_deg, 38.5_deg);
	lambert->SetFrame(bbMin, bbMax, ww, hh, false);

	//render image in input projection - it should take input image and
	//render it 1:1 based on ipImage frame
	ProjectionRenderer pd(lambert);
	pd.AddBorders("E://hranice//ll.csv");
	pd.DrawImage(&rawPngData[0], ww, hh, lambert);
	pd.DrawBorders();
	pd.SaveToFile("D://1_lambert.png");


	//===================================================
	// Build output projection
	//===================================================
			
	IProjectionInfo * eq = new Equirectangular();		
	eq->SetFrame(lambert, false); //same resolution as ipImage frame
	//eq->SetFrame(ipImage, 720 * 2, 360 * 2, false); //user specified resolution

	//render image in output projection
	pd.SetProjection(eq);
	//pd.DrawImage(&rawPngData[0], ww, hh, ipImage);
	
	//compute mapping from input -> output projection
	//newData[index] = oldData[reprojection[index]]
	IProjectionInfo::Reprojection reprojection = IProjectionInfo::CreateReprojection(lambert, eq);
	pd.DrawImage(&rawPngData[0], reprojection);
	
	pd.DrawBorders();
	pd.SaveToFile("D://2_equirect.png");

	return;
}

void test_gfs()
{
	
	unsigned int ww, hh;
	std::vector<uint8_t> rawPngData;

	const char * fileName = "D:\\whole_swap.png";
	//const char * fileName = "D:\\input_tile.png";
	//const char * fileName = "D:\\input_tile2.png";
	
	lodepng::decode(rawPngData, ww, hh, fileName, LodePNGColorType::LCT_GREY);




	//===================================================
	//Build input projection
	//===================================================

	IProjectionInfo::Coordinate bbMin, bbMax;
	if (strcmp(fileName, "D:\\whole_swap.png") == 0)
	{
		bbMax.lon = 180.0_deg; bbMax.lat = 90.0_deg;
		bbMin.lon = -180.0_deg; bbMin.lat = -90.0_deg;
	}

	if (strcmp(fileName, "D:\\input_tile.png") == 0)
	{
		bbMax.lon = -52.000000_deg;  bbMax.lat = 90.000000_deg;
		bbMin.lon = -180.000000_deg; bbMin.lat = -37.822472_deg;
	}

	if (strcmp(fileName, "D:\\input_tile2.png") == 0)
	{
		bbMax.lon = -52.000000_deg;  bbMax.lat = -37.822472_deg;
		bbMin.lon = -180.000000_deg; bbMin.lat = -90.0_deg;
	}

	//create input projection and set its visible frame
	IProjectionInfo * equirect = new Equirectangular();
	equirect->SetFrame(bbMin, bbMax, ww, hh, true);

	//render image in input projection - it should take input image and
	//render it 1:1 based on ipImage frame
	ProjectionRenderer pd(equirect);
	pd.AddBorders("E://hranice//ll.csv");
	//pd.DrawImage(&img.rawData[0], img.w, img.h, equirect);
	//pd.DrawBorders();
	//pd.SaveToFile("D://equi.png");


	//===================================================
	// Build output projection
	//===================================================

	if (bbMin.lat.deg() < -85) bbMin.lat = -85.0_deg;
	if (bbMax.lat.deg() > 85) bbMax.lat = 85.0_deg;



	IProjectionInfo * mercator = new Mercator();
	//mercator->SetFrame(equirect, false); //same resolution as ipImage frame
	//eq->SetFrame(ipImage, 720 * 2, 360 * 2, false); //user specified resolution
	mercator->SetFrame(bbMin, bbMax, ww, hh, false);
	//render image in output projection
	pd.SetProjection(mercator);
	//pd.DrawImage(&rawPngData[0], ww, hh, ipImage);

	//compute mapping from input -> output projection
	//newData[index] = oldData[reprojection[index]]
	IProjectionInfo::Reprojection reprojection = IProjectionInfo::CreateReprojection(equirect, mercator);
	pd.DrawImage(&rawPngData[0], reprojection);

	pd.DrawBorders();
	if (strcmp(fileName, "D:\\whole_swap.png") == 0)
	{
		pd.SaveToFile("D://mercator_whole.png");
	}
	if (strcmp(fileName, "D:\\input_tile.png") == 0)
	{
		pd.SaveToFile("D://mercator_input_tile.png");
	}
	if (strcmp(fileName, "D:\\input_tile2.png") == 0)
	{
		pd.SaveToFile("D://mercator_input_tile2.png");
	}

	auto ii = mercator->ProjectInverse({ 0,0 });
	auto ii2 = mercator->ProjectInverse({ 0, (int)hh });
	printf("%f\n", ii.lat.deg());
	printf("%f\n", ii2.lat.deg());

	
	
	//while (true) {}


}

void test2()
{
	// Load data in input projection
	unsigned int ww, hh;
	std::vector<uint8_t> rawPngData;
	lodepng::decode(rawPngData, ww, hh, "D:\\hrrr2.png", LodePNGColorType::LCT_GREY);

	//===================================================
	//Build input projection
	//===================================================

	IProjectionInfo::Coordinate bbMin, bbA, bbB, bbMax;
	bbMin.lat = 21.1381_deg; bbMin.lon = -122.72_deg;
	//bbA.lat = 47.84364_deg; bbA.lon = -134.0986_deg;
	//bbB.lat = 21.13812_deg; bbB.lon = -72.28046_deg;
	bbMax.lat = 47.84364_deg; bbMax.lon = -60.90137_deg;

	//create input projection and set its visible frame
	IProjectionInfo * ipImage = new LambertConic(38.5_deg, -97.5_deg, 38.5_deg);
	ipImage->SetFrame(bbMin, bbMax, ww, hh, false);


	//===================================================
	// Build output projection
	//===================================================

	IProjectionInfo * eq = new Equirectangular();
	eq->SetFrame(ipImage, 720, 360, false);

	IProjectionInfo::Coordinate outMin, outMax;
	eq->ComputeAABB(outMin, outMax);

	//std::vector<IProjectionInfo::Pixel> reprojection = eq->CreateReprojection(ipImage);

	printf("Min: lat: %f, lon: %f\n", outMin.lat.deg(), outMin.lon.deg());
	printf("Max: lat: %f, lon: %f\n", outMax.lat.deg(), outMax.lon.deg());
}

void test3()
{
	IProjectionInfo * eq = new Equirectangular(0.0_deg);
	eq->SetFrame({ -90.0_deg, -180.0_deg }, { 90.0_deg, 180.0_deg }, 1440, 720, false);

	auto ii = eq->GetTopLeftCorner();
	auto p = ii.lat.deg();
	auto p1 = ii.lon.deg();

	auto stepLat = eq->GetStepLat();
	auto stepLon = eq->GetStepLon();
	auto pxx = stepLat.deg();
	auto p1xx = stepLon.deg();

	printf("x");

}

int main(int argc, char ** argv)
{

	//test1();
	test_gfs();
	//test1();
	return 0;
	std::vector<uint8_t> rawPngData;

	unsigned int ww, hh;
	lodepng::decode(rawPngData, ww, hh, "D:\\hrrr2.png", LodePNGColorType::LCT_GREY);
	
	//IProjectionInfo * ip = new Mercator();
	//IProjectionInfo * ip = new LambertConic(38.5_deg, 262.5_deg, 38.5_deg);
	IProjectionInfo * ip = new LambertConic(38.5_deg, -97.5_deg, 38.5_deg);
	
	//(world)
	IProjectionInfo::Coordinate cMin, cMax;
	cMin.lat = -85.0_deg;
	cMin.lon = -180.0_deg;
	cMax.lat = 82.0_deg;
	cMax.lon = 180.0_deg;

	//USA
	cMin.lat = 60.0_deg;
	cMin.lon = -170.0_deg;
	cMax.lat = 10.0_deg;
	cMax.lon = -65.0_deg;

	ip->SetFrame(cMin, cMax, 1000, 1000, false); 
	auto xxx = ip->ProjectInverse({ 0,0 });

	ProjectionRenderer pd(ip);
	pd.AddBorders("E://hranice//ll.csv");
		

	IProjectionInfo::Coordinate bbMin, bbA, bbB, bbMax;
	bbMin.lat = 21.1381_deg; bbMin.lon = -122.72_deg;
	bbA.lat = 47.84364_deg; bbA.lon = -134.0986_deg;
	bbB.lat = 21.13812_deg; bbB.lon = -72.28046_deg;
	bbMax.lat = 47.84364_deg; bbMax.lon = -60.90137_deg;



	bbA = ip->CalcEndPointShortest(bbMin, -19.25_deg, 1059 * 3);
	bbB = ip->CalcEndPointShortest(bbMin, 90.0_deg, 1799 * 3);
	
	
	bbMax.lat = bbA.lat;
	bbMax.lon = bbB.lon;

	// :corner_lats = 21.13812f, 47.84364f, 47.84364f, 21.13812f;
	// :corner_lons = -122.7195f, -134.0986f, -60.90137f, -72.28046f;

	

	pd.DrawLine(bbMin, bbA);
	pd.DrawLine(bbMin, bbB);
	pd.DrawLine(bbMax, bbA);
	pd.DrawLine(bbMax, bbB);

	pd.DrawBorders();
	pd.SaveToFile("D://88.png");

	return 0;


	IProjectionInfo::Pixel<int> pixelMin = ip->Project(bbMin);
	IProjectionInfo::Pixel<int> pixelAtmp = ip->Project(bbA);
	IProjectionInfo::Pixel<int> pixelB = ip->Project(bbB);

	IProjectionInfo::Pixel<int> pixelA;
	pixelA.x = pixelMin.x;
	pixelA.y = pixelAtmp.y;

	IProjectionInfo::Pixel<int> pixelMax;
	pixelMax.x = pixelB.x;
	pixelMax.y = pixelA.y;

	pd.DrawPoint(bbMin);
	pd.DrawPoint(bbA);
	pd.DrawPoint(bbMax);
	pd.DrawPoint(bbB);
	pd.DrawPoint({ 38.5_deg, -97.5_deg });
	//----------------------------------------------------------------------
	IProjectionInfo * ipImage = new LambertConic(38.5_deg, -97.5_deg, 38.5_deg);
	
	//ipImage->SetFrame(bbMin, ip->ProjectInverse(pixelMax), ww, hh, false);

	auto e = ip->Project(bbMin);
	e.x += ww;
	e.y += hh;
	ipImage->SetFrame(bbMin, bbMax, ww, hh, false);
	
	
	//pd.DrawImage(&rawPngData[0], ww, hh, ipImage);
	
	//pd.DrawLine(ipImage->ProjectInverse({ 0,0 }), ipImage->ProjectInverse({ 719, 359 }));
	//pd.DrawLine(ipImage->ProjectInverse({ 0,359 }), ipImage->ProjectInverse({ 719, 0 }));
	//pd.DrawLine(bbMin, ip->ProjectInverse(pixelMax));
	//pd.SaveToFile("D://img.png");
	//-----------------------------------------------------------------
	
	pd.DrawBorders();
	//pd.DrawParalells();

	//-----------------------------------------------------------------


	//pd.DrawLine(bbMin, bbA);
	//pd.DrawLine(bbMin, bbB);
	//pd.DrawLine(bbMax, bbA);
	//pd.DrawLine(bbMax, bbB);
	
	//-----------------------------------------------------------------


	std::vector<IProjectionInfo::Coordinate> lineA;
	ip->LineBresenham(pixelMin, pixelA,
		[&](int x, int y) -> void {
		IProjectionInfo::Coordinate c = ip->ProjectInverse({ x, y });
		lineA.push_back(c);
	});
	pd.DrawLines(lineA);
	

	std::vector<IProjectionInfo::Coordinate> lineB;
	ip->LineBresenham(pixelMin, pixelB,
		[&](int x, int y) -> void {
		IProjectionInfo::Coordinate c = ip->ProjectInverse({ x, y });
		lineB.push_back(c);
	});
	pd.DrawLines(lineB);
	

	std::vector<IProjectionInfo::Coordinate> lineC;
	ip->LineBresenham(pixelMax, pixelA,
		[&](int x, int y) -> void {
		IProjectionInfo::Coordinate c = ip->ProjectInverse({ x, y });
		lineC.push_back(c);
	});
	pd.DrawLines(lineC);


	std::vector<IProjectionInfo::Coordinate> lineD;
	ip->LineBresenham(pixelMax, pixelB,
		[&](int x, int y) -> void {
		IProjectionInfo::Coordinate c = ip->ProjectInverse({ x, y });
		lineD.push_back(c);
	});
	pd.DrawLines(lineD);


	pd.SaveToFile("D://bbb.png");
	
	//return 0;
	
	//IProjectionInfo * ip2 = new Mercator();
	IProjectionInfo * ip2 = new Equirectangular();

	cMin.lat = 60.0_deg;
	cMin.lon = -150.0_deg;
	cMax.lat = 10.0_deg;
	cMax.lon = -45.0_deg;

	ip2->SetFrame(cMin, cMax, 1000, 1000);
	
	pd.SetProjection(ip2);	

	pd.DrawImage(&rawPngData[0], ww, hh, ipImage);

	pd.DrawBorders();
	/*
	pd.DrawLines(lineA);
	pd.DrawLines(lineB);
	pd.DrawLines(lineC);
	pd.DrawLines(lineD);
	*/
	pd.SaveToFile("D://xxx.png");
	

	
	
	

	delete ip;
	delete ip2;

	return 0;
	
}