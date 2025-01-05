#include <algorithm>
#include <cctype>
#include "FileOptions.h"
#include <filesystem>
#include <iostream>
#include <iterator>
#include "photofilefinder.h"
#include "PhotoFileList.h"
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

// std::filesystem can make lines very long.
namespace fs = std::filesystem;

using DirectoryMap = std::unordered_map<std::string, fs::path>;

using InputPhotoList = std::vector<fs::path>;

struct FindDirectoryData
{
    const std::string errString;
    const std::string mapIndex;
    const std::string target;
};

static fs::path findDirectory(const FindDirectoryData& fDD, fs::path& defaultDir)
{
    fs::path foundDir  = defaultDir;

    if (!fDD.target.empty())
    {
        foundDir = fs::current_path();
        foundDir.append(fDD.target);
        if (!fs::exists(foundDir))
        {
            std::cerr << "The " << fDD.errString << " directory " << foundDir.string() << 
                " can't be found!\n";
            foundDir.clear();
        }
    }

    return foundDir;
}

static DirectoryMap findAllDirectories(FileOptions& fileOptions)
{
    DirectoryMap dirMap;
    std::vector<FindDirectoryData> findDirectoryData =
    {
        {"photo source", "SourceDir", fileOptions.sourceDirectory},
        {"photo target", "TargetDir", fileOptions.targetDirectory},
        {"photo relocation", "RelocDir", fileOptions.relocDirectory}
    };

    fs::path defaultDir = fs::current_path();
    
    for (auto fDDi: findDirectoryData)
    {
        fs::path foundDir = findDirectory(fDDi, defaultDir);
        dirMap.insert({fDDi.mapIndex, foundDir});
        if (fDDi.mapIndex == "SourceDir")
        {
            defaultDir = foundDir;
        }
    }

    return dirMap;
}

static void addFilesToListByExtension(std::filesystem::path& cwd, const std::string& extLC, 
        const std::string& extUC, InputPhotoList& photoList)
{
    auto is_match = [extLC, extUC](auto f) {
        return f.path().extension().string() == extLC ||
            f.path().extension().string() == extUC;
    };

    auto files = fs::directory_iterator{ cwd }
        | std::views::filter([](auto& f) { return f.is_regular_file(); })
        | std::views::filter(is_match);

    std::ranges::copy(files, std::back_inserter(photoList));
}

static InputPhotoList findAllPhotos(fs::path& originsDir, FileOptions& fileOptions)
{
    InputPhotoList tempFileList;

    if (fileOptions.processJPGFiles)
    {
        addFilesToListByExtension(originsDir, ".jpg", ".JPG", tempFileList);
    }

    if (fileOptions.processPNGFiles)
    {
        addFilesToListByExtension(originsDir, ".png", ".PNG", tempFileList);
    }

    return tempFileList;
}

static std::string makeFileNameWebSafe(const std::string& inName)
{
    std::string webSafeName;

    auto toUnderScore = [](unsigned char c) -> unsigned char { return std::isalnum(c)? c : '_'; };

    std::ranges::transform(inName, std::back_inserter(webSafeName), toUnderScore);

    return webSafeName;
}

static std::string makeOutputFileName(
    const fs::path& inputFile,
    const fs::path& targetDir,
    FileOptions& fileOptions
)
{
    std::string ext = inputFile.extension().string();
    std::string outputFileName = inputFile.stem().string();

    if (fileOptions.fixFileName)
    {
        outputFileName = makeFileNameWebSafe(outputFileName);
    }

    if (!fileOptions.resizedPostfix.empty())
    {
        outputFileName += "." + fileOptions.resizedPostfix;
    }

    outputFileName += ext;

    fs::path targetFile = targetDir;
    targetFile.append(outputFileName);

    if (fs::exists(targetFile) && !fileOptions.overWriteFiles)
    {
        std::cerr << "Warning: Attempting to overwrite existing file: " << targetFile << ". Use \'--overwrite\' to overwrite files.\n";
        targetFile.clear();
    }

    return targetFile.string();
}

static PhotoFileList copyInFileNamesToPhotoListAddOutFileNames(
    FileOptions& fileOptions,
    InputPhotoList& inFileList,
    fs::path& targetDir
)
{
    PhotoFileList photoFileList;

    for (auto const& file: inFileList)
    {
        PhotoFile currentPhoto;
        currentPhoto.inputName = file.string();
        currentPhoto.outputName = makeOutputFileName(file, targetDir, fileOptions);
        photoFileList.push_back(currentPhoto);
    }

    return photoFileList;
}

PhotoFileList buildPhotoInputAndOutputList(FileOptions& fileOptions)
{
    PhotoFileList photoFileList;
    DirectoryMap directories = findAllDirectories(fileOptions);

    for (auto& directory: directories)
    {
        if (directory.second.empty())
        {
            return photoFileList;
        }
    }

    fs::path sourceDir = directories.find("SourceDir")->second;;

    InputPhotoList inputPhotoList = findAllPhotos(sourceDir, fileOptions);
    
    if (inputPhotoList.size())
    {
        fs::path targetDir = directories.find("TargetDir")->second;
        photoFileList = copyInFileNamesToPhotoListAddOutFileNames(
            fileOptions, inputPhotoList, targetDir);
    }
    else
    {
        std::cerr << "No photos found to resize!\n";
    }

    return photoFileList;
}
