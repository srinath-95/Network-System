#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<unistd.h>

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
	
void error(const char *msg)
{
        perror(msg);
        exit(1);
}

int main(int argc, char *argv[]) {
        // validating if the user has entered ip address and port no
	if (argc<3)

	{
                fprintf(stderr, "The portno or the serevr address is missing");
                exit(1);
        }
	
	int client_socket,portno,length;
        
	struct sockaddr_in server_address;
        struct hostent *server;
        char buffer [BUF_SIZE];
        char command [30];
        char command_choice [8];
        char file_name [20];
        int status,ACK,NACK = 0;
        int fail_count =0;
        int packet_count =0;
        int size = 0;
        frame frame1;
        struct timeval opt_time_set = {1,0};
	struct timeval opt_time_reset = {0,0};

	portno = atoi(argv[2]);
	// creating socket for the client
        client_socket = socket(AF_INET,SOCK_DGRAM,0);

        if(client_socket<0)
                error("\n ERROR:Opening Socket");
	// validating the host name
        server = gethostbyname(argv[1]);
        if(server == NULL)
                fprintf(stderr,"ERROR,No such host");

        bzero((char*) &server_address,sizeof(server_address));

        // defining the server address
        server_address.sin_family = AF_INET;

	server_address.sin_port = htons(portno);
        bcopy((char *) server->h_addr , (char *) &server_address.sin_addr.s_addr,server->h_length);
	length = sizeof(struct sockaddr_in);

while(1)
{
 	// getting the user command 
	printf ("\n\n Enter the desired operation that you would like to perform:");
        printf ("\n 1 get [filename]");
       	printf ("\n 2 put [filename]");
        printf ("\n 3 delete [filename]");
        printf ("\n 4 ls");
        printf ("\n 5 exit\n ");
		
	memset(command,0,30);
        memset(command_choice,0,8);
        memset(file_name,0,20); 

	fgets(command,30,stdin);
        printf("\n Entered input is: %s \n", command);
        status = sendto(client_socket , command , sizeof(command), 0,(struct sockaddr *) &server_address,length);
        if(status<0)
        	printf("\n ERROR:Writing the input value");

	sscanf(command, "%s %s", command_choice , file_name);  
	printf("%s \n", command_choice);
	printf("%s \n", file_name);

	// case for getting the file from server
	if((strcmp(command_choice,"get")==0) && (file_name != NULL))
	{

		printf("entered get statement");
		memset(&packet_count,0,sizeof(int));
	        memset(buffer,0,sizeof(buffer));
		memset(&frame1,0,sizeof(frame1));	
	
		// receceiving the packet count from the server and sending the acknowlegement
		status = recvfrom(client_socket, &packet_count , sizeof(int),0, (struct sockaddr *) &server_address, (socklen_t *) &length);
		status =  sendto(client_socket,  &packet_count, sizeof(int), 0 , (struct sockaddr *) &server_address, length);
		printf ("\n the packet count is: %d" , packet_count); 
                if(status<0)
                      printf("\n ERROR:reading");

                if(packet_count>0)
                {
       		        status = setsockopt(client_socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&opt_time_set,sizeof(struct timeval));
		       if(status<0)
		       		printf("Set of setsockopt failed");	       
			 FILE *file_get_server;      
                         file_get_server= fopen(file_name,"w");
                         memset(buffer,0,sizeof(buffer));
                         for (long int index=0; index<packet_count; index++)
                         {
			 	// receiving the frame data from server and sending the acknowlegement 
			 	 status = recvfrom(client_socket, &frame1 ,sizeof(frame1),0, (struct sockaddr *) &server_address,(socklen_t *) &length);
				if(status<0)
                       		    printf("\n ERROR:reading");
			 	status =  sendto(client_socket,  &(frame1.seq_no), sizeof(frame1.seq_no), 0 , (struct sockaddr *) &server_address, length);
                                if(frame1.seq_no == index) 
				  {
					// writing the frame data into the file
					  status = fwrite(frame1.buffer,1,frame1.frame_size,file_get_server);
                             		 size = size + sizeof(frame1.buffer);
					 printf(" \n the file size receiving is: %d", size);	
				   }			   
				else
				    index--; 		
					
			}
				// receving the count of franes that got dropped	
		 		 status = recvfrom(client_socket, &fail_count ,sizeof(int),0, (struct sockaddr *) &server_address,(socklen_t *) &length);
				status =  sendto(client_socket,  &fail_count, sizeof(int), 0 , (struct sockaddr *) &server_address, length);
					
				if(fail_count == 0)
				       printf ("\n FILE received");
				else
					printf ("\n FILE not received");	
                                fclose(file_get_server);
				status = setsockopt(client_socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&opt_time_reset,sizeof(struct timeval));
                       		if(status<0)
                                printf("Reset of setsockopt failed");
		} 
                                
	  	else 
                    printf(" \n File does not exist in server");
	memset(buffer,0,sizeof(buffer));
	}	

	// case to put file onto the server
	else if((strcmp(command_choice,"put")==0) && (file_name != NULL))
	{
		memset(buffer,0,sizeof(buffer));
	        printf("\n Entered put case");	
		memset(&packet_count,0,sizeof(int));
		memset(&frame1,0,sizeof(frame1));	
			
       	       FILE *file_put_server;
               file_put_server = fopen(file_name,"r");   
              
	       // checking if file exists or not
	       if( file_put_server == NULL)
               {
    	               printf("\n File does not exist");
                       status = sendto(client_socket, &packet_count , sizeof(int) , 0 , (struct sockaddr *) &server_address, length);
 			status = recvfrom(client_socket, &ACK, sizeof(int), 0 , (struct sockaddr *) &server_address,(socklen_t *) &length);
               }              
               else      
               {        
                       memset(buffer,0,sizeof(buffer));
                       printf("\n File exists");
                       status = setsockopt(client_socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&opt_time_set,sizeof(struct timeval));
                       if(status<0)
                                printf("Set of setsockopt failed");
      			// getting the file size and calculating the packet count		       
                       fseek(file_put_server, 0 , SEEK_END);
                       int file_size = ftell(file_put_server);
                       fseek(file_put_server, 0, SEEK_SET);
                       printf("\n filesize %d", file_size);
                       if(file_size%BUF_SIZE==0)
			      packet_count = (file_size / BUF_SIZE); 
		       else	
			      packet_count = (file_size / BUF_SIZE)+1;
                                        
                       printf ("packet count to send %d \n", packet_count);
		      // sending the packet count that needs to be sent to the server and receiving the acknowledgement 
                       status = sendto(client_socket , &packet_count , sizeof(int), 0, (struct sockaddr *) &server_address, length);
                                        
                       if(status<0)
                              error("ERROR:writing");
				
R:			status = recvfrom(client_socket, &ACK, sizeof(int), 0 , (struct sockaddr *) &server_address,(socklen_t *) &length);

		 	if((ACK != packet_count) && (NACK<100))
			{
				// resending the packet count value
				status = sendto(client_socket , &packet_count , sizeof(int), 0, (struct sockaddr *) &server_address, length);
				if(NACK == 99)
				{
					printf("\n File not sent");
					fail_count++;		
				}	
				NACK++;
				printf("\nResending the packet count: %d ",NACK);
				goto R;
			}	
					
                        for(long int index=0; index<packet_count; index++)
                        {
				memset(&frame1,0,sizeof(frame1)); 
				frame1.seq_no = index;
				NACK = 0;	
				int file_size = fread(frame1.buffer,1,BUF_SIZE,file_put_server); 
                                               	
				frame1.frame_size = file_size; 
				// sending the frame and receving the acknowledgement
				status = sendto(client_socket , &frame1, sizeof(frame1), 0, (struct sockaddr *) &server_address, length);
FA:				status = recvfrom(client_socket, &ACK, sizeof(int), 0 , (struct sockaddr *) &server_address,(socklen_t *) &length);
				if(ACK != frame1.seq_no && NACK <100)
				{
					// resending the frame
					status = sendto(client_socket , &frame1, sizeof(frame), 0, (struct sockaddr *) &server_address, length);		
					if(NACK == 99)
					{
						printf("\n The frame is dropped %ld:", frame1.seq_no);
						fail_count++;	
					}								
					printf("\nResending the frame:%d ",NACK);
					NACK++;	
					goto FA;
				}	
				else if (ACK == frame1.seq_no)
				{

					printf("\n ACK received & Frame sent: %ld ", frame1.seq_no);

				}

                        }
			if(fail_count != 0)
                                printf ("\n the file has not  been sent successfully");
			else
				printf ("\n The file sent successfully");
					
                       fclose(file_put_server);    
		      status = setsockopt(client_socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&opt_time_reset,sizeof(struct timeval));
                       if(status<0)
                                printf("Reset of setsockopt failed"); 
                } 
		memset(buffer,0,sizeof(buffer));
	}

	// case for delete operation
	else if((strcmp(command_choice,"delete")==0) && (file_name != NULL))
	{
		memset(buffer,0,sizeof(buffer));
                status = recvfrom(client_socket, buffer ,sizeof(buffer),0, (struct sockaddr *) &server_address,(socklen_t *) &length);    
                printf("\n The Message from server is: %s ", buffer);   
	}
	
	// case for listing the files on server
	else if (strcmp(command_choice,"ls")==0)
	{
		printf("\n Entered the case to list the file names: \n");
                memset(buffer,0,sizeof(buffer));
                status = recvfrom(client_socket, buffer ,sizeof(buffer),0, (struct sockaddr *) &server_address,(socklen_t *) &length);                    
                printf("\n The list of files are: %s " , buffer);
	}

	// case to exit
	else if(strcmp(command_choice,"exit")==0) 
	{
		goto Q;

	}
	// case to check invalid options		
	else
	{
		printf("\n Alert: Please enter a valid option");
	}

	
}
Q: close(client_socket);
        return 0;
}
	
	

