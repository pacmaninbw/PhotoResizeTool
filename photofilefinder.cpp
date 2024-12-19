#include <algorithm>
#include <cctype>
#include "fileCtrlValues.h"
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
namespace fsys = std::filesystem;

using DirectoryMap = std::unordered_map<std::string, fsys::path>;

static void findDirectory(std::string target, std::string errString, 
     fsys::path defaultDir, std::string dirIndex, DirectoryMap& dirMap
)
{
    fsys::path foundDir  = defaultDir;

    if (!target.empty())
    {
        foundDir = fsys::current_path();
        foundDir.append(target);
        if (!fsys::exists(foundDir))
        {
            std::cerr << "The " << errString << " directory " << target << 
                " can't be found!\n";
            foundDir.clear();
        }
    }

    dirMap.insert({dirIndex, foundDir});
}

static DirectoryMap findAllDirectories(FileCtrlValues& ctrlValues)
{
    DirectoryMap dirMap;

    findDirectory(ctrlValues.sourceDirectory, "photo source", fsys::current_path(), "SourceDir", dirMap);
    fsys::path defaultDir = dirMap.find("SourceDir")->second;

    findDirectory(ctrlValues.imageTargetDirectory, "photo target", defaultDir, "TargetDir", dirMap);
    findDirectory(ctrlValues.imageProcessedDirectory, "photo target", defaultDir, "RelocDir", dirMap);

    return dirMap;
}

static void addFilesToListByExtension(std::filesystem::path cwd, std::string extLC, 
        std::string extUC, std::vector<std::string>& photoList)
{
    auto is_match = [extLC, extUC](auto f) {
        return f.path().extension().string() == extLC ||
            f.path().extension().string() == extUC;
    };

    auto files = fsys::directory_iterator{ cwd }
        | std::views::filter([](auto& f) { return f.is_regular_file(); })
        | std::views::filter(is_match)
        | std::views::transform([](auto& f) { return f.path().string(); });

    std::ranges::copy(files, std::back_inserter(photoList));
}

static std::vector<std::string> findAllPhotos(fsys::path originsDir, FileCtrlValues& ctrlValues)
{
    std::vector<std::string> tempFileList;

    if (ctrlValues.processJPGFiles)
    {
        addFilesToListByExtension(originsDir, ".jpg", ".JPG", tempFileList);
    }

    if (ctrlValues.processPNGFiles)
    {
        addFilesToListByExtension(originsDir, ".png", ".PNG", tempFileList);
    }

    return tempFileList;
}

static std::string changeNonAlphaNumCharsToUnderscore(std::string inName)
{
    std::string webSafeName;

    auto toUnderScore = [](unsigned char c) -> unsigned char { return std::isalnum(c)? c : '_'; };

    std::ranges::transform(inName, std::back_inserter(webSafeName), toUnderScore);

    return webSafeName;
}

static std::string makeOutputFileName(
    std::string inputFileName,
    fsys::path targetDir,
    FileCtrlValues& ctrlValues
)
{
    fsys::path outNameBasis = inputFileName;
    std::string ext = outNameBasis.extension().string();
    std::string outputFileName = outNameBasis.stem().string();

    if (ctrlValues.fixFileName)
    {
        outputFileName = changeNonAlphaNumCharsToUnderscore(outputFileName);
    }

    if (!ctrlValues.resizedPostfix.empty())
    {
        outputFileName += "." + ctrlValues.resizedPostfix;
    }

    outputFileName += ext;

    fsys::path targetFile = targetDir;
    targetFile.append(outputFileName);

    if (fsys::exists(targetFile))
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

static void copyInFilesToPhotoListAddOutFileSpec(
    FileCtrlValues& ctrlValues,
    std::vector<std::string>& inFileList,
    PhotoFileList& photoFileList,
    fsys::path targetDir
)
{
    for (auto file: inFileList)
    {
        PhotoFile currentPhoto;
        currentPhoto.inputName = file;
        currentPhoto.outputName = makeOutputFileName(file, targetDir, ctrlValues);
        if (currentPhoto.outputName.empty())
        {
            // If there was a replacement that the user replied no to, do they want to quit?
            std::cerr << "Do you want to quit?(y / n)\n";
            std::string response;
            std::cin >> response;
            if (response == "y")
            {
                photoFileList.clear();
                return;
            }
        }
        else
        {
            photoFileList.push_back(currentPhoto);
        }
    }
}

PhotoFileList buildPhotoInputAndOutputList(FileCtrlValues& ctrlValues)
{
    PhotoFileList photoFileList;
    DirectoryMap directories = findAllDirectories(ctrlValues);

    for (auto& directory: directories)
    {
        if (directory.second.empty())
        {
            return photoFileList;
        }
    }

    fsys::path sourceDir = directories.find("SourceDir")->second;;

    std::vector<std::string> inputPhotoList = findAllPhotos(sourceDir, ctrlValues);
    
    if (inputPhotoList.size())
    {
        fsys::path targetDir = directories.find("TargetDir")->second;
        copyInFilesToPhotoListAddOutFileSpec(ctrlValues, inputPhotoList, photoFileList, targetDir);
    }
    else
    {
        std::cerr << "No photos found to resize!\n";
    }

    return photoFileList;
}
