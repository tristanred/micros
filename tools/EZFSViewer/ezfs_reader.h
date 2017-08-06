#ifndef EZFS_READER_H
#define EZFS_READER_H

#include <cstring>
#include <fstream>
#include <QList>

struct filesystem_metablock;
struct file_allocation;

class EZFS_Reader
{
public:
    EZFS_Reader();
    ~EZFS_Reader();

    void Open(std::string path);

    bool IsValidFilesystem();

    void ExtractFilesystemData();

private:
    std::ifstream* fileStream;
    bool streamGood;
    bool fileSystemGood;

    struct filesystem_metablock* metablock;

    QVector<struct file_allocation*>* Files;


//    size_t file_allocations_length;
//    struct file_allocation** file_allocations;
};

#endif // EZFS_READER_H
