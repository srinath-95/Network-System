#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<limits.h>
#include<dirent.h> 

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#define BUF_SIZE 1024 

typedef struct{
        long int seq_no;
        char buffer[BUF_SIZE];
        long int frame_size;
}frame;

void error (const char *msg)
{
        perror(msg);
        exit (1);

}

int main(int argc, char *argv[]) {
       // validating for portno if the user has provided 
	if (argc < 2)
        {
                fprintf(stderr, "\n Portno is not provided. Terminating the program");
                exit (1);
        }

        int server_socket,portno;
        int status = 0; 
	int choice,length;
        int packet_count = 0;
        int NACK,ACK = 0;
        int fail_count = 0;

        char command[30];
        char command_choice[8];
        char file_name[20];

        length = sizeof(struct sockaddr_in);
        char cwd[PATH_MAX];

        frame frame1;
        struct timeval opt_time_set= {1,0};
	struct timeval opt_time_reset= {0,0};	
	char buffer [BUF_SIZE];
        struct sockaddr_in server_address,client_address;
	
	// creating socket for the server
        server_socket = socket(AF_INET,SOCK_DGRAM,0);
        if (server_socket<0)
                error ("\n ERROR:Opening socket");

        bzero((char *) &server_address,sizeof(server_address));
        portno = atoi(argv[1]);
        printf ("\n the portno is:%d" , portno);
        // defining the server address
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(portno);
        server_address.sin_addr.s_addr = INADDR_ANY;
	
	// binding the socket
        if(bind(server_socket, (struct sockaddr *) &server_address , sizeof(server_address))<0)
                error("\n ERROR:Binding Failed");

while(1)
{
	// Receiving the type of command from client end
	status = recvfrom(server_socket,command, sizeof(command),0,(struct sockaddr *) &client_address,(socklen_t *) &length);
        if(status<0)
                printf("\n ERROR:Reading the input  value");
        printf("\n The entered value ofchoice is : %s\n", command);
    	
	
	memset(command_choice,0,8);
        memset(file_name,0,20); 
	
 	sscanf(command, "%s %s", command_choice, file_name);
        //printf("%s \n", command_choice);
        //printf("%s \n", file_name); 

	memset(command,0,30);
	
	// Case for getting the file from server
	if((strcmp(command_choice,"get")==0) && (file_name != NULL))
        {

                printf("entered get statement");
		memset(buffer,0,sizeof(buffer));
		memset(&packet_count,0,sizeof(int));		
		memset(&frame1,0,sizeof(frame1));	

		printf("\n The filename is:%s",file_name);
                FILE *file_put_client;
                file_put_client= fopen(file_name,"rb");
		
		// checking if the file exist in server or not		
                if(file_put_client == NULL)
                {
		    printf("\n ERROR:File does not exist");
                    status = sendto(server_socket ,&packet_count , sizeof(int), 0, (struct sockaddr *) &client_address, length);
                    status = recvfrom(server_socket,&packet_count  , sizeof(int),0, (struct sockaddr *) &client_address, (socklen_t *) &length);  
                }
		else
		{
			memset(buffer,0,sizeof(buffer));
			memset(&frame1,0,sizeof(frame1));
                        printf("\n File exists");
			status = setsockopt(server_socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&opt_time_set,sizeof(struct timeval));	
			if(status<0) 
				printf("Set of setsockopt failed");

			// getting the file size
                        fseek(file_put_client, 0 , SEEK_END);
                        int file_size = ftell(file_put_client);
                        fseek(file_put_client, 0, SEEK_SET);
                        printf("filesize %d \n", file_size);
                        
			// calculating the packet count value that needs to be transmitted  	
			if(file_size%BUF_SIZE==0)
				packet_count = (file_size / BUF_SIZE);
			else
				 packet_count = (file_size / BUF_SIZE)+1;
                        
			printf ("\n packet count to send %d ", packet_count);
                      // sending the packet count to client and receiving the acknowledgment 
		        status = sendto(server_socket , &packet_count , sizeof(int), 0, (struct sockaddr *) &client_address, length);
R:                      status = recvfrom(server_socket,&ACK  , sizeof(int),0, (struct sockaddr *) &client_address, (socklen_t *) &length);                                                   
                        if((ACK!=packet_count) && (NACK<100))
	        	{
				// resending the packet count	
				status = sendto(server_socket , &packet_count , sizeof(int), 0, (struct sockaddr *) &client_address, length);
				if(NACK == 99)
				{
					printf("\n Frame not sent");
					fail_count++;
				}	
				printf("\n Resending count of the packet: %d", NACK); 
				NACK++;
				goto R;
			}

			  if(status<0)
                                 printf("\n ERROR:writing");

                          for(long int index=0; index<packet_count; index++)
                          {
                        
				 NACK = 0;		
				 frame1.seq_no = index;
			         int file_size = fread(frame1.buffer,1,BUF_SIZE,file_put_client);
                                 frame1.frame_size = file_size; 
				
				//sending the frame data from server to client and receiving the acknowledgement 
				 status = sendto(server_socket ,&frame1,sizeof(frame1),0, (struct sockaddr *) &client_address, length);
                       
FA:				 status = recvfrom(server_socket, &ACK , sizeof(int),0, (struct sockaddr *) &client_address,(socklen_t *) &length);
                       
				 if(ACK != frame1.seq_no && NACK <100)
				 {
					// resending the frame data	
					status = sendto(server_socket ,&frame1,sizeof(frame1),0, (struct sockaddr *) &client_address, length);

					if(NACK == 99)
					{
						printf("\n The frame is dropped");
						fail_count++;
					}
					printf("\nResending the frame: %d",NACK);	
					NACK++;
					goto FA;

				 }
				 else if(ACK == frame1.seq_no)
					printf("\n ACK received and frame sent: %ld", frame1.seq_no);
			    }	
				// sending the count of frame which were failed to send to client
				status = sendto(server_socket ,&fail_count ,sizeof(int),0, (struct sockaddr *) &client_address, length);
FC:				status = recvfrom(server_socket, &ACK , sizeof(int),0, (struct sockaddr *) &client_address,(socklen_t *) &length);
		
				if(ACK != fail_count)
				{
					// resending the failed frame count	
					status = sendto(server_socket ,&fail_count ,sizeof(int),0, (struct sockaddr *) &client_address, length);
					
					if(NACK == 99)
                                        {
                                                printf("\n The fail_count frame is dropped");
                                        }
                                        NACK++;
	
					printf ("\n file has not been sent successfully");
					goto FC;	
				}	

				else 
					printf("File sent succesfully");
				
                        fclose(file_put_client);    
			status = setsockopt(server_socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&opt_time_reset,sizeof(struct timeval));
                        if(status<0)
                                printf("Reset of setsockopt failed");

		}
       	}
	
	// Case to put file in server
	else if((strcmp(command_choice,"put")==0) && (file_name != NULL))
        {
		printf("\n Entered the case for creating a file");
                memset(buffer,0,sizeof(buffer));
                memset(&packet_count,0,sizeof(int));
		memset(&frame1,0,sizeof(frame1));	
	
		int size = 0;
		FILE *file_get_client;

		// receiving the packet count from client and sending the acknowledgement
		status = recvfrom(server_socket, &packet_count , sizeof(int),0, (struct sockaddr *) &client_address, (socklen_t *) &length);
                status = sendto(server_socket , &packet_count , sizeof(int), 0, (struct sockaddr *) &client_address, length);
		printf ("\n the packet count is: %d" , packet_count);
                
		if(status<0)
                	printf("\n ERROR:reading");
                if(packet_count >0 ) 
                {
                	status = setsockopt(server_socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&opt_time_set,sizeof(struct timeval));
                        if(status<0)
                                printf("Set of setsockopt failed");

			file_get_client = fopen(file_name,"w");
                        printf("\n Starting to write file");
                        memset(buffer,0,sizeof(buffer));
                                       
			for (long int index=0; index<packet_count; index++)
                        {
                        	// receiving the frame from client and sending the acknowledgement	
				status = recvfrom(server_socket,&frame1 , sizeof(frame1),0, (struct sockaddr *) &client_address,(socklen_t *) &length);
                               	if(status<0)
                                       	printf("\n ERROR:reading");
					
				status =  sendto(server_socket,&(frame1.seq_no) ,sizeof(frame1.seq_no),0 , (struct sockaddr *) &client_address, length);
		
				if(frame1.seq_no == index)
				{
                           		// Writing the content of frame to he file	
					status = fwrite(frame1.buffer,1, frame1.frame_size,file_get_client);
					size = size + sizeof(frame1.buffer);	
					printf("\n the file size receiving is: %d", size);	
				}
				else
					index--;
			}
                        fclose(file_get_client);
               		status = setsockopt(server_socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&opt_time_reset,sizeof(struct timeval));
                        if(status<0)
                                printf("Reset of setsockopt failed");
   
		}
                        else
                            printf("\n File does not exist");
       }

	// case for deleting the file on server
	 else if((strcmp(command_choice,"delete")==0) && (file_name != NULL))
         {
	         printf("\n Entered the case for deleting file");
                 memset(buffer,0,sizeof(buffer));
                 printf("The filename is:%s \n",file_name);
                 FILE *file_delete;
                 file_delete = fopen(file_name,"r"); 
                // cheking if the file exist on server 
		if(file_delete == NULL)
		{		
		 	printf("ERROR:File does not exist");
               		strncpy(buffer,"File does not exist",sizeof(buffer));  

			status = sendto(server_socket,buffer,sizeof(buffer),0, (struct sockaddr *) &client_address, length);
		}
		else
		{
			printf("File exists");
                        status = remove(file_name);
                        if(status<0)
			{      
				printf("ERROR:Deleting file"); 
			      	strncpy(buffer,"File delete not success",sizeof(buffer)); 
				status = sendto(server_socket,buffer,sizeof(buffer),0, (struct sockaddr *) &client_address, length);
	
			} 
			else
			{
				strncpy(buffer,"File delete success",sizeof(buffer)); 
			     	status = sendto(server_socket,buffer,sizeof(buffer),0, (struct sockaddr *) &client_address, length); 
		
			}

		}
	 }
	
	// case for listing the files in the directory	
	else if( strcmp(command_choice,"ls")==0)
	{
		 DIR *directory;
                 struct dirent *dir;
                 directory = opendir(".");
                 memset(buffer,0,sizeof(buffer));
                 if(directory)
                 while((dir=readdir(directory)) != NULL)
                 {       
                          strcat(buffer, dir->d_name);
                          strcat(buffer, "\n");   
                 }
                 printf("\n The list of values are: %s", buffer);
                 status = sendto(server_socket, buffer , sizeof(buffer),0, (struct sockaddr *) &client_address, length);
               
		  closedir(directory); 
	}
	
	// case to exit from server
	 else if(strcmp(command_choice,"exit")==0)
         {
          printf("You have entered the Exit case:");       
          goto Q;	 
         }

	// checking for invalid commands from user
	 else
         {
           printf("\n Alert: Please enter a valid option");
          // goto C;
         }

}
Q:  close (server_socket);
        return 0;
}

	
	


