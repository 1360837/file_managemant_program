#ifndef SYS_PROJECT9_H
# define SYS_PROJECT9_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <ctype.h>
#include <curses.h>
#include <locale.h>
#include <pthread.h>
#include <termio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <libgen.h>

#define	MAX_ENTRIES 1024 // tree 최대 항목 수 설정
#define INFO_COL 25 // fileinfo 출력시작열
#define BUF_SIZE 1024 // 경로명 저장변수 크기

typedef	struct	counter
{
	size_t	dirs;
	size_t	files;
}	counter_t;
typedef enum mode {//선택한 작업 명령을 나타내는 변수
	NON, CHMOD, CP, MOVE, RENAME, MKDIR, RM, VI
}mode;
typedef enum command {//종료 명령을 받는 변수
	NOT, QUIT, ERROR
}command;
typedef enum input_state {//입력 모드를 나타내는 변수
	NORMAL, TYPE, SELECT, CHMODE
} input_state;
typedef enum ugo {
	USER, GROUP, OTHER
} ugo;
typedef struct option {//명령의 모든 옵션을 모아놓은 변수
	int a; int l; int r; int p; int d;
} option;
typedef struct rwx{//chmod normal모드 실행시 사용하는 토글변수
	int r; int w; int x;
} rwx;

// 파일명.c
// 파일안에 들어가는 함수들
//chmod.c
void change_mode(char *name, int op1, int op2); // chmod 함수
void do_chmod(const char *name, mode_t new_mode, int op2); // mode 변경을 위한 별도 함수
//cpmv.c
int copy_file(const char *src, const char *dest);
void cpmv(char *src_path,char *dest_path, int mv_flag);
//rm.c
void remove_dir_file(const char *name, int op1);
//mkdir.c
void make_directory(char *name);
//cat.c
void cat(char *name);
// file_info.c
void	do_stat(char *, const char *, int cur, int idx, int row, option op);
void	mode_to_letters(mode_t mode, char mode_str[]);
char	*uid_to_name(uid_t);
char	*gid_to_name(gid_t);
void	show_file_info(struct stat *);
void	tree_fileinfo(const char *directory, counter_t *counter, int cur_posi, char *path, option op);
size_t  find_entries(char entries[MAX_ENTRIES][256], const char *directory, counter_t *counter, option op);

// screen.c
void	show_screen(const char *directory, int *idx, char *path, option op, int mode);
void	print_mode(const char *, int);

#endif
