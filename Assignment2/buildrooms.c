/*************************************************************
** Branden Holloway
** CS344, Program 2
** July 21, 2019
** buildrooms.c
*************************************************************/

#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stddef.h>

//Below Holds array of Room names and types
char* RoomList[10] = {"Study", "Library", "Billiard", "Kitchen", "BallRoom", "Dining", "Cellar", "Attic", "BedRoom", "BathRoom"};
char* RoomType[3] = {"START_ROOM", "MID_ROOM", "END_ROOM"};
char FileName[256];

//Struct to hold the data for each Room
struct Room{
    char* name;
    char* type;
    int connection[6];
    int NumConnects;
};

//Function prototype
void AddConnect();

/*************************************************************
 * CreateRoom(struct)
 * Creates each room by giving it a name, type, number of connections and calling
 * AddConnect to connect random rooms together
*************************************************************/
void CreateRoom(struct Room GameRoom[]){

    srand(time(NULL));                                           //Needed to use rand() 
    int RoomNum[10];                                             //Creates array of ints to choose rooms from below
    int i, j, k, c;

//SETS AN ARRAY 0-9, THEN MIXES LIST UP TO SET ROOMS RANDOMLLY DOWN BELOW    
    for(i = 0; i < 10; i++)
        RoomNum[i] = i;                                          //Populates Room Array
    for(j = 0; j < 10; j++){                                     //Loops through and mixes up the RoomNum array
        int temp = RoomNum[j];
        int RandomRoom = rand() % 10;
        RoomNum[j] = RoomNum[RandomRoom];
        RoomNum[RandomRoom] = temp;
    }

//SETS ROOM NAME
    k = 0;
    while(k < 7){                                                //Loops through the 7 rooms
        int x = RoomNum[k];
        GameRoom[k].name = RoomList[x];                          //Populates name varibale in GameRoom struct
        GameRoom[k].NumConnects = 0;                             //Sets GameRoom struct NumConnects variable to 0
        k++;
    }

//SETS ROOM TYPE
    for(c = 0; c < 7; c++){                                      //Loops through the 7 rooms
        if(c == 0)
            GameRoom[c].type = RoomType[0];                      //First Random Room will always be Start
        else if(c == 6)
            GameRoom[c].type = RoomType[2];                      //Last Random Room will always be End
        else
            GameRoom[c].type = RoomType[1];                      //Set all Rooms in between to Mid
    }

//SETS NUMBER OF ROOM CONNECTIONS
    for(i = 0; i < 7; i++){
        int TotalConnects = rand() % 3 + 2;                      //Choose random number between 3-6
        while(GameRoom[i].NumConnects < TotalConnects){          //Calls AddConnect() for number of Connects that were chose for Room
            AddConnect(GameRoom, i);
        }   
    }
}

/*************************************************************
 * AddConnect(struct, int)
 * Adds connections between rooms
*************************************************************/
void AddConnect(struct Room GameRoom[], int i){

    int j, RoomToConnect, Invalid;
    Invalid = 1;

    while(Invalid){
        Invalid = 0;
        RoomToConnect = rand() % 7;                                 //Gets random room to connect to 
        if(RoomToConnect == i){                                     //Check not same room
            Invalid = 1;                                            //Swap flag to 1 if same room
        }
        for(j = 0; j < GameRoom[i].NumConnects; j++){               //Loop through for number of connections
            if(GameRoom[i].connection[j] == RoomToConnect){
                Invalid = 1;
            }
        }
    }
                                                                    
    GameRoom[i].connection[GameRoom[i].NumConnects] = RoomToConnect;    //Set of instructions sets Connection between rooms, be sure they go both ways 
    GameRoom[i].NumConnects = (GameRoom[i].NumConnects + 1);
    GameRoom[RoomToConnect].connection[GameRoom[RoomToConnect].NumConnects] = i;
    GameRoom[RoomToConnect].NumConnects = (GameRoom[RoomToConnect].NumConnects + 1);

}

/*************************************************************
 * WriteFile(struct, char*)
 * Writes the information of rooms into a directory
*************************************************************/
void WriteFile(struct Room GameRoom[], char* RoomDirect){

    chdir(RoomDirect);
    int i, j;
    for(i = 0; i < 7; i++){
        FILE* RoomFile = fopen(GameRoom[i].name, "a");                  //Opens a Room File with Rooms Name
        fprintf(RoomFile, "ROOM NAME: %s\n", GameRoom[i].name);         //Prints Rooms Name into file
        for(j = 0; j < GameRoom[i].NumConnects; j++){
            fprintf(RoomFile, "CONNECTION %d: %s\n", (j + 1), GameRoom[GameRoom[i].connection[j]].name); //Adds all connection to file
        }
        fprintf(RoomFile, "ROOM TYPE: %s", GameRoom[i].type);           //Adds rooms type to file
        fclose(RoomFile);                                               //Closes file
    }       
}

/*************************************************************
 * PrintRoom(struct)
 * Prints room data, used as a debug tool during program contruction
*************************************************************/
void PrintRoom(struct Room GameRoom[]){
//Prints Room out, Function was jsut used as a debug method before impliments portion that writes to file
    int i = 0, j;
    for(i = 0; i < 7; i++){
        printf("Room Name: %s\n", GameRoom[i].name);
        printf("Room Type: %s\n", GameRoom[i].type);
        printf("Number of Connects: %d\n", GameRoom[i].NumConnects);
        for(j = 0; j < GameRoom[i].NumConnects; j++){
            printf("Connection %d: %s: ", j, GameRoom[GameRoom[i].connection[j]].name);

        }
        printf("\n");
    }
}

/*************************************************************
 * BuildRoomDirect()
 * Constructs the directory that the room files will be stored in
*************************************************************/
char* BuildRoomDirect(){
    
    char* DirectName = "hollowab.rooms.";                   //Sets up Directories name as specified in instrucitons
    int PID = getpid();                                     //Gets a Random PID for directory
    int Permiss = 0777;                                     //Gives Directory full permissions
    sprintf(FileName, "%s%d", DirectName, PID);             //Puts all info into FileName
    mkdir(FileName, Permiss);                               //Finally makes directory with name and PID and permissoin

return FileName;
}

/*************************************************************
 * main()
 * Starts the program, calling the proper functions
*************************************************************/
int main(){

    struct Room GameRoom[7];                                  //Creates array of struct Room
    char* RoomDirect = BuildRoomDirect();                     //Builds Room directory and stores in RoomDirect
    CreateRoom(GameRoom);                                     //Creates GameRoom and puts into GameRoom array
//  PrintRoom(GameRoom);                                      //Used for debug
    WriteFile(GameRoom, RoomDirect);                          //Writes infor the hollowab.rooms.PID directory

return 0;
}