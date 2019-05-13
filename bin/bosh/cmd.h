#ifndef _BOSH_CMD_H
#define _BOSH_CMD_H

/*
BOSHELL 命令相关
*/

//cmd
void cmd_cls(uint32_t argc, char** argv);
void cmd_pwd(uint32_t argc, char** argv);
char *cmd_cd(uint32_t argc, char** argv);
void cmd_ls(uint32_t argc, char** argv);
void cmd_help(uint32_t argc, char** argv);
void cmd_ps(uint32_t argc, char** argv);
int cmd_mkdir(uint32_t argc, char** argv);
int cmd_rmdir(uint32_t argc, char** argv);
int cmd_rm(uint32_t argc, char** argv);
int cmd_cat(int argc, char *argv[]);
int cmd_echo(int argc, char *argv[]);
int cmd_type(int argc, char *argv0[]);
void cmd_dir(uint32_t argc, char** argv);
void cmd_ver(uint32_t argc, char** argv);
void cmd_time(uint32_t argc, char** argv);
void cmd_date(uint32_t argc, char** argv);
int cmd_rename(uint32_t argc, char** argv);
int cmd_move(uint32_t argc, char** argv);
int cmd_copy(uint32_t argc, char** argv);
void cmd_reboot(uint32_t argc, char** argv);
void cmd_exit(uint32_t argc, char** argv);
void cmd_mm(uint32_t argc, char** argv);
void cmd_lsdisk(uint32_t argc, char** argv);
void cmd_ls_sub(char *pathname, int detail);
void cmd_ps_sub();
int cmd_kill(uint32_t argc, char** argv);

#endif  //_BOSH_CMD_H
