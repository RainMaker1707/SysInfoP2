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
    char *buffer = (char*)malloc(512);
    if(!buffer) return EXIT_FAILURE;
    while(read(tar_fd, buffer, 512)){
        if (*buffer == AREGTYPE) break; //end of archive/file '\0'
        tar_header_t *header = (tar_header_t*) buffer;
        // hardcoded value to test if strcmp don't work
        if(header->magic[0] != 'u' || header->magic[1] != 's' || header->magic[2] != 't' ||
                header->magic[3] != 'a' ||  header->magic[4] != 'r' || header->magic[5] != '\0')  result = -1;
        //if(strncmp(header->magic, "ustar", TMAGLEN) != 0) result = -1; // magic value is not ustar
        else if(strncmp(header->version, TVERSION, TVERSLEN) != 0) result = -2;   // version value is not 00
        else if(TAR_INT(header->chksum) != checksum(buffer)) result = -3;// invalid checksum-> dangerous file
        if (result < 0) break; // if it is invalid archive
        if(header->typeflag == REGTYPE ){
            size_t size = TAR_INT(header->size);
            if(size != 0) lseek(tar_fd, 512+size, SEEK_CUR);
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
    char *buffer = (char *) malloc(512);
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
    char *buffer = (char *) malloc(512);
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
    char *buffer = (char *) malloc(512);
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
    char *buffer = (char *) malloc(512);
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
    char *buffer = malloc(512), *temp = malloc(100);
    if(!buffer || !temp) return EXIT_FAILURE;
    tar_header_t *header = (tar_header_t*) buffer;
    strcpy(temp, path);
    if(is_symlink(tar_fd, path)){
        while(read(tar_fd, buffer, 512)) {
            header = (tar_header_t *) buffer; // set header to next one
            if (strcmp(temp, header->linkname)) { // if we find the symlink
                strcat(temp, (const char *) '/'); // add '/' after the directory path // TODO possibly buggy
                lseek(tar_fd, 0, SEEK_SET); // reset the file descriptor pointer
                break;
            }
            if (header->typeflag == REGTYPE && TAR_INT(header->size)) { //if it is a simple file
                int size = TAR_INT(header->size); // get size
                lseek(tar_fd, 512*(size/512 +1), SEEK_CUR); // move to next header
            }
        }
    }
    if(!is_dir(tar_fd, temp)){ // if no directory att the given path exists -> 0
        free(temp); free(buffer); //garbage collection
        return 0; // not a directory or no directory exists at the given path
    }
    size_t entries_length = *no_entries; // set a temporary variable
    *no_entries = 0; // callee resetting the no_entries value
    while(read(tar_fd, buffer, 512) && entries_length >= *no_entries){
        header = (tar_header_t*) buffer;
        if(header->typeflag == REGTYPE && TAR_INT(header->size)) {
            int size = TAR_INT(header->size);
            lseek(tar_fd, 512 + size, SEEK_CUR);
        }
        char *copy = (char*) malloc(100);
        strcpy(copy, temp); // copy because strtok modify his first argument
        char* token = strtok(copy, "/"); // split string with delim="/"
        printf("%s  ----  %s \n", temp, token);
        while(token) { // iterated trough split parts until NULL
            // TODO
            token = strtok(NULL, "/"); // next split part
        }
    }
    free(buffer); free(temp); // garbage collection
    return 1; // successfully listed
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
    if(!is_file(tar_fd, path) && !is_symlink(tar_fd, path)) return -1; // no entry at given path or is not a file

    char* buffer = (char*)malloc(512);
    if(!buffer) return EXIT_FAILURE;
    tar_header_t *header;
    ssize_t size = 0;

    if(is_symlink(tar_fd, path)){ // it's a file but it's a symlink
        read(tar_fd, buffer, 512); // copy header in buffer
        path = ((tar_header_t*)buffer)->linkname; //reset path to the pointed link
    }
    lseek(tar_fd, 0, SEEK_SET); // reset pointer to the beginning of the file descriptor
    while(read(tar_fd, buffer, 512)){ //while pointer is in file allocated memory, step 512 bytes
        header = (tar_header_t*) buffer;
        if(strcmp(header->name, path) == 0 && header->typeflag == REGTYPE) break; // we point on the file
        if(header->typeflag == REGTYPE){
            size = TAR_INT(header->size);
            if(size != 0) lseek(tar_fd, 512*(size/512 +1), SEEK_CUR);  // go to next header
        }
    }

    int point;
    header = (tar_header_t*)buffer;
    if (offset > TAR_INT(header->size)) return -2;
    if(size - offset > *len) point = *len;
    else point = size - offset;
    *len = read(tar_fd, dest, point);
    free(buffer);
    lseek(tar_fd, 0, SEEK_SET);
    return size - *len - offset;
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
