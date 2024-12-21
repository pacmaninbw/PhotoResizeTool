#ifndef FILE_CONTROL_VARIABLES_H_
#define FILE_CONTROL_VARIABLES_H_

#include <string>

struct FileCtrlValues
{
    bool fixFileName = false;
    bool processJPGFiles = true;
    bool processPNGFiles = false;
    std::string sourceDirectory;
    std::string imageTargetDirectory;
	std::string imageProcessedDirectory;
    std::string resizedPostfix;
};

#endif // FILE_CONTROL_VARIABLES_H_
