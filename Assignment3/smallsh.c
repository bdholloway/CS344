/*********************************************
** Branden Holloway
** CS344, Program 3, smallsh
** August 5th, 2019
**********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>                     
#include <unistd.h>

//Variabels to be declared
char UserInput[400];
char* argument[400];
char* InputFile = NULL;  
char* OutputFile = NULL;
//Structs from signal.h, will be used below
//Initiallizing them and setting to empty 
struct sigaction Cmd = {0};
struct sigaction Cmd2= {0};

/*********************************************
** StatusDisplay(int)
** Displays exit value or terminated signal
** 
**********************************************/
void StatusDisplay(int StatValue){
    //WIFEXITED() game from lecture;
    //Evaluates to a non-zero value if status was returned for a child process that terminated normally.
    //https://pubs.opengroup.org/onlinepubs/9699919799/functions/wait.html
    //If terminates normally, display exit value
    if(WIFEXITED(StatValue))          
        printf("exit value %i\n", WEXITSTATUS(StatValue)); 
    //If terminated by signal, display which signal
    else
        printf("terminated by signal %i\n", StatValue);
}

/*********************************************
** SignalCode(int)
** Catches and displays message for signo
** 
**********************************************/
void SignalCode(int signo){
	char* message = "\nCaught SIGINT.\n";
	write(STDOUT_FILENO, message, 38);
}

/*********************************************
** SignalStop(int)
** Catches and displays message from signal
** 
**********************************************/
void SignalStop(int signal){
	char* message = "\nCaught SIGTSTP.\n";
	write(STDOUT_FILENO, message, 25);
	exit(0);
}

/*********************************************
** main()
** Runs program and calls functions when appropriate
** 
**********************************************/
int main(){            
    int status = 0; 
    //Taken from lecture notes, sets up ctrl+z and ctrl+c usage
    //sa_handler and sa_flags are in struct sigaction coming from signal.h
    //https://linux.die.net/man/2/sigaction
    Cmd.sa_handler = SIG_IGN;            
    Cmd.sa_flags = 0;    
    //https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxbd00/rtsigf.htm
    sigfillset(&(Cmd.sa_mask));
    sigaction(SIGINT, &Cmd, NULL);
    
    //Doing the same as just above except on Cmd2 and signalstop
    Cmd2.sa_handler = SignalStop;
    Cmd2.sa_flags = 0;
    sigfillset(&(Cmd2.sa_mask));
    //used to change the action taken by a process on receipt of a specific signal.
    sigaction(SIGTSTP, &Cmd2, NULL);
    
    


    while(1){
        int fgProcess = 1;      
        
        //Shell prompt
        printf(": "); 
        //Clearing output buffer      
        fflush(stdout);     


        //gets to the end, if NULL exit
        if(fgets(UserInput, 400, stdin) == NULL) 
            return 0;

        int itr = 0;
        int cpid;      
        //makes its way through and processes the user input  
        //strtok() parses string using delimiter. Places it in token variable          
        char *token = strtok(UserInput, " \n");     
        while(token != NULL){
            //This checks if its an input file
            if(strcmp(token, "<") == 0){                
                token = strtok(NULL, " \n"); 
                // A copy of source is created dynamically and pointer to copy is returned. Using strdup() function
                //https://www.geeksforgeeks.org/strdup-strdndup-functions-c/
                InputFile = strdup(token);
                token = strtok(NULL, " \n");
            } 
            //This checks if its an output file
            else if(strcmp(token, ">") == 0){     
                token = strtok(NULL, " \n");
                //Does what happend in previous loop, putting result in token variable
                OutputFile = strdup(token);
                token = strtok(NULL, " \n");
            } 
            //This is a background process
            else if(strcmp(token, "&") == 0){
                fgProcess = 0;
                break;
            } 
            else{
                //Save pointer to argument array which ever position itr is at
                argument[itr] = strdup(token);   
                token = strtok(NULL, " \n");
                //incriments itr
                ++itr;                           
            }
        }
        //set argument to NULL at position of itr
        argument[itr] = NULL;
        int fileIn = -1;          
        int fileOut = -1;      
        //Checks if argument has a '#' symbol to start with in the 0 position
        //If true, continue on past and do nothing with information following pound sign
        if(argument[0] == NULL || *(argument[0]) == '#'){            
            continue;
        }

        //"cd" signifies change directory
        else if(strcmp(argument[0], "cd") == 0){  
            //If no argument, change to HOME directory          
            if(argument[1] == NULL) 
                //Change directory to HOME                      
                chdir(getenv("HOME"));
            else
            //If argument, change directory to said directory given
                chdir(argument[1]);
        } 
        //if "status", print the status
        else if(strcmp(argument[0], "status") == 0)    
            //Calls function StatusDisplay() with the current status   
            StatusDisplay(status); 
        //if "exit", then exit the shell
        else if (strcmp(argument[0], "exit") == 0)
            //exit with value of 0
            exit(0); 
        else{
            //https://www.geeksforgeeks.org/fork-system-call/
            cpid = fork();
            switch(cpid){
            //Fork a new process
            //Run switch case
            case 0:    
                //can interupt foreground                
                if(fgProcess){          
                    Cmd.sa_handler = SIG_DFL;
                    //used to change the action taken by a process on receipt of a specific signal.
                    sigaction(SIGINT, &Cmd, NULL);
                }     
                //If input file is not null, enter loop           
                if(InputFile != NULL){                 
                    fileIn = open(InputFile, O_RDONLY);
                    if(fileIn == -1){
                        //If there is an issue with opening and reading file, let user know
                        printf("smallsh: cannot open %s for input\n", InputFile);
                        //Clearing output buffer
                        fflush(stdout);
                        //Exit with a value of 1
                        _Exit(1);
                    }
                    if(dup2(fileIn, 0) == -1){
                        //prints a descriptive error message to stderr.
                        perror("dup2");
                        //Exit with a value of 1
                        _Exit(1);
                    }
                    //Always close file when finished
                    close(fileIn);
                } 
                else if(!fgProcess){
                    //Redirect to null device to ignore information
                    fileIn = open("/dev/null", O_RDONLY);       
                    if(fileIn == -1){
                        perror("open");
                        //Exit with a value of 1
                        _Exit(1);
                    }
                    //system call creates a copy of a file descriptor, using the lowest-numbered unused file descriptor.
                    if(dup2(fileIn, 0) == -1){
                        perror("dup2");
                        //Exit with a value of 1
                        _Exit(1);
                    }
                }
                //If output file is not null, enter loop
                if(OutputFile != NULL){  
                    //open file into variable fileout                       
                    fileOut = open(OutputFile, O_WRONLY | O_CREAT | O_TRUNC, 0744);
                    //if fileout is -1, enter loop
                    if(fileOut == -1) {
                        //If output file cannot be opened, let user know
                        printf("smallsh: cannot open %s\n", OutputFile);
                        //Clearing output buffer
                        fflush(stdout);
                        //Exit with a value of 1
                        _Exit(1);
                    }
                    //system call is similar to dup(), uses descriptor specified 
                    if(dup2(fileOut, 1) == -1){
                        //prints a descriptive error message to stderr.
                        perror("dup2");
                        //Exit with a value of 1
                        _Exit(1);
                    }
                    //Closes file when done
                    close(fileOut);
                }
                //The command given was not recongnized, let user know and change exit status
                if(execvp(argument[0], argument)){ 
                    //Prints out to user that the given command was not valid        
                    printf("smallsh: Command \"%s\" is not valid\n", argument[0]);
                    //Clearing output buffer
                    fflush(stdout);
                    //Exit with a value of 1
                    _Exit(1);
                }
                break;
            //If -1, enter this switch case 
            case -1:   
                //prints a descriptive error message to stderr. 
                perror("fork");
                status = 1;
                break;
            //Default switch case
            default:    
                if (fgProcess) 
                    //Wait for foreground to finish                       
                    waitpid(cpid, &status, 0);
                else{       
                    //Display background PID             
                    printf("Background PID: %i\n", cpid);
                    break;
                }
            }

        }
        //Cleans up argument array
        for(int i = 0; argument[i] != NULL; i++){
            free(argument[i]);
        }
        
        //Free input/output files and set them back to NULL
        free(InputFile);            
        InputFile = NULL;
        free(OutputFile);
        OutputFile = NULL;

        //Check if any process has completed;
        cpid = waitpid(-1, &status, WNOHANG);           
        while(cpid > 0){
            //When done, print out which process (cpid) and that it is done
            printf("background process, %i, is done: ", cpid);
            //Call StatusDisplay with the status
            StatusDisplay(status);
            cpid = waitpid(-1, &status, WNOHANG);
        }
    }
    return 0;
}