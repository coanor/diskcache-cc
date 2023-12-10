#include <string>
#include <filesystem>

#include "err.hpp"

class pos {
public:
	long seek;
	std::filesystem::path name;

	error dumpFile();
	error reset();
};
