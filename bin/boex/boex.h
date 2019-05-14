#ifndef _BOSH_H
#define _BOSH_H

/*
BOSHELL 核心
*/


#define BOSH_NAME "bosh"


#define CMD_LINE_LEN 128
#define MAX_ARG_NR 16

#define PIPE_BUF_LEN 256

extern int pipe_id;


//func
void print_prompt();
int cmd_parse(char * cmd_str, char **argv, char token);
int read_key(char *start, char *buf, int count);
void readline( char *buf, uint32_t count);

void wash_path(char *old_abs_path, char * new_abs_path);
void make_clear_abs_path(char *path, char *final_path);
char* path_parse(char* pathname, char* name_store);
void make_abs_path(const char *path, char *final_path);
void make_path(const char *old_path, char *final_path);

void ls(char *pathname, int detail);

int wait(int *status);
void receive_child_message();

void force_exit_check();

#endif  //_BOSH_H
