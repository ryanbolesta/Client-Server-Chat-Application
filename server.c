/******************************************************************************
        To compile:             make all
        To run:                 ./server <portno>

        Author:                 Ryan Bolesta
        Due Date:               December 5, 2016
        Course:                 CSC328
        Professor Name:         Dr. Frye
        Assignment:             Client-Server Chat Application
        Filename:               server.c
        Purpose:                The server for the client chat application works
				by first creating a socket using socket, bind, and
				listen. Then on each return of the accept call,
				a thread is created to handle that specific client.
				The client's unique nickname(that is checked) and
				sockfd are stored in global arrays in order for all
				client threads to access the information. Each thread
			        loops continuosly until either a ctrl-c or a BYE from
				the client to end the thread.
*******************************************************************************/

#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#define SIZE sizeof(struct sockaddr_in)
#define BUFSIZE 215
#define NAMESIZE 12
#define HELLO  "HELLO"
#define RETRY "RETRY"
#define READY  "READY"
#define MAX_CLIENTS 10


/*
        Function Name:          doIt
        Description:            Handles each client thread, implementing the chat protocol.
				Loops endlessly reading and writing until a ctrl-c or BYE
				message from the client.
        Parameters:             void* clientIdx - the index of the client
        Returns:                n/a
*/
void* doIt(void* clientIdx);

/*
        Function Name:          catcher
        Description:            Handles the SIGINT signal on ctrl-c. The function
				sends a shutting down message to all clients, who then
				send a message back to the server in order to exit
				all threads gracefully.
        Parameters:             int sig
        Returns:                n/a
*/
void catcher(int sig);

/*
        Function Name:          newClientConnect
        Description:            Handles a new client connection by sending HELLO
				and ensuring proper unique nickname registration.
        Parameters:             int fd - the sockfd, int fdIdx - the client index
        Returns:                0 - CTRL-C from the server
				1 - Proper nickname registration
*/
int newClientConnect(int fd, int fdIdx);

/*
        Function Name:          nameFinder
        Description:            Searches the nickname array to determine if the
				name entered is already in the array.
        Parameters:             char* name - the name being searched for
        Returns:                0 - The name is unique and can be added
				1 - The name was found in the array
*/
int nameFinder(char* name);


char fd_array[MAX_CLIENTS]; //Global so threads can access other clients
char** name_array; 			//Global so names can be accessed by all threads
int sockfd;					//Global so signal handler can close it
int highestIdx;				//Global because it can change at any time
int numClients;				//Global so it can be altered when a thread closes


int main(int argc, char **argv)
{
  pthread_t tid[MAX_CLIENTS];  // thread id
  int i, t, newsockfd, clientIdx;
  numClients = 0;
  highestIdx = 0;
  int portno = 15045;

  static struct sigaction act;
  act.sa_handler = catcher;
  sigfillset(&(act.sa_mask));
  sigaction(SIGINT, &act, NULL);

  name_array = malloc(MAX_CLIENTS * sizeof(char*));
  for (i = 0; i < MAX_CLIENTS; i++)
          name_array[i] = malloc((NAMESIZE) * sizeof(char));


  switch(argc){
          case 1:
                break;
          case 2:
                portno = atoi(argv[1]);
				break;
        default:
                printf("Usage: <Executable> <Portno>\n");

  }

  struct sockaddr_in server = {AF_INET, htons(portno), INADDR_ANY}; //15045

  // set up the transport end point
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror("socket call failed");
      exit(-1);
    }   // end if


  // bind and address to the end point
  if (bind(sockfd, (struct sockaddr *)&server, SIZE) == -1)
    {
      perror("bind call failed");
      exit(-1);
    }   // end if bind

  // start listening for incoming connections
  if (listen(sockfd, 5) == -1)
    {
      perror("listen call failed");
      exit(-1);
    }   // end if

  for(;;){
        if ((newsockfd = accept(sockfd, NULL, NULL)) == -1)
    	{
    	           perror("accept call failed");
    	           exit(-1);
    	}   // end if

        numClients++;
	for(clientIdx = 0; (clientIdx < highestIdx) && (fd_array[clientIdx] != -1); clientIdx++);
	fd_array[clientIdx] = newsockfd;
        name_array[clientIdx] = "0000";

        if(clientIdx == highestIdx)
                highestIdx++;

	t = pthread_create(&tid[clientIdx], NULL, doIt, &clientIdx);
        if (t){
                printf("Error in pthread_create: %s\n", strerror(t));
                exit(-1);
        }
}
 return(0);

}

void* doIt(void* clientIdx){

	/*
        Function Name:          doIt
        Description:            Handles each client thread, implementing the chat protocol.
				Loops endlessly reading and writing until a ctrl-c or BYE
				message from the client.
        Parameters:             void* clientIdx - the index of the client
        Returns:                n/a
	*/

    int fdi = *((int*)clientIdx);
    int fd = fd_array[fdi];
    int i;
    int byte;
    char buf[BUFSIZE];
    if(newClientConnect(fd, fdi)){

        for(;;){
	      bzero(buf, BUFSIZE);

              if( (byte = read(fd, buf, BUFSIZE)) < 0){
                      perror("ERROR reading from socket");
		      exit(0);
              }
	      buf[byte] = '\0';
              if(strcmp(buf, "BYE\n") == 0)
			break;


              else if(strcmp(buf, "SHUTDOWN\n") == 0)
                        break;

              int clientHolder = highestIdx;
              for(i=0;i<clientHolder;i++){
                      if (fd_array[i] != fd && fd_array[i] != -1)  /*dont send msg to same client*/
                           if(writen(fd_array[i], buf, strlen(buf)) < 0){
                                   perror("ERROR writing to socket");
				   exit(0);
			   }
              }//end for
         }
     }

     printf("Client #%d [%s] has disconnected \n", fdi+1, name_array[fdi]);
     fd_array[fdi] = -1;
     name_array[fdi] = malloc(4 * sizeof(char));
     strcpy(name_array[fdi], "0000");
     numClients--;
     close(fd);
     pthread_exit(0);
}

int newClientConnect(int fd, int fdIdx){
	/*
        Function Name:          newClientConnect
        Description:            Handles a new client connection by sending HELLO
				and ensuring proper unique nickname registration.
        Parameters:             int fd - the sockfd, int fdIdx - the client index
        Returns:                0 - CTRL-C from the server
				1 - Proper nickname registration
	*/
                char name[NAMESIZE];
                if (writen(fd, HELLO, strlen(HELLO))< 0){
                        perror("ERROR writing to socket");
			exit(0);
		}

                int validName = 0;

                while(validName == 0){
                        bzero(name, NAMESIZE);
                        if (readline(fd, name, NAMESIZE) < 0){
                                perror("ERROR reading from socket");
				exit(0);
			}
			if(strcmp(name, "SHUTDOWN\n") == 0)
				return(0);
                        name[strlen(name)-1] = '\0';
                        if(!nameFinder(name)){ //success
                                validName = 1;
				name_array[fdIdx] = malloc(strlen(name) * sizeof(char));
				strcpy(name_array[fdIdx], name);
                                printf("%s (Client #%d) has connected! \n", name_array[fdIdx], fdIdx+1);
                        }
                        else{  //invalid name
                                if(writen(fd, RETRY, 5) < 0){
                                        perror("ERROR writing to socket");
					exit(0);
				}
                        }
                }

                if(writen(fd, READY, 5) < 0){
                        perror("ERROR writing to socket");
			exit(0);
		}
		return(1);
}


int nameFinder(char* name){
	/*
        Function Name:          nameFinder
        Description:            Searches the nickname array to determine if the
				name entered is already in the array.
        Parameters:             char* name - the name being searched for
        Returns:                0 - The name is unique and can be added
			        1 - The name was found in the array
	*/
        int i;
        int max = highestIdx;

        if(highestIdx > 0){
                for(i = 0; i < max; i++){
                        if(strcmp(name_array[i], name) == 0)
                                return(1);
                }
        }

        return(0);
}

void catcher(int sig){
	/*
        Function Name:          catcher
        Description:            Handles the SIGINT signal on ctrl-c. The function
				sends a shutting down message to all clients, who then
				send a message back to the server in order to exit
				all threads gracefully.
        Parameters:             int sig
        Returns:                n/a
	*/

          printf("\nServer received interrupt signal, closing...\n");
          int i;

          for(i=0;i<=highestIdx;i++){
                if (fd_array[i] != -1)
			if(writen(fd_array[i], "Server shutting down in 5 seconds\n", 40) < 0){
				perror("ERROR writing to socket");
				exit(0);
		        }
          }//end for

          sleep(5);
          close(sockfd);
          exit(0);
}   // end function catcher
