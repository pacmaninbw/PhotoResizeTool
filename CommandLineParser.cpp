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
		("min-width", po::value<std::size_t>(), "The minimum width of the resized photo")
		("max-height", po::value<std::size_t>(), "The maximum height of the resized photo")
		("min-height", po::value<std::size_t>(), "The minimum height of the resized photo")
		("reduce-to-percent", po::value<unsigned int>(), "The new size of the photo as a percentage of the old size")
		("originals-location", po::value<std::string>(), "Where to find the original photos")
		("resized-photo-store", po::value<std::string>(), "Where to save the resized photos")
		("extend-filename", po::value<std::string>(), "Add the specified string to the resized photo")
		("only-jpg", "Only process the JPEG format photos (default)")
		("only-png", "Only process the PNG format photos")
		("jpg-and-png", "Process both the JPEG and PNG format photos")
		("display-resized", "Show the resized photo")
		("maintain-ratio", "Maintain the current ratio of width to height")
		("time-resize", "Time the resizing of the photos")
		("nonalpha-to-underscore", "Change all non alpha numeric characters in filename to underscore")
	;

	return options;
}

static bool fileExtensionCheck(po::variables_map& inputOptions)
{
	bool noUserError = true;

	if (inputOptions.count("only-jpg") && (inputOptions.count("jpg-and-png") || inputOptions.count("only-png")))
	{
	}

	if (!noUserError)
	{
		std::cerr << "use only one of --only-jpg, --only-png or --jpg-and-png.\n";
	}

	return noUserError;
}

static bool convertInputToFileCtrlValues(po::variables_map& inputOptions, FileCtrlValues& fileCtrl)
{
	if (!fileExtensionCheck(inputOptions))
	{
		return false;
	}

	if (inputOptions.count("resized-photo-store")) {
		fileCtrl.imageTargetDirectory = inputOptions["resized-photo-store"].as<std::string>();
	}

	if (inputOptions.count("originals-location")) {
		fileCtrl.sourceDirectory = inputOptions["originals-location"].as<std::string>();
	}

	if (inputOptions.count("extend-filename")) {
		fileCtrl.resizedPostfix = inputOptions["extend-filename"].as<std::string>();
	}

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

	if (inputOptions.count("nonalpha-to-underscore")) {
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
		!inputOptions.count("reduce-to-percent"))
	{
		std::cerr << "A new size must be specified using --reduce-to-percent, --max-width or --max-height\n";
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

	if (inputOptions.count("reduce-to-percent"))
	{
		photoCtrl.reductionToPercentage = inputOptions["reduce-to-percent"].as<unsigned int>();
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
static std::string usageStr = " :\n\tResize all the photos in the specified folder.\n"
	"\tIf no original location is specifed the photos must be in the current folder.\n"
	"\tA maximum size either width or height must be specified. If the current width\n"
	"\tto height ratio should be maintained only specify either the maximum width or\n"
	"\tthe maximum height with  --maintain-ratio.\n"
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

    return covertInputToExecutable(optionMemory, executionCtrl);
}


