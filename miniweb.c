#include <stdio.h>
#include <string.h>  //strlen
#include <sys/socket.h>
#include <arpa/inet.h>  //inet_addr
#include <unistd.h>  //write
#include <fcntl.h>
#include <sys/stat.h>
#include "base64.h"



//Helper functons 

void send404error(char* buffer, int fd){
   int f404 = open("404Response.txt",O_RDONLY);
        int readSize = read(f404,buffer,1023);
        close(f404);
        write(fd,buffer,readSize);
}

int fromHex(char ch) {
  if(ch >= '0' && ch <= '9')
    return (int) ch - '0';
  return (int) ch - 'A' + 10;
}

void decodeURL(char* src,char* dest) {
  while(*src != '\0') {
    if(*src == '%') {
      ++src;
      int n1 = fromHex(*src++);
      int n2 = fromHex(*src++);
      *dest++ = (char) n1*16+n2;
    } else {
      *dest++ = *src++;
    }
  }
  *dest = '\0';
}



void serveRequest(int fd) {
  // Read the request
char buffer[1024];
int bytesRead = read(fd,buffer,1024);
buffer[bytesRead] = '\0';

// Grab the method and URL
char method[128];
char url[128];
sscanf(buffer,"%s %s",method,url);
  


 
  if(strcmp(method, "POST")==0){

    char* linkName = strstr(buffer, "\r\n\r\nurl=");

    if(linkName){
      // escaping the 4 characters that are crap
      linkName = linkName+4;
      char originalURL[1024];
      // decoding the % and others values from url
      decodeURL(linkName, originalURL);

      //adding the links to a file
      FILE* savingFile = fopen("Urls.txt", "a");


      if(savingFile){
              // string data type and new line
        fprintf(savingFile, "%s\n", originalURL);
        

       
        long filePos = ftell(savingFile);
           fclose(savingFile);
        char shrtURL[200];
        encode(filePos,shrtURL);

        char sendBuffer[2000];
        FILE *tempFile = fopen("postTemplate.txt", "r");

        if(tempFile == NULL){

          int f404 = open("posttemperr.txt",O_RDONLY);
        int readSize = read(f404,buffer,1023);
        close(f404);
        write(fd,buffer,readSize);

        } else{
          // if file exists
          int readTemp = fread(buffer, 1,sizeof(sendBuffer),tempFile);
          fclose(tempFile);

          int responseSize = sizeof(sendBuffer);

          if(responseSize>0){

            char* locateX = strstr(sendBuffer, "XXXXXX");

            if(locateX){
              strncpy(locateX, shrtURL, strlen(shrtURL));
              write(fd,sendBuffer, responseSize);
            }

          }

        }

      } else{
       
     int f404 = open("savfileErr.txt",O_RDONLY);
        int readSize = read(f404,buffer,1023);
        close(f404);
        write(fd,buffer,readSize);
      }

    } 
  }else if(strcmp(method, "GET") == 0 && strncmp(url, "/s/", 3)==0){


    char* shturl = url+3;

    unsigned int lastCode = decode (shturl);
    FILE* urlFile = fopen("Urls.txt", "r");


    if(urlFile!= NULL){

      fseek(urlFile, lastCode, SEEK_SET);

      char originalEurl[1000];
      if(fgets(originalEurl, sizeof(originalEurl), urlFile) !=NULL){


        int len  = strlen(originalEurl);

        if(len>0 && originalEurl[len-1]== '\n'){
          originalEurl[len-1] = '\0';
        }

        fclose(urlFile);

        char getResponse[1500];

        snprintf(getResponse, sizeof(getResponse),
        "HTTP/1.1 301 Permanently Moved"
        "location: %s\r\n\r\nurl=", originalEurl
        );

        write(fd, getResponse, strlen(getResponse));
      } else{
        int f404 = open("readingError.txt",O_RDONLY);
        int readSize = read(f404,buffer,1023);
        close(f404);
        write(fd,buffer,readSize);
        fclose(urlFile);

      }

    } else{
      send404error(buffer, fd);
    }

  } else{
    /// add file
  char fileName[128];
  strcpy(fileName,"www");
  strcat(fileName,url);
  int filed = open(fileName,O_RDONLY);

     const char* responseStatus = "HTTP/1.1 200 OK\n";
    const char* responseOther = "Connection: close\nContent-Type: text/html\n";
    // Get the size of the file
    char len[64];
    struct stat st;
    fstat(filed,&st);
    sprintf(len,"Content-Length: %d\n\n",(int) st.st_size);
    // Send the headers
    write(fd,responseStatus,strlen(responseStatus));
    // write(fd,responseOther,strlen(responseOther));
    write(fd,len,strlen(len));
    // Send the file
    while(bytesRead = read(filed,buffer,1023)) {
      write(fd,buffer,bytesRead);
    }
    close(filed);
    

 
    // The user has a requested a file that we don't have.
    // Send them back the canned 404 error response.
    //send404error(buffer, fd);
  }
  
  close(fd);
}


int main() {
  // Create the socket
  int server_socket = socket(AF_INET , SOCK_STREAM , 0);
  if (server_socket == -1) {
    printf("Could not create socket.\n");
    return 1;
  }

  //Prepare the sockaddr_in structure
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons( 8888 );

  // Bind to the port we want to use
  if(bind(server_socket,(struct sockaddr *)&server , sizeof(server)) < 0) {
    printf("Bind failed\n");
    return 1;
  }
  printf("Bind done\n");

  // Mark the socket as a passive socket
  listen(server_socket , 3);

  // Accept incoming connections
  printf("Waiting for incoming connections...\n");
  while(1) {
    struct sockaddr_in client;
    int new_socket , c = sizeof(struct sockaddr_in);
    new_socket = accept(server_socket, (struct sockaddr *) &client, (socklen_t*)&c);
    if(new_socket != -1)
      serveRequest(new_socket);
  }

  return 0;
}
