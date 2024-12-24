#ifndef PHOTO_OPTIONS_H_
#define PHOTO_OPTIONS_H_

#include <string>

struct PhotoOptions
{
	bool displayImage = false;
    bool maintainRatio = false;
    std::size_t maxWdith = 0;
    std::size_t minWidth = 0;
    std::size_t maxHeight = 0;
    std::size_t minHeight = 0;
    unsigned int reductionToPercentage = 0;
};

#endif // PHOTO_OPTIONS_H_
