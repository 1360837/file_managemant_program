#include "sys_project9.h"

// 제출 목록
void remove_dir_file(const char *name, int op1);
// 제출 목록

void remove_dir_file(const char *name, int op1)
{
    struct stat st_buf;
	char buf[BUF_SIZE];
		
	if (stat(name, &st_buf) == -1) 
	{
		perror("stat");
		return;
	}

    if (op1 == 0) // 디렉토리와 내부의 모든 파일을 재귀적으로 삭제
    {
        if (S_ISDIR(st_buf.st_mode)) // 대상이 디렉토리인지 확인
        {
            struct dirent *dirent_ptr = NULL;
            DIR *dir;
            dir = opendir(name);

            if (!dir)
            {
                perror("opendir");
                return;
            }

            while ((dirent_ptr = readdir(dir)) != NULL)
            {
                if (strcmp(dirent_ptr->d_name, ".") == 0 || strcmp(dirent_ptr->d_name, "..") == 0)  continue;

                // 디렉토리 주소 저장
                char fullpath[BUF_SIZE];

                strncpy(fullpath, name, sizeof(fullpath) - 1);
                fullpath[sizeof(fullpath) - 1] = '\0';

                strncat(fullpath, "/", sizeof(fullpath) - strlen(fullpath) - 1);
                strncat(fullpath, dirent_ptr->d_name, sizeof(fullpath) - strlen(fullpath) - 1);

                if (stat(fullpath, &st_buf) == 0 && S_ISDIR(st_buf.st_mode))
                {
                    remove_dir_file(fullpath, op1);
                }
                else
                {
                    if (remove(fullpath) != 0)	perror("remove");
                }
            }

            closedir(dir);

            if (remove(name) == 0){
				memset(buf,'\0',BUF_SIZE);
				snprintf(buf,BUF_SIZE,"Directory (%s) and all its file are removed successfully.\n",name);
				addstr(buf);
				refresh();
			}
			else perror("remove");
        }
        else // 파일 삭제
        {
            if (remove(name) == 0){
				memset(buf,'\0',BUF_SIZE);
				snprintf(buf,BUF_SIZE,"File (%s) removed successfully.\n",name);
				addstr(buf);
				refresh();
			}
			else perror("remove");
		}
    }
    else if (op1 == 1) // 파일은 항상 삭제, 디렉토리는 비어있는 경우에만 삭제
    {
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
            int dir_check = 0;
            
            // 빈 디렉토리인지 확인
            while ((dirent_ptr = readdir(dir)) != NULL)
            {
                if (strcmp(dirent_ptr->d_name, ".") != 0 && strcmp(dirent_ptr->d_name, "..") != 0)
                {
                    dir_check = 1;
                    break;
                }
            }
            
            closedir(dir);

            // 빈 디렉토리만 삭제
            if (dir_check == 0)
            {
                if (remove(name) == 0){
					memset(buf,'\0',BUF_SIZE);
					snprintf(buf,BUF_SIZE,"Directory (%s) removed successfully.\n",name);
					addstr(buf);
					refresh();
				}
                else    perror("remove");
            }
            else{
				memset(buf,'\0',BUF_SIZE);
				snprintf(buf,BUF_SIZE,"Cannot remove directory (%s) - directory is not empty\n",name);
				addstr(buf);
				refresh();
			}
        }
        else // 파일 삭제
        {
            if (remove(name) == 0){
				memset(buf,'\0',BUF_SIZE);
				snprintf(buf,BUF_SIZE,"File (%s) removed successfully.\n",name);
				addstr(buf);
				refresh();
			}
            else    perror("remove");
        }
    }
}