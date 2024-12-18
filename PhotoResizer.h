#ifndef PHOTORESIZER_H
#define PHOTORESIZER_H

#include "photoCtrlValues.h"
#include "PhotoFileList.h"

std::size_t resizeAllPhotosInList(PhotCtrlValues& ctrlValues, PhotoFileList& photoList);

#endif