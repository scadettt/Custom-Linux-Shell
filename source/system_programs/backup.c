#include "system_program.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

void get_datetime(char *buffer, size_t buffer_size) {
    //Get current time as seconds since epoch and return to now variable
    time_t now = time(NULL);

    //converts now into local time and returns a pointer to it
    struct tm *t = localtime(&now);

    //formats the time into YYYYMMDDHHMMSS, stores it in char array buffer
    strftime(buffer, buffer_size, "%Y%m%d%H%M%S", t);
}

void zip_funct(const char *source, const char *destination) {
    //calculate the size of the command string
    size_t command_size = strlen("zip -r ") + strlen(destination) + strlen(source) + 4; //include for quotations

    //allocate require memory space for storing the command string
    char *command = (char *)malloc(command_size);
    if (!command) {
        printf("Error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    //form the full command string
    snprintf(command, command_size, "zip -r '%s' %s", destination, source);
    //printf("command: %s\n", command);

    //execute command and return exit status into result
    int result = system(command);

    //release memory allocated to prevent memory leaks
    free(command);

    //check if execution fails
    if (result != 0) {
        printf("Error: Failed to zip directory or file.\n");
        exit(EXIT_FAILURE);
    }
}

void move_file(const char *source, const char *destination) {
    if (rename(source, destination) != 0) {
        perror("Error: Failed to move file");
        exit(EXIT_FAILURE);
    }
}

int main() {
    //variables
    struct stat path_stat;
    char datetime[64];

    //get the date and time formatted into datetime variable 
    get_datetime(datetime, sizeof(datetime));

    //retrieve directory to zip to, set in .cseshellrc file
    const char *project_dir = getenv("PROJECT_DIR");
    if (!project_dir) {
        fprintf(stderr, "Error: PROJECT_DIR environment variable not set.\n");
        exit(EXIT_FAILURE);
    }
    
    //retrive directory or file to zip
    const char *backup_dir = getenv("BACKUP_DIR");
    if (!backup_dir) {
        printf("Error: BACKUP_DIR environment variable not set.\n");
        exit(EXIT_FAILURE);
    }

    //retrieve backup_dir into struct stat which can be referenced by path_stat
    if (stat(backup_dir, &path_stat) != 0) {
        perror("Error accessing BACKUP_DIR");
        exit(EXIT_FAILURE);
    }
    //check if it is a dir
    int is_directory = S_ISDIR(path_stat.st_mode);

    //Calculte size of path string to zip to and allocate require memory for it
    size_t archive_dir_size = strlen(project_dir) + strlen("/archive") + 1;
    char *archive_dir = (char *)malloc(archive_dir_size);
    if (!archive_dir) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    //formatting the path string
    snprintf(archive_dir, archive_dir_size, "%s/archive", project_dir);
    //printf("archive: %s\n", archive_dir);
    
    // Create archive directory if it doesn't exist
    struct stat st = {0};
    if (stat(archive_dir, &st) == -1) {
        if (mkdir(archive_dir, 0700) != 0) {
            printf("Error creating archive directory");
            free(archive_dir);
            exit(EXIT_FAILURE);
        }
    }

    //Create zip file name and allocate memory for it
    size_t zip_filename_size = strlen("backup") + strlen(datetime) + strlen("_.zip") + 1;
    char *zip_filename = (char *)malloc(zip_filename_size);
    if (!zip_filename) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        free(archive_dir);
        exit(EXIT_FAILURE);
    }
    snprintf(zip_filename, zip_filename_size, "%s_%s.zip", "backup", datetime);

    //create zipfile path string andallocate memory to it
    size_t zip_filepath_size = strlen(archive_dir) + strlen(zip_filename) + 2; //addition 1 for the "/"
    char *zip_filepath = (char *)malloc(zip_filepath_size);
    if (!zip_filepath) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        free(archive_dir);
        free(zip_filename);
        exit(EXIT_FAILURE);
    }
    snprintf(zip_filepath, zip_filepath_size, "%s/%s", archive_dir,zip_filename);
    // printf("zipfilepath: %s\n", zip_filepath);
    // printf("backupdir: %s\n", backup_dir);

    //perform zip
    zip_funct(backup_dir, zip_filepath);
    
    printf("Backup completed successfully: %s\n", zip_filepath);

    //free up the space
    free(archive_dir);
    free(zip_filename);
    free(zip_filepath);

    return 0;
}