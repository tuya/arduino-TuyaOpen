#include "file.h"
#include "LittleFS.h"
#include "SDCard.h"

VFSFILE::VFSFILE(int type)
{
	if (type == LITTLEFS)
		impl = new FS_LITTLEFS();
	else if (type == SDCARD)
		impl = new FS_SDCARD();
	else if (type == FATFS)
		impl;
	else 
		impl = NULL;
}

VFSFILE::~VFSFILE()
{
	impl = NULL;
}

int VFSFILE:: mkdir(const String& path)
{
	return mkdir(path.c_str());
}

int VFSFILE:: mkdir(const char *path)
{	
	if(impl != NULL)
	{
		return impl->mkdir(path);
	}
	return -1;
}

int VFSFILE:: remove(const String& path)
{
	return remove(path.c_str());
}

int VFSFILE:: remove(const char *path)
{
	if(impl != NULL)
	{
		return impl->remove(path);
	}
	return -1;
}

int VFSFILE::exist(const String& path)
{
	return exist(path.c_str());
}

int VFSFILE::exist(const char *path)
{
	if(impl != NULL)
	{
		return impl->exist(path);
	}
	return -1;
}

int VFSFILE:: rename(const String& pathFrom, const String& pathTo)
{
	return rename(pathFrom.c_str(),pathTo.c_str());
}

int VFSFILE:: rename(const char *pathFrom,const char *pathTo) 
{	
	if(impl != NULL)
		return impl->rename(pathFrom,pathTo);
	return -1;
}

TUYA_DIR VFSFILE::openDir(const String& path)
{
	return openDir(path.c_str());
}

TUYA_DIR VFSFILE::openDir(const char *path)
{
	if(impl != NULL)
	{
		return impl->openDir(path);
	}
	return NULL;
}

int VFSFILE::closeDir(TUYA_DIR dir ) 
{
	if(impl != NULL)
	{
		return impl->closeDir(dir);
	}
	return -1;
}

TUYA_FILEINFO VFSFILE::readDir( TUYA_DIR dir) 
{
	if(impl != NULL)
	{
		return impl->readDir(dir);
	}
	return NULL;
}

int VFSFILE::getDirName(TUYA_FILEINFO info,const char** name) 
{
	if(impl != NULL)
	{
		return impl->getDirName(info,name);
	}
	return -1;
}

int VFSFILE::isDirectory(const char *path)
{
	if(impl != NULL)
	{
		return impl->isDirectory(path);
	}
	return -1;
}

TUYA_FILE VFSFILE::open(const String& path)
{
	return open(path.c_str());
}

TUYA_FILE VFSFILE::open(const char *path)
{
	if(impl != NULL)
	{
		TUYA_FILE fd;
		fd = impl->open(path);
		return fd;
	}
	return NULL;
}


TUYA_FILE VFSFILE::open(const String& path, const char * mode)
{
	return open(path.c_str(), mode);
}

TUYA_FILE VFSFILE::open(const char *path, const char * mode)
{
	if(impl != NULL)
	{
		TUYA_FILE fd;
		fd = impl->open(path, mode);
		return fd;
	}
	return NULL;
}

int VFSFILE:: close(TUYA_FILE fd)
{
	if(impl != NULL)
	{
		return impl->close(fd);
	}
	return -1;
}

char VFSFILE::read(TUYA_FILE fd)
{
	if(impl != NULL)
	{
		return impl->read(fd);
	}
	return '\0';
}

int VFSFILE::read(const char *buf,int size,TUYA_FILE fd)
{
	if(impl != NULL)
	{
		return impl->read(buf, size,fd);
	}
	return -1;
}


int VFSFILE::readtillN(char *buf,int size,TUYA_FILE fd)
{
	if(impl != NULL)
	{
		return impl->readtillN(buf, size,fd);
	}
	return -1;
}

int VFSFILE::write(const char *buf,int size,TUYA_FILE fd)
{
	if(impl != NULL)
	{
		return impl->write(buf, size,fd);
	}
	return -1;
}

void  VFSFILE::flush(TUYA_FILE fd)
{
	if(impl != NULL)
	{	
		impl->flush(fd);
	}
}
int  VFSFILE::feof(TUYA_FILE fd)
{
	if(impl != NULL)
	{	
		return impl->feof(fd);
	}
	return -1;
}

int VFSFILE::lseek(TUYA_FILE fd,int offs, int whence)
{
	if(impl != NULL)
	{
		return impl->lseek(fd,offs, whence);
	}
	return -1;
}


int VFSFILE::position(TUYA_FILE fd)
{
	if(impl != NULL)
	{
		return impl->position(fd);	
	}
	return -1;
}

int VFSFILE::filesize(const char *filepath)
{
	if(impl != NULL)
	{
		return impl->filesize(filepath);
	}
	return -1;
}


