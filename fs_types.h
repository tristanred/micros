#ifndef FS_TYPES
#define FS_TYPES

typedef uint32_t file_h;

enum FS_SPECIAL_FILES
{
    ROOT_DIR = 0,
    CURRENT_DIR,
    PARENT_DIR,
    
    FS_COUNT_SPECIAL_FILES
};

enum FS_FILE_ACCESS
{
    FS_READ = 1,
    FS_WRITE = 2
};

enum FS_FILE_FLAGS
{
    FS_FLAG_DIRECTORY = 1,
    
};

#endif
