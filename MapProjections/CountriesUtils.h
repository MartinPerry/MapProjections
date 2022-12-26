#ifndef COUNTRIES_UTILS_H
#define COUNTRIES_UTILS_H

#include <vector>
#include <string>
#include <array>
#include <set>
#include <unordered_map>

#include "./MapProjectionStructures.h"

namespace Projections
{
	class CountriesUtils
	{
	public:
		CountriesUtils();
		~CountriesUtils();

		const std::unordered_map<std::string, std::vector<Coordinate>>& GetBorders() const;

		const std::array<Coordinate, 2> GetCountryBoundingBox(const char* countryId) const;

		void Load(const char* fileName, int useEveryNthPoint = 1);
		
	protected:

		std::unordered_map<std::string, std::vector<Coordinate>> borderDara;
		std::unordered_map<std::string, std::string> names;
		std::unordered_map<std::string, std::set<std::string>> countriesParts;

		std::string LoadFromFile(const char* filePath) const;
		std::vector<std::string> Split(const std::string& s, char delim) const;
	};
};


#endif
