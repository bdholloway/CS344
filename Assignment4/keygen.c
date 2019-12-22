/***********************************
* Branden Holloway
* Program 4 OTP
* keygen.c
************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/***********************************
* int main(int, char *)
* Runs Program
************************************/
int main(int argc, char * argv[]){
	//Seed randome number generator
	srand(time(NULL));
	//If number of arguments is not 2, send error to stderr
	if(argc != 2){
		fprintf(stderr, "Incorrect number of arguments");
		exit(1);
	}
	//Get the length of key from argv[1] passed in from command line
	int KeyLen = atoi(argv[1]);
	//needed to run random number generator
	int i;
	//Loopgenerates keys
	for(i = 0; i < KeyLen; i++){
		//RandNum gets a random number
		int RandNum = rand() % 27;
		//Enter statement if not a 26
		if(RandNum != 26){
			printf("%c", 'A'+(char)RandNum);
		}
		else{
			//If RandNum is 26, then add space
			printf(" ");
		}
	}
	//need to print out newline
	printf("\n");
	return 0;
}