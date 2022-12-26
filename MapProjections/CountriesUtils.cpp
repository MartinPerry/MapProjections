#include "./CountriesUtils.h"

#include <sstream>

using namespace Projections;

CountriesUtils::CountriesUtils()
{
}

CountriesUtils::~CountriesUtils()
{
}

const std::unordered_map<std::string, std::vector<Coordinate>>& CountriesUtils::GetBorders() const
{
	return this->borderDara;
}

const std::array<Coordinate, 2> CountriesUtils::GetCountryBoundingBox(const char* countryId) const
{	
	auto tmp = countriesParts.find(countryId);
	if (tmp == countriesParts.end())
	{
		return std::array<Coordinate, 2>();
	}
	
	Coordinate minVal;
	minVal.lat = Latitude::rad(std::numeric_limits<float>::max());
	minVal.lon = Longitude::rad(std::numeric_limits<float>::max());
	Coordinate maxVal;
	maxVal.lat = Latitude::rad(std::numeric_limits<float>::lowest());
	maxVal.lon = Longitude::rad(std::numeric_limits<float>::lowest());

	for (const auto& it : tmp->second)
	{
		auto jt = borderDara.find(it);
		for (const auto& c : jt->second)
		{
			if (minVal.lat > c.lat) minVal.lat = c.lat;
			if (minVal.lon > c.lon) minVal.lon = c.lon;

			if (maxVal.lat < c.lat) maxVal.lat = c.lat;
			if (maxVal.lon < c.lon) maxVal.lon = c.lon;
		}
	}

	return { minVal, maxVal };
}

void CountriesUtils::Load(const char* fileName, int useEveryNthPoint)
{
	std::string content = this->LoadFromFile(fileName);


	std::vector<std::string> lines = this->Split(content, '\n');

	std::string countryId;
	std::string countryName;
	int tmp = 0;

	for (size_t i = 0; i < lines.size(); i++)
	{
		std::vector<std::string> line = this->Split(lines[i], ';');
		if (line.size() <= 2)
		{
			continue;
		}
		
		if (line.size() > 5)
		{
			countryId = line[5];
			countryName = line[7];
			tmp = 0;

			this->names[countryId] = countryName;
		}
		else
		{
			if (tmp % useEveryNthPoint != 0)
			{
				tmp++;
				continue;
			}
			
			std::string key = countryId;
			key += "_";
			key += line[3];

			if (!key.empty() && key[key.size() - 1] == '\r')
			{
				key.erase(key.size() - 1);
			}

			countriesParts[countryId].insert(key);

			Coordinate point;
			point.lon = Longitude::deg(atof(line[0].c_str()));
			point.lat = Latitude::deg(atof(line[1].c_str()));
			this->borderDara[key].push_back(point);

			tmp++;
		}

	}
}

/// <summary>
/// Load input data from file
/// </summary>
/// <param name="filePath"></param>
/// <returns></returns>
std::string CountriesUtils::LoadFromFile(const char* filePath) const
{
	FILE* f = NULL;  //pointer to file we will read in
	my_fopen(&f, filePath, "rb");
	if (f == NULL)
	{
		printf("Failed to open file: \"%s\"\n", filePath);
		return "";
	}

	fseek(f, 0L, SEEK_END);
	long size = ftell(f);
	fseek(f, 0L, SEEK_SET);

	char* data = new char[size + 1];
	fread(data, sizeof(char), size, f);
	fclose(f);

	data[size] = 0;
	std::string tmp = data;
	delete[] data;

	return tmp;
};

std::vector<std::string> CountriesUtils::Split(const std::string& s, char delim) const
{
	std::stringstream ss;
	ss.str(s);
	std::string item;
	std::vector<std::string> elems;
	while (std::getline(ss, item, delim)) 
	{
		elems.push_back(item);
	}

	return elems;
}