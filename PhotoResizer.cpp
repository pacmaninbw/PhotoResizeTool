#include <opencv2/opencv.hpp>
#include "photoCtrlValues.h"
#include "PhotoFileList.h"
#include "PhotoResizer.h"

static cv::Mat resizePhoto(cv::Mat& image, std::size_t newWdith, std::size_t newHeight)
{
    cv::Size newSize(newWdith, newHeight);

    cv::Mat resizedImage;

    cv::resize(image, resizedImage, newSize);

    // Prevent memory leak
    image.release();

    return resizedImage;
}

static cv::Mat resizePhotoByWidthMaintainGeometry(cv::Mat& image, std::size_t maxWdith)
{
    if (static_cast<std::size_t>(image.cols)  <= maxWdith)
    {
        return image;
    }

    double ratio = static_cast<double>(maxWdith) / static_cast<double>(image.cols);
    std::size_t newHeight = static_cast<int>(image.rows * ratio);

    return resizePhoto(image, maxWdith, newHeight);
}

static cv::Mat resizePhotoByHeightMaintainGeometry(cv::Mat& image, std::size_t maxHeight)
{
    if (static_cast<std::size_t>(image.rows)  <= maxHeight)
    {
        return image;
    }

    double ratio = static_cast<double>(maxHeight) / static_cast<double>(image.rows);
    std::size_t newWidth = static_cast<int>(image.cols * ratio);

    return resizePhoto(image, newWidth, maxHeight);
}

static cv::Mat resizePhotoByPercentage(cv::Mat& image, unsigned int percentage)
{
    double percentMult = static_cast<double>(percentage)/100.0;

// Retain the current photo geometry.
    std::size_t newWidth = static_cast<int>(image.rows * percentMult);
    std::size_t newHeight = static_cast<int>(image.cols * percentMult);

    return resizePhoto(image, newWidth, newHeight);
}

static bool saveResizedPhoto(cv::Mat& resizedPhoto, std::string webSafeName)
{

    bool saved = cv::imwrite(webSafeName, resizedPhoto);

    if (!saved) {
        std::cerr << "Could not write image " << webSafeName << " to file!\n";
    }

    // Prevent memory leak.
    resizedPhoto.release();

    return saved;
}

static cv::Mat resizeByUserSpecification(cv::Mat& image, PhotCtrlValues ctrlValues)
{
    if (ctrlValues.maxWdith > 0 && ctrlValues.maxHeight > 0)
    {
        return resizePhoto(image, ctrlValues.maxWdith, ctrlValues.maxHeight);
    }

    if (ctrlValues.reductionToPercentage > 0)
    {
        return resizePhotoByPercentage(image, ctrlValues.reductionToPercentage);
    }

    if (ctrlValues.maintainRatio)
    {
        if (ctrlValues.maxWdith > 0)
        {
            return resizePhotoByWidthMaintainGeometry(image, ctrlValues.maxWdith);
        }
        if (ctrlValues.maxHeight > 0)
        {
            return resizePhotoByHeightMaintainGeometry(image, ctrlValues.maxHeight);
        }
        std::cerr << "Neither width nor height were specified with"
            " --maintain-ratio, can't resize photo!\n";
        return image;
    }
    else
    {
        if (ctrlValues.maxWdith > 0 && ctrlValues.maxHeight == 0)
        {
            return resizePhotoByWidthMaintainGeometry(image, ctrlValues.maxWdith);
        }
        if (ctrlValues.maxHeight > 0 && ctrlValues.maxWdith == 0)
        {
            return resizePhotoByHeightMaintainGeometry(image, ctrlValues.maxHeight);
        }
    }

    return image;
}

static bool resizeAndSavePhoto(PhotoFile imageName, PhotCtrlValues ctrlValues)
{
    cv::Mat image = cv::imread(imageName.inputName);
    if (image.empty()) {
        std::cerr << "Could not read image " << imageName.inputName << "!\n";
        return false;
    }

    cv::Mat resized = resizeByUserSpecification(image, ctrlValues);

    if (ctrlValues.displayImage)
    {
        cv::imshow("Resized Image", resized);
        cv::waitKey(0);
    }

    return saveResizedPhoto(resized, imageName.outputName);
}

std::size_t resizeAllPhotosInList(PhotCtrlValues& ctrlValues, PhotoFileList& photoList)
{
    std::size_t resizedCount = 0;

    for (auto picture: photoList)
    {
        if (resizeAndSavePhoto(picture, ctrlValues))
        {
            ++resizedCount;
        }
    }

    return resizedCount;
}

