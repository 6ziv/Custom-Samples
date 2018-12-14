#ifndef ZIPREADER_H
#define ZIPREADER_H
#include "include/ZipArchive.h"
#include "include/ZipArchiveEntry.h"
#include "include/ZipFile.h"
#include <map>
class ZipReader
{
public:
    static std::map<std::string,std::string> Ext;
    static void InitMap();
    ZipArchive::Ptr archive;
    ZipArchiveEntry::Ptr CurrentFile;
    bool LoadZip(const char *zipname);
    bool LoadFile(const char *filename);
    std::istream *GetStream();
    size_t GetSize();
    void getMIMEtype(char **type);
};

#endif // ZIPREADER_H
