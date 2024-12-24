#ifndef PROGRAMOPTIONS_H_
#define PROGRAMOPTIONS_H_

/*
 * Storage for environment and commandline argument variables. These structures
 * contain most of the information to execute the program after the command 
 * line has been processed.
 * 
 * The list of photo files is separate.
 */
#include "FileOptions.h"
#include "PhotoOptions.h"
#include <string>

struct ProgramOptions
{
    std::string progName;
	bool enableExecutionTime = false;
    FileOptions fileOptions;
    PhotoOptions photoOptions;
};

#endif // PROGRAMOPTIONS_H_
