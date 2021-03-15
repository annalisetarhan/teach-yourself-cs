//
//  Files.c
//  OSTEP
//
//  Created by Annalise Tarhan on 3/13/21.
//

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

/*
 Question 1
 
 Adding an empty file to a directory increases the directory's
 size by 32 bytes and increases the number of references by one.
 */

void my_stat(char *file_name) {
    struct stat info;
    
    if (stat(file_name, &info) == -1) {
        printf("Failed to print %s\n", file_name);
        perror("Stat failure: ");
        exit(EXIT_FAILURE);
    }
    
    printf("File name:    %s\n", file_name);
    printf("File type:    ");
    
    // Shamelessly borrowed from man7.org/linux/man-pages/man2/lstat.2.html
    switch (info.st_mode & S_IFMT) {
        case S_IFBLK:  printf("block device\n");            break;
        case S_IFCHR:  printf("character device\n");        break;
        case S_IFDIR:  printf("directory\n");               break;
        case S_IFIFO:  printf("FIFO/pipe\n");               break;
        case S_IFLNK:  printf("symlink\n");                 break;
        case S_IFREG:  printf("regular file\n");            break;
        case S_IFSOCK: printf("socket\n");                  break;
        default:       printf("unknown?\n");                break;
    }
    
    // And from gist.github.com/jcamino/8969205
    printf("Permissions:  ");
    printf( (S_ISDIR(info.st_mode)) ? "d" : "-");
    printf( (info.st_mode & S_IRUSR) ? "r" : "-");
    printf( (info.st_mode & S_IWUSR) ? "w" : "-");
    printf( (info.st_mode & S_IXUSR) ? "x" : "-");
    printf( (info.st_mode & S_IRGRP) ? "r" : "-");
    printf( (info.st_mode & S_IWGRP) ? "w" : "-");
    printf( (info.st_mode & S_IXGRP) ? "x" : "-");
    printf( (info.st_mode & S_IROTH) ? "r" : "-");
    printf( (info.st_mode & S_IWOTH) ? "w" : "-");
    printf( (info.st_mode & S_IXOTH) ? "x" : "-");
    printf("\n");
    
    printf("Size:         %lli\n", info.st_size);
    printf("Blocks:       %lli\n", info.st_blocks);
    printf("Block size:   %i\n", info.st_blksize);
    printf("References:   %i\n", info.st_nlink);
    printf("Owner:        %u\n", info.st_uid);
    printf("Group:        %u\n", info.st_gid);
    printf("Last access:  %s\n", ctime(&info.st_atime));
    
}

/*
 Question 2
 
 Use show_details instead of the -l flag
 (1 for more info, 0 for default)
 */

void my_ls(char *dir_name, int show_details) {
    
    if (dir_name == NULL) {
        dir_name = getcwd(NULL, MAXPATHLEN);
        printf("%s\n", dir_name);
    }
    
    DIR *dir_stream = opendir(dir_name);
    
    if (dir_stream == NULL) {
        perror("opendir failure");
        exit(EXIT_FAILURE);
    }
    
    struct dirent *next_entry = readdir(dir_stream);
    char *full_path = malloc(MAXPATHLEN);
    while (next_entry != NULL) {
        if (show_details) {
            
            /* Make fully qualified path name */
            char *file_name = next_entry->d_name;
            char *slash = "/";
            memset(full_path, 0x00, MAXPATHLEN);
            strcat(full_path, dir_name);
            strcat(full_path, slash);
            strcat(full_path, file_name);
            
            my_stat(full_path);
            printf("\n");
        } else {
            printf("%s\n", next_entry->d_name);
        }
        next_entry = readdir(dir_stream);
    }
    free(full_path);
}

/*
 Question 3
 
 The question asks for a certain number of lines to be read and printed, but
 I couldn't find a standard line size to use, or support for reading a single
 line in any of the given interfaces. I chose 100 bytes. It's arbitrary. And
 cuts 'lines' mid-word. (And could cause problems for multi-byte characters.)
 */

void tail(char *file_name, int num_lines) {
    int fd = open(file_name, O_RDONLY);
    
    if (fd < 0) {
        printf("File could not be opened");
        exit(EXIT_FAILURE);
    }
    
    // Get file size
    struct stat file_info;
    int rc = stat(file_name, &file_info);
    
    if (rc < 0) {
        perror("Couldn't get file info");
        exit(EXIT_FAILURE);
    }
    
    long long file_size = file_info.st_size;
    
    // Calculate total number of bytes to read
    int line_size = 100;
    int bytes_to_read = line_size * num_lines;
    
    // Only seek to the end of the file if we're not going to read the whole file anyway
    if (file_size > bytes_to_read) {
        
        // Seek to end of file
        off_t new_offset = lseek(fd, 0, SEEK_END);
        
        if (new_offset < 0) {
            perror("Couldn't seek to end of file");
            exit(EXIT_FAILURE);
        }
        
        // Reposition num_lines earlier
        int i;
        int neg_line_size = line_size * -1;     // Moving backwards, so offset should be negative
        for (i = 0; i < num_lines; i++) {
            new_offset = lseek(fd, neg_line_size, SEEK_CUR);
            if (new_offset < 0) {
                perror("Error using lseek");
                exit(EXIT_FAILURE);
            }
        }
    }
    
    // Print each line until end of file
    char buf[line_size];
    for (int i = 0; i < num_lines; i++) {
        read(fd, buf, line_size);
        write(STDOUT_FILENO, buf, line_size);
    }
    printf("\n\n");
    close(fd);
}

/*
 Question 4
 */

void rc_search(char *start_dir, int nested_count) {
    
    // Print directory name
    for (int i = 0; i < nested_count; i++) {
        printf("   ");
    }
    printf("%s\n", start_dir);

    // Open directory
    DIR *dir_stream = opendir(start_dir);
    
    if (dir_stream == NULL) {
        printf("Couldn't open %s\n", start_dir);
        perror("");
        return;
    }
    
    struct dirent *next_entry;
    struct stat entry_info;
    char *entry_name;
    
    char *full_path = malloc(MAXPATHLEN);

    // Iterate through directory's files
    while ((next_entry = readdir(dir_stream)) != NULL) {
        entry_name = next_entry->d_name;
        
        // Skip . and .. files
        if (!strcmp(entry_name, ".") || !strcmp(entry_name, "..")) {
            continue;
        }
        
        // Make fully qualified path name
        char *slash = "/";
        memset(full_path, 0x00, MAXPATHLEN);
        strcat(full_path, start_dir);
        strcat(full_path, slash);
        strcat(full_path, entry_name);

        if (stat(full_path, &entry_info) == -1) {
            printf("Problem with %s\n", entry_name);
            perror("");
            continue;
        }
        
        // Recursively search child directory
        if ((entry_info.st_mode & S_IFMT) == S_IFDIR) {
            rc_search(full_path, nested_count+1);
            
        // Print file name
        } else {
            for (int i = 0; i < nested_count; i++) {
                printf("   ");
            }
            printf("   %s\n", entry_name);
        }
    }
    free(full_path);
}

void recursive_search(char *start_dir) {
    
    // By default, use current working directory
    if (start_dir == NULL) {
        start_dir = getcwd(NULL, MAXPATHLEN);
    }
    
    rc_search(start_dir, 0);
}

