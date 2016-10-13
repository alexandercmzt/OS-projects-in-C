#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

/*
Alexander Chatron-Michaud
260611509
COMP310
This work is mine and mine alone.
Please contact me at achatr@cim.mcgill.ca if there are any issues in marking this.
*/

int hist = 0;
char historyList[512][64];
int errorlist[512];
char jobList[512][64];
pid_t jobpid[512];
int jobActiveList[512];
int stdout_copy;


/////////////////////////////////////////////////////////////////////////////////////////
//THIS METHOD FREES THE MEMORY IN THE BUFFERS USED TO GRAB THE COMMANDS
/////////////////////////////////////////////////////////////////////////////////////////

void freecmd(char * args[]) {
    int i;
    for (i = 0; i < 20; i++)
    {
        free(args[i]);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//THIS METHOD CHANGES THE FILE DESCRIPTOR TABLE FOR OUTPUT REDIRECTION
/////////////////////////////////////////////////////////////////////////////////////////

void setOutputFile(char *outputfile) {
    stdout_copy = dup(1);
    close(1);
    fopen(outputfile, "w");
}

/////////////////////////////////////////////////////////////////////////////////////////
//THIS METHOD RESETS THE FILE DESCRIPTOR TABLE 
/////////////////////////////////////////////////////////////////////////////////////////

void resetOutput(char *outputfile) {
    close(1);
    dup(stdout_copy);
    printf("Output written to %s\n", outputfile);
}

/////////////////////////////////////////////////////////////////////////////////////////
//THIS METHOD BRINGS A METHOD FROM THE JOBLIST TO THE FOREGROUND 
/////////////////////////////////////////////////////////////////////////////////////////

void foreground(char *number) {
    if (isdigit(*number)){
        pid_t *buf = (pid_t *)malloc(5*sizeof(pid_t *));
        pid_t *buf2 = (pid_t *)malloc(5*sizeof(pid_t *));
        if (jobActiveList[atoi(number)] == 1) {
                jobActiveList[atoi(number)] = 0;
                *buf2 = jobpid[atoi(number)];
                *buf = waitpid(*buf2, NULL, 0);
        }
        else {
            printf("A background job with that number could not be found. Please refer to the numbers on the left in the jobs list.\n");
        }
        free(buf);
        free(buf2);
    }
    else {
        printf("Incorrect usage of fg.\n");
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//THIS METHOD IMPLEMENTS THE PWD USING GETCWD
/////////////////////////////////////////////////////////////////////////////////////////

void getpwd () {
    char *buf;
    char *pwdptr;
    pwdptr = getcwd(buf, 1024);
    printf("Current working directory: %s\n", pwdptr);
    free(buf);
}

/////////////////////////////////////////////////////////////////////////////////////////
//THIS METHOD PRINTS THE JOBS IN THE JOBLIST BASED ON THEIR ACTICE STATUS AND PID 
/////////////////////////////////////////////////////////////////////////////////////////

void getJobs () {
    pid_t *buf = (pid_t *)malloc(5*sizeof(pid_t *));
    int i;
    for (i=1; i<=hist; i++) {
        if (jobActiveList[i] == 1) {
            if ((*buf = waitpid(jobpid[i], NULL, WNOHANG)) == jobpid[i]) {
                jobActiveList[i] = 0;
            }
            else {
                printf("[%d] %s", i, jobList[i]);
            }
        }
    }
    free(buf);
}

/////////////////////////////////////////////////////////////////////////////////////////
//THIS METHOD PARSES THE ALTARGS POINTER, A REFERENCED COMMAND FROM THE HISTORY LIST 
/////////////////////////////////////////////////////////////////////////////////////////

void showHistory() {
    printf("This is the history list. You may enter any of these numbers as commands to execute them again. If you reference an erroneous command, that reference call will not be entered in history (hence any number skips.) The most recent programs (up to 10 of them) are shown, including erroneous ones: \n\n");
    int i;
    for (i = 0; i < 10; i++)
    {
        if ((hist-i) == 0){
            break;
        }
        if (errorlist[hist-i] == 3) {
            printf("%d. ^ Reference made via history list to program above\n\n", hist-i);
            continue;
        }
        if (errorlist[hist-i] == 5) {
            continue;
        }
        printf("%d. %s\n", hist-i, historyList[hist-i]);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//THIS METHOD PRINTS VARIOUS ERRORS CODED BY NUMBERS GIVEN IN THE MAIN FUNCTION 
/////////////////////////////////////////////////////////////////////////////////////////

void errorPrint(int a) {
    if (a == 0) {
        printf("The given program from history was erroneous and hence was not executed.\n");
        errorlist[hist] = 5;
    }
    else if (a == 1) {
        printf("Attempts to reference a program that was also a reference from historyList (reference to references) were not described as required in the assignment.\n");
        errorlist[hist] = 5;
    }
    else if (a == 2) {
        printf("You're trying to execute a reference to a another reference to an erroneous program, or a reference to a reference to a reference. A little overboard, no? (program not executed.)\n");
        errorlist[hist] = 5;
    }
    else if (a == 3) {
        printf("A history command for that number could not be found.\n");
        errorlist[hist] = 1;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//THIS METHOD PARSES THE ARGS AND RDARGS POINTERS (IT HAS BEEN MODIFIED FROM THE ORIGINAL)
/////////////////////////////////////////////////////////////////////////////////////////

int getcmd(char *prompt, char *args[], int *background) {
    int length, i = 0;
    char *token, *loc;
    char *line;
    size_t linecap = 0;

    printf("%s", prompt);
    length = getline(&line, &linecap, stdin);

    if (length <= 0) {
        exit(-1);
    }

    hist++;
    strcpy(historyList[hist], line);
    strcpy(jobList[hist], line);
    jobActiveList[hist] = 0;
    errorlist[hist] = 2;

    // Check if background is specified..
    if ((loc = index(line, '&')) != NULL) {
        *background = 1;
        *loc = ' ';
    } else
    *background = 0;

    while ((token = strsep(&line, " \t\n")) != NULL) {
        int j;
        for (j = 0; j < strlen(token); j++)
            if (token[j] <= 32)
                token[j] = '\0';
            if (strlen(token) > 0)
                args[i++] = token;
    }
    args[i++] = (char *)'\0';
    return i;
}

/////////////////////////////////////////////////////////////////////////////////////////
//THIS METHOD PARSES THE ALTARGS POINTER, A REFERENCED COMMAND FROM THE HISTORY LIST 
/////////////////////////////////////////////////////////////////////////////////////////
int historypgrmParse(char *copy, char *args[], int *background) {
    int length, i = 0;
    char *token, *loc;
    char *line;
    size_t linecap = 0;

    strcpy(line,copy);

    hist++;
    strcpy(historyList[hist], line);
    strcpy(jobList[hist], line);
    jobActiveList[hist] = 0;
    errorlist[hist] = 2;

    // Check if background is specified..
    if ((loc = index(line, '&')) != NULL) {
        *background = 1;
        *loc = ' ';
    } else
    *background = 0;

    while ((token = strsep(&line, " \t\n")) != NULL) {
        int j;
        for (j = 0; j < strlen(token); j++)
            if (token[j] <= 32)
                token[j] = '\0';
            if (strlen(token) > 0)
                args[i++] = token;
    }
    args[i++] = (char *)'\0';
    return i;
}


int main() {
    char *args[20];
    int bg;
    int redirect;
    char *RDargs[20];
    char *outputfile;
    char *ALTargs[20];
    while (1) {
        int cnt = getcmd("\n>>  ", args, &bg);
        if (cnt == -1) {
            printf("Error reading program.\n");
            continue;
        }
        redirect = -1;
        int i;
        for(i =0; i< cnt-2; i++) {
            if (strcmp(args[i], ">") == 0) {
                redirect = i;
                break;
            }
        }
        /////////////////////////////////////////////////////////////////////////////////////////
        //NOTE: THE CODE FOLLOWING THIS POINT IS PARTITIONED INTO THREE PARTS:
        //1. COMMANDS WITH OUTPUT REDIRECTION (USES RDARGS)
        //2. HISTORY REFERENCED COMMANDS (USES ALTARGS)
        //3. NORMAL EXECUTION (USES ARGS)
        //REFER TO THE COMMENTS BELOW FOR THE SECTIONS
        /////////////////////////////////////////////////////////////////////////////////////////
        

        /////////////////////////////////////////////////////////////////////////////////////////
        //THE CODE FROM THIS POINT UNTIL THE NEXT COMMENT IS ONLY FOR OUTPUT REDIRECTION
        /////////////////////////////////////////////////////////////////////////////////////////
        if (redirect != -1) {
            int i;
            for (i = 0; i < redirect; i++) {
                RDargs[i] = args[i];
            }
            outputfile = args[redirect+1];
            setOutputFile(outputfile);
            if (RDargs[0] != NULL) {
                if (strcmp(RDargs[0], "cd") == 0 && RDargs[1] != NULL) {
                    chdir(RDargs[1]);
                } 
                else if (strcmp(RDargs[0], "history") == 0) {
                    showHistory();
                }
                else if (strcmp(RDargs[0], "jobs") == 0) {
                    getJobs();
                } 
                else if (strcmp(RDargs[0], "fg") == 0) {
                    if (RDargs[1] != NULL) {
                        foreground(RDargs[1]);
                    }
                    else printf("Incorrect usage of fg\n");
                } 
                else if (strcmp(RDargs[0], "exit") == 0) {
                    exit(1);
                }
                else if (strcmp(RDargs[0], "pwd") == 0) {
                    getpwd();
                }
                else {
                    errorlist[hist] = 1;
                    int status;
                    pid_t pid = fork();
                    if (pid == 0) {
                        execvp(RDargs[0], RDargs);
                        printf("There was an error executing the program.\n");
                        exit(1);
                    }
                    else if (pid<0) {
                        printf("There was an error while forking the process.\n");
                        exit(1);
                    }
                    else {
                        errorlist[hist] = 2;
                        if (bg == 0) {
                            int a = 
                            waitpid(pid,&status,0);
                            if (WEXITSTATUS(status) == 1) {
                                errorlist[hist] = 1;
                            }
                        }
                        else {
                            jobActiveList[hist] = 1;
                            jobpid[hist] = pid;
                        }
                    }
                }
                resetOutput(outputfile);
                continue;
            }   
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        //THE CODE FROM THIS POINT UNTIL THE NEXT COMMENT IS ONLY FOR HISTORY REFERENCED COMMANDS
        /////////////////////////////////////////////////////////////////////////////////////////

        if (historyList[atoi(args[0])] != NULL && atoi(args[0]) < hist) {
            if (errorlist[atoi(args[0])] == 1) {
                errorPrint(0);
                continue;
            }
            else if (errorlist[atoi(args[0])] == 3) {
                errorPrint(1);
                continue;
            }
            else if (errorlist[atoi(args[0])] == 5) {
                errorPrint(2);
                continue;
            }
            else if (errorlist[atoi(args[0])] == 2) {
                errorlist[hist] = 3;
                char *args2 = historyList[atoi(args[0])];
                cnt = historypgrmParse(args2, ALTargs, &bg);
                if (args[0] != NULL) {
                    if (strcmp(ALTargs[0], "cd") == 0 && ALTargs[1] != NULL) {
                        chdir(ALTargs[1]);
                    } 
                    else if (strcmp(ALTargs[0], "history") == 0) {
                        showHistory();
                    }
                    else if (strcmp(ALTargs[0], "jobs") == 0) {
                        getJobs();
                    } 
                    else if (strcmp(ALTargs[0], "fg") == 0) {
                        if (ALTargs[1] != NULL) {
                            foreground(ALTargs[1]);
                        }
                        else printf("Incorrect usage of fg\n");
                    } 
                    else if (strcmp(ALTargs[0], "exit") == 0) {
                        exit(1);
                    }
                    else if (strcmp(ALTargs[0], "pwd") == 0) {
                        getpwd();
                    }
                    else {
                        errorlist[hist] = 1;
                        int status;
                        pid_t pid = fork();
                        if (pid == 0) {
                            execvp(ALTargs[0], ALTargs);
                            printf("There was an error executing the program.\n");
                            exit(1);
                        }
                        else if (pid<0) {
                            printf("There was an error while forking the process.\n");
                            exit(1);
                        }
                        else {
                            errorlist[hist] = 2;
                            if (bg == 0) {
                                int a = 
                                waitpid(pid,&status,0);
                                if (WEXITSTATUS(status) == 1) {
                                    errorlist[hist] = 1;
                                }
                            }
                            else {
                                jobActiveList[hist] = 1;
                                jobpid[hist] = pid;
                            }
                        }
                    }
                    continue;
                }
            }
        }
        else {
            errorPrint(3);
            continue;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
        //THE CODE FFROM THIS POINT ONWARD IS THE NORMAL COMMAND EXECUTION
        /////////////////////////////////////////////////////////////////////////////////////////

        if (args[0] != NULL) {
            if (strcmp(args[0], "cd") == 0 && args[1] != NULL) {
                chdir(args[1]);
            } 
            else if (strcmp(args[0], "history") == 0) {
                showHistory();
            }
            else if (strcmp(args[0], "jobs") == 0) {
                getJobs();
            } 
            else if (strcmp(args[0], "fg") == 0) {
                if (args[1] != NULL) {
                    foreground(args[1]);
                }
                else printf("Incorrect usage of fg\n");
            } 
            else if (strcmp(args[0], "exit") == 0) {
                exit(1);
            }
            else if (strcmp(args[0], "pwd") == 0) {
                getpwd();
            }
            else {
                errorlist[hist] = 1;
                int status;
                pid_t pid = fork();
                if (pid == 0) {
                    execvp(args[0], args);
                    printf("There was an error executing the program.\n");
                    exit(1);
                }
                else if (pid<0) {
                    printf("There was an error while forking the process.\n");
                    exit(1);
                }
                else {
                    errorlist[hist] = 2;
                    if (bg == 0) {
                        int a = 
                        waitpid(pid,&status,0);
                        if (WEXITSTATUS(status) == 1) {
                            errorlist[hist] = 1;
                        }
                    }
                    else {
                        jobActiveList[hist] = 1;
                        jobpid[hist] = pid;
                    }
                }
            }
        }
    }
}

