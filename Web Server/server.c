#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<limits.h>
#include<dirent.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#define BUF_SIZE 1024


int main(int argc, char *argv[]) {
	int portno=0;
        int server_socket=0;
	int client_socket=0;
	int length = 0;
	int status =0 ;
	int set = 1;
	int buffer_len=0;
	int fil_size=0;
	struct sockaddr_in server_address,client_address;
	char buffer[BUF_SIZE];
	char buffer_recv[BUF_SIZE];
        char command [5];
        char file_name [50];
	char filename [50];
	char version [10];
	char *ext = NULL;
	char webpage[100];
	char web_return[50];
	char *postdata = NULL;
	char postdata_content[BUF_SIZE];
	
	FILE *fil;
	portno = atoi(argv[1]);
	struct timeval on;

	 length = sizeof(struct sockaddr_in);

	// creating socket for the server
        server_socket = socket(AF_INET,SOCK_STREAM,0);
	if (server_socket<0)
                printf("\n ERROR:Opening socket");

	setsockopt(server_socket, SOL_SOCKET,SO_REUSEADDR,&set,sizeof(int));
        server_address.sin_family = AF_INET;
  	server_address.sin_addr.s_addr = htonl(INADDR_ANY); //host to network long
  	server_address.sin_port = htons((unsigned short)portno); //host to network short

        // binding the socket
        if(bind(server_socket, (struct sockaddr *) &server_address , sizeof(server_address))<0)
                printf("\n ERROR:Binding Failed");
	else
		printf("\n Binding done");
	// listening for requests from client
	listen(server_socket,8);

	while(1)
	{
		// accept connection from client
		client_socket = accept(server_socket, (struct sockaddr *) &client_address,(socklen_t*) &length);
		if(client_socket == -1)
			printf("ERROR: Connection failed");
		printf("\n Client Connection successful\n");

		printf("\n The client sock value is:%d",client_socket);
		// forking to handle multiple requests
		if(!fork())
		{
			// child process, closing server socket
R:			close(server_socket);
			memset(buffer_recv,0,1024);
			
			// receive the request from client
			recv(client_socket,buffer_recv,1024,0);
			memset(command,0,5);
                        memset(file_name,0,50);
                        memset(filename,0,50);
                        memset(version,0,10);
			memset(buffer,0,1024);
			
			//seperating the Method, URL and the version
			strcpy(buffer,buffer_recv);
			sscanf(buffer,"%s%s%s",command, file_name,version);

			printf("\nthe file sent from client is:%s",buffer);
			buffer_len = strlen(buffer);
			printf("\n the buffer len is :%d",buffer_len);
			printf("\n the command is: %s",command);
			printf("\n the file name is: %s",file_name);
			printf("\n the version is: %s",version);
			strcpy(filename,".");
			strcat(filename,file_name);
			printf("\n file name is:%s",filename);
			int str = 0;
			str = strcmp(filename,"./");
			if(str==0)
				strcpy(filename,"index.html");	
			char *conn_ret=strstr(buffer,"Connection: keep-alive");	
			printf("\nCONN_RET: %s",conn_ret);
			
			int fil_size_check =0;	
			// setting the timer
			if(conn_ret != NULL)
			{
				on.tv_sec = 10;
				int setsockstatus=0;
                                setsockstatus = setsockopt(client_socket,SOL_SOCKET,SO_RCVTIMEO,(const char*)&on,sizeof(struct timeval));
                        	printf("\nset_sock_status:%d",setsockstatus);
			}
                        else
                        {
                                on.tv_sec = 0;
				int setsockstatus2=0;
				setsockstatus2 = setsockopt(client_socket,SOL_SOCKET,SO_RCVTIMEO,(const char*)&on,sizeof(struct timeval)); 
                        	printf("\nreset_sock_status:%d",setsockstatus2);
			}
			
			fil = fopen(filename, "r");
                       	if(fil != NULL)
			{
				fseek(fil,0,SEEK_END);
                                fil_size_check = ftell(fil);
                                printf("\n file_size_check: %d",fil_size_check);
				fseek(fil,0,SEEK_SET);
			}
			// validation for GET requests
			if(strcmp(command,"GET")==0 && (fil_size_check>0) && (strcmp(version,"HTTP/1.1")==0 || strcmp(version,"HTTP/1.0")==0))
			{
				printf("\nfile exists");
				int file_size = 0;
				int file_size_read = 0;
				int head_len = 0;
				fseek(fil,0,SEEK_END);
                                file_size = ftell(fil);
                                fseek(fil,0,SEEK_SET);
				ext = strrchr(filename,'.');
				printf("\n The extension is: %s", ext);
				memset(webpage,0,100);
				
				if(conn_ret != NULL)
                        	{
					sprintf(webpage,"HTTP/1.1 200 OK\r\nContent-Type:%s\r\nContent-Length:%d\r\nConnection: keep-alive\r\n\r\n",ext,file_size);

				}
				else
					sprintf(webpage,"HTTP/1.1 200 OK\r\nContent-Type:%s\r\nContent-Length:%d\r\nConnetion: Close\r\n\r\n",ext,file_size);
				printf("\nfilesize %d \n", file_size);

				char buffer1[file_size];
				memset(buffer1,0,sizeof(buffer1));
				file_size_read = fread(buffer1,1,file_size,fil);
                                
                                printf("\n file _size_read %d", file_size_read);
				
                                head_len = strlen(webpage);
                                printf ("\n head_len%d", head_len);
                                
				// displaying the header content that needs to sent
				printf("\n The head content is :\n%s", webpage);
				
				// displaying the file content
				printf("\n file content: %s", buffer1);
					
				// sending the header and the file content
				write(client_socket,webpage,strlen(webpage));
				status = write(client_socket,buffer1,sizeof(buffer1));
					if(status<0)
						printf("\nALERT: File not sent properly");
					else
						printf("\nFile sent successfully");
				
				//Connection keep-alive check
				if(conn_ret != NULL)
				{
					printf("\n Entered Continue statement");
					if(fil != NULL)
                                        	fclose(fil);
                                        goto R;
				}
				else
				{	
					if(fil != NULL)
                                                fclose(fil);	
					close(client_socket);
                                        exit(0);
				}
			}
			//Validations for POST requests
			else if (strcmp(command,"POST")==0 && (fil_size_check>0) && (strcmp(version,"HTTP/1.1")==0 || strcmp(version,"HTTP/1.0")==0))
			{
				int post_content_size = 0;
				//int file_read_size = 0;
				memset(postdata_content,0,sizeof(postdata_content));
				memset(webpage,0,sizeof(webpage));
				printf("\nfile exists");
                                fseek(fil,0,SEEK_END);
                                
				fil_size = ftell(fil);
                                fseek(fil,0,SEEK_SET);
                                printf("\nfilesize %d \n", fil_size);
				
				char *buffer_old = malloc(fil_size);	
				char *buffer_new = malloc(fil_size);
				if(buffer_new == NULL)
				{
					printf("\nerror on malloc\n");
				}
				if(buffer_old == NULL)
                                {
                                        printf("\nerror on malloc\n");
                                }

                                fread(buffer_old,1,fil_size,fil);
				strcpy(buffer_new,buffer_old);
                                printf("\n the file content is: %s",buffer_new);
				
				// getting the postdata content from the request
				postdata = strrchr(buffer,'\n');
				postdata++;
				printf("\n The postdata is: %s",postdata);
                                
				// getting the extension
				ext = strrchr(filename,'.');
                                printf("\n Extension is: %s", ext);
				
				strcpy(postdata_content,"<html><body><pre><h1>");
				strcat(postdata_content,postdata);
				strcat(postdata_content,"</h1></pre>");
				
				// displaying the postdata content to be sent
				printf("\n The post data content is:%s",postdata_content);

				printf("\nfilesize %d \n",fil_size);
				int post_data_len = 0;
				post_data_len = strlen(postdata_content);
				printf("\n The post data size:%d",post_data_len);
				
				post_content_size = fil_size + strlen(postdata_content);
				printf("\n Postdata content size: %d", post_content_size);	
				if(conn_ret != NULL)
                                {
                                        sprintf(webpage,"HTTP/1.1 200 OK\r\nContent-Type:%s\r\nContent-Length:%d\r\nConnection: keep-alive\r\n\r\n",ext,post_content_size);

                                }
                                else
                                        sprintf(webpage,"HTTP/1.1 200 OK\r\nContent-Type:%s\r\nContent-Length:%d\r\nConnetion: Close\r\n\r\n",ext,post_content_size);

				// displaying the header to be sent
				printf("\n the webpage content is: %s",webpage);
				printf("\n The client socket value is: %d", client_socket);	

				// sending the header, postdata and the file content
				status = write(client_socket,webpage,strlen(webpage));
                                if(status>0)
                                        printf("\n1. webpage sent");
				else
					printf("\n1. ERROR:webpage not sent");
				status = write(client_socket,postdata_content,strlen(postdata_content));
				if(status>0)
                                        printf("\n2. postdata content sent");
				status = write(client_socket,buffer_new,strlen(buffer_new));
				if(status>0)
                                        printf("\n3. File sent");
				else
					printf("\n4. File not sent");
                                free(buffer_new);
				free(buffer_old);
				
				//Connection keep-alive check
				if(conn_ret != NULL)
				{
                                        printf("\n Entered Continue statement");
                                        if(fil != NULL)
						fclose(fil);
					goto R;
                                }
                                else
                                {
                                        if(fil != NULL)
                                                fclose(fil);
					close(client_socket);
                                        exit(0);
                                }
			
			}
			
			// Condition for BAD request
			else if ( buffer_len>0 )
			{
				printf("\n FILE does not exist");
				memset(web_return,0,50);
				status = write(client_socket,"HTTP/1.1 500 Internal Server Error",strlen("HTTP/1.1 500 Internal Server Error"));
				printf("\n The status value is %d:", status);
				char web_return2[]="<html><body><font size =6><b>HTTP/1.1 500 Internal Server Error<b></font></body></html>";
				int web_ret_length=strlen(web_return2);
				
				sprintf(web_return,"Content-length:%d",web_ret_length);
				write(client_socket,web_return,strlen(web_return));
				write(client_socket,"Content-Type: html\n\n",strlen("Content-Type: html\n\n"));
				write(client_socket,"<html><body><font size =6><b>HTTP/1.1 500 Internal Server Error<b></font></body></html>",strlen("<html><body><font size =6><b>HTTP/1.1 500 Internal Server Error<b></font></body></html>"));	
				printf("\n the connection status is:%s",conn_ret);
                                if(conn_ret != NULL)
				{
                                        printf("\n Entered Continue statement");
					memset(web_return,0,50);
                                        if(fil!=NULL)
						fclose(fil);
                                        goto R;

                                }
                                else
                                {
                                        if(fil != NULL)
                                                fclose(fil);
					printf("\n Entered bad comment else statement");
					close(client_socket);
                                        exit(0);
                                }
	
			}
			// Closing the socket to handle other requests
			else
			{
				printf("\n Entered the else loop");
                                if(conn_ret != NULL)
				{
                                        printf("\n Entered Continue statement");
					if(fil != NULL)
                                        	fclose(fil);
                                        goto R;

                                }
                                else
                                {
                                        if(fil != NULL)
                                                fclose(fil);
					close(client_socket);
                                        exit(0);
                                }

			}
			if(fil != NULL)
				fclose(fil);
			close(client_socket);
			printf("\n CLOSING");
			exit(0);
		}

		close(client_socket);
	}

	close(server_socket);
	return 0;
}
