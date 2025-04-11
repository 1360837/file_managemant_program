#include "sys_project9.h"

void make_directory(char *name)
{
	char buf[BUF_SIZE];
	if (mkdir(name, 0775) == -1) // default permission setting
	{
		memset(buf,'\0',BUF_SIZE);
		snprintf(buf,BUF_SIZE,"Failed to make directory (%s).\n",name);
		addstr(buf);
		refresh();
        return;
    }
	memset(buf,'\0',BUF_SIZE);
	snprintf(buf,BUF_SIZE,"Directory (%s) has been created.\n",name);
	addstr(buf);
	refresh();
}