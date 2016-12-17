#include "ProjectionDebugger.h"

#include "./lodepng.h"

//=======================================================================
// Debug renderer
//
//=======================================================================

ProjectionDebugger::ProjectionDebugger(IProjectionInfo * proj)
	: rawData(nullptr), proj(nullptr)
{
	this->SetProjection(proj);
}

ProjectionDebugger::~ProjectionDebugger()
{
	delete[] rawData;
	rawData = nullptr;
}

void ProjectionDebugger::SetProjection(IProjectionInfo * proj)
{
	this->proj = proj;
	delete[] rawData;
	rawData = new uint8_t[proj->GetFrameWidth() * proj->GetFrameHeight()];
	this->Clear();
}

void ProjectionDebugger::Clear()
{
	memset(rawData, 0, proj->GetFrameWidth() * proj->GetFrameHeight() * sizeof(uint8_t));
}

void ProjectionDebugger::SaveToFile(const char * fileName)
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


std::string ProjectionDebugger::LoadFromFile(const char * filePath)
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

std::vector<std::string> ProjectionDebugger::Split(const std::string &s, char delim)
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

void ProjectionDebugger::AddBorders(const char * fileName, int useNthPoint)
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
			if (tmp % useNthPoint != 0)
			{
				tmp++;
				continue;
			}

			std::string key = keyName;
			key += "_";
			key += line[3];


			IProjectionInfo::Coordinate point;
			point.lon = GeoCoordinate::deg(atof(line[0].c_str()));
			point.lat = GeoCoordinate::deg(atof(line[1].c_str()));
			this->debugBorder[key].push_back(point);

			tmp++;
		}

	}


}

void ProjectionDebugger::DrawBorders()
{

	for (auto it = this->debugBorder.begin(); it != this->debugBorder.end(); it++)
	{
		std::vector<IProjectionInfo::Coordinate> & b = it->second;

		for (size_t i = 0; i < b.size(); i++)
		{
			IProjectionInfo::Coordinate p = b[i % b.size()];
			IProjectionInfo::Coordinate p1 = b[(i + 1) % b.size()];



			IProjectionInfo::Pixel pp1 = proj->Project(p);
			IProjectionInfo::Pixel pp2 = proj->Project(p1);


			this->CohenSutherlandLineClipAndDraw(pp1.x, pp1.y, pp2.x, pp2.y);
		}

	}
}

void ProjectionDebugger::DrawParalells()
{
	double lonStep = 10;
	double latStep = 5;

	for (double lat = -90; lat <= 90; lat += latStep)
	{
		for (double lon = -180; lon <= 180 - lonStep; lon += lonStep)
		{
			IProjectionInfo::Coordinate p;
			p.lat = GeoCoordinate::deg(lat);
			p.lon = GeoCoordinate::deg(lon);

			IProjectionInfo::Coordinate p1;
			p1.lat = GeoCoordinate::deg(lat);
			p1.lon = GeoCoordinate::deg(lon + lonStep);

			IProjectionInfo::Pixel pp1 = proj->Project(p);
			IProjectionInfo::Pixel pp2 = proj->Project(p1);


			this->CohenSutherlandLineClipAndDraw(pp1.x, pp1.y, pp2.x, pp2.y);
			//this->DrawLine(pp1.x, pp1.y, pp2.x, pp2.y);
		}
	}
}

void ProjectionDebugger::DrawLine(IProjectionInfo::Coordinate start,
	IProjectionInfo::Coordinate end, int stepCount)
{
	double difLat = (end.lat.rad() - start.lat.rad());
	double difLon = (end.lon.rad() - start.lon.rad());

	double lonStep = difLon / stepCount;
	double latStep = difLat / stepCount;


	IProjectionInfo::Coordinate p = start;

	for (int i = 0; i < stepCount; i++)
	{
		IProjectionInfo::Coordinate p1 = p;
		p1.lat = GeoCoordinate::rad(p1.lat.rad() + latStep);
		p1.lon = GeoCoordinate::rad(p1.lon.rad() + lonStep);

		IProjectionInfo::Pixel pp1 = proj->Project(p);
		IProjectionInfo::Pixel pp2 = proj->Project(p1);

		this->CohenSutherlandLineClipAndDraw(pp1.x, pp1.y, pp2.x, pp2.y);

		p = p1;
	}
}

void ProjectionDebugger::DrawPoint(IProjectionInfo::Coordinate p)
{
	int size = 5;
	IProjectionInfo::Pixel center = proj->Project(p);

	IProjectionInfo::Pixel a = center;
	IProjectionInfo::Pixel b = center;
	IProjectionInfo::Pixel c = center;
	IProjectionInfo::Pixel d = center;

	a.x -= size; a.y -= size;
	b.x += size; b.y -= size;
	c.x += size; c.y += size;
	d.x -= size; d.y += size;

	this->CohenSutherlandLineClipAndDraw(a.x, a.y, b.x, b.y);
	this->CohenSutherlandLineClipAndDraw(b.x, b.y, c.x, c.y);
	this->CohenSutherlandLineClipAndDraw(c.x, c.y, d.x, d.y);
	this->CohenSutherlandLineClipAndDraw(d.x, d.y, a.x, a.y);

}

void ProjectionDebugger::DrawLines(const std::vector<IProjectionInfo::Coordinate> & points)
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

int ProjectionDebugger::ComputeOutCode(double x, double y)
{
	int code = INSIDE;

	if (x < 0) code |= LEFT;
	else if (x >= proj->GetFrameWidth()) code |= RIGHT;

	if (y < 0) code |= BOTTOM;
	else if (y >= proj->GetFrameHeight()) code |= TOP;

	return code;
}


void ProjectionDebugger::CohenSutherlandLineClipAndDraw(double x0, double y0, double x1, double y1)
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
			double x, y;

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
		IProjectionInfo::Pixel start, end;
		start.x = x0; start.y = y0;
		end.x = x1; end.y = y1;

		proj->LineBresenham(start, end,
			[&](int x, int y) -> void {
			this->rawData[x + y * proj->GetFrameWidth()] = 255;
		});
	}
}

void ProjectionDebugger::DrawImage(uint8_t * imData, int w, int h, IProjectionInfo * imProj)
{
	for (int y = 0; y < this->proj->GetFrameHeight(); y++)
	{
		for (int x = 0; x < this->proj->GetFrameWidth(); x++)
		{			
						
			IProjectionInfo::Coordinate cc = this->proj->ProjectInverse({ x,y });
			IProjectionInfo::Pixel p = imProj->Project(cc);

			if (p.x < 0) continue;
			if (p.y < 0) continue;
			if (p.x >= w) continue;
			if (p.y >= h) continue;

			this->rawData[x + y * this->proj->GetFrameWidth()] = imData[p.x + p.y * w];

		}
	}



	/*
	IProjectionInfo::Pixel topLeft = proj->Project(topLeftCorner);

	for (int y = topLeft.y; y < topLeft.y + h; y++)
	{
	for (int x = topLeft.x; x < topLeft.x + w; x++)
	{
	this->rawData[x + y * proj->GetFrameWidth()] = imData[(x - topLeft.x) + (y - topLeft.y) * w];
	}
	}
	*/
}

