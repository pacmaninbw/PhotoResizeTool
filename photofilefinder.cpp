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

static void findDirectory(std::string& target, const std::string& errString, 
     fs::path& defaultDir, const std::string& dirIndex, DirectoryMap& dirMap
)
{
    fs::path foundDir  = defaultDir;

    if (!target.empty())
    {
        foundDir = fs::current_path();
        foundDir.append(target);
        if (!fs::exists(foundDir))
        {
            std::cerr << "The " << errString << " directory " << target << 
                " can't be found!\n";
            foundDir.clear();
        }
    }

    dirMap.insert({dirIndex, foundDir});
}

static DirectoryMap findAllDirectories(FileOptions& fileOptions)
{
    DirectoryMap dirMap;
    fs::path defaultDir = fs::current_path();
    
    findDirectory(fileOptions.sourceDirectory, "photo source", defaultDir, "SourceDir", dirMap);
    defaultDir = dirMap.find("SourceDir")->second;

    findDirectory(fileOptions.targetDirectory, "photo target", defaultDir, "TargetDir", dirMap);
    findDirectory(fileOptions.relocDirectory, "photo target", defaultDir, "RelocDir", dirMap);

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

    if (fs::exists(targetFile))
    {
        std::cerr << "Do you want to replace the photo: " << targetFile.string() << "?\n(y | n)>>\n";
        std::string response;
        std::cin >> response;
        if (response == "n")
        {
            targetFile.clear();
        }
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
        if (currentPhoto.outputName.empty())
        {
            // If there was a replacement that the user replied no to, do they want to quit?
            std::cerr << "Do you want to quit?(y / n)\n";
            std::string response;
            std::cin >> response;
            if (response == "y")
            {
                photoFileList.clear();
                return photoFileList;
            }
        }
        else
        {
            photoFileList.push_back(currentPhoto);
        }
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
