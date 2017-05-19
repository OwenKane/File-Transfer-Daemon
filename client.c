#include<stdio.h> //printf
#include<string.h>    //strlen
#include<stdlib.h>
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include <fcntl.h> // for open
#include <unistd.h> // for close
 
int menu();

int main(int argc , char *argv[]){
	
	int SID;
	struct sockaddr_in server;
	char username[500];
	char server_reply[2000];
	char server_success_reply[2000];
	char password[500];
	char file_name[2000];
	char choice[50];
	char file_contents[2000];
	char file_message[50];
	char* str;
	int selected = 10;
	
	//Create socket
	SID = socket(AF_INET , SOCK_STREAM , 0);
	if (SID == -1){
		printf("Error creating socket");
	}else{
		puts("Socket created");
	}
	 
	//set socket variables
	server.sin_family = AF_INET; 
	server.sin_addr.s_addr = INADDR_ANY; //Bind to all local interfaces
	server.sin_port = htons( 8888 ); //set the prot
 
	//Connect to remote server
	if (connect(SID , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}
	 
	puts("Connected\n");
	 
	//keep communicating with server
	while(1){
		
		printf("Enter Username : ");
		scanf("%s" , username);
		
		printf("Enter Password : ");
		scanf("%s" , password);
		 
		send(SID, username, strlen(username), 0);
		send(SID, password, strlen(password), 0);
		
		 
		//Receive a reply from the server
		if( recv(SID , server_reply , 2000 , 0) < 0){
			puts("recv failed");
			break;
		}
		 
		puts("Server reply :");
		puts(server_reply);
		
		if(strcmp(server_reply, "Found")  == 0) {
				
			puts("**** Choose directory to transfer file **** ");
			puts("1. Sales Directory");
			puts("2. Promotions Directory");
			puts("3. Offer Directory");
			puts("4. Marketing Directory");
			puts("5. Root Directory");
			
			scanf("%s",&choice);
			
			printf("Option Choosing %s \n",choice);
			send(SID, choice, strlen(choice), 0);
			
			puts("Enter file name to transfer to the directory");
			scanf("%s" , file_name);
	
			// Open file to transfer contents over.
			FILE *file = fopen( file_name, "r" );  
			char c;
			
			
			if (file) {
				// send the file name
				send(SID, file_name, strlen(file_name), 0);
				int num_of_chars = 0;
				while ((c = getc(file)) != EOF) {
					num_of_chars++;
				}
				
				// Reset to start of file.
				fseek(file, 0, SEEK_SET);

				char* file_contents = (char*)malloc(sizeof(char) * num_of_chars);

				// read in file contents
				int i = 0;
				while ((c = getc(file)) != EOF) {
					file_contents[i] = c;
					i++;
				}
				printf("%s", file_contents);
				
				// send the file to the server.
				send(SID, file_contents, strlen(file_contents), 0);
				fclose(file);
				
			}
			
			printf("\nFile choosing for transfer: %s \n", file_name);
			break;
			
		} else {
			puts("User not found, try again");
			break;
		}
		
	}
	
	close(SID);
	return 0;
}

