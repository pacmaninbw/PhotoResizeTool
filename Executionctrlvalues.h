#ifndef EXECUTION_CONTROL_VARIABLES_H
#define EXECUTION_CONTROL_VARIABLES_H

/*
 * Storage for environment and commandline argument variables. These structures
 * contain most of the information to execute the program after the command 
 * line has been processed.
 * 
 * The list of photo files is separate.
 */
#include "fileCtrlValues.h"
#include "photoCtrlValues.h"
#include <string>

struct ExecutionCtrlValues
{
    std::string progName;
	bool enableExecutionTime = false;
    FileCtrlValues fCtrlV;
    PhotCtrlValues pCtrlV;
};

#endif // EXECUTION_CONTROL_VARIABLES_H
