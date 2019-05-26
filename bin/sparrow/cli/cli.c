#include "cli.h"
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "vm.h"
#include "core.h"

//执行脚本文件
void runFile(const char* path) {
   char* lastSlash = strrchr((char *)path, '/');
   if (lastSlash != NULL) {
      char* root = (char*)malloc(lastSlash - path + 2);
      memcpy(root, path, lastSlash - path + 1);
      root[lastSlash - path + 1] = '\0';
      rootDir = root;
   }

   VM* vm = newVM();
   char* sourceCode = readFile(path);
   executeModule(vm, OBJ_TO_VALUE(newObjString(vm, path, strlen(path))), sourceCode);
   freeVM(vm);
}

//运行命令行
void runCli(void) {
   VM* vm = newVM();
   char sourceLine[MAX_LINE_LEN];
   printf("maque Version: 0.1\n");
   while (true) {
      printf(">>> "); 
      
      //若读取失败或者键入quit就退出循环
      /*if (!fgets(sourceLine, MAX_LINE_LEN, stdin) ||
	    memcmp(sourceLine, "quit", 4) == 0) {
	 break;
      }
      executeModule(vm, OBJ_TO_VALUE(newObjString(vm, "cli", 3)), sourceLine);*/
   }
   freeVM(vm);
}
