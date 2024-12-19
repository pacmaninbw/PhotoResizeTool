#include "CommandLineParser.h"
#include "Executionctrlvalues.h"
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
		ExecutionCtrlValues executionCtrl;

		if (processCommandLine(argc, argv, executionCtrl))
		{
			PhotoFileList photoFiles = buildPhotoInputAndOutputList(executionCtrl.fCtrlV);

			if (photoFiles.size() > 0)
			{
				UtilityTimer stopWatch;

				if (resizeAllPhotosInList(executionCtrl.pCtrlV, photoFiles) != photoFiles.size())
				{
					std::cerr << "Not all photos were resized\n";
					executionStatus = EXIT_FAILURE;
				}

				std::string report(std::to_string(photoFiles.size()) + " photos resized\n");

				if (executionCtrl.enableExecutionTime)
				{
					stopWatch.stopTimerAndReport(report);
				}
				else
				{
					std::cout << report;
				}
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

