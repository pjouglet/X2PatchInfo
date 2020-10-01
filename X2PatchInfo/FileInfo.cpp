#include "FileInfo.h"

#include <iostream>

FileInfo::FileInfo(std::string name, unsigned int size, unsigned int checksum, unsigned int fileTime, bool isKomFile) :
_name(name), _size(size), _checksum(checksum), _fileTime(fileTime), _isKomFile(isKomFile) {}
