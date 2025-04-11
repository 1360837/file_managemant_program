#include "sys_project9.h"

pthread_mutex_t show_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t done_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t show_cond = PTHREAD_COND_INITIALIZER;//화면 출력을 알리는 cond
pthread_cond_t done_cond = PTHREAD_COND_INITIALIZER;//사용자의 작업지시가 완료되었음을 알리는 cond
pthread_cond_t state_cond = PTHREAD_COND_INITIALIZER;//메인스레드가 작업을 종료하고 입력스레드를 깨울때 사용하는 cond
void *showthread(void *arg);
void *getcommand(void *arg);
void execute_mode();
void tty_mode(int how);
void set_noecho_mode();
void type_mode();
void show_chmod_normal();
void normal_and_select(char ch);
void togle_filter(char ch);
int ctoi(char *str);

command now_command = NOT;//커멘드 입력받을 변수
mode now_mode = NOT;//수행하고자 하는 명령을 저장하는 변수
int focus = 0;//포커스하고있는 파일의 인덱스
char file[BUF_SIZE];//파일의 경로를 저장하는 변수
char path[BUF_SIZE];//현재 작업중인 경로
input_state state = NORMAL;//입력 모드 관리용 변수(노말, 입력, 선택)
char buf[BUF_SIZE];//다용도 버퍼
option now_option;//옵션 저장용 구조체
rwx now_rwx;//chmod normal모드 토글
ugo now_ugo;//chmod실행시 현재 변경중인 범주 표시

int main(void){
	pthread_t getcmd_t;//커멘드 입력받는 작업을 하는 스레드
	pthread_t show_t;//화면출력함수를 호출하는 작업을 하는 스레드
	
	memset(&now_option,0,sizeof(now_option));
	now_option.l = 1;
	memset(file,'\0',BUF_SIZE);
	memset(path,'\0',BUF_SIZE);

	setlocale(LC_ALL, "");	// 트리 구조 문자 인코딩 UTF-8 설정
	
	initscr();		// ncurses 모드 시작
	clear();		// 화면 초기화
	cbreak();		// 사용자 입력을 버퍼링 없이 처리
	noecho();		// 입력된 문자 화면에 표시 안 함
	keypad(stdscr, TRUE);	// 키패드 활성화
	curs_set(0); // 커서 숨김
	flushinp();  // 입력 버퍼를 비우기
	
	start_color();		// 색상 사용 활성화
	// 색상 쌍 설정 (파일 타입 별 출력 색상)
	init_pair(1, COLOR_BLUE, COLOR_BLACK); // 1번 쌍: 파란색 글자, 검정색 배경
	init_pair(2, COLOR_GREEN, COLOR_BLACK); // 2번 쌍: 초록색 글자, 검정색 배경
	if (getcwd(path, BUF_SIZE) == NULL){
		perror("getcwd");
	}
	
	show_screen(path, &focus, file, now_option, now_mode);//초기 화면 출력
	refresh();
	pthread_create(&getcmd_t, NULL, getcommand, NULL);//커멘드 입력 스레드 작동
	pthread_create(&show_t, NULL, showthread, NULL);//화면 출력 스레드 작동
	while (1){
		pthread_mutex_lock(&done_mutex);
		pthread_cond_wait(&done_cond, &done_mutex);//작업 완료 시그널 대기
		
		if (now_command == QUIT)
			break;
		execute_mode();
		if (state != SELECT && state != CHMODE){ //경로선택모드, chmode일시 변수 초기화 과정을 스킵
			now_command = NOT;
			now_mode = NON;
			memset(&now_option,0,sizeof(now_option));
			now_option.l = 1;
		}
		memset(file,'\0',BUF_SIZE);
		pthread_mutex_unlock(&done_mutex);
	}
	pthread_join(show_t, NULL);
	pthread_join(getcmd_t, NULL);
	
	endwin();
	return 0;
}
void execute_mode() {
    if (now_mode == CHMOD){
		int pms = 0;
		state = CHMODE;
		if (now_option.p == 0){
			for (int i=0;i<3;i++){
				int pms_buf = 0;
				now_rwx.r = 1;
				now_rwx.w = 1;
				now_rwx.x = 1;
				now_ugo = i;
				pthread_cond_signal(&show_cond);
				pthread_cond_wait(&done_cond, &done_mutex);
				if (now_rwx.r == 1){
					pms_buf += 4;
				}
				if (now_rwx.w == 1){
					pms_buf += 2;
				}
				if (now_rwx.x == 1){
					pms_buf += 1;
				}
				for (int j=2;j>i;j--)
					pms_buf *= 8;
				pms += pms_buf;
			}
		}
		else if (now_option.p == 1){
			state = TYPE;
			ungetch(KEY_RESIZE); //입력 큐에 KEY_RESIZE입력으로 getch()대기중인 입력스레드 작동시킴.
			addstr("Usage: <USER><GROUP><OTHER>\n");
			addstr("Enter Three number between 1 and 7 mode\n");
			refresh();
			pthread_mutex_lock(&state_mutex);
			pthread_cond_wait(&state_cond, &state_mutex); //입력스레드의 TYPE모드 종료 시그널 대기
			pthread_mutex_unlock(&state_mutex);
			pms = ctoi(buf);
			if (pms == 0){
				now_command = ERROR;
				pthread_cond_signal(&show_cond);//출력 신호
				return;
			}
		}
		pthread_mutex_lock(&show_mutex);
		change_mode(file,pms,now_option.r);
		pthread_mutex_unlock(&show_mutex);
		state = NORMAL;
	}
	else if ((now_mode == CP) || (now_mode == MOVE)){
		if (state == NORMAL){ //cp를 처음 실행했을 때(사용자가 cp 상태에서 엔터를 눌렀을 때)
			state = SELECT; //경로선택모드로 변경
			memset(buf,'\0',BUF_SIZE);
			strcpy(buf,file);
		}
		else if (state == SELECT){ //사용자가 경로선택을 마쳤을 때
			pthread_cond_signal(&show_cond);//출력 신호
			pthread_mutex_lock(&show_mutex);
			struct stat info;
			stat(file,&info);
			if (!S_ISDIR(info.st_mode)){//디렉토리가 아닐때
				memset(file,'\0',BUF_SIZE);
				strcpy(file,".");
			}
			if (now_mode == CP)
				cpmv(buf,file,0);
			else if (now_mode == MOVE)
				cpmv(buf,file,1);
			state = NORMAL; //입력모드를 토글모드로 변경
			pthread_mutex_unlock(&show_mutex);
		}
	}
	else if (now_mode == RM){
		pthread_mutex_lock(&show_mutex);
		remove_dir_file(file,now_option.d);
		pthread_mutex_unlock(&show_mutex);
	}
	else if ((now_mode == RENAME) || (now_mode == MKDIR) || (now_mode == VI)){
		state = TYPE; //type모드로 변경
		ungetch(KEY_RESIZE); //입력 큐에 KEY_RESIZE입력으로 getch()대기중인 입력스레드 작동시킴.
        addstr("filename: ");
		refresh();
		pthread_mutex_lock(&state_mutex);
		pthread_cond_wait(&state_cond, &state_mutex); //입력스레드의 TYPE모드 종료 시그널 대기
		pthread_mutex_unlock(&state_mutex);
		if (now_mode == RENAME){
			pthread_mutex_lock(&show_mutex);
			cpmv(file,buf,1);
			pthread_mutex_unlock(&show_mutex);
		}
		else if (now_mode == MKDIR)
			make_directory(buf);
		else if (now_mode == VI){
			pid_t pid = fork();
            if (pid == 0) { //자식 프로세스
                char *args[] = {"vi", buf, NULL};
                execvp("vi", args); //vi실행 자식 프로세스 종료
				perror("execvp"); //실행 실패 시 에러 출력
				exit(EXIT_FAILURE);
            }
			else if (pid > 0){ //부모 프로세스
				int status;
				waitpid(pid,&status, 0); //자식 프로세스가 종료되기를 기다림
				initscr();		// ncurses 모드 시작
				clear();		// 화면 초기화
				keypad(stdscr, TRUE);	// 키패드 활성화
			}
			else
				perror("fork");
		}
		pthread_cond_signal(&state_cond);//입력스레드 깨움
	}
	pthread_cond_signal(&show_cond);//출력 신호
}
void *showthread(void *arg){
	while (1){
		pthread_mutex_lock(&show_mutex);
		pthread_cond_wait(&show_cond, &show_mutex); //입력스레드에서 show_cond 시그널을 기다림.
		if (now_command == QUIT){ //사용자가 q를 입력했을 때 출력스레드 종료
			pthread_mutex_unlock(&show_mutex);
			break;
		}
		memset(file,'\0',BUF_SIZE);
		memset(path,'\0',BUF_SIZE);
		if (getcwd(path, BUF_SIZE) == NULL){
			perror("getcwd");
		}
		show_screen(path, &focus, file, now_option, now_mode); //화면 출력과 포커스 중인 파일 경로 받아옴
		pthread_mutex_unlock(&show_mutex);
		if (now_command == ERROR){
			addstr("Wrong Input");
			state = NORMAL;
			now_command = NOT;
		}
		if (state == SELECT)
			addstr("Select target path\n");
		else if (state == CHMODE && now_option.p == 0){
			show_chmod_normal();
		}
		refresh();
	}
	return NULL;
}
void *getcommand(void *arg){
	int ch;
	while (1) {
		if (state == TYPE){ //type 모드일때
			pthread_mutex_lock(&state_mutex);
			type_mode();
			pthread_cond_signal(&state_cond); //메인에 입력 종료를 알림
			pthread_cond_wait(&state_cond, &state_mutex);//메인스레드에서 작업을 종료할때까지 대기
			pthread_mutex_unlock(&state_mutex);
			continue;
		}
		else{
			ch = getch();
			if (ch == 'q' || ch == 'Q'){
				now_command = QUIT;
				pthread_cond_signal(&done_cond);//종료 신호
				pthread_cond_signal(&show_cond);//출력 신호
				return NULL;
			}
			else if (ch == '\n'){
				if (state == SELECT){
					
				}
				pthread_cond_signal(&done_cond);//입력 완료 신호
			}
			else if (ch == KEY_UP){//위쪽 화살표를 눌렀을 때
				focus--;
				pthread_cond_signal(&show_cond);
			}
			else if (ch == KEY_DOWN){//아래쪽
				focus++;
				pthread_cond_signal(&show_cond);
			}
			else if (ch == KEY_RIGHT){//오른쪽
				focus = 0; //포커스 초기화
				pthread_mutex_lock(&show_mutex);
				struct stat info;
				stat(file,&info);
				if (S_ISDIR(info.st_mode)){//디렉토리일때
					chdir(file);
				}
				else{//일반 파일일때
					endwin();
					tty_mode(0);
					set_noecho_mode();
					cat(file);
					while(getchar() != 27);//esc입력시 cat종료
					tty_mode(1);
					initscr();
				}
				pthread_mutex_unlock(&show_mutex);
				pthread_cond_signal(&show_cond);
			}
			else if (ch == KEY_LEFT){
				chdir("..");
				focus = 0; //포커스 초기화
				pthread_cond_signal(&show_cond);
			}
			else if (ch == 27){
				now_mode = NON;
				state = NORMAL;
				memset(&now_option,0,sizeof(option));
				now_option.l = 1;
				pthread_cond_signal(&show_cond);
			}
			if (state == SELECT){
				continue;
			}
		}
		togle_filter(ch);
	}
}
int ctoi(char *str){
	if (strlen(str) != 3) {
        return 0;
    }
    for (int i = 0; i < 3; i++) {
        if (str[i] < '0' || str[i] > '7') {
            return 0;
        }
    }
    int result = 0;
    for (int i = 0; i < 3; i++) {
        result = result * 8 + (str[i] - '0');
    }

    return result;
}
void show_chmod_normal(){
	char temp[7] = {' ', ' ', ' ', ' ', ' ', ' ', '\0'};
	if (now_ugo == USER){
		addstr("[SET USER MODE]\n");
	}
	else if (now_ugo == GROUP){
		addstr("[SET GROUP MODE]\n");
	}
	else if (now_ugo == OTHER){
		addstr("[SET OTHRE MODE]\n");
	}
	addstr("Press 'r', 'w', 'x' to turn each option on or off.\n");
	if (now_rwx.r == 1) temp[0] = 'r';
	if (now_rwx.w == 1) temp[2] = 'w';
	if (now_rwx.x == 1) temp[4] = 'x';
	addstr(temp);
}
void type_mode(){
	nocbreak(); //버퍼링 처리
	echo(); //에코모드
	curs_set(1); //커서 표시
	memset(buf,'\0',sizeof(buf));
    getnstr(buf, BUF_SIZE - 1); //입력
	cbreak();
	noecho();
	curs_set(0);
	state = NORMAL; //토글모드로 변경
}
void togle_filter(char ch){
	if (state == CHMODE && now_option.p == 0){
		if (ch == 'r' || ch == 'R'){
			now_rwx.r ^= 1;
			pthread_cond_signal(&show_cond);
		}
		else if (ch == 'w' || ch == 'W'){
			now_rwx.w ^= 1;
			pthread_cond_signal(&show_cond);
		}
		else if (ch == 'x' || ch == 'X'){
			now_rwx.x ^= 1;
			pthread_cond_signal(&show_cond);
		}
		return;
	}
	if (ch >= '1' && ch <= '7'){//숫자 커멘드 입력시
		now_mode = ch - '0';
		memset(&now_option,0,sizeof(option));//현재 입력된 옵션 토글 초기화
		now_option.l = 1;
		pthread_cond_signal(&show_cond);
	}
	else if (ch == 'a' || ch == 'A'){
		now_option.a ^= 1;
		pthread_cond_signal(&show_cond);
	}
	else if (ch == 'l' || ch == 'L'){
		now_option.l ^= 1;
		pthread_cond_signal(&show_cond);
	}
	else if (ch == 'r' || ch == 'R'){
		now_option.r ^= 1;
		pthread_cond_signal(&show_cond);
	}
	else if (ch == 'p' || ch == 'P'){
		now_option.p ^= 1;
		pthread_cond_signal(&show_cond);
	}
	else if (ch == 'd' || ch == 'D'){
		now_option.d^= 1;
		pthread_cond_signal(&show_cond);
	}
}
void tty_mode(int how){
	static struct termios orig_mode;
	static int orig_flags;
	if (how == 0){
		tcgetattr(0, &orig_mode);
		orig_flags = fcntl(0, F_GETFL);
	}
	else if (how == 1){
		tcsetattr(0, TCSANOW, &orig_mode);
		fcntl(0, F_SETFL, orig_flags);
	}
}
void set_noecho_mode(){
	struct termios ttyinfo;
	if (tcgetattr(0, &ttyinfo) == -1){
		perror("tcgetattr");
		exit(EXIT_FAILURE);
	}
	ttyinfo.c_lflag &= ~ICANON;
	ttyinfo.c_lflag &= ~ECHO;
	ttyinfo.c_cc[VMIN] =1;

	if (tcsetattr(0, TCSANOW, &ttyinfo) == -1){
		perror("tcsetattr");
		exit(EXIT_FAILURE);
	}
}