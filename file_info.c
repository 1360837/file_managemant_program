#include "sys_project9.h"

// tree
void	tree_fileinfo(const char *directory, counter_t *counter, int cur_posi, char *path, option op)
{
	char	entries[MAX_ENTRIES][256]; // 파일 또는 디렉토리 이름 길이 최대 20
	size_t	size, idx = 0;
	size = find_entries(entries, directory, counter, op);
	if (size == 0)
		return ;
	
	print_mode(directory, 1);
	
	for (int row = 4; row < LINES - 10; row++)
	{
		move(row, 0);
		if (idx == size - 1)
			addstr("└── ");
		else
			addstr("├── ");
		do_stat(entries[idx], directory, cur_posi, idx, row, op);
		if (idx == cur_posi)
			snprintf(path, 512, "%s/%s", directory, entries[idx]);
		if (idx == size - 1) break ;
		idx++;
	}

}
size_t	find_entries(char entries[MAX_ENTRIES][256], const char *directory, counter_t *counter, option op)
{
	struct	dirent *file_dirent;
	DIR	*dir_handle;
	size_t	size = 0;
	
	dir_handle = opendir(directory);
	if (!dir_handle)
	{
		addstr("Cannot open directory ");
		addstr(directory);
		return 0;
	}
	counter->dirs++;
	
	while ((file_dirent = readdir(dir_handle)) != NULL)
	{
		if (file_dirent->d_name[0] == '.' && op.a == 0) continue ; // 숨김 파일 무시

		if (size < MAX_ENTRIES)
		{
			strcpy(entries[size++], file_dirent->d_name);
			counter->files++;
		}
		else
			break ;
	}
	closedir(dir_handle);


	for (size_t i = 0; i + 1 < size; i++)	// 알파벳 순 정렬
	{
		for (size_t j = i + 1; j < size; j++)
		{
			if (strcmp(entries[i], entries[j]) > 0)
			{
				char	temp[256];
				strcpy(temp, entries[i]);
				strcpy(entries[i], entries[j]);
				strcpy(entries[j], temp);
			}
		}
	}
	return size;
}
// ls
void	do_stat(char *filename, const char *dirname, int cur, int idx, int row, option op)
{
	struct	stat buf;
	char	name[100];
	char	mode[10];

	memset(name, 0, sizeof(name));
	strcpy(name, dirname);
	strcat(name, "/");
	strcat(name, filename);
	if (stat(name, &buf) == -1)
		perror(filename);
	else
	{
		mode_to_letters(buf.st_mode, mode);
		if (cur == idx)
			standout();
		if (mode[0] == 'd')
			print_mode(filename, 1);
		else if (mode[9] == 'x')
			print_mode(filename, 2);
		else
			addstr(filename);
		standend();
		if (op.l == 1){
			move(row, INFO_COL);
			addstr(mode);
			show_file_info(&buf);
		}
	}
}
void	show_file_info(struct stat *info_p)
{
	char	buf[50];

	snprintf(buf, sizeof(buf), "%2d %-5s %-5s %5ld %.12s\n", 
			(int)info_p->st_nlink,
			uid_to_name(info_p->st_uid), gid_to_name(info_p->st_gid),
			(long)info_p->st_size, 4 + ctime(&info_p->st_mtime));
	addstr(buf);
}
void	mode_to_letters(mode_t mode, char mode_str[])
{
	strcpy(mode_str, "----------");

	if (S_ISDIR(mode)) mode_str[0] = 'd';
	if (S_ISCHR(mode)) mode_str[0] = 'c';
	if (S_ISBLK(mode)) mode_str[0] = 'b';

	if (mode & S_IRUSR) mode_str[1] = 'r';
	if (mode & S_IWUSR) mode_str[2] = 'w';
	if (mode & S_IXUSR) mode_str[3] = 'x';

	if (mode & S_IRGRP) mode_str[4] = 'r';
	if (mode & S_IWGRP) mode_str[5] = 'w';
	if (mode & S_IXGRP) mode_str[6] = 'x';

	if (mode & S_IROTH) mode_str[7] = 'r';
	if (mode & S_IWOTH) mode_str[8] = 'w';
	if (mode & S_IXOTH) mode_str[9] = 'x';
}
char	*uid_to_name(uid_t uid)
{
	struct	passwd *pw_ptr = NULL;
	static	char	uid_str[10];

	if ((pw_ptr = getpwuid(uid)) == NULL)
	{
		sprintf(uid_str, "%d", uid);
		return uid_str;
	}
	else
		return pw_ptr->pw_name;
}
char	*gid_to_name(gid_t gid)
{
	struct	group	*gp_ptr = NULL;
	static	char	gid_str[10];

	if ((gp_ptr = getgrgid(gid)) == NULL)
	{
		sprintf(gid_str, "%d", gid);
		return gid_str;
	}
	else
		return gp_ptr->gr_name;
}
