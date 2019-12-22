/***********************************
* Branden Holloway
* Program 4 OTP
* otp_enc.c
************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

//Helpful Websites used; Also used alot of information from CS372 Projects I made(sockets etc.)
//http://man7.org/linux/man-pages/man7/ip.7.html
//http://man7.org/linux/man-pages/man3/bzero.3.html
//https://www.geeksforgeeks.org/fork-system-call/
//https://www.geeksforgeeks.org/socket-programming-cc/
//https://beej.us/guide/bgc/

/*******************************************************************************
 * CreateAddress(char)
 * Creates address
 ******************************************************************************/
struct addrinfo * CreateAddress(char * port){
	int status;
	struct addrinfo hints;
	struct addrinfo * res;
	//Clear hints for size of hints
	memset(&hints, 0, sizeof hints);
    //These variables come from imported library 
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
    //If status comes back as incorrect enter statement and print to stderr, then exit
	if((status = getaddrinfo(NULL, port, &hints, &res)) != 0){
		fprintf(stderr, "getaddrinfo error: %s\nDid you enter the correct IP/Port?\n", gai_strerror(status));
		exit(1);
	}
	//return struct res
	return res;
}

/*******************************************************************************
 * int CreateSocket(struct addrinfo *)
 * Takes in res struct from above
 ******************************************************************************/
int CreateSocket(struct addrinfo * res){
	int sockfd;
    //Enter statemetn if sockfd is equal to 1, print error to stderr, then exit
	if((sockfd = socket((struct addrinfo *)(res)->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
		fprintf(stderr, "Error in creating socket\n");
		exit(1);
	}
    //return sockfd int
	return sockfd;
}

/*******************************************************************************
 * ConnectSocket(int, struct)
 * calls connect socket
 ******************************************************************************/
void ConnectSocket(int sockfd, struct addrinfo * res){
	int status;
    //if == to -1 then enter loop, print error to stderr and exit
	if ((status = connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1){
		fprintf(stderr, "Error: Can't connect socket\n");
		exit(1);
	}
}

/*******************************************************************************
 * SendFile(int, int)
 * Sends file
 ******************************************************************************/
void SendFile(int fd, int sockfd) {
	//track bytes read and wrote
	int BytesRead;
	int BytesWrote;
	//sending and recieving buffer
	char buffer[1024];
	//loop send file
	while(1) {
		//reads from file
		BytesRead = read(fd, buffer, sizeof(buffer));
        //if BytesRead is 0, means done enter and clsoe fd
		if(BytesRead == 0) {
			//when done, close fd
			close(fd);
			break;
		}
		
        int i;
        //Loop through sending packets till finished
		for(i = 0; i < BytesRead; i += BytesWrote) {
			BytesWrote = write(sockfd, buffer + i, BytesRead - i);
            //if less than 0, enter statement, send msg to stderr and exit
			if (BytesWrote < 0) {
				fprintf(stderr, "Error writing to socket\n");
				exit(1);
			}
		}
	}
    //erase buffer fo rsize of buffer
	memset(buffer, '\0', sizeof(buffer));
	BytesRead = read(sockfd, buffer, sizeof(buffer));
}

/*******************************************************************************
 * int HandShake(int)
 *
 ******************************************************************************/
int HandShake(int sockfd){
	char * identity = "opt_enc";
	send(sockfd, identity, strlen(identity),0);
	//get the response from the server
	char buffer[100];
    //erases buffer for size of buffer
	memset(buffer, 0, sizeof(buffer));
    //receive
	recv(sockfd, buffer, sizeof(buffer),0);
	int valid = 0;
	//compares buffer to valid, if same enter loop
	if(strcmp(buffer, "Valid") == 0){
		valid = 1;
	}
	return valid;
}

/*******************************************************************************
 * char * RecvFile(int, int)
 * receives file and returns string
 ******************************************************************************/
char * RecvFile(int new_fd, int MsgLen){
	//allocate a receive buffer and read variables
	char * to_receive = malloc(MsgLen * sizeof(char));
	int BytesRead = 0;
	int i = 0;
	//Loop to begin receiving the file
	for(i = 0; i< MsgLen; i+= BytesRead){
		BytesRead = read(new_fd, to_receive + i, MsgLen -i);
        //If bytesread is less than 0, enter loop and send error to stderr, then exit
		if(BytesRead < 0){
			fprintf(stderr, "Error in receiving file\n");
			_Exit(2);
		}
	}
	// echo finished response
	char * finished = "opt_enc_d f";
	send(new_fd, finished, strlen(finished),0);
	return to_receive;
}

/*******************************************************************************
 * FileCheck(int)
 * Chekcs file for invalid chars
 ******************************************************************************/
int FileCheck(int fd){
	//allocate variabel for buffer
	char buffer[100];
    //erase buffer for size of buffer
	memset(buffer, 0, sizeof(buffer));
	//Check for valid caharacters
	while(read(fd, buffer, 1) != 0){
        //if contains invalid chars, enter statemtnk send msg to stderr, then exit
		if(((buffer[0] < 'A' || buffer[0] > 'Z') && buffer[0] != ' ') && buffer[0] != '\n'){
			fprintf(stderr, "File contains invalid characters\n");
			exit(1);
		}
	}
	//return length
	return lseek(fd, 0, SEEK_END);
}


/*******************************************************************************
 * HandleRequest(int, char, char)
 * Handles requests
 ******************************************************************************/
void HandleRequest(int sockfd, char * filename, char * keyname){
	//varify identity
	int valid = HandShake(sockfd);
    //If not valid, enter statemnt, send msg to stderr then exit
	if(!valid){
		fprintf(stderr,"Client not accepted\n");
		exit(1);
	}
	//Open files
	int FileFD = open(filename, O_RDONLY);
	int KeyFD = open(keyname, O_RDONLY);
    //if KeyFD less than one, enter statemnt send msg to stderr then exit
	if (KeyFD < 1){
		fprintf(stderr, "Error opening key\n");
		exit(1);
	}
    //if FileFD less than one, enter statemnt send msg to stderr then exit
	if (FileFD < 1){
		fprintf(stderr, "Error opening file\n");
		exit(1);
	}
	//Call FileCheck on both files
	int FileLen = FileCheck(FileFD);
	int KeyLen = FileCheck(KeyFD);
    //if FileLen is larger than KeyLen, key is to short
    //send msg to stderr and exit
	if(FileLen > KeyLen){
		fprintf(stderr, "Error: Key is too short\n");
		exit(1);
	}
    //Close both files
	close(FileFD);
	close(KeyFD);
	// tell the daemon the size of the strings
	char file_length_s[20];
    //erase contents
	memset(file_length_s, 0, sizeof(file_length_s));
	char key_length_s[20];
    //erase contents
	memset(key_length_s, 0, sizeof(key_length_s));
	sprintf(file_length_s, "%d", FileLen);
	sprintf(key_length_s, "%d", KeyLen);
	//Send length of file
	send(sockfd, file_length_s, strlen(file_length_s), 0);
	recv(sockfd, file_length_s, sizeof(file_length_s), 0);
	//Send length of key
	send(sockfd, key_length_s, strlen(key_length_s), 0);
	recv(sockfd, key_length_s, sizeof(key_length_s), 0);
	// send them
	int filefd = open(filename,O_RDONLY);
	int keyfd = open(keyname, O_RDONLY);
	SendFile(filefd, sockfd);
	SendFile(keyfd, sockfd);
	//Clsoe both files
	close(filefd);
	close(keyfd);
	//GEt encrypted file back
	char * encrypted = RecvFile(sockfd, FileLen);
	//Print out encrypted
	printf("%s", encrypted);
    //Free mem alocated
	free(encrypted);
}

/*******************************************************************************
 * main(int, char*)
 * Runs program and calls appropriate functions
 ******************************************************************************/
int main(int argc, char *argv[]){
	//Makes sure there are 4 arguments, if wrong enter statment and send msg to stderr then exit
	if(argc != 4){
		fprintf(stderr, "Invalid number of arguments\n");
		fprintf(stderr, "Usage: opt_enc filename keyname portnumber\n");
		exit(1);
	}
	//Opens agrv 1 and then checks for invalid chars with FileCheck
	int fd = open(argv[1], O_RDONLY);
    //if fd is 0 enter statement and send msg to stderr then exit
	if(fd < 0){
		fprintf(stderr, "There was an error opening %s\n", argv[1]);
		exit(1);
	}
	FileCheck(fd);
    //close fd
	close(fd);
	//does as above loop for agrv 2
	fd = open(argv[2], O_RDONLY);
	if(fd < 0){
		fprintf(stderr, "There was an error opening %s\n", argv[2]);
		exit(1);
	}
	FileCheck(fd);
    //close fd
	close(fd);
	// set up socket
	struct addrinfo * res = CreateAddress(argv[3]);
	int sockfd = CreateSocket(res);
	ConnectSocket(sockfd, res);
	//calls handlerequest on two files
	HandleRequest(sockfd, argv[1], argv[2]);
    //free up res struct
	freeaddrinfo(res);
    //close socket
	close(sockfd);
	return 0;
}