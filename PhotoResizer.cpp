#include <opencv2/opencv.hpp>
#include "PhotoOptions.h"
#include "PhotoFileList.h"
#include "PhotoResizer.h"

static cv::Mat resizePhoto(cv::Mat& photo, const std::size_t newWdith, const std::size_t newHeight)
{
    cv::Size newSize(newWdith, newHeight);

    cv::Mat resizedPhoto;

    cv::resize(photo, resizedPhoto, newSize, 0, 0, cv::INTER_AREA);

    // Prevent memory leak
    photo.release();

    return resizedPhoto;
}

static cv::Mat resizePhotoByWidthMaintainGeometry(cv::Mat& photo, const std::size_t maxWdith)
{
    if (static_cast<std::size_t>(photo.cols)  <= maxWdith)
    {
        return photo;
    }

    double ratio = static_cast<double>(maxWdith) / static_cast<double>(photo.cols);
    std::size_t newHeight = static_cast<int>(photo.rows * ratio);

    return resizePhoto(photo, maxWdith, newHeight);
}

static cv::Mat resizePhotoByHeightMaintainGeometry(cv::Mat& photo, const std::size_t maxHeight)
{
    if (static_cast<std::size_t>(photo.rows)  <= maxHeight)
    {
        return photo;
    }

    double ratio = static_cast<double>(maxHeight) / static_cast<double>(photo.rows);
    std::size_t newWidth = static_cast<int>(photo.cols * ratio);

    return resizePhoto(photo, newWidth, maxHeight);
}

static cv::Mat resizePhotoByPercentage(cv::Mat& photo, const unsigned int percentage)
{
    double percentMult = static_cast<double>(percentage)/100.0;

// Retain the current photo geometry.
    std::size_t newWidth = static_cast<int>(photo.cols * percentMult);
    std::size_t newHeight = static_cast<int>(photo.rows * percentMult);

    return resizePhoto(photo, newWidth, newHeight);
}

static bool saveResizedPhoto(cv::Mat& resizedPhoto, const std::string webSafeName)
{

    bool saved = cv::imwrite(webSafeName, resizedPhoto);

    if (!saved) {
        std::cerr << "Could not write photo " << webSafeName << " to file!\n";
    }

    // Prevent memory leak.
    resizedPhoto.release();

    return saved;
}

static cv::Mat resizeByUserSpecification(cv::Mat& photo, const PhotoOptions& photoOptions)
{
    if (photoOptions.maxWdith > 0 && photoOptions.maxHeight > 0)
    {
        return resizePhoto(photo, photoOptions.maxWdith, photoOptions.maxHeight);
    }

    if (photoOptions.scaleFactor > 0)
    {
        return resizePhotoByPercentage(photo, photoOptions.scaleFactor);
    }

    if (photoOptions.maintainRatio)
    {
        if (photoOptions.maxWdith > 0)
        {
            return resizePhotoByWidthMaintainGeometry(photo, photoOptions.maxWdith);
        }
        if (photoOptions.maxHeight > 0)
        {
            return resizePhotoByHeightMaintainGeometry(photo, photoOptions.maxHeight);
        }
        std::cerr << "Neither width nor height were specified with"
            " --maintain-ratio, can't resize photo!\n";
        return photo;
    }
    else
    {
        if (photoOptions.maxWdith > 0 && photoOptions.maxHeight == 0)
        {
            return resizePhotoByWidthMaintainGeometry(photo, photoOptions.maxWdith);
        }
        if (photoOptions.maxHeight > 0 && photoOptions.maxWdith == 0)
        {
            return resizePhotoByHeightMaintainGeometry(photo, photoOptions.maxHeight);
        }
    }

    return photo;
}

static bool resizeAndSavePhoto(const PhotoFile& photoFile, const PhotoOptions& photoOptions)
{
    // Possibly file already exists and user did not specify --overwrite
    if (photoFile.outputName.empty())
    {
        return false;
    }

    cv::Mat photo = cv::imread(photoFile.inputName);
    if (photo.empty()) {
        std::cerr << "Could not read photo " << photoFile.inputName << "!\n";
        return false;
    }

    cv::Mat resized = resizeByUserSpecification(photo, photoOptions);

    if (photoOptions.displayResized)
    {
        cv::imshow("Resized Photo", resized);
        cv::waitKey(0);
    }

    return saveResizedPhoto(resized, photoFile.outputName);
}

std::size_t resizeAllPhotosInList(const PhotoOptions& photoOptions, const PhotoFileList& photoList)
{
    std::size_t resizedCount = 0;

    for (auto photo: photoList)
    {
        if (resizeAndSavePhoto(photo, photoOptions))
        {
            ++resizedCount;
        }
    }

    return resizedCount;
}

