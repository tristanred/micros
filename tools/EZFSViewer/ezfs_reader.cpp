#include "ezfs_reader.h"

#include "ezfs_types.h"

#include <QFile>

EZFS_Reader::EZFS_Reader()
{
    fileStream = NULL;
    metablock = NULL;

    Files = new QVector<struct file_allocation*>();
}

EZFS_Reader::~EZFS_Reader()
{
    if(fileStream->is_open())
        fileStream->close();

    if(fileStream != NULL)
        delete(fileStream);

    if(metablock != NULL)
        delete(metablock);
}


void EZFS_Reader::Open(std::string path)
{
    fileStream = new std::ifstream();
    fileStream->open(path.c_str(), std::ios::in);

    if(!fileStream->good())
    {
        streamGood = false;
    }

    streamGood = true;

    fileStream->seekg(512, std::ios::beg);

    char buf[4];
    fileStream->read(buf, 4);

    if(std::strncmp(buf, "ezfs", 4) == 0)
    {
        fileSystemGood = true;
    }
    else
    {
        fileSystemGood = false;
    }
}

bool EZFS_Reader::IsValidFilesystem()
{
    return streamGood && fileSystemGood;
}

void EZFS_Reader::ExtractFilesystemData()
{
    if(fileStream != NULL && fileStream->good())
    {
        metablock = new struct filesystem_metablock();

        fileStream->seekg(512, std::ios::beg);

        fileStream->read(reinterpret_cast<char*>(metablock), sizeof(struct filesystem_metablock));

        uint64_t start = metablock->allocation_area_start;
        uint64_t length = metablock->allocation_area_length;

        fileStream->seekg(start, std::ios::beg);

        for(int i = 0; i < MAX_FILES_NUM; i++)
        {
            struct file_allocation* alloc = new struct file_allocation();

            fileStream->read(reinterpret_cast<char*>(alloc), sizeof(struct file_allocation));

            Files->append(alloc);
        }
    }
}

