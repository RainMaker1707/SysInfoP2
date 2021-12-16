#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib_tar.h"

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_dump(const uint8_t *bytes, size_t len) {
    for (int i = 0; i < len;) {
        printf("%04x:  ", (int) i);

        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) {
            printf("%c ", bytes[i]);
        }
        printf("\n");
    }
}
/*
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s tar_file\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1] , O_RDONLY);
    if (fd == -1) {
        perror("open(tar_file)");
        return -1;
    }

    int ret = check_archive(fd);
    printf("check_archive returned %d\n", ret);

    return 0;
}
 */

int main(int argc, char **argv){
    int tar_fd = open("./archive2.tar", O_RDONLY);
    printf("IS DIR with dir? -- %d (1)\n", is_dir(tar_fd, "archive/dir/not_dir/"));
    printf("IS DIR with file? -- %d (0)\n", is_dir(tar_fd, "archive/file.txt"));
    printf("IS FILE with dir? -- %d (0)\n", is_file(tar_fd, "archive/dir/"));
    printf("IS FILE with file? -- %d (1)\n", is_file(tar_fd, "archive/dir/file2.txt"));
    printf("EXISTING FILE -- %d (1)\n", exists(tar_fd, "archive/dir/file2.txt"));
    printf("IS FILE with file? -- %d (1)\n", is_file(tar_fd, "archive/file.txt"));
    printf("IS FILE with file? -- %d (1)\n", is_file(tar_fd, "archive/dir/not_dir/file3.txt"));
    printf("IS FILE with TAR? -- %d (1)\n", is_file(tar_fd, "archive/dir/archive.tar"));
    printf("%d\n", check_archive(tar_fd));
    return EXIT_SUCCESS;
}