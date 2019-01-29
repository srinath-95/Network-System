README file for UDP Socket Programming

For UDP socket programming, I have built two C programs, one acts as the server and other as client.

At Client:
1. The user can give the desired operation of command that he can do.
2. In this program I have given the user to do 5 operations
	(i) To put a file from client to server.
	(ii) To get a file from server to client.
	(iii) To delete an existing file on server.
	(iv) To list the file in the server directory.
	(v) To exit from the program at both server and client end.
3. The input for the above command are like below
	(i) get [filename]
	(ii) put [filename]
	(iii) delete [filename]
	(iv) ls
	(v) exit
4. For any commands other than above, it would throw an alert statement to enter a valid option.
5. Reliable transfer is implemented for file transfer.


At Server:
1. The server accepts all the commands entered in the client end and does the desired operation as per the user command.
2. For any command out of the desired command, a corresponding messages can be viewed at client side and user can act accordingly.
3. Also for reliable transfer of files, a timing has been set so that the operation of commands happen reliably.
4. The server will exit only when it reads exit from client, if not it would be in loop and would wait for command from client.


How to run:
Following are the commands to use to run the file.

Server:
--> make server
--> ./serv [portno]

Client: 
--> make client
--> ./client [serverIP] [portno]