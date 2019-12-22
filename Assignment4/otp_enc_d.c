/***********************************
* Branden Holloway
* Program 4 OTP
* otp_enc_d.c
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
#include <sys/wait.h>

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
 * BindSocket(int, struct)
 * Binds the port and socket given
 ******************************************************************************/
void BindSocket(int sockfd, struct addrinfo * res){
    //if bind has an error, enter statment send msg to stderr and exit
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        //close socket
		close(sockfd);
		fprintf(stderr, "Error in binding socket\n");
		exit(1);
	}
}

/*******************************************************************************
 *ListneSocket(int)
 *listens on port
 ******************************************************************************/
void ListneSocket(int sockfd){
    //if listen has error enter statment, sned msg to stderr then exit
	if(listen(sockfd, 5) == -1){
        //close socket
		close(sockfd);
		fprintf(stderr, "Error in listening on socket\n");
		exit(1);
	}
}

/*******************************************************************************
 * SendFile(int, char *, int)
 * utilizes socket to send file
 ******************************************************************************/
void SendFile(int new_fd, const char * message, int MsgLen){
	int BytesWrote = 0;
	int i = 0;
	//loop send file
	for (i = 0; i < MsgLen; i+=BytesWrote){
        //writes bytes wrote to message
		BytesWrote = write(new_fd, message, message + i);
        //if byteswrote is less than 0, enter statment send error to stderr and exit
		if(BytesWrote < 0){
			fprintf(stderr, "Error in writing to socket\n");
			_Exit(2);
		}
	}
	//receives done
	char buffer[20];
    //Erase buffer info
	memset(buffer, 0, sizeof(buffer));
	recv(new_fd, buffer, sizeof(buffer), 0);
}

/*******************************************************************************
 * HandShake(int)
 *
 ******************************************************************************/
int HandShake(int new_fd){
    //Variabel decliration
	char buffer[20];
    //erase buffer information for size of buffer
	memset(buffer, 0, sizeof(buffer));
	//get client infor
	recv(new_fd, buffer, sizeof(buffer),0);
	//if buffer is equal to opt_dec then enter and return a 1 else return false
	if(strcmp(buffer, "opt_enc") == 0){
		return 1;
	}
	return 0;
}


/*******************************************************************************
 * RecvFile(int, int)
 * Receives file and returns contnt as string
 ******************************************************************************/
char * RecvFile(int new_fd, int MsgLen){
	// keep track of the loop var and bytes read
	int bytesread = 0;
	int i;
	//allocate a string for the file
	char * receive = malloc(MsgLen * sizeof(char));
	memset(receive, '\0', sizeof(receive));
	// begin receiving the file
	for(i = 0; i< MsgLen; i+= bytesread){
		bytesread = read(new_fd, receive + i, MsgLen -i);
        //if bytesread is less than 0, send msg to stderr and exit
		if(bytesread < 0){
			fprintf(stderr, "Error in receiving file\n");
			_Exit(2);
		}
	}
	char * finished = "opt_enc_d f";
	send(new_fd, finished, strlen(finished),0);
	return receive;
}

/*******************************************************************************
 * void EncryptMsg(char, char, int)
 *encrypts file with a key
 ******************************************************************************/
void EncryptMsg(char * message, char * key, int message_length){
	int i, MsgNum, KeyNum, FinalNum;
    //26 letters of the alphabet + new line
	int alphabet = 27;
	for(i = 0; i < message_length; i++){
        //if not newline enter
		if(message[i] != '\n'){
			//Convert message and key at their index to ints
			if(message[i] == ' '){
				MsgNum = 26;
			}
			else{
				MsgNum = message[i] -'A';
			}
			if(key[i] == ' '){
				KeyNum = 26;
			}
			else{
				KeyNum = key[i] - 'A';
			}
			//once all are converted do below math
			FinalNum = (MsgNum + KeyNum) % alphabet;
			//place result in finalnum, if 26 then space else get char
			if(FinalNum == 26){
				message[i] = ' ';
			}
			else{
				message[i] = 'A' + (char)FinalNum;
			}
		}
	}
}


/*******************************************************************************
 * void HandleRequest(int)
 * handles clietn request
 ******************************************************************************/
void HandleRequest(int new_fd){
	int ValidClient = HandShake(new_fd);
    //if its not correct, then exit statemetn send msg to stderr, exit
	if (!ValidClient){
		fprintf(stderr, "Invalid Client\n");
        //send that its invalid
		char invalid[] = "Invalid";
		send(new_fd, invalid, strlen(invalid),0);
		_Exit(2);
	}
    //if its valid then send its valid
	char valid[] = "Valid";
	send(new_fd, valid, strlen(valid), 0);
	//get file length
	char buffer[10];
    //erase buffer for size of buffer
	memset(buffer, 0, sizeof(buffer));
	recv(new_fd, buffer, sizeof(buffer), 0);
	int MsgLen = atoi(buffer);
	//send length of the file
	send(new_fd, buffer, strlen(buffer),0);
	//Get the length of the key
    //erase buffer for size of buffer
	memset(buffer, 0, sizeof(buffer));
	recv(new_fd, buffer, sizeof(buffer), 0);
	int KeyLen = atoi(buffer);
	//Send key length 
	send(new_fd, buffer, strlen(buffer),0);
	//Call recvfile and put into message for msklen
	char * message = RecvFile(new_fd, MsgLen);
	//call recvfile and put into key for keylen
	char * key = RecvFile(new_fd, KeyLen);
    //call EncryptMsg on message, with key
	EncryptMsg(message, key, MsgLen);
	//send the file back
	SendFile(new_fd, message, MsgLen);
	//Free allocated space for message and eky
	free(message);
	free(key);
	exit(0);
}


/*******************************************************************************
 * void wait_for_connection
 * 
 * waits for a new connection to the server
 * Args: the file descriptor to wait on
 ******************************************************************************/
void wait_for_connection(int sockfd){
	// create a container for the connection
	struct sockaddr_storage their_addr;
	// create a size for the connection
    socklen_t addr_size;
	// create a new file descriptor for the connection
	int new_fd;
	// status variable
	int status;
	// pid variable;
	pid_t pid;
	// run forever
	while(1){
		// get the address size
		addr_size = sizeof(their_addr);
		// accept a new client
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
		// if there is no new client keep waiting
		if(new_fd == -1){
			fprintf(stderr, "Error in accepting connection\n");
			continue;
		}
		// fork to let a new process handle the new socket
		pid = fork();
		// if there was an error, say so
		if(pid == -1){
			fprintf(stderr, "Error in fork\n");
		}
		else if(pid == 0){
			// child process
			close(sockfd);
			HandleRequest(new_fd);
			close(new_fd);
		}
		else{
			// parent process
			close(new_fd);
			while (pid > 0){
				pid = waitpid(-1, &status, WNOHANG);
			}
		}
	}
}

/*******************************************************************************
 * main(int, char)
 * Runs program and calls appropriate functions
 ******************************************************************************/
int main(int argc, char *argv[]){
    //checks number of arguments is 2, if not send msg to stderr and exit
	if(argc != 2){
		fprintf(stderr, "Invalid number of arguments\n");
		exit(1);
	}
	printf("Server open on port %s\n", argv[1]);
	//calls CreateAddress to make address in res struct
	struct addrinfo * res = CreateAddress(argv[1]);
	//Takes address info and creates socket
	int sockfd = CreateSocket(res);
	//takes socket and binds with port
	BindSocket(sockfd, res);
	//runs listen function on port
   	ListneSocket(sockfd);
	//waits for connection
	wait_for_connection(sockfd);
	//frees up allocated mem
	freeaddrinfo(res);
}