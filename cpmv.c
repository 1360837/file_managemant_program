#include "sys_project9.h"

// 제출 목록
int copy_file(const char *src, const char *dest);
void cpmv(char *src_path,char *dest_path, int mv_flag);
// 제출 목록
int copy_file(const char *src, const char *dest) {
	int src_fd, dest_fd;
	char buffer[BUF_SIZE];
	ssize_t bytes_read, bytes_written;

	src_fd = open(src, O_RDONLY);
	if (src_fd == -1) {
		perror("open");
		return -1;
	}

	dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (dest_fd == -1) {
		perror("open");
		close(src_fd);
		return -1;
	}

	while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
		bytes_written = write(dest_fd, buffer, bytes_read);
		if (bytes_written != bytes_read) {
			perror("write");
			close(src_fd);
			close(dest_fd);
			return -1;
		}
	}

	if (bytes_read == -1) {
		perror("read");
	}

	close(src_fd);
	close(dest_fd);
	return (bytes_read == -1) ? -1 : 0;
}
void cpmv(char *src_path,char *dest_path, int mv_flag) {
	struct stat dest_stat;
	char new_dest_path[BUF_SIZE];
	const char *src_filename = basename((char *)src_path);

	// 목적지 파일 존재 여부 확인
	if (stat(dest_path, &dest_stat) == 0) {
		// 목적지가 디렉토리라면
		if (S_ISDIR(dest_stat.st_mode)) {
			snprintf(new_dest_path, sizeof(new_dest_path), "%s/%s", dest_path, src_filename);
		}
		else {
			// 목적지가 파일이면 현재 디렉토리에 복사/이동
			snprintf(new_dest_path, sizeof(new_dest_path), "./%s", src_filename);
		}
	}
	else {
		// 목적지가 존재하지 않을 경우 현재 디렉토리에 복사/이동
		snprintf(new_dest_path, sizeof(new_dest_path), "%s", dest_path);
	}

	if (mv_flag == 0) { // 복사 (cp)
		copy_file(src_path, new_dest_path);
	}
	else if (mv_flag == 1) { // 이동 (mv)
		if (rename(src_path, new_dest_path) == -1) {
			perror("move");
		}
	}
}