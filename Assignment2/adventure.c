/**************************************
* Branden Holloway
* CS344, Program 2
* 22 July, 2019
* adventure.c
**************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>             //https://en.wikibooks.org/wiki/C_Programming/POSIX_Reference/dirent.h
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>


char* RoomList[10] = {"Study", "Library", "Billiard", "Kitchen", "BallRoom", "Dining", "Cellar", "Attic", "BedRoom", "BathRoom"};   //Same room names and room types
char* RoomType[3] = {"START_ROOM", "END_ROOM", "MID_ROOM"};
pthread_mutex_t mutex;                                  //mutex reference for below Thread Function

//Function prototypes that were needed to seize warnings by compiler.
void PrintTime();
void P_threadTwo();

//Stuct to house the data of Room, same as used in buildrooms.c
struct Room{            
    char* name;
    char* type;
    int connection[6];
    int NumConnects;    
};


/*************************************************************
 * GetDirect()
 * Function gets directory that was most recently made with the hollowab.rooms.
 * 
*************************************************************/
char* GetDirect(){
  time_t t;                                  //data type that gets system time            
  int i = 0;
  char* buff = malloc(sizeof(char) * 64);
  DIR* Direct = opendir(".");               //Creates Directory Direct and opens it
  struct dirent* dirp;                      //A struct with member d_name that will be used below                   
  struct stat statbuff;
  
  if(Direct != NULL){                       //Checks if Direct is NULL, if it isn't, it enters loop
    while(dirp = readdir(Direct)){
      if(stat(dirp->d_name, &statbuff) == 0 && S_ISDIR(statbuff.st_mode) && strncmp(dirp->d_name, "hollowab.rooms.", 12)==0) {
        t = statbuff.st_mtime;
        if(t > i){
          strcpy(buff, dirp->d_name);           
          i = t;
        }
      }
    }
    closedir(Direct);                        //Close the directory Direct
  }
  return buff;                               //Return buff to use as directory
}

/*************************************************************
 * StartRoom()
 * Will call GetDirect() to get newest directory will hollowab.rooms. name and find the 
 * room that should be the starting point
*************************************************************/
char* StartRoom(){                        
  char* directory = GetDirect();                                          //Calls GetDirect() and stores in Directory variable
  char* start = malloc(sizeof(char) * 20);                                //
  
  DIR* dir;                                 
  struct dirent* loc;
  
  if((dir = opendir(directory)) != NULL){                                 //Checks if directory is NULL, if not enter loop
    while((loc = readdir(dir)) != NULL){
      if(!(strcmp(loc->d_name, "..")) || !(strcmp(loc->d_name, "."))){    
        continue;
      }
      strcpy(start, loc->d_name);                                         //Copy loc->d_name into start variable
          return start;                                                   //return start to be used below in Play() Function
      printf("%s", loc->d_name);
      break;
    }
    closedir(dir);                                                        //Close directory dir           
    free(dir);                                                            //Free Directory  
  }
}

/*************************************************************
 * GetRoom()
 * Gets Room information
*************************************************************/
int GetRoom(struct Room* GameRoom, char roomName[256]){        
  char File[256];
  sprintf(File, GetDirect());                             
  char FileName[256];
  sprintf(FileName, "./%s/%s", File, roomName);           
  
  FILE* myFile;                                             //Gets FILE myFile
  myFile = fopen(FileName, "r");                            //Opens myFile
  if(myFile == NULL){                                       //If myFile is empty, Exit program
    return -1;
  }
  
  GameRoom->name = RoomList[0];           
  GameRoom->NumConnects = 0;           
  char eachLine[256];                  
  fgets(eachLine, 256, myFile);                             //Gets line from myFile and puts into eachLine Variable
  
  char* name = strtok(eachLine, " ");                       //Breaks up the eachline string into tokens seperated by space dilim.
  name = strtok(NULL, " ");                                   
  name = strtok(NULL, "\n");            
  
  int i;
  for(i = 0; i < 10; i++){              
    if(strcmp(name, RoomList[i]) == 0){                     //Compares name with RoomList at the i index, if == 0, place into GameRoom name variable
      GameRoom->name = RoomList[i];
      break;
    }
  }
  
  while(fgets(eachLine, 256, myFile) != NULL){              //Gets line from myFile and put sinto eachLine variable
    char* Locator = strtok(eachLine, " ");                 
    if(strcmp(Locator, "CONNECTION") == 0){                 //Locator Variable looks for a match with CONNECTION, 
      Locator = strtok(NULL, " ");                          //When connection is found, breaks into this look
      Locator = strtok(NULL, "\n");
      for(i = 0; i < 10; i++){                  
        if(strcmp(Locator, RoomList[i]) == 0){
          GameRoom->connection[GameRoom->NumConnects] = i;
          break;
        }
      }
      GameRoom->NumConnects++;                 
    }
    
    else{
      Locator = strtok(NULL, " ");             
      Locator = strtok(NULL, "\n");
      for(i = 0; i < 10; i++){
        if(strcmp(Locator, RoomType[i]) == 0){
          GameRoom->type = RoomType[i];
          break;
        }
      }
    }
  }
  return 0;                                    
}

/*************************************************************
 * checkConnection()
 * Checks the connection betweent two rooms
*************************************************************/
int checkConnection(struct Room* GameRoom, char RoomName[50]){
  int i;
  for(i = 0; i < GameRoom->NumConnects; i++){                      
    if(strcmp(RoomName, RoomList[GameRoom->connection[i]]) == 0){   //Checks the RoomName and the Connection
      return 1;                                                     //If a connection, return 1, otherwise return 0
    }
  }
  return 0;
}

/*************************************************************
 * Play()
 * Plays the Game
*************************************************************/
void Play(char start[50]){          
  int counter = 0, GameOver = 0, Temp;                        

  char RoomName[100];                   
  char* PathTaken[500];                                             //Will hold users full path to print out at end
  struct Room* GameRoom = malloc(sizeof(struct Room));
  Temp = GetRoom(GameRoom, start);                                  //Calls GetRoom to get a room
  
  do{
    printf("\nCURRENT LOCATION: %s\n", GameRoom->name);             //Print game info
    printf("POSSIBLE CONNECTIONS:");
    printf(" %s,", RoomList[GameRoom->connection[0]]);              //Gets number of connections for room
    for(Temp = 1; Temp < GameRoom->NumConnects; Temp++){   
      printf(" %s,", RoomList[GameRoom->connection[Temp]]);         //For the # of Connections, loops through and prints all possible connections to screen
    }
    printf(". \nWHERE TO? >");                                      //Asks user for where to go, next line takes the input in
    scanf("%s", RoomName);             
    
    if(strcmp(RoomName, "time") == 0){                              //Checks if the user is asking for the time
        P_threadTwo();
        PrintTime();
    }
    else if(checkConnection(GameRoom, RoomName) == 0){              //If input is not time, check the connection to make sure its valid
      printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");  //If input is invalid, output error message
    }
    else{                                   
      PathTaken[counter] = GameRoom->name;                          //Adds the Room name to the PathTaken Array to keep track of full path
      counter++;                                                    //Incriment counter to keep track of # of moves user has used
      Temp = GetRoom(GameRoom, RoomName);      
    }
    

    if(GameRoom->type == "END_ROOM"){                               //Checks to see if the room type is == to "END_ROOM",      
      GameOver = 1;                                                 //if true, change GameOver to true(1).
    } 
  }while(GameOver != 1);                
  int i;
  printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");            
  printf("YOU TOOK %i STEPS. YOUR PATH TO VICTORY WAS:\n", counter);
  for(i = 0; i < counter; i++){          
    printf("%s\n", PathTaken[i]);                                   //Print out the total path the user took room by room
  }
  free(GameRoom);                                                   //Frees GameRoom to avoid memory leaks
}


/*************************************************************
 * WTimeFile()
 * Write the current day and time to the a File
*************************************************************/
void* WTimeFile(){
    //Referenced https://www.geeksforgeeks.org/strftime-function-in-c/ 
    //after being told in Program 2 instructions, directing us to use strftime().
    FILE* TFile = fopen("CurrentTime.txt", "w+");                                 //Opening/Creating CurrentTime.txt
    struct tm *CurrTime;
    char buff[100];
    time_t t;
    time(&t);
    CurrTime = localtime(&t);
    strftime(buff, sizeof(buff), "%I:%M%P, %A, %B %d, %Y", CurrTime); 
    fprintf(TFile, "%s\n", buff);                                                 //Printing to CurrentTime.txt
    fclose(TFile);                                                                //Closing CurrentTime.txt File    

return TFile;
}

/*************************************************************
 * PrintTime()
 * Print the time out to the screen
*************************************************************/
void PrintTime(){
    FILE* TFile = fopen("CurrentTime.txt", "r");                                  //Opening CurrentTime.txt File to be used below
    char buff[256];
    while(fgets(buff, 256, TFile) != NULL){
        printf("%s\n", buff);                                                     //Gets data from CurrentTime.txt
    }
    fclose(TFile);                                                                //Closing CurrentTime.txt File
}

/*************************************************************
 * P_threadTwo()
 * Starts the second thread for the time aspect of program
*************************************************************/
void P_threadTwo(){
  //Referenced Class Lecture Videos
    pthread_t thr2;
    int thrID;
  //https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_init.html
  //https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_lock.html
    pthread_mutex_init(&mutex, NULL);                                             //Initialises mutex referencing mutex that was set above, NULL is used to call default mutex attributes
    pthread_mutex_lock(&mutex);                                                   //Locks mutex object
    thrID = pthread_create(&thr2, NULL, WTimeFile, NULL);                         //Creates the new thread,Null is called so use of default attributes
    pthread_mutex_unlock(&mutex);                                                 //Unlocks mutex object
    pthread_mutex_destroy(&mutex);                                                //Destroys mutex object refereced above
}

/*************************************************************
 * main()
 * Begins adventure program
*************************************************************/
int main(){

    WTimeFile();                                              //Must call incase user checks time first, Creates and populates CurrentTime.txt
    char Start[256];                                          //Create holder for the beginning room
    strcpy(Start, StartRoom());                               //Get Room from StartRoom() and copy into Start
    Play(Start);                                              //Call play() and pass into it the beginning room

  return 0;
}