#include "CommandLineParser.h"
#include "ProgramOptions.h"
#include <iostream>
#include "PhotoFileList.h"
#include "photofilefinder.h"
#include "PhotoResizer.h"
#include "UtilityTimer.h"

int main(int argc, char* argv[])
{
	int executionStatus = EXIT_SUCCESS;
	std::locale::global(std::locale{""});
	std::clog.imbue(std::locale{});

	try
	{
		ProgramOptions programOptions;
		std::size_t resizeCount = 0;

		if (processCommandLine(argc, argv, programOptions))
		{
			PhotoFileList photoFiles = buildPhotoInputAndOutputList(programOptions.fileOptions);

			UtilityTimer stopWatch;

			resizeCount = resizeAllPhotosInList(programOptions.photoOptions, photoFiles);
			if (resizeCount != photoFiles.size())
			{
				std::cerr << "Not all photos were resized\n";
				executionStatus = EXIT_FAILURE;
			}

			std::string report(std::to_string(resizeCount) + " of " + 
				std::to_string(photoFiles.size()) + " photos resized\n");

			if (programOptions.enableExecutionTime)
			{
				stopWatch.stopTimerAndReport(report);
			}
			else
			{
				std::cout << report;
			}
		}
	}

	catch (const std::exception &ex)
	{
		std::cerr << "Error: " << ex.what() << "\n";
		return EXIT_FAILURE;
	}

	return executionStatus;
}

