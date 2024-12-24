#ifndef PHOTORESIZER_H_
#define PHOTORESIZER_H_

#include "PhotoOptions.h"
#include "PhotoFileList.h"

std::size_t resizeAllPhotosInList(const PhotoOptions& ctrlValues, const PhotoFileList& photoList);

#endif // PHOTORESIZER_H_
