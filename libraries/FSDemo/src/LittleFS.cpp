#include "LittleFS.h"
#include "tal_memory.h"

static lfs_size_t lfs_flash_addr;
static int user_provided_block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    OPERATE_RET ret = tkl_flash_read(lfs_flash_addr + c->block_size * block + off, (uint8_t*)buffer, size);
    if (OPRT_OK != ret) {
        return LFS_ERR_IO;
    }
    return LFS_ERR_OK;
}

static int user_provided_block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    OPERATE_RET ret = tkl_flash_write(lfs_flash_addr + c->block_size * block + off, (uint8_t*)buffer, size);
    if (OPRT_OK != ret) {
        return LFS_ERR_IO;
    }
    return LFS_ERR_OK;
}

static int user_provided_block_device_erase(const struct lfs_config *c, lfs_block_t block)
{
    OPERATE_RET ret = tkl_flash_erase(lfs_flash_addr + c->block_size * block, c->block_size);
    if (OPRT_OK != ret) {
        return LFS_ERR_IO;
    }
    return LFS_ERR_OK;
}


static int user_provided_block_device_sync(const struct lfs_config *c)
{
    return LFS_ERR_OK;
}

static int mount()
{
    TUYA_FLASH_BASE_INFO_T info;
    tkl_flash_get_one_type_info(TUYA_FLASH_TYPE_UF, &info);
    lfs_flash_addr = info.partition[0].start_addr;

    static struct lfs_config lfs_cfg = {0};

    lfs_cfg.read = user_provided_block_device_read;
    lfs_cfg.prog = user_provided_block_device_prog;
    lfs_cfg.erase = user_provided_block_device_erase;
    lfs_cfg.sync = user_provided_block_device_sync;
    lfs_cfg.read_size = info.partition[0].block_size;
    lfs_cfg.prog_size = info.partition[0].block_size;
    lfs_cfg.block_size = info.partition[0].block_size;
    lfs_cfg.block_count = info.partition[0].size / info.partition[0].block_size;
    lfs_cfg.cache_size = info.partition[0].block_size;
    lfs_cfg.lookahead_size = lfs_cfg.block_count / 8 + (8 - (lfs_cfg.block_count / 8));
    lfs_cfg.block_cycles = 500;
    lfs_t *lfs;
    lfs = tal_lfs_get();
    // mount the filesystem
    int err = lfs_mount(lfs, &lfs_cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err) {
        lfs_format(lfs, &lfs_cfg);
        err = lfs_mount(lfs, &lfs_cfg);
    }

    return err;
}

FS_LITTLEFS::FS_LITTLEFS()
{
    ismounted = false;
    int ret = mount();
    if(ret >= 0)
        ismounted = true;
}

FS_LITTLEFS::~FS_LITTLEFS()
{
}

int createDirRecursive(const char *path) {
    // Create directory
    int ret = tal_fs_mkdir(path);
    if(ret == 0)
        return 0;
    else {
        // If create failed, check the error type
        switch (ret) {
            case LFS_ERR_EXIST:
                return 0;
            case LFS_ERR_NOENT:
                // If the parent directory does not exist, create the parent directory recursively
            {
                // Find the position of the last slash in the path
                size_t len = strlen(path) + 1; // Include the null terminator
                char *parentPath  = (char*)tal_malloc(len);
                if (parentPath) {
                    strcpy(parentPath, path);
                }
                char *lastSlash = strrchr(parentPath, '/');
                if (lastSlash != NULL) {
                    *lastSlash = '\0'; // Cut off the last part of the path
                    int result = createDirRecursive(parentPath);
                    tal_free(parentPath);
                    if (result != 0) {
                        return result; // Failed to create parent directory
                    }
                    // 再次尝试创建当前目录
                    return tal_fs_mkdir(path);
                } else {
                    tal_free(parentPath);
                    return -1; // No parent directory
                }
            }
            default:
                // Other errors
                return -1;
        }
    }
}

int FS_LITTLEFS:: mkdir(const char *path)
{

    if(!ismounted)
        return -1;
    return createDirRecursive(path);
    
}

int FS_LITTLEFS::remove(const char *path)
{	
    if(!ismounted)
        return -1;
    return tal_fs_remove(path);
}

int FS_LITTLEFS::exist(const char *path)
{
    BOOL_T is_exist = false;
    if(!ismounted)
    {
        return -1; 
    }
    tal_fs_is_exist(path,&is_exist);
	return is_exist;
}

int FS_LITTLEFS::rename(const char *pathFrom,const char *pathTo)
{
    if(!ismounted)
        return -1;
    return tal_fs_rename(pathFrom, pathTo);
}

TUYA_DIR FS_LITTLEFS::openDir(const char *path)
{
    if(!ismounted)
    {
        int ret = mount();
        if(ret >= 0)
            ismounted = true;
        else 
            return NULL;
    }
    TUYA_DIR dir;
    tal_dir_open(path, &dir);
    return dir;
}

int FS_LITTLEFS:: closeDir(TUYA_DIR dir) 
{
    if(!ismounted || !dir) 
        return -1;
    else
        return tal_dir_close(dir);
}

TUYA_FILEINFO FS_LITTLEFS::readDir(TUYA_DIR dir) 
{
    if(!ismounted || !dir){
        return NULL;
    }
    TUYA_FILEINFO info = NULL;
    tal_dir_read(dir,&info);
    return info;
}

int FS_LITTLEFS:: getDirName(TUYA_FILEINFO info,const char** name)
{
    if(!ismounted || !info)
    {
        return -1;
    }
	return tal_dir_name(info,name);
}

int FS_LITTLEFS:: isDirectory(const char *path)
{
    if(!ismounted)
        return -1;
    TUYA_FILEINFO info = NULL;
    BOOL_T is_dir = false;
    TUYA_DIR dir = NULL;
    int ret = 0;
    ret = tal_dir_open(path, &dir);
    if(ret < 0)
        return ret;
    tal_dir_read(dir,&info);
    tal_dir_is_directory(info,&is_dir);
    tal_dir_close(dir);
    return is_dir;
}


TUYA_FILE FS_LITTLEFS::open(const char *path)
{
    if(!ismounted) {
        int ret = mount();
        if(ret >= 0)
            ismounted = true;
        else 
            return NULL;
    }
    TUYA_FILE fd;
    fd = tal_fopen(path,"a+");
    return fd;
}

TUYA_FILE FS_LITTLEFS::open(const char *path, const char* mode)
{
    if(!ismounted)
    {
        int ret = mount();
        if(ret >= 0)
            ismounted = true;
        else 
            return NULL;
    }
    TUYA_FILE fd;
    fd = tal_fopen(path,mode);
    return fd;
}

int FS_LITTLEFS::close(TUYA_FILE fd)
{
    if(!ismounted||!fd)
        return -1;
    return tal_fclose(fd);
}

char FS_LITTLEFS:: read(TUYA_FILE fd)
{
    if(!ismounted||!fd)
    {
        return '\0';
    }
    return tal_fgetc(fd);
}
int FS_LITTLEFS::read(const char *buf,int size,TUYA_FILE fd)
{
    if(!ismounted||!fd ||!buf||!size)
    {
        return -1;
    }
    return tal_fread((void*)buf, size, fd);
}


int FS_LITTLEFS::readtillN(char *buf, int size,TUYA_FILE fd)
{
    if(!ismounted||!fd ||!buf||!size)
    {
        return -1;
    }
    tal_fgets(buf, size, fd);
    return OPRT_OK;
}

int FS_LITTLEFS::write(const char *buf,int size,TUYA_FILE fd)
{
    if(!ismounted||!fd ||!buf||!size)
    {
        return -1;
    }
    return tal_fwrite((void*)buf, size, fd);
}

void FS_LITTLEFS::flush(TUYA_FILE fd)
{
    if(!ismounted||!fd)
    {
        return;
    }
    tal_fflush(fd);
    tal_fsync(fd);
}

int FS_LITTLEFS::feof(TUYA_FILE fd)
{
    if(!ismounted||!fd)
    {
        return -1 ;
    }
    return tal_feof(fd);
}
int FS_LITTLEFS::lseek(TUYA_FILE fd,int offs, int whence)
{
    if(!ismounted||!fd)
    {
        return -1;
    }
	return tal_fseek(fd, offs, whence);
}

int FS_LITTLEFS::position(TUYA_FILE fd)
{
    if(!ismounted||!fd)
    {
        return -1;
    }
    return tal_ftell(fd);
}

int FS_LITTLEFS::filesize(const char *filepath)
{
    if(!ismounted)
        return -1;
    return tal_fgetsize(filepath);
}
