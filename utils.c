// utils.c
// Lisa Frye
// Utility Functions used with sockets
// Includes functions: readn, readline, writen, str_echo, str_cli


#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 512


/********************************************************/
/* Read "n" bytes from a descriptor.                    */
/* Use in place of read() when fd is a stream socket.   */
/********************************************************/

int readn (fd, ptr, nbytes)
     register   int fd;
     register   char *ptr;
     register   int nbytes;
{
  int nleft,  nread;
  
  nleft = nbytes;
  while (nleft > 0){
    nread = read(fd, ptr, nleft);
    if (nread < 0)
      return (nread);/* error, return < 0 */
    else if (nread == 0)
      break;/* EOF */

    nleft  -=  nread;
    ptr     += nread;
  }
  return (nbytes - nleft);/* return >= 0 */
}  // end function readn



/********************************************************/
/* Write "n" bytes to a descriptor.                     */
/* Use in place of write() when fd is a stream socket.  */
/********************************************************/

int writen (fd, ptr, nbytes)
     register   int fd;
     register   char       *ptr;
     register    int nbytes;
{
  int nleft,  nwritten;
  
  nleft = nbytes;
  while (nleft > 0)   {
    nwritten = write (fd, ptr, nleft);
    if (nwritten <= 0)
      return (nwritten);/* error */

    nleft -= nwritten;
    ptr    += nwritten;
  }
  return (nbytes - nleft);
}   // end function writen


/*********************************************************************/
/* Read a line from a descriptor.  Read the line one byte at a time, */
/* looking for the newline.  We store the newline in the buffer,     */
/* then folloe it with a null (the same as fgets (3)).               */
/* We return the number of characters up to, but not including,      */
/* the null (the same as strlen (3)).                                */
/*********************************************************************/

int readline (fd, ptr, maxlen)
     register   int fd;
     register   char       *ptr;
     register    int maxlen;
{
  int n,  rc;
  char c;

  for (n = 1; n < maxlen; n++)  {
    if (   (rc = read (fd, &c, 1)) == 1)  {
      *ptr++ = c;
      if (c == '\n')
	break;
    }  else if (rc == 0)  {
      if (n == 1)
	return (0);/* EOF, no data read */
      else 
	break;/* EOF, some data was read */

    } else
      return (-1);/* error */
  }
  *ptr = 0;
  return (n);
}  // end function readline


/******************************************************/
/* Read a stream socket one line at a time, and write */
/* each line back to the sender.                      */
/*                                                    */
/* Return when the connection is terminated.          */
/******************************************************/

str_echo(sockfd)
int sockfd;
{
  int n, n2;
  char line[MAXLINE];
  char*temp = (char *)malloc(sizeof(char)*MAXLINE);

  for ( ; ; )  {
    n = readline (sockfd, line, MAXLINE);
    if (n == 0)
      return;       /* connection terminated */
    else if (n < 0)
      perror ("str_echo: readline error");

    //fputs("In str_echo:\n", stdout);
    fputs(line, stdout);

    strcpy(temp, "Server performed action!\n");
    n2 = strlen(temp);
    if (writen ( sockfd, temp, n2) != n2)
      perror ("str_echo: writen error");
  }   // end for
}   // end function str_echo


/***********************************************************************/
/* Read the contents of the FILE *fp, write each line to the           */
/* stream socket ( to the server process),  then read a line back from */
/*                                                                     */
/* Return to the caller when an EOF is encountered on the input file.  */
/***********************************************************************/

str_cli (str, sockfd)
register char   *str;
register int sockfd;
{
  int n;
  char recvline [MAXLINE + 1];

  n = strlen (str);
  if (writen(sockfd, str, n)  !=n)
    perror ("str_cli: writen error on socket");

  /*
   * Now read a line from the socket and write it to 
   * our standard output.
   */

  n = readline (sockfd, recvline, MAXLINE);
  if (n < 0)
    perror("str_cli: readline error");
  recvline[n] = 0;           /* null terminate */

  //fputs ("In str_cli\n", stdout);
  fputs (recvline, stdout);

}   // end function str_cli
