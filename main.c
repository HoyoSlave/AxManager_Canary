//Credit: t.me/Koneko_dev (t.me/Droid_ch)
#define _GNU_SOURCE
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
namespace fs = std::filesystem;
constexpr size_t READAHEAD_CHUNK = 64 * 1024 * 1024;
void preloadFile(const std::string& filepath) {
  int fd = open(filepath.c_str(), O_RDONLY);
  if (fd == -1) return;
  struct stat st;
  if (fstat(fd, &st) == -1 || !S_ISREG(st.st_mode)) {
    close(fd);
    return;
  }
  off_t fileSize = st.st_size;
  posix_fadvise(fd, 0, fileSize, POSIX_FADV_SEQUENTIAL);
  posix_fadvise(fd, 0, fileSize, POSIX_FADV_WILLNEED);
  bool readaheadFailed = false;
  off_t offset = 0;
  while (offset < fileSize) {
    size_t len = (fileSize - offset < static_cast<off_t>(READAHEAD_CHUNK))
           ? (fileSize - offset)
           : READAHEAD_CHUNK;
    if (readahead(fd, offset, len) == -1) {
      readaheadFailed = true;
      break;
    }
    offset += len;
  }
  if (readaheadFailed || offset < fileSize) {
    lseek(fd, 0, SEEK_SET);
    std::vector<char> buffer(1024 * 1024);
    while (read(fd, buffer.data(), buffer.size()) > 0);
  }
  close(fd);
}
void processPath(const std::string& pathStr) {
  fs::path p(pathStr);
  if (!fs::exists(p)) return;
  if (fs::is_regular_file(p)) {
    preloadFile(p.string());
  } else if (fs::is_directory(p)) {
    for (const auto& entry : fs::recursive_directory_iterator(p)) {
      if (fs::is_regular_file(entry.status())) {
        preloadFile(entry.path().string());
      }
    }
  }
}
int main(int argc, char* argv[]) {
  if (argc < 2) return 1;
  for (int i = 1; i < argc; ++i) {
    processPath(argv[i]);
  }
  return 0;
}