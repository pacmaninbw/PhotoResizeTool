#ifndef COMMAND_LINE_PARSER_H_
#define COMMAND_LINE_PARSER_H_

#include <expected>
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

enum class CommandLineStatus
{
    NoErrors,
    HelpRequested,
    HasErrors
};

auto parseCommandLine(int argc, char* argv[]) -> std::expected<ProgramOptions, CommandLineStatus>;

#endif // COMMAND_LINE_PARSER_H_
