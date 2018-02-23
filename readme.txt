******************************************************************************
        Author:                 Ryan Bolesta
        Due Date:               December 5, 2016
        Course:                 CSC328
        Professor Name:         Dr. Frye
        Assignment:             Client-Server Chat Application
	Filename:		readme.txt
*******************************************************************************

FILE/FOLDER MANIFEST
*******************************************************************************
	server.c - The server for the TCP chat application. Works by first creating 
			   a socket using socket, bind, and listen. Then on each return of 
			   the accept call, a thread is created to handle that specific 
			   client. The client's unique nickname(that is checked) and sockfd 
			   are stored in global arrays in order for client threads to access 
			   the information. Each thread loops continuosly until either a 
			   ctrl-c or a BYE from the client to end the thread.
			   
	client.c - The client for the TCP chat application. takes an optional hostname 
			   and port number command line arg and connects to the server.
			   After getting the initial HELLO from the server, the client 
			   infinitely reads and writes to and from the server until the user 
			   enters the BYE message, or the server shuts down. The message entered by 
			   the user will echo to all clients. 
	
	utils.c - used utility functions readn, readline, and writen to ensure all bytes
			  were received.
			
	makefile - used to compile and link chat application files.
			  

HOW TO BUILD AND RUN CLIENTS
********************************************************************************
	1.) Compile
				make all

	2.) Run server (optional command line arguments)
				./server <portnumber>

	3.) Run clients (optional command line arguments) 10 client maximum
				./client <hostname> <portnumber>
				
	4.) Enter nickname on client(assumes you wont enter SHUTDOWN as your name)
				Two possible outcomes
					1.) RETRY - nickname not unique, try again
					2.) READY - nickname unique, ready to cha
t
	5.) You may now chat. Nothing happens when you send a message unless
		multiple clients are connected. Type "BYE" to exit the client.
		
	6.) CTRL-C server shutdown - within the server, ctrl-c will gracefully
		close all client programs and then the server.
		
TEST PLAN
**********************************************************************************
	Cases:
		1.) Ctrl-C shutdown with multiple clients. Have some still being prompted 
		    for nicknames.
			
		2.) BYE shutdown on a client, then connect a new client to make sure it
		    connects properly, and can take the old client's name.
		
		3.) Test on different hostname and port numbers.
		
		4.) Check for unique nickname
		
		5.) Make sure entirety of all messages get printed to all client.
		

DISCUSSION
***********************************************************************************
	I had to make several drastic design changes while constructing this program.
	I started the program by blindly doing processes because I always assumed they'd
	be easier to work with. When I encountered the issue of needing shared memory
	I had to transition the whole server to threads. 
	
	Another major problem I had was dealing with the blocking input on the client. 
	When the server ends on a ctrl-c it was extremely difficult to end the client 
	gracefully because it was blocked on fgets until it returned by keyboard input.
	I resolved this by using fctrl to unblock read. 
	
	I also had several extremely obscure errors that really just made no sense. 
	For example, at one point, simply declaring a pointer to a hostent struct, made
	my nickname verification cause error. Several of these weird errors frustrated me 
	to the point of just transitioning to a single client server and client. As i was
	doing the transition, I realized a more effective way to do multi-client by not
        even using threads or processes at all in the client and switched back.
			
