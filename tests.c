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
    int tar_fd = open("./archive4.tar", O_RDONLY);
    printf("IS DIR with dir? -- %d (1)\n", is_dir(tar_fd, "archive/dir/not_dir/"));
    printf("IS DIR with file? -- %d (0)\n", is_dir(tar_fd, "archive/file.txt"));
    printf("IS FILE with dir? -- %d (0)\n", is_file(tar_fd, "archive/dir/"));
    printf("IS FILE with file? -- %d (1)\n", is_file(tar_fd, "archive/dir/file2.txt"));
    printf("EXISTING FILE -- %d (1)\n", exists(tar_fd, "archive/dir/file2.txt"));
    printf("IS FILE with file? -- %d (1)\n", is_file(tar_fd, "archive/file.txt"));
    printf("IS FILE with file? -- %d (1)\n", is_file(tar_fd, "archive/dir/not_dir/file3.txt"));
    printf("IS FILE with TAR? -- %d (1)\n", is_file(tar_fd, "archive/dir/archive.tar"));
    printf("IS LINK with dir -- %d (0)\n", is_symlink(tar_fd, "archive/dir/"));
    printf("IS LINK with file -- %d (0)\n", is_symlink(tar_fd, "archive/dir/file.txt"));
    printf("IS LINK with symlink -- %d (1)\n", is_symlink(tar_fd, "archive/link"));
    printf("Valid archive -- %d (>0)\n", check_archive(tar_fd));



    uint8_t *dest = (uint8_t*)malloc(sizeof(uint8_t)*512);
    size_t len = 512;
    printf("%zd\n", read_file(tar_fd, "archive/file.txt", 0, dest, &len));
    printf("%s\n", dest);
    free(dest);

    lseek(tar_fd, 0, SEEK_SET);
    uint8_t *dest2 = (uint8_t*)malloc(sizeof(uint8_t)*512);
    size_t len2 = 512;
    printf("%zd\n", read_file(tar_fd, "archive/dir/file2.txt", 0, dest2, &len2));
    printf("%s\n", dest2);
    free(dest2);

    uint8_t *dest3 = (uint8_t*)malloc(sizeof(uint8_t)*512);
    size_t len3 = 512;
    printf("%zd\n", read_file(tar_fd, "archive/link", 0, dest3, &len3));
    printf("%s\n", dest3);
    free(dest3);

    printf("Test 1 --- %d (1)\n", path_helper("archive/dir/", "archive/dir/"));
    printf("Test 2 --- %d (0)\n", path_helper("archive/", "archive/dir/not_dir/"));
    printf("Test 3 --- %d (1)\n", path_helper("archive/", "archive/file.txt"));
    printf("Test 4 --- %d (0)\n", path_helper("archive/dir/", "archive/dir/not_dir/file3.txt"));
    printf("Test 5 --- %d (0)\n", exists(tar_fd, "archive/dir/no_dir/"));
    printf("Test 6 --- %d (0)\n", exists(tar_fd, "../archive/dir/"));


    char** entries = (char**)malloc(sizeof(char) * 100);
    for (int i = 0; i < 100; i++) entries[i] = (char*) malloc(sizeof(char)*100);
    size_t *no_entries = (size_t*)malloc(sizeof(size_t));
    *no_entries = 5;
    printf("V: %d\n", list(tar_fd, "archive/dir/", entries, no_entries));
    for(int i = 0; i < *no_entries; i++) printf("LIST: %s\n", entries[i]);
    free(entries);
    return EXIT_SUCCESS;
}