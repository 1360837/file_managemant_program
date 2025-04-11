#include "sys_project9.h"

// 제출 목록
void change_mode(char *name, int pms, int op2); // chmod 함수
void do_chmod(const char *name, mode_t new_mode, int op2); // mode 변경을 위한 별도 함수
// 제출 목록

// option.r - 0이면 디렉토리에 속한 모든 파일의 속성 변경, 1이면 디렉토리의 속성만 변경
void change_mode(char *name, int pms, int op2)
{
	mode_t new_mode = 0;

	mode_t read_mask[3] = {S_IRUSR, S_IRGRP, S_IROTH};
    mode_t write_mask[3] = {S_IWUSR, S_IWGRP, S_IWOTH};
    mode_t exe_mask[3] = {S_IXUSR, S_IXGRP, S_IXOTH};

    for (int i = 0; i < 3; i++) {
        if (pms & (0400 >> (i * 3))) new_mode |= read_mask[i];
        if (pms & (0200 >> (i * 3))) new_mode |= write_mask[i];
        if (pms & (0100 >> (i * 3))) new_mode |= exe_mask[i];
	}

	do_chmod(name, new_mode, op2);
}

void do_chmod(const char *name, mode_t new_mode, int op2)
{
	if (op2 == 0) // 디렉토리에 속한 모든 파일의 속성 변경
	{
		struct stat st_buf;
		
		if (stat(name, &st_buf) == -1) 
		{
			perror("stat");
			return;
		}

		if (chmod(name, new_mode) == -1)
		{
			perror("chmod");
			return;
		}

		// 대상이 파일인지 디렉토리인지 확인
		if (S_ISDIR(st_buf.st_mode))
		{
			DIR *dir = opendir(name);
			if (!dir)
			{
				perror("opendir");
				return;
			}

			struct dirent *dirent_ptr = NULL;
        	while ((dirent_ptr = readdir(dir)) != NULL)
			{
				if (strcmp(dirent_ptr->d_name, ".") == 0 || strcmp(dirent_ptr->d_name, "..") == 0)	continue;

				char fullpath[BUF_SIZE]; // 디렉토리의 주소를 저장할 배열

				// 디렉토리 주소 저장
            	strncpy(fullpath, name, sizeof(fullpath) - 1);
            	fullpath[sizeof(fullpath) - 1] = '\0';

            	if (fullpath[strlen(fullpath) - 1] != '/')
				{
            	    strncat(fullpath, "/", sizeof(fullpath) - strlen(fullpath) - 1);
            	}

            	strncat(fullpath, dirent_ptr->d_name, sizeof(fullpath) - strlen(fullpath) - 1);
            	do_chmod(fullpath, new_mode, op2); // 재귀적으로 디렉토리 및 모든 파일의 속성 변경
        	}

        	closedir(dir);
    	}
	}
	else if (op2 == 1) // 해당 파일 또는 디렉토리의 속성만 변경
	{
		if (chmod(name, new_mode) == -1)
		{
			perror("chmod");
			return;
		}
	}
}
