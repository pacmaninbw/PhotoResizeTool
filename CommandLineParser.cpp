#include <boost/program_options.hpp>
#include "CommandLineParser.h"
#include "Executionctrlvalues.h"
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

static std::string simplify_name(char *path)
{
	return std::filesystem::path{path ? path : "PhotoResizeTool"}.filename().string();
}

namespace po = boost::program_options;

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
		("only-jpg", "Only process the JPEG format photos (default)")
		("only-png", "Only process the PNG format photos")
		("jpg-and-png", "Process both the JPEG and PNG format photos")
		("display-resized", "Show the resized photo")
		("time-resize", "Time the resizing of the photos")
	;

	return options;
}

static bool fileExtensionErrorCheck(po::variables_map& inputOptions)
{
	bool noUserError = true;

	if (inputOptions.count("only-jpg") && (inputOptions.count("jpg-and-png") || inputOptions.count("only-png")))
	{
		noUserError = false;
	}

	if (inputOptions.count("jpg-and-png") && (inputOptions.count("only-jpg") || inputOptions.count("only-png")))
	{
		noUserError = false;
	}

	if (inputOptions.count("only-png") && (inputOptions.count("only-jpg") || inputOptions.count("jpg-and-png")))
	{
		noUserError = false;
	}

	if (!noUserError)
	{
		std::cerr << "use only one of --only-jpg, --only-png or --jpg-and-png.\n";
	}

	return noUserError;
}

static void setExtensionValues(po::variables_map& inputOptions, FileCtrlValues& fileCtrl)
{
	// Provide a default of JPG files.
	if (inputOptions.count("only-jpg") ||
		(!inputOptions.count("only-jpg") && !inputOptions.count("only-png") &&
		!inputOptions.count("jpg-and-png"))) {
		fileCtrl.processJPGFiles = true;
		fileCtrl.processPNGFiles = false;
	}

	if (inputOptions.count("only-png")) {
		fileCtrl.processJPGFiles = false;
		fileCtrl.processPNGFiles = true;
	}

	if (inputOptions.count("jpg-and-png")) {
		fileCtrl.processJPGFiles = true;
		fileCtrl.processPNGFiles = true;
	}
}

static bool convertInputToFileCtrlValues(po::variables_map& inputOptions, FileCtrlValues& fileCtrl)
{
	if (!fileExtensionErrorCheck(inputOptions))
	{
		return false;
	}

	setExtensionValues(inputOptions, fileCtrl);

	if (inputOptions.count("save-dir")) {
		fileCtrl.imageTargetDirectory = inputOptions["save-dir"].as<std::string>();
	}

	if (inputOptions.count("source-dir")) {
		fileCtrl.sourceDirectory = inputOptions["source-dir"].as<std::string>();
	}

	if (inputOptions.count("extend-filename")) {
		fileCtrl.resizedPostfix = inputOptions["extend-filename"].as<std::string>();
	}

	if (inputOptions.count("web-safe-name")) {
		fileCtrl.fixFileName = true;
	}

	return true;
}

static bool maintainRatioCheck(po::variables_map& inputOptions, PhotCtrlValues& photoCtrl)
{
	if (inputOptions.count("maintain-ratio")) {
		if (inputOptions.count("max-height") && inputOptions.count("max-width"))
		{
			std::cerr << "Only one of --max-width or --max-height can be specified with maintain-ratio\n";
			return false;
		}

		if (!inputOptions.count("max-height") && !inputOptions.count("max-width"))
		{
			std::cerr << "A maximum size, either width or height must be specified with maintain-ratio\n";
			return false;
		}

		photoCtrl.maintainRatio = true;
	}

	return true;
}

static bool photoSizeRequirements(po::variables_map& inputOptions, PhotCtrlValues& photoCtrl)
{
	if (!maintainRatioCheck(inputOptions, photoCtrl))
	{
		return false;
	}

	if (!inputOptions.count("max-height") && !inputOptions.count("max-width") &&
		!inputOptions.count("percentage"))
	{
		std::cerr << "A new size must be specified using --percentage, --max-width or --max-height\n";
		return false;
	}

	if (inputOptions.count("max-width"))
	{
		photoCtrl.maxWdith = inputOptions["max-width"].as<std::size_t>();
	}

	if (inputOptions.count("max-height"))
	{
		photoCtrl.maxHeight = inputOptions["max-height"].as<std::size_t>();
	}

	if (inputOptions.count("percentage"))
	{
		photoCtrl.reductionToPercentage = inputOptions["percentage"].as<unsigned int>();
	}

	return true;
}

static bool convertInputToPhotoCtrlValues(po::variables_map& inputOptions, PhotCtrlValues& photoCtrl)
{

	if (!photoSizeRequirements(inputOptions, photoCtrl))
	{
		return false;
	}

	if (inputOptions.count("display-resized"))
	{
		photoCtrl.displayImage = true;
	}

	return true;
}

static bool covertInputToExecutable(po::variables_map& inputOptions, ExecutionCtrlValues& executionCtrl)
{
	if (!convertInputToPhotoCtrlValues(inputOptions, executionCtrl.pCtrlV))
	{
		return false;
	}

	if (!convertInputToFileCtrlValues(inputOptions, executionCtrl.fCtrlV))
	{
		return false;
	}

	if (inputOptions.count("time-resize")) {
		executionCtrl.enableExecutionTime = true;
	}

	return true;
}

static const int MinArgCount = 2;
static std::string usageStr =
	" :\n\tReduce the size of all the photos in the specified folder.\n"
	"\tIf no source location is specifed the photos must be in the current folder.\n"
	"\tA reduction size value must be specified, either by maximum width, maixmum\n"
	"\theight or a percentage of the current size.\n"
;

bool processCommandLine(int argc, char* argv[], ExecutionCtrlValues& executionCtrl)
{
	executionCtrl.progName = simplify_name(argv[0]);

	po::options_description options = addOptions();

	po::variables_map optionMemory;        
	po::store(po::parse_command_line(argc, argv, options), optionMemory);
	po::notify(optionMemory);    

	if (argc < MinArgCount)
	{
		std::cout << executionCtrl.progName << usageStr << "\n";
		std::cout << options << "\n";
		return false;
	}

	if (optionMemory.count("help")) {
		std::cout << options << "\n";
		return false;
	}

	if (!covertInputToExecutable(optionMemory, executionCtrl))
	{
		std::cout << executionCtrl.progName << usageStr << "\n";
		std::cout << options << "\n";
		return false;
	}

    return true;
}


