#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_FILE_SIZE 1024
#define MAX_INPUT_LENGTH 100
#define MAX_COMMAND_Count 100
#define MAX_FILENAME_LENGTH 100

char* result[MAX_COMMAND_Count] ;

typedef struct {
    char *filenameArgs ;
    int commandIdx ;
} readerArgs ;

typedef struct {
    char *filenameArgs1 ;
    char *filenameArgs2 ;
    int commandIdx ;
    int queryType ;
} writerArgs ;

typedef struct {
    int id;
    char name[256];  // Adjust the size according to your needs.
    sem_t semaphore;
} FileEntry;

sem_t read_lock;
sem_t write_lock;
sem_t read_try_lock;
sem_t write_try_lock;
int read_count = 0;
int write_count = 0;

char asd_file[MAX_FILE_SIZE];

char* read_file(const char *file_name) {
    // sem_wait(&read_try_lock);
    // sem_wait(&read_lock);
    read_count++;
    // if (read_count == 1) {
    //     sem_wait(&write_lock);
    // }
    // sem_post(&read_lock);
    // sem_post(&read_try_lock);

    FILE *file = fopen(file_name, "r");
    char* resultString = (char*)malloc(100 * sizeof(char));
    if (file) {
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);

        char *file_data = (char *)malloc(file_size + 1);
        if (file_data) {
            fread(file_data, 1, file_size, file);
            file_data[file_size] = '\0';

            char str[100];
            sprintf(str, "Read %s of %ld bytes with %d readers and %d writers present\n",
                file_name, file_size, read_count, 0);
            strcat( resultString, str) ;

            free(file_data);
        }
        fclose(file);
    }

    // sem_wait(&read_lock);
    read_count--;
    // if (read_count == 0) {
    //     sem_post(&write_lock);
    // }
    // sem_post(&read_lock);
    return resultString;
}

void *reader_thread(void *arg) {
    readerArgs *temp = (readerArgs *)arg;
    result[temp->commandIdx] = read_file(temp->filenameArgs);
    // printf("%d = command number\n", temp->commandIdx) ;
}

char* write1(const char *filename1, const char *filename2) {
    // sem_wait(&read_try_lock);
    // sem_wait(&read_lock);
    write_count++;
    // if (read_count == 1) {
    //     sem_wait(&write_lock);
    // }
    // sem_post(&read_lock);
    // sem_post(&read_try_lock);

    FILE *file1 = fopen(filename1, "a");
    FILE *file2 = fopen(filename2, "r");

    if (file1 == NULL) {
        printf("Error: Cannot open file '%s' for appending.\n", filename1);
    } else if (file2 == NULL) {
        printf("Error: Cannot open file '%s' for reading.\n", filename2);
        fclose(file1);
    } else {
        fprintf(file1, "\n") ;
        char ch;
        while ((ch = fgetc(file2)) != EOF) {
            if (ch == '\n') {
                fputc('\n', file1);
            } else {
                fputc(ch, file1);
            }
        }

        // printf("Content appended from '%s' to '%s' with new lines.\n", filename2, filename1);

        fclose(file1);
        fclose(file2);
    }

    FILE *file = fopen(filename1, "r");
    char* resultString = (char*)malloc(100 * sizeof(char));
    if (file) {
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);

        char *file_data = (char *)malloc(file_size + 1);
        if (file_data) {
            fread(file_data, 1, file_size, file);
            file_data[file_size] = '\0';

            char str[100];
            sprintf(str, "Writing to %s added %ld bytes with %d readers and %d writers present\n",
                filename1, file_size, read_count, write_count);
            strcat( resultString, str) ;

            free(file_data);
        }
        fclose(file);
    }

    // sem_wait(&read_try_lock);
    // sem_wait(&read_lock);
    write_count--;
    // if (read_count == 1) {
    //     sem_wait(&write_lock);
    // }
    // sem_post(&read_lock);
    // sem_post(&read_try_lock);
    
    return resultString ;
}

char* write2(const char *filename, const char *content) {
    write_count++ ;
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        printf("Error: Cannot open file '%s' for appending.\n", filename);
    } else {
        fprintf(file, "%s", content);
        fclose(file);
        // printf("Content appended to '%s'.\n", filename);
    }
    file = fopen(filename, "r");
    char* resultString = (char*)malloc(100 * sizeof(char));
    if (file) {
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);

        char *file_data = (char *)malloc(file_size + 1);
        if (file_data) {
            fread(file_data, 1, file_size, file);
            file_data[file_size] = '\0';

            char str[100];
            sprintf(str, "Writing to %s added %ld bytes with %d readers and %d writers present\n",
                filename, file_size, read_count, write_count);
            strcat( resultString, str) ;

            free(file_data);
        }
        fclose(file);
    }
    write_count-- ;
    return resultString ;
}

void *writer_thread(void *arg) {
    writerArgs *temp = (writerArgs *)arg;
    if ( temp->queryType == 1 ) result[temp->commandIdx] = write1(temp->filenameArgs1, temp->filenameArgs2);
    else result[temp->commandIdx] = write2(temp->filenameArgs1, temp->filenameArgs2); ;
    // printf("%d = command number\n", temp->commandIdx) ;
}

int countFilesInDirectory(const char *path) {
    int count = 0;
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            count++;
        }
    }

    closedir(dir);
    return count;
}

FileEntry* initializeFileEntries(const char *path, int *count) {
    int numFiles = countFilesInDirectory(path);
    if (numFiles < 0) return NULL;

    *count = numFiles;
    FileEntry* files = (FileEntry *)malloc(numFiles * sizeof(FileEntry));
    
    if (files == NULL) {
        perror("malloc");
        return NULL;
    }

    DIR *dir = opendir(path);
    struct dirent *entry;
    int fileId = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            files[fileId].id = fileId;
            strcpy(files[fileId].name, entry->d_name);
            sem_init(&files[fileId].semaphore, 0, 1);
            fileId++;
        }
    }

    closedir(dir);
    return files;
}

int getFileId(const char* filename, FileEntry* files,int filecount){
    for(int i=0; i<filecount; i++){
        if(strcmp(filename, files[i].name)==0){
            return files[i].id;
        }
    }
    return -1;
}

int main() {
    char directoryPath[PATH_MAX];
    if (getcwd(directoryPath, sizeof(directoryPath)) == NULL) {
        perror("getcwd");
        return 1; 
    }

    // printf("Current directory path: %s\n", directoryPath);

    int fileCount;
    FileEntry *fileEntries = initializeFileEntries(directoryPath, &fileCount);

    // if (fileEntries != NULL) {
    //     printf("Number of files in the directory: %d\n", fileCount);
    //     printf("Name of the first file: %s\n", fileEntries[0].name);
    //     free(fileEntries);
    // } 
    // else printf("Error initializing file entries.\n");

    int rwCommand[MAX_COMMAND_Count], queryCommand[MAX_COMMAND_Count], p1Command[MAX_COMMAND_Count] ;
    char* p2Command[MAX_COMMAND_Count] ;
    int commandCount = 0 ;
    char input[MAX_INPUT_LENGTH];
    //Input
    while (1) {
        // printf("Enter a command: ");
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        int len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        if (strcmp(input, "exit") == 0) {
            // printf("Exiting the program.\n");
            break;
        } 
        else if (strncmp(input, "read", 4) == 0) {
            char filename[MAX_FILENAME_LENGTH];
            if (sscanf(input, "read %s", filename) == 1) {
                // printf("Read command input with file name: %s\n", filename);
                rwCommand[commandCount] = 0 ;
                p1Command[commandCount] = getFileId( filename, fileEntries, fileCount) ;
            } else {
                printf("Invalid 'read' command\n");
            }
        } 
        else if (strncmp(input, "write 1", 7) == 0) {
            char filename1[MAX_FILENAME_LENGTH];
            char filename2[MAX_FILENAME_LENGTH];
            if (sscanf(input, "write 1 %s %s", filename1, filename2) == 2) {
                // printf("Write 1 command input with filenames: %s %s\n", filename1, filename2);
                rwCommand[commandCount] = 1 ;
                queryCommand[commandCount] = 1 ;
                p1Command[commandCount] = getFileId( filename1, fileEntries, fileCount) ;
                p2Command[commandCount] = filename2 ;
            } else {
                printf("Invalid 'write 1' command.\n");
            }
        } 
        else if (strncmp(input, "write 2", 7) == 0) {
            char filename[MAX_FILENAME_LENGTH];
            char content[MAX_INPUT_LENGTH];
            if (sscanf(input, "write 2 %s %[^\n]", filename, content) == 2) {
                // printf("Write 2 command input with filename: %s and text: %s\n", filename, content);
                rwCommand[commandCount] = 1 ;
                queryCommand[commandCount] = 2 ;
                p1Command[commandCount] = getFileId( filename, fileEntries, fileCount) ;
                p2Command[commandCount] = content ;
            } else {
                printf("Invalid 'write 2' command.\n");
            }
        } 
        else {
            --commandCount ;
            printf("Invalid command.\n");
        }
        ++commandCount ;
    }
    
    pthread_t threads[commandCount] ;
    for( int i=0 ; i<commandCount ; ++i ) {
        if ( rwCommand[i] == 0 ) {
            int sz = strlen(fileEntries[p1Command[i]].name) + 1 ;
            char *fname = (char*) malloc(sizeof(char)*(sz)) ;
            strcpy( fname, fileEntries[p1Command[i]].name) ;
            readerArgs *temp = (readerArgs*) malloc(sizeof(readerArgs)) ;
            temp->filenameArgs = fname ;
            temp->commandIdx = i ;
             
            pthread_create(&threads[i], NULL, reader_thread, temp);
        }
        else {
            for ( int j=0 ; j<i ; ++j ) pthread_join(threads[j], NULL);
            
            int sz1 = strlen(fileEntries[p1Command[i]].name) + 1, sz2 = strlen(p2Command[i]) + 1 ;
            char *fname1 = (char*) malloc(sizeof(char)*(sz1)) ;
            char *fname2 = (char*) malloc(sizeof(char)*(sz2)) ;
            strcpy( fname1, fileEntries[p1Command[i]].name) ;
            strcpy( fname2, p2Command[i]) ;
            writerArgs *temp = (writerArgs*) malloc(sizeof(writerArgs)) ;
            temp->filenameArgs1 = fname1 ;
            temp->filenameArgs2 = fname2 ;
            temp->commandIdx = i ;
            temp->queryType = queryCommand[i] ;
            
            pthread_create(&threads[i], NULL, writer_thread, temp);
        }
    }

    for( int i=0 ; i<commandCount ; ++i ) {
        pthread_join(threads[i], NULL) ;
        printf("%s", (result[i])) ;
    }

    return 0;
}