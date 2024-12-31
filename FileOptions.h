#ifndef FILE_OPTIONS_H_
#define FILE_OPTIONS_H_

#include <string>

struct FileOptions
{
    bool fixFileName = false;
    bool processJPGFiles = true;
    bool processPNGFiles = false;
    bool overWriteFiles = false;
    std::string sourceDirectory;
    std::string targetDirectory;
	std::string relocDirectory;
    std::string resizedPostfix;
};

#endif // FILE_OPTIONS_H_
