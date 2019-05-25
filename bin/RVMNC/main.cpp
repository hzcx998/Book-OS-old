//RVM Native Caller 负责RVM与原生系统的API交互
#include <map>
#include <iostream>
#include <string>
#include "caller.h"
extern "C"{
#include <stdlib.h>
}

using namespace std;
int main(int argc, char *argv0[]) {
	auto id = pipe_create(sizeof(CallMessage) * 15);	//最多缓存15个调用请求
	auto fid = fopen("c:/rvmnc.rvmn",'w');
	fwrite(fid,&id,sizeof(int32_t));
	fclose(fid);
	//开始监听
	CallMessage* buffer = new CallMessage[15];
	while(true) {
		auto size = pipe_read(id,buffer);
		if (!size) {
			continue;
		}
		//否则开始处理
		for (int i = 0;i < 15;i ++) {
			if (buffer[i].Empty) {
				break;
			}
			if (buffer[i].DataType == CallerType::RegisterProgram) {	//注册程序
				

				continue;
			}
		}
	}
	return 0;
}