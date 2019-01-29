#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
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

int portno=0;
int time_out=0;
int server_socket=0;
int client_socket=0;
int host_socket=-1;
int host_fd = 0;
int set = 1;
int length = 0;
int status =0;
int file_size =0;
int block_status =0;
int ip_block_status =0;
int mins = 0;
int secs =0;
int diff = 0;

char command [5];
char file_name[500];
char version[20];
char file_name_dir[500];
char filname2[50];
char hostname [500];
struct sockaddr_in server_address,client_address, host_address;
struct hostent *hostbyname;
struct in_addr **hostname_list;
struct tm *timeinfo;

//time_t rawtime = time(NULL);

char buffer[BUF_SIZE];
char buffer_recv[BUF_SIZE];
char buffer_read[4096];

char *filename = NULL;
char *folderdir = NULL;
char *url_name = NULL;
char *dom_name = NULL;
char *ip_exists = NULL;
char *cache_exists = NULL;
char *ip_cache_exists = NULL;
char *ext = NULL;
char *time_ptr = NULL;
//char *buffer_read = NULL;
char  IP[20];
char webpage[100];
char domain_name[30];
char domain_name2[30];
char present_time[50];
char web_return[50];

const char *domain[5];


FILE *fil2;
FILE *fil;  
FILE *fil_block;
FILE *fil_ip;


// Establish connection with browser
void connection(int portno)
{

length = sizeof(struct sockaddr_in);
server_socket = socket(AF_INET,SOCK_STREAM,0);
if (server_socket<0)
       printf("\n ERROR:Opening socket");

setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&set,sizeof(int));
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

}

int block_check()
{
	int file_size_block = 0;
	char *block_exists = NULL;
	//char *ip_block_exists = NULL;
	char buffer_read_block[1024];
 	//char web_return[50];
	memset(buffer_read_block,0,1024);
	fil_block =  fopen("blocked.txt","r");
        if(fil_block != NULL)
        {
		printf("\n Entered Blocked block");	
	
		fseek(fil_block,0,SEEK_END);
        	file_size_block = ftell(fil_block);
        	printf("\n file_size_check of blocked file: %d",file_size_block);
        	fseek(fil_block,0,SEEK_SET);
        	fread(buffer_read_block,1,file_size_block,fil_block);
        	block_exists = strstr(buffer_read_block,domain_name);
		fclose(fil_block);
		if(block_exists != NULL) 
		{	
			printf("\n blocked");
			memset(web_return,0,50);
                        status = write(client_socket,"HTTP/1.1 403 Forbidden Error",strlen("HTTP/1.1 403 Forbidden Error"));
                        printf("\n The status value is %d:", status);
                        char web_return2[]="<html><body><font size =6><b>HTTP/1.1 403 Forbidden Error<b></font></body></html>";
                        int web_ret_length=strlen(web_return2);

                        sprintf(web_return,"Content-length:%d",web_ret_length);
                        write(client_socket,web_return,strlen(web_return));
                        write(client_socket,"Content-Type: html\n\n",strlen("Content-Type: html\n\n"));
                        write(client_socket,"<html><body><font size =6><b>HTTP/1.1 403 Forbidden Error<b></font></body></html>",strlen("<html><body><font size =6><b>HTTP/1.1 403 Forbidden Error<b></font></body></html>"));

			return 1;
		}	
		else
			return 0;
	}
	else
	{
		printf("\n Problem in opening block file");
		return 1;
	}
}

// Check if IP is blocked
int ip_block_check()
{
        int  ip_file_size_block = 0;
        char *ip_block_exists = NULL;
        char ip_buffer_read_block[1024];
        memset(ip_buffer_read_block,0,1024);
        fil_block =  fopen("blocked.txt","r");
        if(fil_block != NULL)
        {
                printf("\n Entered IP Blocked block");

                fseek(fil_block,0,SEEK_END);
                ip_file_size_block = ftell(fil_block);
                printf("\n file_size_check of blocked file: %d",ip_file_size_block);
                fseek(fil_block,0,SEEK_SET);
                fread(ip_buffer_read_block,1,ip_file_size_block,fil_block);
                printf("\n The IP addr is:%s",IP);
                ip_block_exists = strstr(ip_buffer_read_block,IP);
                fclose(fil_block);
                if(ip_block_exists != NULL)
                {
                        printf("\n ip_block_check blocked");
                        memset(web_return,0,50);
                        status = write(client_socket,"HTTP/1.1 403 Forbidden Error",strlen("HTTP/1.1 403 Forbidden Error"));
                        printf("\n The status value is %d:", status);
                        char web_return2[]="<html><body><font size =6><b>HTTP/1.1 403 Forbidden Error<b></font></body></html>";
                        int web_ret_length=strlen(web_return2);

                        sprintf(web_return,"Content-length:%d",web_ret_length);
                        write(client_socket,web_return,strlen(web_return));
                        write(client_socket,"Content-Type: html\n\n",strlen("Content-Type: html\n\n"));
                        write(client_socket,"<html><body><font size =6><b>HTTP/1.1 403 Forbidden Error<b></font></body></html>",strlen("<html><body><font size =6><b>HTTP/1.1 403 Forbidden Error<b></font></body></html>"));

                        return 1;

                }
                else
		{
                       printf("\n check, exit from ip block");
		       	return 0;
        
		}
	}
	else
        {
                printf("\n Problem in opening block file");
                return 1;
        }
}

// getting the data from local cache
void getdatafromcach()
{
	int file_size_read = 0;
	char *ptr3 = NULL;
        ptr3 = strstr(file_name,"/");
        printf("\n ptr3: %s", ptr3);
	
	printf("\n Web present in cached data");
	printf("\n ------- Getting data from cache");
	printf("\n the file name is: %s", file_name);
	strcpy(filname2,"./");
        strcat(filname2,file_name);

	if(strcmp(ptr3,"/")==0)
                strcat(filname2,"index.html");

	printf(" \n The file to open is: %s",filname2);
	fil2 = fopen(filname2,"r");
	if(fil2 != NULL)
	{
		file_size = 0;
		fseek(fil2,0,SEEK_END);
		file_size = ftell(fil2);
		printf("\n file_size: %d",file_size);
		fseek(fil2,0,SEEK_SET);

		//char *send_buffer = malloc(file_size);
		char send_buffer[file_size];

		if(send_buffer == NULL)
			printf("\n ERROR: Malloc");
		printf("\n The size of send buffer is: %ld",sizeof(send_buffer));
		file_size_read = fread(send_buffer,1,file_size,fil2);
		printf("\n The file size read is:%d",file_size_read);
		printf("\n The file content is %s",send_buffer);
		//printf("\n The length of send_buffer is:%ld",strlen(send_buffer));
		status = write(client_socket,send_buffer,file_size);
		if(status<0)
			printf("\n File not sent properly");
		else
			printf("\n File sent successfully");

		memset(send_buffer,0,sizeof(send_buffer));
		//free(send_buffer);
		fclose(fil2);
	}
	else
		printf("\n File does not exist");
}

// Getting IP address using gethostbyname
int getIP()
{
	hostbyname = gethostbyname(domain_name);
	if(hostbyname != NULL)
	{
		printf("\n ---------Entered to get the IP");
		hostname_list = (struct in_addr **) hostbyname->h_addr_list;
		for(int i =0; hostname_list[i] != NULL; i++)
		{
		 strcpy(IP, inet_ntoa(*hostname_list[i]));
		 printf("\nThe IP addr is :%s", IP);
		}
		return 1;
	}
	else
	{
		printf("\n ---------- ERROR: getting host name");
		memset(web_return,0,50);
                status = write(client_socket,"HTTP/1.1 403 Hostname not resolved",strlen("HTTP/1.1 403 Hostname not resolved"));
                printf("\n The status value is %d:", status);
                char web_return_bad[]="<html><body><font size =6><b>HTTP/1.1 403 Hostname not resolved<b></font></body></html>";
                int web_ret_length=strlen(web_return_bad);

                sprintf(web_return,"Content-length:%d",web_ret_length);
                write(client_socket,web_return,strlen(web_return));
                write(client_socket,"Content-Type: html\n\n",strlen("Content-Type: html\n\n"));
                write(client_socket,"<html><body><font size =6><b>HTTP/1.1 403 Hostname not resolved<b></font></body></html>",strlen("<html><body><font size =6><b>HTTP/1.1 403 Hostname not resolved<b></font></body></html>"));
		return 0;
	}
}

// Establishing connection with http server
void connecthostserver()
{
	int status1 = 1;
	char bufftemp[BUF_SIZE];
  	char mk_dir[20];	
	printf("\n---- Connecting to HTTP server");
	memset(bufftemp,0,1024);
	bzero(&host_address,sizeof(host_address));
	host_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	host_address.sin_family = AF_INET;
	host_address.sin_port = htons(80);
	host_address.sin_addr.s_addr = inet_addr(IP);
	
	host_socket = connect(host_fd,(struct sockaddr*)&host_address,sizeof(struct sockaddr));
	if(host_socket<0)
	{
		printf("\n Connection not success with http server");
		printf("\n ---------- ERROR: Connection with Server");
                memset(web_return,0,50);
                status = write(client_socket,"HTTP/1.1 ERROR: Connection with Server",strlen("HTTP/1.1 ERROR: Connection with Server"));
                printf("\n The status value is %d:", status);
                char web_return_bad[]="<html><body><font size =6><b>HTTP/1.1 ERROR: Connection with Server<b></font></body></html>";
                int web_ret_length=strlen(web_return_bad);

                sprintf(web_return,"Content-length:%d",web_ret_length);
                write(client_socket,web_return,strlen(web_return));
                write(client_socket,"Content-Type: html\n\n",strlen("Content-Type: html\n\n"));
                write(client_socket,"<html><body><font size =6><b>HTTP/1.1 ERROR: Connection with Server<b></font></body></html>",strlen("<html><body><font size =6><b>HTTP/1.1 ERROR: Connection with Server<b></font></body></html>"));
	}	
	printf("\n Entered connect to host server");
	FILE *fil3;
	FILE *fil4;	
	char filname[50];
	char *ptr = NULL;
	char *ptr2 = NULL;
	char *browser_check = NULL;
		
	char file_buffer[200];
	int i =0;
	time_t rawtime = time(NULL);
	time (&rawtime);
        timeinfo = localtime (&rawtime);
	if(cache_exists!= NULL)
	{
		printf("\n CHECK");
		sprintf(file_buffer,"\n%s %d %d",file_name,timeinfo->tm_min ,timeinfo->tm_sec);
		fil4 = fopen("cached.txt","w+");
		if(fil4 != NULL)
		{
			while(file_buffer[i])
			{
				*cache_exists = file_buffer[i];
		                i++;
                		cache_exists++;
			}
			
			fprintf(fil4, "\n%s",buffer_read);
			printf("\n success in writing");
			//free(buffer_read);
			//buffer_read = NULL;
			fclose(fil4);
		}
		else
		{
			printf("\n Problem in filopen in write mode");
			//free(buffer_read);
                        //buffer_read = NULL;

		}
			//fclose(fil4);
	}	
	//writing to cache file
	else
	{
		//free(buffer_read);
                //buffer_read = NULL;
		fprintf(fil,"\n%s %d %d" ,file_name,timeinfo->tm_min ,timeinfo->tm_sec);
	}
	ptr2 = strrchr(file_name,'/');
	printf("\n ptr2: %s", ptr2);
	strcpy(file_name_dir,file_name);	
	ptr = strrchr(file_name_dir,'/');
	*ptr = '\0';

	printf("\n The path of the directory is: %s",file_name_dir);
	sprintf(mk_dir,"mkdir -p %s",file_name_dir);
	system(mk_dir);
	
	printf("\n ckeck");
	strcpy(filname,"./");
	strcat(filname,file_name);
	browser_check = strstr(buffer,"Accept");
	if(browser_check != NULL)
		strcpy(bufftemp,buffer);
	else		
		sprintf(bufftemp,"GET %s HTTP/1.1\nHost: %s\nConnection: close\n\n",ptr2,domain_name);
	
	if(strcmp(ptr2,"/")==0)
		strcat(filname,"index.html");
	printf("\n the file to be created is: %s", filname);  	
	fil3 = fopen(filname,"w");
	memset(buffer_recv,0,1024);
	if(fil3 != NULL)
	{
		printf("\n opened File");
		printf("\n the request to send is %s",bufftemp);
		status = send(host_fd,bufftemp,strlen(bufftemp),0);
		if(status<0)
                       printf("ERROR: Sending request to server");
	               
	       printf("\n -------- Getting data from HTTP server");	
       	        do{
               		status1 = recv(host_fd,buffer_recv,1024,0);
                	printf("\n the status val is %d",status1);
                	//printf("\n the buffer content recv is: %s", buffer_recv);
			fwrite(buffer_recv,1,status1,fil3);
			if(!(status1<=0))
				status = send(client_socket,buffer_recv,status1,0);
			printf("\n----- the status val while writing is %d",status);
			memset(buffer_recv,0,1024);
               	}while(status1 >0);
		if(status1 <0)
			printf("\n ERROR");	
		fclose(fil3);
	}
	else
		printf("\n Problem in opening file");
	close(host_fd);
}

//getting IP from cache
int IP_cache()
{
	printf("\n Entered the IP_cache block");
	FILE *fil_ip;
	int file_size = 0;
	char buffer_ip_cache[4096];
	memset(buffer_ip_cache,0,4096);
	fil_ip = fopen("IP_cache.txt","r");
	if(fil_ip != NULL)
	{	
		fseek(fil_ip,0,SEEK_END);
        	file_size = ftell(fil_ip);
        	printf("\n file_size: %d",file_size);
        	fseek(fil_ip,0,SEEK_SET);
		fread(buffer_ip_cache,1,file_size,fil_ip);
                ip_cache_exists = strstr(buffer_ip_cache,domain_name);
		fclose(fil_ip);
		if(ip_cache_exists != NULL)
		{
			sscanf(ip_cache_exists,"%*s %s",IP);
			return 1;

		}
		else 
			return 0;
	}
		return 0;
}


int main(int argc, char *argv[]) {

	portno = atoi(argv[1]);
	time_out = atoi(argv[2]);
	connection(portno);
while(1)
{
	// accept connection from client
	while((client_socket = accept(server_socket, (struct sockaddr *) &client_address,(socklen_t*) &length))>0)
	{
        	if(client_socket == -1)
                        printf("ERROR: Connection failed");
                printf("\n Client Connection successful\n");

                printf("\n The client sock value is:%d",client_socket);

		if(!fork())
                {
                        // child process, closing server socket
                        close(server_socket);
                        memset(buffer,0,1024);

                        // receive the request from client
                        recv(client_socket,buffer,1024,0);
                        memset(command,0,5);
                        memset(file_name,0,500);
			memset(hostname,0,500);
			memset(domain_name,0,30);
			memset(version,0,20);
			memset(buffer_read,0,4096);
			printf("\n The received data is :%s",buffer);
                        //seperating the Method, URL and the version
                        //strcpy(buffer,buffer_recv);
                        sscanf(buffer,"%s%s%s",command,hostname,version);
			printf("\n The command is:%s",command);
			printf("\n The hostname is:%s",hostname);
			printf("\n The version is:%s",version);

			// Processing only GET requests	
			if((strcmp(command,"GET")==0) && ((strcmp(version,"HTTP/1.0")==0) || (strcmp(version,"HTTP/1.1")==0)) && (strncmp(hostname,"http://",7)==0))
			{
				//const char *a[2];
				int index = 0;
				int fil_size =0;
				int ip_status =0;
				int ip_cache_status = 0;
				int count = 0;
				int flag = 0;
				int flag2 = 0;
				printf("\n Entered GET command");
				printf("\n the URL is %s",hostname);
			
				filename = strstr(hostname,"//");
				filename++;
				filename++;
				//printf("\n the new file name is: %s",filename);
				strcpy(file_name,filename);
				
				folderdir = strrchr(file_name,'/');
				if(folderdir != NULL)
				{	
					folderdir++;
					printf("\n The folder directory is:%s",folderdir);
				}
				else 
				{	
					int len = strlen(file_name);
					file_name[len] = '/';
					file_name[len+1] = '\0';

					folderdir = strrchr(file_name,'/');
					folderdir++;
					printf("The folder directory is:%s",folderdir);
				}	
				url_name = strtok(filename,"//");
				printf("\n the new file name is: %s",file_name);
				while(url_name!=NULL)
                                { 
                                 	printf("\n The url name is: %s", url_name);
				 	domain[index] = url_name;
					url_name = strtok(NULL,"//");
                                 	index++;
                                }
                                strcpy(domain_name,domain[0]);
                                printf("\n The domain name is: %s", domain_name);
				memset(IP,0,20);
				
				char *pch = strchr(domain_name,'.');
				while(pch!=NULL){
					count++; 
					pch = strchr(pch+1,'.');
				}	
				
				if(domain_name[0]>=48 && domain_name[0]<=57)
				{
					flag2 = 1;
				}
				if(count == 3 && flag2 ==1)
				{
					strcpy(IP,domain_name);
					printf("\n The IP from bowser is:%s",IP);
					ip_status = 1;
					flag =1;
				}
				
				block_status = block_check();

				if(block_status == 1)
					printf("This site is blocked");
				else
				{	
					printf("\n This site is not blocked");
					if(flag == 0)
					{
						ip_cache_status = IP_cache();
						printf("\n ip_cache_status is:%d",ip_cache_status);
					}
					if(flag == 1)
						printf("\n IP success");
					if(ip_cache_status == 1 && flag ==0)
					{
						printf("\n ip_exists in ip_cache");
						ip_status = 1;
					}
					else if(flag == 0)
					{
						ip_status = getIP();
						printf("\nThe IP address is: %s",IP);
						fil_ip = fopen("IP_cache.txt","a+");
						if(fil_ip != NULL){
						fprintf(fil_ip,"\n%s %s",domain_name,IP);
						fclose(fil_ip);
						}
						else
							printf("\n Problem in file open");
						
					}
					else
						printf("\n ERRROR");
					if(ip_status == 1)
					{	
						printf("\n2 The IP address is: %s",IP);

				        	ip_block_status= ip_block_check();
						if(ip_block_status == 1)
                                        		printf("This site is blocked");
                                		else
                                		{
						fil = fopen("cached.txt", "a+");
						if(fil != NULL)
						{
						fseek(fil,0,SEEK_END);
                                                printf("\n check1");
                                                fil_size = ftell(fil);
                                                printf("\n check2");
                                                printf("\n file_size_check: %d",fil_size);
                                                fseek(fil,0,SEEK_SET);
						//buffer_read = (char*)calloc(1,file_size);

                                                fread(buffer_read,1,fil_size,fil);
						int min = 0;
						int sec = 0;
                                                cache_exists = strstr(buffer_read,file_name);
                                                	if(cache_exists != NULL)
                                                	{
							printf("\n WEB Present In CACHE");
							time_t rawtime = time(NULL);
							time (&rawtime);
        						timeinfo = localtime (&rawtime);
							sscanf(cache_exists,"%*s %d %d",&mins, &secs);
							printf("\n mins: %d",mins);
        						printf("\n secs: %d",secs);
							printf("\n the time is: %d",timeinfo->tm_min);
							printf("\n time in sec:%d",timeinfo->tm_sec);
							min = (timeinfo->tm_min);
							sec = (timeinfo->tm_sec);
							diff = (((min)*60 + (sec)) - (mins*60+secs));
        						diff = abs(diff);
							//diff = 60;
        						printf("\n the difference is %d",diff);
							if(time_out == 0)
								time_out =60;
							
							if(diff>time_out)
								{
									fclose(fil);
									connecthostserver();
								}
							else
								getdatafromcach();
                                                	}       
                                                	else
                                                	{
                                                        	printf("\n Web not in cached data");
                                                        	connecthostserver();
								fclose(fil);	
                                                        	getdatafromcach();
							}
						}
						else
							printf("\n Problem in opening cache file");
						//free(buffer_read);
                				//buffer_read = NULL;

					}
				}
				}
				printf("\n check");
			}
			else
			{
                        memset(web_return,0,50);
                        status = write(client_socket,"HTTP/1.1 400 Bad Request",strlen("HTTP/1.1 400 Bad Request"));
                        printf("\n The status value is %d:", status);
                        char web_return_bad[]="<html><body><font size =6><b>HTTP/1.1 400 Bad Request Error<b></font></body></html>";
                        int web_ret_length=strlen(web_return_bad);

                        sprintf(web_return,"Content-length:%d",web_ret_length);
                        write(client_socket,web_return,strlen(web_return));
                        write(client_socket,"Content-Type: html\n\n",strlen("Content-Type: html\n\n"));
                        write(client_socket,"<html><body><font size =6><b>HTTP/1.1 400 Bad Request Error<b></font></body></html>",strlen("<html><body><font size =6><b>HTTP/1.1 400 Bad Request Error<b></font></body></html>"));

			}
		//close(host_fd);
		close(client_socket);
		}
		close(client_socket);
	}
		//close(client_socket);
}

close(server_socket);
return 0;
}
