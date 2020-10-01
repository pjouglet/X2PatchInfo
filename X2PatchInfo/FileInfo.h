#pragma once
#include <string>
#include <vector>

class FileInfo
{
public:
	FileInfo(std::string, unsigned int, unsigned int, unsigned int, bool);
	std::string getName() const { return _name; }
	unsigned int getSize() const { return _size; }
	unsigned int getChecksum() const { return _checksum; }
	unsigned int getFileTime() const { return _fileTime; }
	bool isKomFile() const { return _isKomFile;  }

private:
	std::string _name;
	unsigned int _size;
	unsigned int _checksum;
	unsigned int _fileTime;
	bool _isKomFile;
};
