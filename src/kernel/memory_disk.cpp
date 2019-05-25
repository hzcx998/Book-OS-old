
#include <new>
extern "C" {
#include <sys/memory_disk.h>
#include <memory.h>
#include <types.h>
#include <string.h>
}
struct file {
    bool ability = true;
    char* file_name;
    void* memory_start;
    unsigned long long size;
    unsigned long long max_size;
} *files;
unsigned int now_file_number = 0;
void init_memory_disk() {
    files = new file[MAX_FILE];
    return;
}
bool file_exist(char *file_name) {
    for (int i = 0;i < MAX_FILE;i ++) {
        if (strcmp(files[i].file_name,file_name)) {
            return true;
        }
    }
    return false;
}
bool delete_file(char* file_name) {
    //开始寻找
    for (int i = 0;i < MAX_FILE;i ++) {
        if (strcmp(files[i].file_name,file_name)) { //找到了
            delete files[i].memory_start;
            files[i].ability = true;
            now_file_number --;
            return true;
        }
    }
    return false;
}
bool new_file(char* file_name,unsigned long long max_size) {
    if (now_file_number == MAX_FILE) {
        return false;
    }
    //开始寻找空余位置
    if (file_exist(file_name)) {
        return false;
    }
    for (unsigned int i = 0;i < MAX_FILE;i ++) {
        if (files[i].ability) {
            files[i].file_name = file_name;
            files[i].max_size = max_size;
            files[i].memory_start = malloc(0);
            files[i].ability = false;
            break;
        }

    }
    now_file_number ++;
    return true;
}