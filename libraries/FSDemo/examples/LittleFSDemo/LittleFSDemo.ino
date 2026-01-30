/**
 * @file LittleFSDemo.ino
 * @brief LittleFS basic operations demo
 * 
 * This example demonstrates basic LittleFS file operations including:
 * - File creation and writing
 * - File reading
 * - Directory operations
 * - File information retrieval
 * 
 * @note Make sure LittleFS is properly mounted before running this example
 */

#include "File.h"

VFSFILE fs(LITTLEFS);

/*
*** test1 ***
test the file base operation,open,close,read write seek position size 
*************
*/
void test1()
{
  int size =  0;
  TUYA_FILE fd = NULL;
  
  char wbuff[] = "Hello, World!!!\nHello Everyone!!!\n";
  char rbuff[40];
  char rrbuff[40];
  memset(rbuff,0,sizeof(rbuff));
  memset(rrbuff,0,sizeof(rrbuff));

  fd = fs.open("/test.txt","tw");
  fs.write(wbuff,sizeof(wbuff),fd); 
  fs.close(fd);

  size = fs.filesize("/test.txt");
  Serial.print("size:");
  Serial.println(size);                         //35

  fs.open("/test.txt","r");
  fs.read(rbuff,11,fd);                        
  Serial.println(rbuff);                        //Hello, World
  char ch;
  ch = fs.read(fd);
  Serial.println(ch);                           //  d
  fs.close(fd);

  fs.open("/test.txt","r");
  fs.readtillN(rrbuff,20,fd);               
  Serial.println(rrbuff);                       //Hello, World!!!\n

  memset(rrbuff,0,sizeof(rrbuff));
  fs.lseek(fd,-19,SEEK_END);
  int pos = fs.position(fd);
  Serial.print("pos:  "); 
  Serial.println(pos);                          //35-19=16

  fs.read(rrbuff,20,fd);
  Serial.println(rrbuff);                      // o Everyone
  fs.close(fd);

}
/*
*** test2 ***
test the file system operation about mkdir, remove, rename, opendir, readdir
*************
*/
void test2()
{
  int ret = -1;
  TUYA_DIR dir = NULL;
  TUYA_FILEINFO info = NULL;

  char name_content[20];                         // to store the name read from directory
  memset(name_content,0,sizeof(name_content));
	const char *name_content_ptr = name_content;

  ret= fs.mkdir("/user/ttt/yt");    
  Serial.println(ret);                           // 0 success

  ret= fs.exist("/user/ttt/yt");
  Serial.println(ret);                           // 1 exist

  dir = fs.openDir("/user/ttt");            
  info = fs.readDir(dir);
  if(info == NULL )
    Serial.println("read fail");
  fs.getDirName(info, &name_content_ptr);
  Serial.println(name_content_ptr);             //  .
  ret= fs.closeDir(dir);
  Serial.println(ret);                          //  0
  
  ret = fs.rename("/user/ttt/yt","/user/ttt/yy");
  Serial.println(ret);                           //  0

  ret = fs.remove("/user/ttt/yy");                    
  Serial.println(ret);                           // 0 
  
  ret = fs.remove("/user/ttt");
  Serial.println(ret);         

  ret = fs.remove("/user");
  Serial.println(ret);          

}


/*
*** test3 ***
open two files, read from one file and write to another file
*************
*/
void test3()
{
  TUYA_FILE fd1;
  TUYA_FILE fd2;
  char msg[] = "this is a test file, hello world!";
  char buf1[10];
  memset(buf1,0,sizeof(buf1));
  char buf2[40];
  memset(buf2,0,sizeof(buf2));
  int cnt = 0;

  fd1 = fs.open("/test_1.txt","tw");
  fs.write(msg,sizeof(msg),fd1);
  fs.close(fd1);

  fd1 = fs.open("/test_1.txt","r");
  fd2 = fs.open("/test_2.txt","tw");
  while(!fs.feof(fd1))
  {
    fs.read(buf1,10,fd1);
    fs.write(buf1,10,fd2);
    cnt+=1;
  }
  Serial.println(cnt);
  fs.close(fd1);
  fs.close(fd2);

  fd2 = fs.open("/test_2.txt","r");
  fs.read(buf2,40,fd2);
  Serial.println(buf2);
  fs.close(fd2);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  delay(1000);
  test1();
  
  Serial.println();
  delay(1000);
  test2();
  
  Serial.println();
  delay(1000);
  test3();
}

void loop() {
  Serial.print("...\n");
  delay(1000);
}
