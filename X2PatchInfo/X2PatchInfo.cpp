#include <iostream>
#include <filesystem>
#include "kom.h"
#include "FileInfo.h"

#define patchinfo_filename "patchinfo.xml"

std::vector<std::unique_ptr<FileInfo>> _fileInfos;
std::string processName = "";

void read_file(std::string filename) {
    filename.erase(0, 2);
    if (filename == processName || filename == patchinfo_filename)
        return;
    std::cout << "Reading " << filename << std::endl;
    Komfile komfile;
    bool isKom = false;
    if (filename.find_last_of(".kom") == filename.length() - 1)
        isKom = true;

	if(!isKom)
	{
        std::ifstream filestream(filename.c_str(), std::ios_base::in | std::ios_base::binary);
        filestream.seekg(0, std::ios_base::end);
        unsigned int size = filestream.tellg();
        unsigned int adler32 = AdlerCheckSum::adler32(filestream, 0, size, true);
        filestream.close();
        _fileInfos.push_back(std::make_unique<FileInfo>(filename, size, adler32, 0, false));
	}
    else
    {
	    if(!komfile.Open(filename))
	    {
            std::cout << "Can't open file " << filename << std::endl;
            return;
	    }
        _fileInfos.push_back(std::make_unique<FileInfo>(filename, komfile.GetFileSize(), komfile.GetAdler32(), komfile.GetFileTime(), true));
    }
}

void read_directory(std::string path) {
    for (auto& p : std::filesystem::directory_iterator(path)) {
        std::string test = p.path().string();
        if (p.is_directory()) {
            read_directory(test);
            continue;
        }
        read_file(test);
    }
}

void write_patch_info_file()
{
    std::ofstream file(patchinfo_filename, std::ios_base::binary | std::ios_base::out);
    if (!file.is_open()) 
        return;

    std::stringstream stream;
    stream << "<Patchinfo> \r\n";
    stream << "\t<Files>\r\n";
    for (auto& file_info : _fileInfos)
    {
        char adler32string[16];
        sprintf_s(adler32string, "%08x", file_info->getChecksum());

        char filetimestring[16];
        sprintf_s(filetimestring, "%08x", file_info->getFileTime());
        stream << "\t\t<File";
        stream << " Name=\"" << file_info->getName() << "\"";
        stream << " Size=\"" << file_info->getSize() << "\"";
        stream << " Checksum=\"" << adler32string << "\"";
    	if(file_info->isKomFile())
    	{
            stream << " FileTime=\"" << filetimestring << "\"";

    	}
        else
        {
            stream << " FileTime=\"" << 0 << "\"";
        }
        stream << "/>\r\n";
    }
    stream << "\t</Files>\r\n";
    stream << "</Patchinfo>";

    file << stream.str();
    stream.clear();
    file.close();
}

std::string get_process_name() {
    std::wstring buf;
    buf.resize(MAX_PATH);
    do
    {
        unsigned int len = GetModuleFileNameW(NULL, &buf[0], static_cast<unsigned int>(buf.size()));
        if (len < buf.size())
        {
            buf.resize(len);
            break;
        }

        buf.resize(buf.size() * 2);
    } while (buf.size() < 65536);

    return std::filesystem::path(buf).filename().string();
}

int main() {
    processName = get_process_name();
    read_directory("./");
    write_patch_info_file();
    return 0;
}
