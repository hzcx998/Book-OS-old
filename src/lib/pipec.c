#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

/*
管道写入需要等待管道处于连接状态，既没有数据在传输
*/
int32_t pipe_write(uint32_t pipe_id,void* data,uint32_t size)
{
	int ret;
	
	//循环控制
	do {
		ret = _pipe_write(pipe_id, data, size);
	} while (ret == -1) ;
	
	return ret;
}
/*
管道取消要等管道处于链接状态，既没有数据在传输
*/
int32_t pipe_cancel(uint32_t pipe_id)
{
	int ret;
	
	//循环控制
	do {
		ret = _pipe_cancel(pipe_id);
	} while (ret == -1) ;
	
	return ret;
}
