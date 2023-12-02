#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_INPUT_LENGTH 1024
#define MAX_PATH_SIZE 1024

int directoryType = -1 ; // 0 -> file, 1 -> Folder, 2 -> Link
bool directoryFound = false ;

//Recursive function to search for a folder or file in the folder with path 'folderPath'
void search_directory(char* directoryName, char* folderPath) {
    DIR *currentDirectoryStream;
    struct dirent *currentEntry;
    
    //Open the directory stream
    currentDirectoryStream = opendir(folderPath);
    if (currentDirectoryStream == NULL) {
        printf("Couldn't Open the directory at %s", folderPath);
        return ;
    }
    //Iterate through entries in the stream
    while ((currentEntry = readdir(currentDirectoryStream)) != NULL) {
        //Skip '.' and '..' entries
        if ( strcmp(currentEntry->d_name, ".") == 0 || strcmp(currentEntry->d_name, "..") == 0 ) continue ;

        //if the entry is a directory
        if ( currentEntry->d_type == DT_DIR ) {
            //recursively search within this directory with 'newFolderPath' as the path
            char newFolderPath[MAX_PATH_SIZE];
            strcpy(newFolderPath, folderPath);
            strcat(newFolderPath, "/");
            strcat(newFolderPath, currentEntry->d_name);

            search_directory( directoryName, newFolderPath) ;
            directoryType = 1 ;
        }
        //if the entry is a file
        else if ( currentEntry->d_type == DT_REG ) directoryType = 0 ;
        //else link
        else if ( currentEntry->d_type == DT_LNK ) directoryType = 2 ;

        //Check if the current entry matches the target
        if ( strcmp( directoryName, currentEntry->d_name) == 0 ) {
            if ( directoryType == 0 ){
                printf("File Found : %s/%s\n", folderPath, directoryName) ;
            }
            else if ( directoryType == 1 ) {
                printf("Directory Found : %s/%s\n", folderPath, directoryName) ;
            }
            else if ( directoryType == 2 ) {
                printf("Link Found : %s/%s\n", folderPath, directoryName) ;
            }

            directoryFound = true ;
        }
    }

    directoryType = -1 ; //Reset directoryType for next iteration
    closedir(currentDirectoryStream);
    return ;
}

int main(int argc, char* argv[]) {
    //Invalid input handling
    if (argc != 2) {
        printf("Error: Invalid arguments (More than 1 arguments found)\n");
        return 1;
    }
    
    char* input = argv[1];
    char *currPath ;
    // Dynamically allocate memory to store the current working directory path
    size_t size = pathconf(".", _PC_PATH_MAX);
    if ((currPath = (char *)malloc((size_t)size)) != NULL) {
        //Get the current working directory path
        if (getcwd(currPath, size) != NULL) ;
        else {
            printf("Error: Couldn't retrive Current Working Directory\n");
            return 0 ;
        }
    } 
    else {
        printf("Error: Malloc memmory Allocation failed\n");
        return 0 ;
    }

    // Call the recursive function with directory name and
    // current working directory as currPath.
    search_directory( input, currPath) ;
    if ( !directoryFound ) {
        printf("The target \'%s\' was not found in the current directory.\n", input) ;
    }
    free(currPath);
    return 0 ;
}