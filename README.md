# Book-OS
Book is a 32-bit operating system based on x86 platform.  
Book 是一个基于x86平台的32位操作系统。  
Enjoy it!  
尽情享受吧！  
邮箱 E-mail: 2323168280@qq.com  

官网 Website: www.book-os.org  

版本发布 Update:  
    v0.4 2019/5/13  
    v0.39 2019/5/2  
    v0.3 2019/4/6  
    v0.2 2019/2/21  
    v0.1 2019/1/7  
    
截屏 ScreenShot:  
![image](https://github.com/huzichengdevelop/Book-OS/blob/master/screenshoot/start.jpg)  
![image](https://github.com/huzichengdevelop/Book-OS/blob/master/screenshoot/desktop.jpg)  

操作指令 command in makefile  
make		-compile, link（编译并且链接）  
make all 	-compile, link and run os in vm（编译链接写入磁盘并且在虚拟机中运行）  
make qemu	-run in qemu（直接在qemu虚拟机中运行）  
make bochs	-run in bochs（直接在bochs虚拟机中运行）  
make bochsdbg	-run in bochsdbg（运行bochsdbg调试器）  
make clean	-clean all .o, .bin, .elf file（删除所有产生的临时文件）  

# 我们理想的目标是  
可以在自己的电脑上运行这个操作系统！  
可以在这个操作系统上使用继承开发环境开发应用程序!  
可以用这个操作系统编译这个操作系统（自举）!  

# Our ideal goal  
We can use this OS on our computer！  
We can use IDE to make application on this OS!  
We can use this OS to compile this OS!  
