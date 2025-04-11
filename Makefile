NAME = file_menagement

CC = gcc
CFLAGS = -Wall
LDFLAGS = -lncursesw -lpthread
RM  = rm -f
AR = ar
ARFLAGS = crs

INCLUDES = ./sys_project9.h

SRCS = 	main.c		\
		screen.c	\
		file_info.c	\
		chmod.c		\
		cpmv.c		\
		mkdir.c		\
		cat.c		\
		rm.c		\
	   
OBJS = $(SRCS:.c=.o)

all: $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	   $(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

clean:
	$(RM) $(RMFLAGS) $(OBJS)

fclean: clean
	$(RM) $(RMFLAGS) $(NAME)

re: fclean all


.PHONY: all clean fclean re
