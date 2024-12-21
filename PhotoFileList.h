#ifndef PHOTOFILELIST_H_
#define PHOTOFILELIST_H_

#include <string>
#include <vector>

struct PhotoFile
{
    std::string inputName;
    std::string outputName;
};

using PhotoFileList = std::vector<PhotoFile>;

#endif // PHOTOFILELIST_H_
