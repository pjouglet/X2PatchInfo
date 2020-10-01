//
// Created by darkza on 9/30/20.
//


#include "kom.h"
#include "adler32.h"

bool DeleteFileForce(std::string filename)
{
    if (DeleteFileA(filename.c_str()) == 0)
    {
        if (GetLastError() == ERROR_ACCESS_DENIED)
        {
            if (GetFileAttributesA(filename.c_str()) & FILE_ATTRIBUTE_READONLY)
            {
                SetFileAttributesA(filename.c_str(), FILE_ATTRIBUTE_NORMAL);
                if (DeleteFileA(filename.c_str()) != 0)
                    return true;
            }
        }
        else if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            return true;
        }
    }
    else
    {
        return true;
    }
    return false;
}

KSubfile::KSubfile(std::string str, std::ifstream& stream, int headersize)
    :filename(str), algorithm(infilate), adler32(0), size(0), compressedsize(0), filetime(0), offset(0), iscalcadler(false)
{
    stream.read((char*)&size, 4);
    stream.read((char*)&compressedsize, 4);
    stream.read((char*)&offset, 4);
    offset += headersize;
    filetime = 0;
}

KSubfile::KSubfile(std::string str, xmlNode* node, int* offset)
    :filename(str), algorithm(infilate), adler32(0), size(0), compressedsize(0), filetime(0), offset(0), iscalcadler(true)
{
    xmlAttr* attribute = node->properties;
    while (attribute)
    {
        if (attribute->children)
        {
            if (strcmp((char*)attribute->name, "Checksum") == 0)
            {
                sscanf_s((char*)attribute->children->content, "%x", &adler32);
            }
            else if (strcmp((char*)attribute->name, "CompressedSize") == 0)
            {
                sscanf_s((char*)attribute->children->content, "%d", &compressedsize);
            }
            else if (strcmp((char*)attribute->name, "Size") == 0)
            {
                sscanf_s((char*)attribute->children->content, "%d", &size);
            }
            else if (strcmp((char*)attribute->name, "Algorithm") == 0)
            {
                sscanf_s((char*)attribute->children->content, "%d", &algorithm);
            }
            else if (strcmp((char*)attribute->name, "FileTime") == 0)
            {
                sscanf_s((char*)attribute->children->content, "%x", &filetime);
            }
        }
        attribute = attribute->next;
    }

    this->offset = (*offset);
    (*offset) += compressedsize;
}

bool KSubfile::WriteCompressed(std::ostream& stream)
{
    std::ifstream filestream(filename.c_str(), std::ios_base::in | std::ios_base::binary);

    filestream.seekg(offset, std::ios_base::beg);

    int totallen = compressedsize;
    if (totallen < 0)
        return false;

    unsigned char buffer[2048];
    int len = 2048;
    while (totallen > 0)
    {
        if (totallen < 2048)
        {
            len = totallen;
        }
        else
        {
            len = 2048;
        }

        filestream.read((char*)buffer, len);
        stream.write((char*)buffer, len);
        totallen -= len;
    }

    filestream.close();

    return true;
}



Komfile::KOM_TYPE Komfile::CheckKom(std::string filename)
{
    std::ifstream filestream(filename.c_str(), std::ios_base::in | std::ios_base::binary);
    if (filestream.is_open())
    {
        char magicword[52];
        filestream.read(magicword, 52);
        filestream.close();
        if (strcmp(magicword, "KOG GC TEAM MASSFILE V.0.3.") == 0)
        {
            return CUR_KOM;
        }
        else if (strcmp(magicword, "KOG GC TEAM MASSFILE V.0.2.") == 0 || strcmp(magicword, "KOG GC TEAM MASSFILE V.0.1.") == 0)
        {
            return OLD_KOM;
        }
    }
    return NOT_KOM;
}

bool ReadInFile(std::ifstream& filestream, char* buf, int size)
{
    int pos = filestream.tellg();
    filestream.read(buf, size);
    if ((int)filestream.tellg() - pos == size)
        return true;

    filestream.close();
    return false;
}

bool Komfile::Open(std::string filename)
{
    Close();
    if (GetFileAttributesA(filename.c_str()) == INVALID_FILE_ATTRIBUTES)
        return false;

    std::ifstream filestream(filename.c_str(), std::ios_base::in | std::ios_base::binary);
    if (filestream.is_open())
    {
        filestream.seekg(0, std::ios_base::beg);
        int pos = filestream.tellg();
        char magicword[52];

        if (ReadInFile(filestream, magicword, 52) == false) return false;

        unsigned int size;
        char compressed[4];

        if (ReadInFile(filestream, (char*)&size, 4) == false) return false;
        if (ReadInFile(filestream, (char*)&compressed, 4) == false) return false;

        if (strcmp(magicword, "KOG GC TEAM MASSFILE V.0.3.") == 0)
        {
            if (ReadInFile(filestream, (char*)&filetime, 4) == false) return false;
            if (ReadInFile(filestream, (char*)&adler32, 4) == false) return false;
            if (ReadInFile(filestream, (char*)&headersize, 4) == false) return false;

            char* header = new char[headersize + 1];

            if (ReadInFile(filestream, header, headersize) == false)
            {
                delete header;
                return false;
            }
            header[headersize] = '\0';

            xmlDoc* doc;
            doc = xmlReadMemory(header, headersize, "Komfiles.xml", NULL, 0);
            delete header;

            if (!doc)
            {
                filestream.close();
                return false;
            }

            int offset = HEADEROFFSET + headersize;
            xmlNode* files = xmlDocGetRootElement(doc);
            if (files->type == XML_ELEMENT_NODE && strcmp((char*)files->name, "Files") == 0)
            {
                xmlNode* file = files->children;
                while (file)
                {
                    if (file->type == XML_ELEMENT_NODE && strcmp((char*)file->name, "File") == 0)
                    {
                        std::string key;
                        xmlAttr* attribute = file->properties;
                        while (attribute)
                        {
                            if (strcmp((char*)attribute->name, "Name") == 0 && attribute->children)
                            {
                                key = (char*)attribute->children->content;
                                break;
                            }
                            attribute = attribute->next;
                        }
                        subfiles.insert(std::map<std::string, KSubfile>::value_type(key, KSubfile(filename, file, &offset)));
                    }
                    file = file->next;
                }
            }
            xmlFreeDoc(doc);
        }
        else if (strcmp(magicword, "KOG GC TEAM MASSFILE V.0.2.") == 0 || strcmp(magicword, "KOG GC TEAM MASSFILE V.0.1.") == 0)
        {

            for (unsigned int i = 0; i < size; i++)
            {
                char key[60];
                if (!ReadInFile(filestream, key, 60)) return false;
                subfiles.insert(std::map<std::string, KSubfile>::value_type(key, KSubfile(filename, filestream, 60 + size * 72)));
            }
        }
        else
        {
            filestream.close();
            return false;
        }
    }
    filestream.seekg(0, std::ios_base::end);
    fileSize = filestream.tellg();
    filestream.close();
    return true;
}

void Komfile::Close()
{
    subfiles.clear();

    filetime = 0;
    adler32 = 0;
    headersize = 0;
}


unsigned int Komfile::GetFileTime()
{
    unsigned int filetime = 0;
    std::map<std::string, KSubfile>::iterator i = subfiles.begin();
    while (i != subfiles.end())
    {
        filetime += i->second.GetFileTime();
        i++;
    }
    return filetime;
}

bool Komfile::Verify(std::string filename)
{
    Komfile kom;
    if (kom.Open(filename) == false)
        return false;

    return kom.Verify();
}

bool Komfile::Verify()
{
    std::map<std::string, KSubfile>::iterator i = subfiles.begin();
    while (i != subfiles.end())
    {
        if (i->second.Verify() == false)
            return false;
        i++;
    }
    return true;
}


bool KSubfile::Verify()
{
    std::stringstream stream;

    if (WriteCompressed(stream) == false)
        return false;

    if (AdlerCheckSum::adler32(stream, 0, compressedsize) != adler32)
        return false;

    return true;
}

unsigned int KSubfile::GetAdler32()
{
    if (!iscalcadler)
    {
        std::stringstream stream;
        WriteCompressed(stream);
        adler32 = AdlerCheckSum::adler32(stream, 0, compressedsize);
        iscalcadler = true;
    }
    return adler32;
}


bool Komfile::LeftOuterJoin(Komfile& left, Komfile& right)
{
    std::map<std::string, KSubfile>::iterator iter_left = left.subfiles.begin();
    while (iter_left != left.subfiles.end())
    {
        std::map<std::string, KSubfile>::iterator iter_right = right.subfiles.find(iter_left->first);

        if (iter_right != right.subfiles.end())
        {
            KSubfile subfile(iter_left->second);
            iter_right->second.GetAdler32();
            if (iter_left->second == iter_right->second)
            {
                unsigned int uiTime = iter_left->second.GetFileTime();
                subfile = iter_right->second;
                subfile.SetFileTime(uiTime);
            }
            subfiles.insert(std::map<std::string, KSubfile>::value_type(iter_left->first, subfile));
        }
        else
        {
            subfiles.insert(std::map<std::string, KSubfile>::value_type(iter_left->first, iter_left->second));
        }
        iter_left++;
    }
    return true;
}
