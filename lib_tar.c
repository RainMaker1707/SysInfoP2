#include "lib_tar.h"
#include <stdio.h> // todo remove only here to use print on debug

/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of non-null headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
int check_archive(int tar_fd) {
    int result = 0;
    char *buffer = (char*)malloc(sizeof(char)*512);
    if(!buffer) return EXIT_FAILURE;
    while(read(tar_fd, buffer, 512)){
        if (*buffer == AREGTYPE) break; //end of archive/file '\0'
        tar_header_t *header = (tar_header_t*) buffer;
        // hardcoded value to test if strcmp don't work
        if(strncmp(header->magic, TMAGIC, TMAGLEN) != 0) result = -1; // magic value is not ustar
        else if(strncmp(header->version, TVERSION, TVERSLEN) != 0) result = -2;   // version value is not 00
        else if(TAR_INT(header->chksum) != checksum(buffer)) result = -3;// invalid checksum-> dangerous file
        if (result < 0) break; // if it is invalid archive
        if(header->typeflag == REGTYPE ){
            size_t size = TAR_INT(header->size);
            if(size != 0) lseek(tar_fd, 512*(size/512 +1), SEEK_CUR);
        }
        result++; // one header successfully passed
    }
    free(buffer); //garbage buffer
    lseek(tar_fd, 0, SEEK_SET); //reset file descriptor pointer
    return result;
}

/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
int exists(int tar_fd, char *path) {
    if(path == NULL) return 0;
    char *buffer = (char *) malloc(sizeof(char)*512);
    if(!buffer) return EXIT_FAILURE;
    while(read(tar_fd, buffer, 512)){
        tar_header_t *header = (tar_header_t*) buffer;
        // if it exists
        if (strncmp(header->name, path, strlen(path) - 1) == 0){
            lseek(tar_fd, 0, SEEK_SET); // reset file descriptor pointer
            free(buffer); //garbage buffer
            return 1;  //  we found the directory
        }
        // if it is a simple file
        if( header->typeflag == REGTYPE ) {
            size_t size = TAR_INT(header->size); // get the size of this file
            if(size != 0) lseek (tar_fd, 512*(size/512 +1), SEEK_CUR);  // Go to next header
        }
    }
    free(buffer); //garbage buffer
    lseek(tar_fd, 0, SEEK_SET); // reset file descriptor pointer
    return 0; // not exists xor not valid archive
}

/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */
int is_dir(int tar_fd, char *path) {
    if(path == NULL) return 0;
    char *buffer = (char *) malloc(sizeof(char)*512);
    if(!buffer) return EXIT_FAILURE;
    while(read(tar_fd, buffer, 512)){
        tar_header_t *header = (tar_header_t*) buffer;
        // if it is the directory we search for
        if (strcmp(header->name, path) == 0 &&  header->typeflag == DIRTYPE){
            free(buffer); //garbage buffer
            lseek(tar_fd, 0, SEEK_SET); // reset file descriptor pointer
            return 1;  //  we found the directory
        }
        // if it is a simple file
        if( header->typeflag == REGTYPE ) {
            size_t size = TAR_INT(header->size); // get the size of this file
            if(size != 0) lseek (tar_fd, 512*(size/512 +1), SEEK_CUR);  // Go to next header
        }
    }
    free(buffer); //garbage buffer
    lseek(tar_fd, 0, SEEK_SET); // reset file descriptor pointer
    return 0; // not found xor was a file xor not a valid archive
}

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */
int is_file(int tar_fd, char *path){
    if(path == NULL) return 0;
    char *buffer = (char *) malloc(sizeof(char)*512);
    if(!buffer) return EXIT_FAILURE;
    while(read(tar_fd, buffer, 512)){
        tar_header_t *header = (tar_header_t*) buffer;
        // if it is the file we search for
        if (strcmp(header->name, path) == 0 && header->typeflag == REGTYPE){
            free(buffer); //garbage buffer
            lseek(tar_fd, 0, SEEK_SET); // reset file descriptor pointer
            return 1;  //  we found the directory
        }
        // if it is a simple file but not the one we search for
        if( header->typeflag == REGTYPE ) {
            size_t size = TAR_INT(header->size); // get the size of this file
            if(size != 0) lseek (tar_fd, 512*(size/512 +1), SEEK_CUR);  // Go to next header
        }
    }
    free(buffer); //garbage buffer
    lseek(tar_fd, 0, SEEK_SET); // reset file descriptor pointer
    return 0; // not found xor was a file xor not a valid archive
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */
int is_symlink(int tar_fd, char *path) {
    char *buffer = (char *) malloc(sizeof(char)*512);
    if(!buffer) return EXIT_FAILURE;
    while(read(tar_fd, buffer, 512)){
        tar_header_t *header = (tar_header_t*) buffer;
        // if it is the directory we search for
        //printf("H: -  %s *** P: -  %s *** T: -  %c\n", header->name, path, header->typeflag);
        if (strcmp(header->name, path) == 0 &&  header->typeflag == SYMTYPE){
            free(buffer); //garbage buffer
            lseek(tar_fd, 0, SEEK_SET); // reset file descriptor pointer
            return 1;  //  we found the directory
        }
        // if it is a simple file
        if( header->typeflag == REGTYPE ) {
            size_t size = TAR_INT(header->size); // get the size of this file
            if(size != 0) lseek (tar_fd, 512*(size/512 +1), SEEK_CUR);  // Go to next header
        }
    }
    free(buffer); //garbage buffer
    lseek(tar_fd, 0, SEEK_SET); // reset file descriptor pointer
    return 0; // not found xor was a file xor not a valid archive
}


/**
 * Lists the entries at a given path in the archive.
 * list() does not recurse into the directories listed at the given path.
 *
 * Example:
 *  dir/          list(..., "dir/", ...) lists "dir/a", "dir/b", "dir/c/" and "dir/e/"
 *   ├── a
 *   ├── b
 *   ├── c/
 *   │   └── d
 *   └── e/
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry path.
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entries in `entries`.
 *                   The callee set it to the number of entries listed.
 *
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */
int list(int tar_fd, char *path, char **entries, size_t *no_entries) {
    char *buffer = (char *) malloc(sizeof(char)*512);
    char *temp = (char*) malloc(sizeof(char)*100); // same length as the linkname or the name of a header
    if(!buffer || !temp) return EXIT_FAILURE;
    strcpy(temp, path);
    if( is_symlink(tar_fd, path) ){
        strcpy(temp, "nonononon"); // TODO DEBUG
    }
    if(!is_dir(tar_fd, temp)){
        free(buffer); free(temp); //garbage collection
        lseek(tar_fd, 0, SEEK_SET); //reset file descriptor pointer
        *no_entries = 0;
        return 0;
    }
    int entered = 0;
    while(read(tar_fd, buffer, 512) && entered < *no_entries){
        tar_header_t *header = (tar_header_t*)buffer;
        if(header->typeflag == REGTYPE && TAR_INT(header->size) != 0){
            lseek(tar_fd, 512*(TAR_INT(header->size)/512 +1), SEEK_CUR); // go to next header
        }
        if(path_helper(path, header->name) && not_in_entries(entries, header->name, entered)) {
            strcpy(entries[entered++], header->name);
        }
    }
    free(buffer); free(temp); //garbage collection
    lseek(tar_fd, 0, SEEK_SET);//reset file descriptor pointer
    *no_entries = entered;
    return 1;
}

/**
 * Search if headerPath is in the good path file and not in a other directory or subdirectory
 *
 * @param path
 * @param headerPath
 * @return 0 if ... 1 else
 */
int path_helper(char *path, char* headerPath){
    if(strncmp(path, headerPath, strlen(path)-1) != 0) return 0; // check first part of path
    for(int i = strlen(path); i < strlen(headerPath); i++) {
        if(headerPath[i] == '/' && i+1 < strlen(headerPath) && headerPath[i+1] != '\0') return 0;
    }
    return 1;
}

int not_in_entries(char** entries, char* path, int len){
    for(int i = 0; i < len; i++) if (strcmp(entries[i], path) == 0) return 0;
    return 1;
}

/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read to reach
 *         the end of the file.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len) {
    if(!exists(tar_fd, path)) return -1;
    char* buffer = (char*)malloc(sizeof(char)*512);
    if(!buffer) return EXIT_FAILURE;
    if(is_symlink(tar_fd, path)){
        while(read(tar_fd, buffer, 512)){
            tar_header_t *header = (tar_header_t*)buffer;
            if(strcmp(header->name, path) == 0 && header->typeflag == SYMTYPE) {
                *path = *header->linkname; // update path
                lseek(tar_fd, 0, SEEK_SET); // reset file descriptor pointer before recursion
                return read_file(tar_fd, path, offset, dest, len);
            }

            if (header->typeflag == REGTYPE && TAR_INT(header->size)) { //if it is a simple file with size >0
                int size = TAR_INT(header->size); // get size
                lseek(tar_fd, 512*(size/512 +1), SEEK_CUR); // move to next header
            }
        }
    }
    if(is_file(tar_fd, path)){
        while(read(tar_fd, buffer, 512)){
            tar_header_t *header = (tar_header_t*)buffer;
            if(strcmp(path, header->name) == 0 && header->typeflag == REGTYPE) break; // we are on the good header
            if (header->typeflag == REGTYPE && TAR_INT(header->size)) { //if it is file but not the good one
                int size = TAR_INT(header->size); // get size
                lseek(tar_fd, 512*(size/512 +1), SEEK_CUR); // move to next header
            }
        }
        tar_header_t *header = (tar_header_t*)buffer;
        if(offset >= TAR_INT(header->size)) return -2; // offset is outside of file length
        size_t temp = TAR_INT(header->size) - offset; // get the file size without the offset
        lseek(tar_fd, offset, SEEK_CUR); // move to after the offset
        *len = read(tar_fd, dest, temp > *len ? *len:temp); // read the file partially or in its entirety dependant of len
        lseek(tar_fd, 0, SEEK_SET); // reset file descriptor  pointer
        free(buffer); // garbage buffer
        return temp - *len; // return size stay to read
    }
    free(buffer);
    lseek(tar_fd, 0, SEEK_SET);
    return -1; //exclusive return error
}


/** @param: buffer A buffer which contains 512 characters including the header
 * @return the checksum excluding the header
 */
long checksum(char* buffer){
    long result = 0; // long to compare with TAR_INT without any conversion error
    for(int i = 0; i < 512; i++){
        if(i >= 148 && i <= 155) result += 32; // add 4 bytes (sizeof(int))
        else result = result + *(buffer + i); //exclude header checksum
    }
    return result;
}
