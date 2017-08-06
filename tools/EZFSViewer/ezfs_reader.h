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
    QVector<struct file_allocation*>* Files;
    struct filesystem_metablock* metablock;

    EZFS_Reader();
    ~EZFS_Reader();

    void Open(std::string path);

    bool IsValidFilesystem();

    void ExtractFilesystemData();

    QByteArray GetFileData(int index);

private:
    std::ifstream* fileStream;
    bool streamGood;
    bool fileSystemGood;

};

#endif // EZFS_READER_H
