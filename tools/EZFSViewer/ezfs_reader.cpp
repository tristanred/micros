#include "ezfs_reader.h"

#include <fstream>
#include "ezfs_types.h"

EZFS_Reader::EZFS_Reader()
{
    fileStream = NULL;
    metablock = NULL;
    file_allocations = NULL;

    file_allocations = (struct file_allocation**)malloc(sizeof(struct file_allocation*) * MAX_FILES_NUM);
}

EZFS_Reader::~EZFS_Reader()
{
    if(fileStream->is_open())
        fileStream->close();

    if(fileStream != NULL)
        delete(fileStream);

    if(metablock != NULL)
        delete(metablock);

    for(int i = 0; i < file_allocations_length; i++)
    {
        if(file_allocations[i] != NULL)
        {
            delete(file_allocations[i]);
        }
    }

    delete(file_allocations);
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

    if(strncmp(buf, "ezfs", 4) == 0)
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

            file_allocations[i] = alloc;
        }

        auto one = file_allocations[0];
        auto two= file_allocations[1];

        int i = 0;
    }
}
