
#include "./lodepng.h"
#include "./MapProjection.h"
#include "./Projections.h"
#include "./ProjectionDebugger.h"

int main(int argc, char ** argv)
{

	std::vector<uint8_t> rawPngData;

	unsigned int ww, hh;
	lodepng::decode(rawPngData, ww, hh, "D:\\tmp.png", LodePNGColorType::LCT_GREY);
	
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

	ProjectionDebugger pd(ip);	

		

	IProjectionInfo::Coordinate bbMin;
	bbMin.lat = 21.1381_deg; bbMin.lon = -122.72_deg; //-123
				
	IProjectionInfo::Coordinate bbA = ip->CalcEndPointDirect(bbMin, 0.0_deg, 1059 * 3);
	IProjectionInfo::Coordinate bbB = ip->CalcEndPointDirect(bbMin, 80.0_deg, 1799 * 3);
	

	IProjectionInfo::Coordinate bbMax;
	bbMax.lat = bbA.lat;
	bbMax.lon = bbB.lon;


	IProjectionInfo::Pixel pixelMin = ip->Project(bbMin);
	IProjectionInfo::Pixel pixelAtmp = ip->Project(bbA);
	IProjectionInfo::Pixel pixelB = ip->Project(bbB);

	IProjectionInfo::Pixel pixelA;
	pixelA.x = pixelMin.x;
	pixelA.y = pixelAtmp.y;

	IProjectionInfo::Pixel pixelMax;
	pixelMax.x = pixelB.x;
	pixelMax.y = pixelA.y;


	auto oo = ip->ProjectInverse(pixelMax);
	double lon = oo.lon.deg();
	double lat = oo.lat.deg();

	//----------------------------------------------------------------------
	IProjectionInfo * ipImage = new LambertConic(38.5_deg, -97.5_deg, 38.5_deg);
	ipImage->SetFrame(bbMin, ip->ProjectInverse(pixelMax), ww, hh, false);
	
		
	pd.DrawImage(&rawPngData[0], ww, hh, ipImage);
	
	//pd.DrawLine(ipImage->ProjectInverse({ 0,0 }), ipImage->ProjectInverse({ 719, 359 }));
	//pd.DrawLine(ipImage->ProjectInverse({ 0,359 }), ipImage->ProjectInverse({ 719, 0 }));
	//pd.DrawLine(bbMin, ip->ProjectInverse(pixelMax));
	//pd.SaveToFile("D://img.png");
	//-----------------------------------------------------------------

	pd.AddBorders("E://hranice//ll.csv");
	pd.DrawBorders();
	//pd.DrawParalells();

	//-----------------------------------------------------------------


	pd.DrawLine(bbMin, bbA);
	pd.DrawLine(bbMin, bbB);
	pd.DrawLine(bbMax, bbA);
	pd.DrawLine(bbMax, bbB);
	
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

	return 0;
	
	IProjectionInfo * ip2 = new Mercator();

	cMin.lat = 60.0_deg;
	cMin.lon = -150.0_deg;
	cMax.lat = 10.0_deg;
	cMax.lon = -45.0_deg;

	ip2->SetFrame(cMin, cMax, 1000, 1000);
	
	pd.SetProjection(ip2);	
	pd.DrawBorders();

	pd.DrawLines(lineA);
	pd.DrawLines(lineB);
	pd.DrawLines(lineC);
	pd.DrawLines(lineD);
	
	pd.SaveToFile("D://xxx.png");
	

	
	
	

	delete ip;
	delete ip2;

	return 0;
	
}