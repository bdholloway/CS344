/***********************************
* Branden Holloway
* Program 4 OTP
* otp_dec_d.c
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
	if(strcmp(buffer, "opt_dec") == 0){
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
 * void DecryptMsg(char *, char *, int)
 * decrypts file with key
 ******************************************************************************/
void DecryptMsg(char * message, char * key, int MsgLen){
	int i, MsgNum, KeyNum, FinalNum ;
    //alphabet is 26 + newline
	int alphabet = 27;
	for (i = 0; i < MsgLen; i++){
		if (message[i] != '\n'){
            //convert message at index to num
			if(message[i] == ' '){
				MsgNum = 26;
			}
			else{
				MsgNum = message[i] -'A';
			}
            //convert key at index to num
			if(key[i] == ' '){
				KeyNum = 26;
			}
			else{
				KeyNum = key[i] - 'A';
			}
			//to get funal num, do below arithmatic
			FinalNum = (MsgNum - KeyNum + alphabet) % alphabet;
			//if Final is 26, make it a space for message at index
			if(FinalNum == 26){
				message[i] = ' ';
			}
            //if not 26, then get char
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
    //call DecryptMsg on message, with key
	DecryptMsg(message, key, MsgLen);
	//send the file back
	SendFile(new_fd, message, MsgLen);
	//Free allocated space for message and eky
	free(message);
	free(key);
	exit(0);
}


/*******************************************************************************
 * void WaitConnect
 * wait for connection to server
 ******************************************************************************/
void WaitConnect(int sockfd){
	//create variable sturct, get form imported libarary
	struct sockaddr_storage Addr;
    socklen_t AddrSize;
	int new_fd, status;
	pid_t pid;
	//will loop thorugh waiting for connection
	while(1){
		//gets size of AddrSize
		AddrSize = sizeof(Addr);
		//if a client comes accept
		new_fd = accept(sockfd, (struct sockaddr *)&Addr, &AddrSize);
		//as long as there is no client, wait
		if(new_fd == -1){
			fprintf(stderr, "Error in accepting connection\n");
			continue;
		}
		//fork to let new process ahndle connection
		pid = fork();
		//if error, enter statement and send msg to stderr, but dont exit
		if(pid == -1){
			fprintf(stderr, "Error in fork\n");
		}
		else if(pid == 0){
			//close socket
			close(sockfd);
			HandleRequest(new_fd);
            //close
			close(new_fd);
		}
		else{
			close(new_fd);
			while (pid > 0){
				pid = waitpid(-1, &status, WNOHANG);
			}
		}
	}
}

/*******************************************************************************
 * main(int, char*)
 * run program, call appropriate functions
 ******************************************************************************/
int main(int argc, char *argv[]){
    //be sure argument number is 2, if not send msg to stderr and exit
	if(argc != 2){
		fprintf(stderr, "Invalid number of arguments\n");
		exit(1);
	}
    //print out what port open on
	printf("Server open on port %s\n", argv[1]);
	//Create address with info given
	struct addrinfo * res = CreateAddress(argv[1]);
	//Create socket with address info
	int sockfd = CreateSocket(res);
	//bind address and port 
	BindSocket(sockfd, res);
	//begin to listen
   	ListneSocket(sockfd);
	//wait for connection, up to 5
	WaitConnect(sockfd);
	//free struct
	freeaddrinfo(res);
}