#ifndef _CONST_H_
#define _CONST_H_

/*
Version: -v0.1 2018/9/26~2019/1/7
Version: -v0.2 2019/1/8~2019/2/21
Version: -v0.3 2019/2/22~2019/4/6
Version: -v0.39 2019/4/7~2019/5/2
Version: -v0.4 2019/5/3~2019/5/13
*/

#define OS_NAME "Book OS"
#define OS_VERSION "0.4"
#define OS_COPYRIGHT "(C) 2018-2019 by Book OS developers. All rights reserved."
#define OS_AUTHER "Hu Zicheng"

/*
如果实在其它头文件里面调用另外一个模块的内容，就写sys/...
如果是在同一个头文件里面调用本模块另外一个内容，就写 模块/...

例如：
同模块内部
>>core/thread.h
#include <core/process.h>
不同模块
>>gui/basic.h
#include <sys/core.h>
*/


#endif

