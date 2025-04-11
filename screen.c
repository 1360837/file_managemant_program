#include "sys_project9.h"
// 함수 구현
void	show_screen(const char *directory, int *idx, char *path, option op, int mode)
{
	char	line[100];
	counter_t	counter = {1, 0};
	
	clear();		// 화면 초기화

	if (*idx < 0)
		*idx = 0;
	
	addstr("-option---------------------------------------------------------------\n");
	print_option(op, mode);
	addstr("----------------------------------------------------------------------\n");

	tree_fileinfo(directory, &counter, *idx, path, op);
	
	if (*idx >= counter.files)
		*idx = counter.files - 1;
	
	move(LINES - 10, 0);
	snprintf(line, sizeof(line), 
			"%zu directories, %zu files\n", counter.dirs - 1, counter.files);
	addstr(line);
	addstr("-mode-----------------------------------------------------------------\n");
	print_command(mode);
	addstr("----------------------------------------------------------------------\n");
}
void	print_option(option op, int mode)
{
	switch (mode) {
		case 0:  // mode가 0일 때: 'a', 'l' 옵션 출력
			if (op.a == 1) {
				standout();  // 강조
				addstr(" a ");
				standend();
			}
			else {
				addstr(" a ");
			}
			if (op.l == 1) {
				standout();  // 강조
				addstr(" l ");
				standend();
			} else {
				addstr(" l ");
			}
			break;
		case 1:  // mode가 1일 때: 'r', 'p' 옵션 출력
			if (op.r == 1) {
				standout();  // 강조
				addstr(" r ");
				standend();
			}
			else {
				addstr(" r ");
			}
			if (op.p == 1) {
				standout();  // 강조
				addstr(" p ");
				standend();
			}
			else {
				addstr(" p ");
			}
			break;
		case 6:  // mode가 6일 때: 'd' 옵션 출력
			if (op.d == 1) {
				standout();  // 강조
				addstr(" d ");
				standend();
			}
			else {
				addstr(" d ");
			}
			break;
        // 나머지 mode (2, 3, 4, 5, 7): 옵션 출력 없음
        default:
            break;
    }
	addstr("\n");
}
void	print_command(int mode)
{
	char	command[7][10] = {"1.CHMOD", "2.COPY", "3.MOVE", "4.RENAME", "5.MKDIR", "6.REMOVE", "7.VI"};

	for (int i = 0; i < 7; i++)
	{
		move(LINES - 8, i * 10);
		if (mode == i + 1)
		{
			standout();
			addstr(command[i]);
			standend();
		}
		else
			addstr(command[i]);
	}
	addstr("\n");
}
void	print_mode(const char *name, int mode)
{
	attron(COLOR_PAIR(mode));
	attron(A_BOLD);
	addstr(name);
	attroff(COLOR_PAIR(mode));
	attroff(A_BOLD);
}
