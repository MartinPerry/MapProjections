#include "ProjectionRenderer.h"

#include "./lodepng.h"
#include "Projections.h"

using namespace Projections;

//=======================================================================
// Debug renderer
//
//=======================================================================

/// <summary>
/// ctor
/// </summary>
/// <param name="proj"></param>
template <typename Proj>
ProjectionRenderer<Proj>::ProjectionRenderer(Proj * proj)
	: rawData(nullptr), externalData(false), proj(nullptr)
{
	this->SetProjection(proj);
}

/// <summary>
/// dtor
/// </summary>
template <typename Proj>
ProjectionRenderer<Proj>::~ProjectionRenderer()
{
	if (!externalData)
	{
		delete[] rawData;
		rawData = nullptr;
	}
}

/// <summary>
/// Set current projection - rewrites projection from ctor
/// It also clears current data, because new projection can have
/// different frame size
/// </summary>
/// <param name="proj"></param>
template <typename Proj>
void ProjectionRenderer<Proj>::SetProjection(Proj * proj)
{
	this->proj = proj;
	if (!externalData)
	{
		delete[] rawData;
		rawData = new uint8_t[proj->GetFrameWidth() * proj->GetFrameHeight()];
	}
	this->Clear();
}

/// <summary>
/// Clear data
/// </summary>
template <typename Proj>
void ProjectionRenderer<Proj>::Clear()
{
	memset(rawData, 0, proj->GetFrameWidth() * proj->GetFrameHeight() * sizeof(uint8_t));
}

template <typename Proj>
void ProjectionRenderer<Proj>::SetRawDataTarget(uint8_t * target)
{
	if (target == nullptr)
	{
		this->rawData = nullptr;
		this->rawData = new uint8_t[proj->GetFrameWidth() * proj->GetFrameHeight()];

		this->externalData = false;
		return;
	}
	if (!externalData)
	{
		delete[] rawData;
	}
	this->rawData = target;
	this->externalData = true;
}

/// <summary>
/// Save data to *.png file
/// </summary>
/// <param name="fileName"></param>
template <typename Proj>
void ProjectionRenderer<Proj>::SaveToFile(const char * fileName)
{

	lodepng::encode(fileName, this->rawData, proj->GetFrameWidth(), proj->GetFrameHeight(),
		LodePNGColorType::LCT_GREY);

	/*
	FILE * f = NULL;
	//my_fopen(&f, fileName.GetConstString(), "wb");
	f = fopen(fileName, "wb");

	if (f == NULL)
	{
	printf("Failed to open file %s (%s)", fileName, strerror(errno));
	return;
	}
	fwrite(rawData, sizeof(uint8_t), proj->GetFrameWidth() * proj->GetFrameHeight(), f);
	fclose(f);
	*/
}

template <typename Proj>
void ProjectionRenderer<Proj>::FillData(std::vector<uint8_t> & output)
{
	output.resize(proj->GetFrameWidth() * proj->GetFrameHeight());
	memcpy(&output[0], this->rawData, output.size());
}


/// <summary>
/// Load input data from file
/// </summary>
/// <param name="filePath"></param>
/// <returns></returns>
template <typename Proj>
std::string ProjectionRenderer<Proj>::LoadFromFile(const char * filePath)
{
	FILE *f = NULL;  //pointer to file we will read in
	f = fopen(filePath, "rb");
	if (f == NULL)
	{
		printf("Failed to open file: \"%s\"\n", filePath);
		return "";
	}

	fseek(f, 0L, SEEK_END);
	long size = ftell(f);
	fseek(f, 0L, SEEK_SET);

	char * data = new char[size + 1];
	fread(data, sizeof(char), size, f);
	fclose(f);

	data[size] = 0;
	std::string tmp = data;
	delete[] data;

	return tmp;
};

template <typename Proj>
std::vector<std::string> ProjectionRenderer<Proj>::Split(const std::string &s, char delim)
{
	std::stringstream ss;
	ss.str(s);
	std::string item;
	std::vector<std::string> elems;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}

	return elems;
}

/// <summary>
/// Load borders from file in format "lat;lon\n"
/// </summary>
/// <param name="fileName"></param>
/// <param name="useEveryNthPoint"></param>
template <typename Proj>
void ProjectionRenderer<Proj>::AddBorders(const char * fileName, int useEveryNthPoint)
{
	std::string content = LoadFromFile(fileName);


	std::vector<std::string> lines = Split(content, '\n');

	std::string keyName;
	int tmp = 0;

	for (size_t i = 0; i < lines.size(); i++)
	{
		std::vector<std::string> line = Split(lines[i], ';');
		if (line.size() <= 2)
		{
			continue;
		}
		if (line.size() > 5)
		{
			keyName = line[7];
			tmp = 0;
		}
		else
		{
			if (tmp % useEveryNthPoint != 0)
			{
				tmp++;
				continue;
			}

			std::string key = keyName;
			key += "_";
			key += line[3];


			Coordinate point;
			point.lon = GeoCoordinate::deg(atof(line[0].c_str()));
			point.lat = GeoCoordinate::deg(atof(line[1].c_str()));
			this->debugBorder[key].push_back(point);

			tmp++;
		}

	}


}

/// <summary>
/// Draw borders
/// </summary>
template <typename Proj>
void ProjectionRenderer<Proj>::DrawBorders()
{

	for (auto it = this->debugBorder.begin(); it != this->debugBorder.end(); it++)
	{
		std::vector<Coordinate> & b = it->second;

		for (size_t i = 0; i < b.size(); i++)
		{
			Coordinate p = b[i % b.size()];
			Coordinate p1 = b[(i + 1) % b.size()];



			Pixel<int> pp1 = proj->Project<int>(p);
			Pixel<int> pp2 = proj->Project<int>(p1);


			this->CohenSutherlandLineClipAndDraw(pp1.x, pp1.y, pp2.x, pp2.y);
		}

	}
}

/// <summary>
/// Draw parallels
/// </summary>
template <typename Proj>
void ProjectionRenderer<Proj>::DrawParalells()
{
	double lonStep = 10;
	double latStep = 5;

	for (double lat = -90; lat <= 90; lat += latStep)
	{
		for (double lon = -180; lon <= 180 - lonStep; lon += lonStep)
		{
			Coordinate p;
			p.lat = GeoCoordinate::deg(lat);
			p.lon = GeoCoordinate::deg(lon);

			Coordinate p1;
			p1.lat = GeoCoordinate::deg(lat);
			p1.lon = GeoCoordinate::deg(lon + lonStep);
			
			Pixel<int> pp1 = proj->Project<int>(p);
			Pixel<int> pp2 = proj->Project<int>(p1);


			this->CohenSutherlandLineClipAndDraw(pp1.x, pp1.y, pp2.x, pp2.y);
			//this->DrawLine(pp1.x, pp1.y, pp2.x, pp2.y);
		}
	}
}

/// <summary>
/// Draw line from [start] to [end] with a stepCount
/// </summary>
/// <param name="start"></param>
/// <param name="end"></param>
/// <param name="stepCount"></param>
template <typename Proj>
void ProjectionRenderer<Proj>::DrawLine(Coordinate start, Coordinate end, int stepCount)
{
	double difLat = (end.lat.rad() - start.lat.rad());
	double difLon = (end.lon.rad() - start.lon.rad());

	double lonStep = difLon / stepCount;
	double latStep = difLat / stepCount;


	Coordinate p = start;

	for (int i = 0; i < stepCount; i++)
	{
		Coordinate p1 = p;
		p1.lat = GeoCoordinate::rad(p1.lat.rad() + latStep);
		p1.lon = GeoCoordinate::rad(p1.lon.rad() + lonStep);

		Pixel<int> pp1 = proj->Project<int>(p);
		Pixel<int> pp2 = proj->Project<int>(p1);

		this->CohenSutherlandLineClipAndDraw(pp1.x, pp1.y, pp2.x, pp2.y);

		p = p1;
	}
}

/// <summary>
/// Draw point - point is represented by small square
/// </summary>
/// <param name="p"></param>
template <typename Proj>
void ProjectionRenderer<Proj>::DrawPoint(Coordinate p)
{
	int size = 5;
	Pixel<int> center = proj->Project<int>(p);

	Pixel<int> a = center;
	Pixel<int> b = center;
	Pixel<int> c = center;
	Pixel<int> d = center;

	a.x -= size; a.y -= size;
	b.x += size; b.y -= size;
	c.x += size; c.y += size;
	d.x -= size; d.y += size;

	this->CohenSutherlandLineClipAndDraw(a.x, a.y, b.x, b.y);
	this->CohenSutherlandLineClipAndDraw(b.x, b.y, c.x, c.y);
	this->CohenSutherlandLineClipAndDraw(c.x, c.y, d.x, d.y);
	this->CohenSutherlandLineClipAndDraw(d.x, d.y, a.x, a.y);

}

/// <summary>
/// Draw lines created by points
/// </summary>
/// <param name="points"></param>
template <typename Proj>
void ProjectionRenderer<Proj>::DrawLines(const std::vector<Coordinate> & points)
{
	if (points.size() <= 1)
	{
		return;
	}

	for (size_t i = 0; i < points.size() - 1; i++)
	{
		this->DrawLine(points[i], points[i + 1]);
	}

}

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
void ProjectionRenderer<Proj>::DrawImage(uint8_t * imData, int w, int h, Proj * imProj)
{
	for (int y = 0; y < this->proj->GetFrameHeight(); y++)
	{
		for (int x = 0; x < this->proj->GetFrameWidth(); x++)
		{			
						
			Coordinate cc = this->proj->ProjectInverse({ x,y });
			Pixel<int> p = imProj->Project<int>(cc);

			if (p.x < 0) continue;
			if (p.y < 0) continue;
			if (p.x >= w) continue;
			if (p.y >= h) continue;

			this->rawData[x + y * this->proj->GetFrameWidth()] = imData[p.x + p.y * w];

		}
	}

}

/// <summary>
/// Draw image based on re-projection pixels
/// e.g.: currentData[index] = imData[reprojection[index]]
/// </summary>
/// <param name="imData"></param>
/// <param name="reproj"></param>
template <typename Proj>
void ProjectionRenderer<Proj>::DrawImage(uint8_t * imData, const Reprojection & reproj)
{	
	ProjectionRenderer<Proj>::ReprojectImage(imData, this->rawData, reproj);
}

template <typename Proj>
void ProjectionRenderer<Proj>::ReprojectImage(uint8_t * fromData, uint8_t * toData, const Reprojection & reproj)
{
	for (int y = 0; y < reproj.outH; y++)
	{
		for (int x = 0; x < reproj.outW; x++)
		{
			int index = x + y * reproj.outW;
			if ((reproj.pixels[index].x == -1) && (reproj.pixels[index].y == -1))
			{
				continue;
			}


			int origIndex = reproj.pixels[index].x + reproj.pixels[index].y * reproj.inW;
			toData[index] = fromData[origIndex];
		}
	}
}

/// <summary>
/// Set pixel value
/// </summary>
/// <param name="p"></param>
/// <param name="val"></param>
template <typename Proj>
void ProjectionRenderer<Proj>::SetPixel(const Pixel<int> & p, uint8_t val)
{
	this->rawData[p.x + p.y * this->proj->GetFrameWidth()] = val;
}

/// <summary>
/// Codes for Cohen-Sutherlan
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <returns></returns>
template <typename Proj>
int ProjectionRenderer<Proj>::ComputeOutCode(double x, double y)
{
	int code = INSIDE;

	if (x < 0) code |= LEFT;
	else if (x >= proj->GetFrameWidth()) code |= RIGHT;

	if (y < 0) code |= BOTTOM;
	else if (y >= proj->GetFrameHeight()) code |= TOP;

	return code;
}

/// <summary>
/// Cohen-Sutherland line clipping
/// </summary>
/// <param name="x0"></param>
/// <param name="y0"></param>
/// <param name="x1"></param>
/// <param name="y1"></param>
template <typename Proj>
void ProjectionRenderer<Proj>::CohenSutherlandLineClipAndDraw(double x0, double y0, double x1, double y1)
{
	// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
	int outcode0 = ComputeOutCode(x0, y0);
	int outcode1 = ComputeOutCode(x1, y1);
	bool accept = false;

	double xmin = 0;
	double xmax = proj->GetFrameWidth() - 1;

	double ymin = 0;
	double ymax = proj->GetFrameHeight() - 1;

	while (true) {
		if (!(outcode0 | outcode1)) { // Bitwise OR is 0. Trivially accept and get out of loop
			accept = true;
			break;
		}
		else if (outcode0 & outcode1) { // Bitwise AND is not 0. (implies both end points are in the same region outside the window). Reject and get out of loop
			break;
		}
		else {
			// failed both tests, so calculate the line segment to clip
			// from an outside point to an intersection with clip edge
			double x = 0;
			double y = 0;

			// At least one endpoint is outside the clip rectangle; pick it.
			int outcodeOut = outcode0 ? outcode0 : outcode1;

			// Now find the intersection point;
			// use formulas y = y0 + slope * (x - x0), x = x0 + (1 / slope) * (y - y0)
			if (outcodeOut & TOP) {           // point is above the clip rectangle
				x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
				y = ymax;
			}
			else if (outcodeOut & BOTTOM) { // point is below the clip rectangle
				x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
				y = ymin;
			}
			else if (outcodeOut & RIGHT) {  // point is to the right of clip rectangle
				y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
				x = xmax;
			}
			else if (outcodeOut & LEFT) {   // point is to the left of clip rectangle
				y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
				x = xmin;
			}

			// Now we move outside point to intersection point to clip
			// and get ready for next pass.
			if (outcodeOut == outcode0) {
				x0 = x;
				y0 = y;
				outcode0 = ComputeOutCode(x0, y0);
			}
			else {
				x1 = x;
				y1 = y;
				outcode1 = ComputeOutCode(x1, y1);
			}
		}
	}
	if (accept)
	{
		// Following functions are left for implementation by user based on
		// their platform (OpenGL/graphics.h etc.)
		//DrawRectangle(xmin, ymin, xmax, ymax);
		//LineSegment(x0, y0, x1, y1);
		//DrawLine(x0, y0, x1, y1);
		Pixel<int> start, end;
		start.x = static_cast<int>(x0); start.y = static_cast<int>(y0);
		end.x = static_cast<int>(x1); end.y = static_cast<int>(y1);

		proj->LineBresenham(start, end,
			[&](int x, int y) -> void {
			this->rawData[x + y * proj->GetFrameWidth()] = 255;
		});
	}
}


//=====

template class ProjectionRenderer<LambertConic>;
template class ProjectionRenderer<Mercator>;
template class ProjectionRenderer<Equirectangular>;
template class ProjectionRenderer<PolarSteregographic>;