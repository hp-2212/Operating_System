#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

int historyCurr = -1 ;
char *historyArray[1000] ;

void printErr(int type) {
    if ( type == 1 ) printf("Error : Invalid Command \n") ;
    else if ( type == 2 ) printf("Error : Invalid Arguments \n") ;
}

void shellPipeRun(char *userArgs[], int word_count) {
    char *userArgs1[2048];
    char *userArgs2[2048];
    int word_count1 = 0, word_count2 = 0;
    //splitting user argument into two arguments separated by |
    for ( int i = 0 ; i < word_count; i++) {
        if (strcmp(userArgs[i], "|") == 0) {
            i++;
            while (i < word_count) {
                userArgs2[word_count2++] = userArgs[i++];
            }
            userArgs2[word_count2] = NULL ;
            break;
        }
        userArgs1[word_count1++] = userArgs[i];
    }
    userArgs1[word_count1] = NULL ;

    int pipeEnds[2];

    if (pipe(pipeEnds) < 0) {
        printf("Error: Couldn't Start Piping \n");
        return;
    }

    int child1 = fork();

    if (child1 < 0) {
        fprintf(stderr, "Fork Failed\n");
        return ;
    } else if (child1 == 0) {
        close(pipeEnds[0]);
        dup2(pipeEnds[1], STDOUT_FILENO);
        close(pipeEnds[1]);

        if (execvp(userArgs1[0], userArgs1) < 0) {
            printf("Could not execute first command ..\n");
            exit(0);
        }
    } else {
        int child2 = fork();

        if (child2 < 0) {
            fprintf(stderr, "Fork Failed\n");
            return ;
        }
        else if (child2 == 0) {
            close(pipeEnds[1]);
            dup2(pipeEnds[0], STDIN_FILENO);
            close(pipeEnds[0]);
            if (execvp(userArgs2[0], userArgs2) < 0) {
                printf("\nCould not execute second command ..\n");
                exit(0);
            }
        } else {
            close(pipeEnds[0]);
            close(pipeEnds[1]);
            wait(NULL);
            wait(NULL);
        }
    }
}

void shellRun(char *userArgs[], int word_count) {

    int rc = fork() ;

    if ( rc < 0 ) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if ( rc == 0 ) {

        if ( !strcmp(userArgs[0], "cd") ) {
            char currentDirectory[100];
            getcwd(currentDirectory, sizeof(currentDirectory)) ;

            strcat(currentDirectory, "/") ;
            strcat(currentDirectory, userArgs[1]) ;

            char *newDirectory = currentDirectory ;
            int result = chdir(newDirectory);
        }
        else if ( !strcmp(userArgs[0], "history") ) {
            if ( word_count < 3 ) {

                for (int i=0 ; i<2048 ; ++i ) {
                    if ( userArgs[1][i] == '\0' ) break ;
                    else if ( userArgs[1][i] < '0' || userArgs[1][i] > '9' ) {
                        printErr(2) ;
                        return ;
                    }
                }
                
                int n = atoi(userArgs[1]) ;
                int start = historyCurr-n+1 ; 
                if ( start < 0 ) start = 0 ;
                for ( int i = start ; i<= historyCurr ; ++i ) {
                    printf("  %d %s \n", i, historyArray[i]);
                }
            }
            else printErr(2) ;
        }
        else {
            if (  execvp(userArgs[0], userArgs) < 0 ) {
                printf("Error in User Command \n") ;
            }
        }
    }
    else wait(NULL) ;
}

int main (int argc, char *argv[]) {

    char input[2048] ;
    char *userArgs[2048] ;

    while (1) {
        char currentDirectory[100];
        getcwd(currentDirectory, sizeof(currentDirectory)) ;
        char cmdPrompt[100] = "MTL458" ; 
        strcat(currentDirectory, "$ ");
        strcat(cmdPrompt, currentDirectory);

        printf("%s", cmdPrompt);                //reading input string 
        fgets(input, sizeof(input), stdin) ;
        if (input[strlen(input) - 1] == '\n'){
            input[strlen(input) - 1] = '\0' ;
        }

        if (strcmp(input,"\0") == 0) continue ; //Case of no input

        //saving inputs into historyArray
        historyCurr = historyCurr + 1 ;
        historyArray[historyCurr] = strdup(input) ;

        int word_count = 0;
        bool valid = true ;
        bool piped = false;
        int pipeIdx = -1  ;

        const char delimiter[] = " " ;
        char *token = strtok(input, delimiter);
        while (token != NULL) {
            if (word_count < 2048) {
                userArgs[word_count] = token ;
                if (strcmp(token ,"|") == 0 ) {
                    piped = true ;
                    pipeIdx = word_count ;
                }
                word_count++ ;
            }
            else valid = false ;

            token = strtok(NULL, delimiter);
        }

        if (valid) {
            if (piped) shellPipeRun(userArgs, word_count) ;
            else shellRun(userArgs, word_count) ;
        }
        else printErr(1) ;

        // clearing userArgs after previous command finished
        for(int i=0 ; i<2048 ; ++i) userArgs[i] = NULL ;
    }
    return 0 ;
}