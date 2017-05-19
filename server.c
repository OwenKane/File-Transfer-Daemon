#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include <sys/types.h>
#include <signal.h>
#include <time.h>

#define TEXT_FILE "/Users/owenkane/Desktop/Submission/text.txt"
#define MARKETING "/Users/owenkane/Desktop/Submission/server/Marketing/"
#define OFFERS "/Users/owenkane/Desktop/Submission/server/Offers/"
#define PROMOTIONS "/Users/owenkane/Desktop/Submission/server/Promotions/"
#define SALES "/Users/owenkane/Desktop/Submission/server/Sales/"
#define ROOT "/Users/owenkane/Desktop/Submission/server/"
 
//Thread function
void *connection_handler(void *);

//Logging function
void logFile(char file_mod[], char editUser[]);
 
int main(int argc , char *argv[]){
	
	int s; //Socket
	int cs; //Client socket
	int connSize , *new_sock;
	struct sockaddr_in server , client;
	 
	//Create socket
	s = socket(AF_INET , SOCK_STREAM , 0);
	if (s == -1){
		printf("Could not create socket");
	}else{
		printf("Socket successfully created");
	}
	 
	//Set the sockaddr_in variables
	server.sin_port = htons( 8888 );
	server.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY inds to all local interfaces
	server.sin_family = AF_INET; //Use IPV4 protocoll
	 
	//Bind
	if( bind(s,(struct sockaddr *)&server , sizeof(server)) < 0){
		perror("bind Issue");
		return 1;
	}else{
		printf("Bind completed\n");
	}
	
	//Listen for a connection
	listen(s , 3);
	 
	//Accept and incoming connection
	printf("Waiting for incoming connections from client\n");
	connSize = sizeof(struct sockaddr_in);
	
	while((cs = accept(s, (struct sockaddr *)&client, (socklen_t*)&connSize))){
		printf("Connection accepted\n");
		 
		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = cs;
		 
		if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0){
			
			perror("Could not create thread");
			return 1;
		}
		printf("Handler assigned to client\n");
	}
	 
	if (cs < 0)
	{
		perror("accept failed");
		return 1;
	}
	 
	return 0;
}
 

pthread_mutex_t lock_x;

void *connection_handler(void *s)
{
	//Get the socket descriptor
	int sock = *(int*)s;
	int read_size;
	char *message , client_message[2000];
	char username[2000];
	char password[2000];
	char msg_client[50];
	char fileName[50];
	char t_status[10];
	int index = 0;
	char *success_message;
	char *failure_message;
	char *file_transfer_message;
	
	char file_line[200];
	char userInfo[200], passInfo[200];
	int userAuth = 0;
	char file_file_contents[2000];


	FILE *fptr;
	//Receive a message from client
	while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
	{
		if(index == 0) {
			printf("Username");
			strcpy(username, client_message);
			puts(username);
		} else if(index == 1) {
			printf("Password");
			strcpy(password, client_message);
			puts(password);
			break;
		}
		index++;
	}
	
	pthread_mutex_lock(&lock_x); //Lock
	
	// Read in the password file
	// Check credentials
	FILE *auth_file = fopen(TEXT_FILE, "r");
	
	while(fgets(file_line, 80, auth_file)) {
		sscanf(file_line, "%s  %s", userInfo, passInfo);
					
		if((strcmp(username, userInfo) == 0) && (strcmp(password, passInfo) == 0)) {
			userAuth = 1;
			break;
		}		
	}
	
	fclose(auth_file);
	pthread_mutex_unlock(&lock_x);//unlock
	
	if(userAuth == 1) {
		message = "Credentials found\n";
		success_message = "Found";
		write(sock , success_message , strlen(success_message));	
		
	} else {
		message = "Credentials not found\n";
		failure_message = "Not found";
		write(sock , failure_message , strlen(failure_message));
	}
	
	int step = 0;
	char file_name[2000], file_contents[2000], file_path[2000];
	
	while( (read_size = recv(sock , msg_client , 2000 , 0)) > 0 ){
		
		if(step == 1) {
			printf("File name: ");
			puts(msg_client);
			strcat(file_name, msg_client);
		}

		if(step == 2) {
			printf("file_contents of file: ");
			puts(msg_client);
			strcat(file_contents, msg_client);
		}
		
		
		if( strcmp(msg_client, "1") == 0) {
			printf("Sales directory selected");
			strcpy(file_path, SALES);
		}else if (strcmp(msg_client, "2") == 0) {
			printf("Promotion directory selected");
			strcpy(file_path, PROMOTIONS);
		}else if(strcmp(msg_client, "3") == 0) {
			printf("Offer directory selected");
			strcpy(file_path, OFFERS);
		}else if( strcmp(msg_client, "4") == 0) {
			printf("Marketing directory selected");
			strcpy(file_path, MARKETING);
		}else if( strcmp(msg_client, "5") == 0) {
			printf("Root directory selected");
			strcpy(file_path, ROOT);
		}else{
			printf("Invalid Option!");
		} 
		step++;	
	}
	
	strcat(file_path, file_name);
	
	//Log event
	logFile(file_path, username);

	fclose(fptr);

	pthread_mutex_lock(&lock_x);//lock
	
	FILE *fileTran = fopen(file_path, "w");
	if(fileTran == NULL) {
		printf("File %s Cannot be opened file on server.\n", file_name);
		file_transfer_message = "File transfer failed";
	}
	else {
		int length = 0;
		length = strlen( file_contents );
		
		for(int i = 0 ; i < length; i++) {
			fputc(file_contents[i], fileTran); 
		}
		
		fclose(fileTran);
		pthread_mutex_unlock(&lock_x);
		
	}
 
	if(read_size == 0){
		printf("Client disconnected");
		fflush(stdout);
	}
	else if(read_size == -1){
		perror("rec failed");
	}
		 
	//Free the socket pointer
	free(s);
	 
	return 0;
}

void logFile(char file_mod[], char editUser[]){
	FILE *fp;
	fp = fopen("/Users/owenkane/Desktop/Submission/log.txt","a");
	
	time_t now;
	time(&now);/* get current time; same as: now = time(NULL)  */ 
	
	printf("\n %s \n",editUser);
	
	if(fp == NULL) {
		printf("Error opening file");
		exit(1);
	}

	fprintf(fp,"\n---------------------------New-Entry----------------------------\n");
	fprintf(fp,"Time : %s",ctime(&now));
	fprintf(fp,"File : %s\n",file_mod);
	fprintf(fp,"User : %s\n",editUser);
	fprintf(fp,"----------------------------------------------------------------");
	fclose(fp);
}

