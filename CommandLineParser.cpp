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

enum class PhotoOptionError
{
	no_error,
	maintain_ratio_both_specified,
	maintain_ration_no_size,
	too_many_sizes,
	no_size
};

enum class ProgramOptionStatus
{
	no_error,
	photo_option_error,
	file_opetion_error
};

static po::options_description addOptions()
{
	po::options_description options("Options and arguments");
	options.add_options()
		("help", "Show this help message")
		("max-width", po::value<std::size_t>(), "The maximum width of the resized photo")
		("max-height", po::value<std::size_t>(), "The maximum height of the resized photo")
		("maintain-ratio", "Maintain the current ratio of width to height")
		("percentage", po::value<unsigned int>(), "The new size of the photo as a percentage of the old size")
		("source-dir", po::value<std::string>(), "Where to find the original photos")
		("save-dir", po::value<std::string>(), "Where to save the resized photos")
		("extend-filename", po::value<std::string>(), "Add the specified string to the resized photo")
		("web-safe-name", "Change all non alpha numeric characters in filename to underscore")
		("all-jpg-files", "Process all the JPEG format photos")
		("all-png-files", "Process all the PNG format photos")
		("display-resized", "Show the resized photo")
		("time-resize", "Time the resizing of the photos")
	;

	return options;
}


static FileOptions processFileOptions(po::variables_map& inputOptions)
{
	FileOptions fileOptions;

	if (inputOptions.count("all-jpg-files")) 
	{
		fileOptions.processJPGFiles = true;
	}

	if (inputOptions.count("all-png-files")) {
		fileOptions.processPNGFiles = true;
	}

	if (inputOptions.count("save-dir")) {
		fileOptions.targetDirectory = inputOptions["save-dir"].as<std::string>();
	}

	if (inputOptions.count("source-dir")) {
		fileOptions.sourceDirectory = inputOptions["source-dir"].as<std::string>();
	}

	if (inputOptions.count("extend-filename")) {
		fileOptions.resizedPostfix = inputOptions["extend-filename"].as<std::string>();
	}

	if (inputOptions.count("web-safe-name")) {
		fileOptions.fixFileName = true;
	}

	return fileOptions;
}

static PhotoOptionError checkRatioForErrors(po::variables_map& inputOptions)
{
	if (inputOptions.count("max-height") && inputOptions.count("max-width"))
	{
		std::cerr << "Only one of --max-width or --max-height can be specified with maintain-ratio\n";
		return PhotoOptionError::maintain_ratio_both_specified;
	}

	if (!inputOptions.count("max-height") && !inputOptions.count("max-width"))
	{
		std::cerr << "A maximum size, either width or height must be specified with maintain-ratio\n";
		return PhotoOptionError::maintain_ration_no_size;
	}

	return PhotoOptionError::no_error;
}

static bool hasSize(po::variables_map& inputOptions)
{
	return inputOptions.count("max-height") || inputOptions.count("max-width") ||
		inputOptions.count("percentage");
}

static auto processPhotoOptions(po::variables_map& inputOptions) -> 
	std::expected<PhotoOptions, PhotoOptionError>
{
	PhotoOptions photoCtrl;
	
	if (inputOptions.count("maintain-ratio")) {
		PhotoOptionError ratioCheck = checkRatioForErrors(inputOptions);
		if (ratioCheck != PhotoOptionError::no_error)
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

		photoCtrl.scaleFactor = (inputOptions.count("percentage")) ?
			inputOptions["percentage"].as<unsigned int>() : 0;
	}
	else
	{
		std::cerr << "A new size must be specified using --percentage, --max-width or --max-height\n";
		return std::unexpected(PhotoOptionError::no_size);
	}

	if (inputOptions.count("display-resized"))
	{
		photoCtrl.displayResized = true;
	}

	return photoCtrl;
}

static auto processProgramOptions(po::variables_map& inputOptions) -> 
	std::expected<ProgramOptions, ProgramOptionStatus>
{
	ProgramOptions programOptions;

	if (const auto pOptions = processPhotoOptions(inputOptions); pOptions.has_value())
	{
		programOptions.photoOptions = *pOptions;
	}
	else
	{
		return std::unexpected(ProgramOptionStatus::photo_option_error);
	}

	programOptions.fileOptions = processFileOptions(inputOptions);

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

auto parseCommandLine(int argc, char* argv[]) -> 
	std::expected<ProgramOptions, CommandLineStatus>
{
	ProgramOptions programOptions;

	std::string progName = simplifyName(argv[0]);

	po::options_description options = addOptions();

	if (argc < MinArgCount)
	{
		std::cout << progName << usageStr << "\n";
		std::cout << options << "\n";
		return std::unexpected(CommandLineStatus::HasErrors);
	}

	po::variables_map optionMemory;        
	po::store(po::parse_command_line(argc, argv, options), optionMemory);
	po::notify(optionMemory);    

	if (optionMemory.count("help")) {
		std::cout << options << "\n";
		return std::unexpected(CommandLineStatus::HelpRequested);
	}

	if (const auto progOptions = processProgramOptions(optionMemory); progOptions.has_value())
	{
		programOptions = *progOptions;
		programOptions.progName = progName;
	}
	else
	{
		std::cout << progName << usageStr << "\n";
		std::cout << options << "\n";
		return std::unexpected(CommandLineStatus::HasErrors);
	}

	return programOptions;
}


