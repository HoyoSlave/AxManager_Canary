//Credit: t.me/Koneko_dev (t.me/Droid_ch)
#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
int main(int argc, char *argv[]) {
  if (argc < 2) return 0;
  for (int i = 1; i < argc; i++) {
    int fd = open(argv[i], O_RDONLY);
    if (fd != -1) {
      posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
      posix_fadvise(fd, 0, 0, POSIX_FADV_WILLNEED);
      close(fd);
    }
  }
  return 0;
}