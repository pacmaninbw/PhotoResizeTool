#include <algorithm>
#include <boost/program_options.hpp>
#include "CommandLineParser.h"
#include <expected>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

static std::string simplifyName(char *path)
{
	return std::filesystem::path{path ? path : "PhotoResizeTool"}.filename().string();
}

namespace po = boost::program_options;

static const char optionStarter = '-';

enum class ProgOptStatus
{
	NoErrors,
	HasPhotoOptionError,
	HasFileOptionError,
	NoSize,
	MaintainRatioBothSpecified,
	MaintainRatioNoSize,
	MissingArgument,
	TooManySizes
};

static po::options_description addOptions()
{
	po::options_description options("Options and arguments");
	options.add_options()
		("help", "Show this help message")
		("max-width", po::value<std::size_t>(), "The maximum width of the resized photo")
		("max-height", po::value<std::size_t>(), "The maximum height of the resized photo")
		("maintain-ratio", "Maintain the current ratio of width to height")
		("scale-factor", po::value<unsigned int>(),
			"The new size of the photo as a percentage of the old size")
		("source-dir", po::value<std::string>(), "Where to find the original photos")
		("save-dir", po::value<std::string>(), "Where to save the resized photos")
		("extend-filename", po::value<std::string>(),
			"Add the specified string to the resized photo")
		("overwrite", "Overwrite existing output files")
		("web-safe-name", "Change all non alpha numeric characters in filename to underscore")
		("all-jpg-files", "Process all the JPEG format photos")
		("all-png-files", "Process all the PNG format photos")
		("display-resized", "Show the resized photo")
		("time-resize", "Time the resizing of the photos")
	;

	return options;
}

/*
 * hasArgument is a workaround for the fact that the boost::program_options
 * doesn't or can't check to see if the string following an option is another
 * option. 
 */
static auto hasArgument(po::variables_map& inputOptions, const std::string& option) -> 
	std::expected<std::string, ProgOptStatus>
{
	std::string argument("");
	if (!inputOptions.count(option))
	{
		return argument;	// No argument to check
	}

	argument = inputOptions[option].as<std::string>();
	po::options_description options = addOptions();

	if (argument[0] == optionStarter)
	{
		/*
		 * Remove any preceeding optionStarter, boost::program_options does not store
		 * the option start.
		 */
		auto startOptionText = std::find_if_not(argument.begin(), argument.end(),
			[](char c) { return c == optionStarter; });
		if (startOptionText != argument.end())
		{
			if (options.find_nothrow(std::string(startOptionText, argument.end()), true))
			{
				std::cerr << "The option " << option << " is missing the required argument!\n";
				return std::unexpected(ProgOptStatus::MissingArgument);
			}
		}
	}

	return argument;
}

static auto checkFileOptionsWithArguments(po::variables_map& inputOptions) -> 
	std::expected<FileOptions, ProgOptStatus>
{
	FileOptions fileOptions;
	std::vector<std::string> optionsForArgCheck = {"save-dir", "source-dir", "extend-filename"};
	std::vector<std::string *> fileOptionValues = 
	{
		{&fileOptions.targetDirectory},
		{&fileOptions.sourceDirectory},
		{&fileOptions.resizedPostfix}
	};
	ProgOptStatus hasArguments = ProgOptStatus::NoErrors;
	
	// Process as many options as possible.
	std::size_t i = 0;
	for (auto optionToCheck: optionsForArgCheck)
	{
		const auto argCheck = hasArgument(inputOptions, optionToCheck);
		if (argCheck.has_value())
		{
			*fileOptionValues[i] = *argCheck;
		}
		else
		{
			hasArguments = (hasArguments == ProgOptStatus::NoErrors)?
				argCheck.error() : hasArguments;
		}
		++i;
	}

	if (hasArguments != ProgOptStatus::NoErrors)
	{
		return std::unexpected(hasArguments);
	}

	return fileOptions;
}

static auto processFileOptions(po::variables_map& inputOptions) -> 
	std::expected<FileOptions, ProgOptStatus>
{
	FileOptions fileOptions;

	const auto argCheck = checkFileOptionsWithArguments(inputOptions);
	if (argCheck.has_value())
	{
		fileOptions = *argCheck;
	}
	else
	{
		return std::unexpected(argCheck.error());
	}

	if (inputOptions.count("all-jpg-files")) 
	{
		fileOptions.processJPGFiles = true;
	}

	if (inputOptions.count("all-png-files")) {
		fileOptions.processPNGFiles = true;
	}

	if (inputOptions.count("web-safe-name")) {
		fileOptions.fixFileName = true;
	}

	if (inputOptions.count("overwrite"))
	{
		fileOptions.overWriteFiles = true;
	}

	return fileOptions;
}

static ProgOptStatus checkRatioForErrors(po::variables_map& inputOptions)
{
	if (inputOptions.count("max-height") && inputOptions.count("max-width"))
	{
		std::cerr << "Only one of --max-width or --max-height can be specified with maintain-ratio\n";
		return ProgOptStatus::MaintainRatioBothSpecified;
	}

	if (!inputOptions.count("max-height") && !inputOptions.count("max-width"))
	{
		std::cerr << "A maximum size, either width or height must be specified with maintain-ratio\n";
		return ProgOptStatus::MaintainRatioNoSize;
	}

	return ProgOptStatus::NoErrors;
}

static bool hasSize(po::variables_map& inputOptions)
{
	return inputOptions.count("max-height") || inputOptions.count("max-width") ||
		inputOptions.count("scale-factor");
}

static auto processPhotoOptions(po::variables_map& inputOptions) -> 
	std::expected<PhotoOptions, ProgOptStatus>
{
	PhotoOptions photoCtrl;
	
	if (inputOptions.count("maintain-ratio")) {
		ProgOptStatus ratioCheck = checkRatioForErrors(inputOptions);
		if (ratioCheck != ProgOptStatus::NoErrors)
		{
			return std::unexpected(ratioCheck);
		}
		photoCtrl.maintainRatio = true;
	}

	if (hasSize(inputOptions))
	{
		photoCtrl.maxWdith = (inputOptions.count("max-width")) ?
			inputOptions["max-width"].as<std::size_t>() : 0;

		photoCtrl.maxHeight = (inputOptions.count("max-height")) ?
			inputOptions["max-height"].as<std::size_t>() : 0;

		photoCtrl.scaleFactor = (inputOptions.count("scale-factor")) ?
			inputOptions["scale-factor"].as<unsigned int>() : 0;
	}
	else
	{
		std::cerr << "A new size must be specified using --percentage, --max-width or --max-height\n";
		return std::unexpected(ProgOptStatus::NoSize);
	}

	if (inputOptions.count("display-resized"))
	{
		photoCtrl.displayResized = true;
	}

	return photoCtrl;
}

static auto processProgramOptions(po::variables_map& inputOptions,
	const std::string& progName) -> std::expected<ProgramOptions, ProgOptStatus>
{
	ProgramOptions programOptions;

	programOptions.progName = progName;

	if (const auto pOptions = processPhotoOptions(inputOptions); pOptions.has_value())
	{
		programOptions.photoOptions = *pOptions;
	}
	else
	{
		return std::unexpected(pOptions.error());
	}

	if (const auto fOptions = processFileOptions(inputOptions); fOptions.has_value())
	{
		programOptions.fileOptions = *fOptions;
	}
	else
	{
		return std::unexpected(fOptions.error());
	}

	if (inputOptions.count("time-resize")) {
		programOptions.enableExecutionTime = true;
	}

	return programOptions;
}

static const int MinArgCount = 2;
static std::string usageStr =
	" :\n\tReduce the size of all the photos in the specified folder.\n"
	"\tIf no source location is specified the photos must be in the current folder.\n"
	"\tA reduction size value must be specified, either by maximum width, maximum\n"
	"\theight or a percentage of the current size.\n"
;

static CommandLineStatus usage(const std::string& progName,
	const po::options_description& options,
	const std::string errorMessage
	)
{
	if (errorMessage.length())
	{
		std::cerr << errorMessage << "\n";
	}
	std::cerr << progName << usageStr << "\n" << options << "\n";
	return CommandLineStatus::HasErrors;
}

static CommandLineStatus help(const std::string& progName,
	const po::options_description& options)
{
	std::cout << progName << options << "\n";
	return CommandLineStatus::HelpRequested;
}

auto parseCommandLine(int argc, char* argv[]) -> 
	std::expected<ProgramOptions, CommandLineStatus>
{
	ProgramOptions programOptions;
	std::string progName = simplifyName(argv[0]);
	po::options_description options = addOptions();

	if (argc < MinArgCount)
	{
		return std::unexpected(usage(progName, options,
			"Missing the required new size for the resized photos."));
	}

	po::variables_map optionMemory;        
	try
	{
		po::store(po::parse_command_line(argc, argv, options), optionMemory);
		po::notify(optionMemory);    
	}
	/*
	 * Handle any exceptions thrown by boost::program_options.
	 */
	catch(const std::exception& e)
	{
		return std::unexpected(usage(progName, options, e.what()));
	}
	
	if (optionMemory.count("help")) {
		return std::unexpected(help(progName, options));
	}

	if (const auto progOptions = processProgramOptions(optionMemory, progName); progOptions.has_value())
	{
		programOptions = *progOptions;
	}
	else
	{
		return std::unexpected(usage(progName, options, ""));
	}

	return programOptions;
}


