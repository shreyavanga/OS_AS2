#include <bits/stdc++.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include<string>
#include<arpa/inet.h>
#include <sys/time.h>
// #define PORT 8080
// #define NI_MAXHOST 1024
// #define NI_MAXSERV 32
using namespace std;

struct clientdata{
  string ip;
  int port;
};


int checkexistence(struct clientdata data){
  cout<<"exists";
  return 1;
}


int main(){

  //vector<struct> v[10];
  struct clientdata data[10];
  char str[10];

  int opt = 1;

  int PORT;
  cout<<"enter port";
  cin>>PORT;
  //create a socket
  int server_sock;
  server_sock = socket(AF_INET,SOCK_STREAM,0);
  if(server_sock<0){
    perror("Socket creation failed");
    exit(1);
  }
  struct sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(PORT);
//  hint.sin_addr.s_addr = INADDR_ANY;
  int addrlen = sizeof(hint);
  // sprintf(str,"%" PRIu32,hint.sin_addr.s_addr);

  inet_pton(AF_INET, "127.0.0.1", &hint.sin_addr);
//  hint.sin_addr.s_addr = INADDR_ANY;

  //bind the socket to an ip/PORT

  if(bind(server_sock,(sockaddr*)&hint,sizeof(hint)) == -1){
    perror("error while binding");
    exit(1);
  }
  if(listen(server_sock,SOMAXCONN) == -1){
    perror("error occured while listening");
    exit(1);
  }

  //mark the socket for listening
   struct sockaddr_in client;
   int clientlen = sizeof(client);
  // socklen_t clientSize = sizeof(client);
  char host[NI_MAXHOST];
  char svc[NI_MAXSERV];

  int clientSocket;
  clientSocket = accept(server_sock,(struct sockaddr*)&client,(socklen_t*)&clientlen);
  if(clientSocket <0){
    perror("couldn't establish connection");
    exit(1);
  }
  //  struct clientdata data;
  char *clientip = inet_ntoa(client.sin_addr);
  //char *clientport = (char*)PORT;
  //cout<<ip;
  //string clientport = to_string(PORT);
  int clientport = PORT;
  cout<<"string  = "<<clientip<<endl;
  //  cout<<"string  = "<<clientport<<endl;
  //close the listening socket
  data.ip = clientip;
  data.port = clientport;
  cout<<data.ip<<"  "<<data.port<<endl;
  //v.push_back(data);
memset(host,0,NI_MAXHOST);
memset(svc,0,NI_MAXSERV);
  //while receiving,display message
  char buf[4096];
  // FILE *fp;
  // fp = fopen("/home/ubuntu/Desktop/Desktop/testing.txt","wb");
  // int file_size;
  // int byteRec = recv(clientSocket,&file_size,sizeof(file_size),0);
  // int n;
  // while((n = recv(clientSocket,buf,1024,0))>0 && file_size>0){
  //   fwrite(buf,sizeof(char), n, fp);
  //   memset(buf,'\0',1024);
  //   file_size = file_size-n;
  // }
  /*file sending perfect*/
/*  FILE *fp;
  fp = fopen("/home/ubuntu/Desktop/Desktop/ss.txt","rb");
  fseek(fp,0,SEEK_END);
  int size = ftell(fp);
  rewind(fp);
  int serversend = send(clientSocket,&size,sizeof(60),0);
  int n;
  while((n=fread(buf,sizeof(char),1024,fp))>0 && size>0){
    serversend = send(clientSocket,buf,n,0);
      if(serversend<0){
        perror("couldnt send data to server");
        continue;
      }
      memset(buf,'\0',1024);
      size = size - n;
  }
  fclose(fp);*/
  /*file sending perfect till here*/
  while(true){
    memset(buf,0,4096);
    int byteRec = recv(clientSocket,buf,4096,0);
    if(byteRec < 0){
      perror("connection issue");
      break;
    }
    if(byteRec == 0){
      perror("client disconnected ");
      break;
    }
    cout<<"received : "<<string(buf,0,byteRec)<<endl;
    if(strcmp(buf,"checkexistence")){
      //call the checkexistencefucntion
      cout<<"checking for existence";
      int check = checkexistence(data);
        send(clientSocket,check,sizeof(check),0);
    }

    send(clientSocket,buf,byteRec+1,0);
  }

  //close socket
    close(clientSocket);
close(server_sock);


return 0;
}
