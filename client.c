/******************************************************************************
        To compile:             make all
        To run:                 ./client <hostname> <portno>

        Author:                 Ryan Bolesta
        Due Date:               December 5, 2016
        Course:                 CSC328
        Professor Name:         Dr. Frye
        Assignment:             Client-Server Chat Application
        Filename:               client.c
        Purpose:		The client takes an optional hostname and port
				number command line arg and connects to the server.
				After getting the initial HELLO from the server,
				the client infinitely reads and writes to and from
				the server until the user enters the BYE message,
				or the server shuts down. The message entered by
				the user will echo to all clients.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#define SIZE 200
#define NAMESIZE 12
#define MSGSIZE 215
#define IPADDR   "156.12.127.18"  // csit

/*
        Function Name:          getInput
        Description:            Allows nonblocking input for the chat.
				When read returns an error, the function checks
				if the errno = EAGAIN, where it will then call
				the checkServer function to check if the server
				has any incoming messages for the client.
        Parameters:             int fd - file descriptor of the server
				char* ptr - message buffer
				int maxlen - size of the input
        Returns:                n/a
*/
void getInput(int fd, char* ptr, int maxlen);

/*
        Function Name:          checkServer
        Description:            Allows for nonblocking read from the server.
				Server checks if there is anything, if not,
				it breaks from the loop and exits the function.
				If there is something to read, it reads it and
				checks for the shutdown message.
        Parameters:             int fd - file descriptor of the server
        Returns:                n/a
*/
void checkServer(int fd);


int validName;
char NICK[NAMESIZE];

int main(int argc, char **argv) {
          int sockfd;
          struct sockaddr_in server;
          int t;
          struct hostent *host;
          char str;
          char buf[SIZE];
          char rcv[MSGSIZE];
          char msg[MSGSIZE+2];
          char header[NAMESIZE+2];
          validName = 0;
          char* hostname = "acad"; //default hostname
          int portno = 15045;		//default port
          char nickStatus[NAMESIZE]; //default nickname status
          int fd;

          switch(argc){
                  case 1:
                        break;
                  case 3: //user entered port # and hostname
                        portno = atoi(argv[2]);
                  case 2:
                        hostname = argv[1];
                        break;
                default:
                        printf("Usage: <Executable> <Hostname> <Portno>\n");  //usage clause

          }


          host = gethostbyname(hostname);
          if (host == NULL) {
                  fprintf(stderr,"**ERROR** hostname %s does not exist \n", hostname);
                  exit(0);
          }

          server.sin_family = AF_INET;
          server.sin_port = htons(portno);
          bcopy((char *)host->h_addr, (char *)&server.sin_addr.s_addr, host->h_length);

          if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            {
              perror("socket call failed");
              exit(-1);
            }   // end if

          // connect the socket to the server's address
          if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1)
            {
              perror("connect call failed");
              exit(-1);
            }   // end if


          bzero(msg, MSGSIZE);
          if(readn(sockfd, msg, 5) < 0){
                  perror("ERROR recving from socket");
        	  exit(0);
          }
          if(strcmp("HELLO", msg) != 0){
                  printf("Client did not receive HELLO from server");
                  exit(-1);
          }
          printf("Successful connection to server! Enter nickname\n");
          bzero(NICK, NAMESIZE);
          getInput(sockfd, NICK, NAMESIZE);

        	//Nickname loop
          while(1){
                    if(writen(sockfd, NICK, strlen(NICK)) < 0)
                        perror("ERROR writing to socket");

                    if(readn(sockfd, nickStatus, 5) < 0)
                        perror("ERROR n reading from socket");

                    if(strcmp(nickStatus, "READY") == 0)
                    {
                        NICK[strlen(NICK)-1] = '\0';
        		validName = 1;
                        printf("The nickname %s is valid, you may now chat, send BYE to quit\n", NICK);
                        break;
                    }
                    else if(strcmp(nickStatus, "RETRY") == 0){
                        printf("Nickname already chosen, try again:\n");
                        bzero(NICK, NAMESIZE);
        		getInput(sockfd, NICK, NAMESIZE);
                        continue;
                    }
            }

        	//Read/write loop
            while(1){
                    bzero(buf, MSGSIZE);
                    bzero(header, NAMESIZE);
                    strcpy(header, NICK);
                    strcat(header, ": ");
                    getInput(sockfd, buf, MSGSIZE);

                    if(strcmp(buf, "BYE\n") != 0){
                            strcat(header, buf);
                            if(writen(sockfd, header, strlen(header)) < 0){
                                    perror("ERROR writing to socket");
				    exit(0);
			    }
                            printf("Message sent! \n\n");
                    }
                    else{
                            if(writen(sockfd, "BYE\n", 4) < 0){
                                        perror("ERROR writing to socket");
					exit(0);
			    }
                            break;
                    }
            }
            close(sockfd);
            return(0);
}

void getInput(int fd, char* ptr, int maxlen){
	/*
        Function Name:          getInput
        Description:            Allows nonblocking input for the chat.
				When read returns an error, the function checks
				if the errno = EAGAIN, where it will then call
				the checkServer function to check if the server
				has any incoming messages for the client.
        Parameters:             int fd - file descriptor of the server
				char* ptr - message buffer
				int maxlen - size of the input
        Returns:                n/a
	*/
        fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK); //unblock
        for(;;){
                if(readline(0, ptr, maxlen) == -1){
                        if(errno == EAGAIN){
                                checkServer(fd);
                                continue;
                        }
                        else
                        perror("error reading input");
                }
                else
                    break;
            }
           fcntl(0, F_SETFL,0); //reblock

}

void checkServer(int fd){
	/*
        Function Name:          checkServer
        Description:            Allows for nonblocking read from the server.
				Server checks if there is anything, if not,
				it breaks from the loop and exits the function.
				If there is something to read, it reads it and
				checks for the shutdown message.
        Parameters:             int fd - file descriptor of the server
        Returns:                n/a
        */

        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK); //unblock
        char buf[MSGSIZE];
        bzero(buf, MSGSIZE);

        if(readline(fd, buf, MSGSIZE) == -1){
                if(errno != EAGAIN){
                        perror("error reading from socket");
                        exit(0);
                }
        }
        else{
                if(strcmp(buf, "Server shutting down in 5 seconds\n") == 0){
			printf("%s",buf);
			fcntl(fd, F_SETFL, 0);
                        if(writen(fd, "SHUTDOWN\n", 9) < 0){
                                    perror("ERROR writing to socket");
				    exit(0);
			}
                        close(fd);
		        printf("Client now closed \n");
                        exit(0);
                }
                else if(validName != 0)
                        printf("%s",buf);
        }
        fcntl(fd, F_SETFL, 0); //reblock
}
