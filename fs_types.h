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
    FS_WRITE = 2,
    FS_READ_WRITE = 3
};

enum FS_FILE_FLAGS
{
    FS_FLAGS_NONE = 0,
    FS_FLAG_DIRECTORY = 1,
    
};

#endif
