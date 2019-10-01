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
// #define PORT 8080
// #define NI_MAXHOST 1024
// #define NI_MAXSERV 32
using namespace std;

int main(){
  //create a Socket
  int sockfd = socket(AF_INET,SOCK_STREAM,0);
  if(sockfd < 0){
    perror("error while creating socket");
    exit(1);
  }
  //create a hint structure for server we're connecting to
  int port;
  cin>>port;
  // string ipAddress = "127.0.0.1";
  struct sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(port);
  hint.sin_addr.s_addr = INADDR_ANY;
  //inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);
  //connect to server on Socket
  int connRes = connect(sockfd,(struct sockaddr*)&hint,sizeof(hint));
  if(connRes<0){
    perror("couldnt connect with the server");
    exit(1);
  }
  //send data
  /*file sending perfect*/
  char buf[4096];
/*  char buf[4096];
  memset(buf,0,4096);
  FILE *fp;
  fp = fopen("/home/ubuntu/Desktop/Desktop/testing.txt","wb");
  int file_size;
  int byteRec = recv(sockfd,&file_size,sizeof(file_size),0);
  int n;
  while((n = recv(sockfd,buf,1024,0))>0 && file_size>0){
    fwrite(buf,sizeof(char), n, fp);
    memset(buf,'\0',1024);
    file_size = file_size-n;
  }*/
  // FILE *fp;
  // fp = fopen("/home/ubuntu/Desktop/Desktop/ss.txt","rb");
  // fseek(fp,0,SEEK_END);
  // int size = ftell(fp);
  // rewind(fp);
  // int serversend = send(sockfd,&size,sizeof(60),0);
  // int n;
  // while((n=fread(buf,sizeof(char),1024,fp))>0 && size>0){
  //   serversend = send(sockfd,buf,n,0);
  //     if(serversend<0){
  //       perror("couldnt send data to server");
  //       continue;
  //     }
  //     memset(buf,'\0',1024);
  //     size = size - n;
  // }
//   fclose(fp);
   // if(serversend<0){
   //   perror("couldnt send data to server");
   //   continue;
   // }
   int serversend;
   int byteRec;
   char ch;
  // serversend = send(sockfd)
  string userip;
  string user;
  string password;
  char checkexistence[]="checkexistence";
  serversend = send(sockfd,checkexistence,strlen(checkexistence),0);
   byteRec = recv(sockfd,buf,4096,0);
    cout<<"received : "<<string(buf,0,byteRec)<<endl;
  do{

    cout<<">signup first";
    //cin>>ch;
  //  if(ch == 'y'){

    //  string checkexistence;
        char *clientip = inet_ntoa(hint.sin_addr);
        int clientport = port;
        char res[50];


        sprintf(res,"%d",clientport);
      //  checkexistence  = "checkexistence";

        serversend = send(sockfd,checkexistence,strlen(checkexistence),0);
         byteRec = recv(sockfd,buf,4096,0);
         break;
        //int byteRec = recv(sockfd,buf,4096,0);
        if(byteRec < 0){
          perror("connection issue");
          break;
        }
        if(byteRec == 0){
          perror("client disconnected ");
          break;
        }
        cout<<"received : "<<string(buf,0,byteRec)<<endl;
    //     if(buf[0] == '0'){
    //       cout<<"please sign up"<<endl;
    //       /*send a bit to tell server that create_user function needs to be called*/
    //       //signup
    //         //login
    //     }
    //     else if(buf[0] == '1'){
    //       //login
    //         /*send a bit to tell server that login_user function needs to be called*/
    //       cout<<"enter username and password"<<endl;
    //       cin>>user;
    //       cin>>password;
    //     }
    //
    //
    //
    //
    //
    //
    //
    // getline(cin,userip);
    //  serversend = send(sockfd,userip.c_str(),userip.size()+1,0);
    // if(serversend<0){
    //   perror("couldnt send data to server");
    //   continue;
    // }
    // //waitfor server response
    // memset(buf,0,4096);
    //  byteRec = recv(sockfd,buf,4096,0);
    // //int byteRec = recv(sockfd,buf,4096,0);
    // if(byteRec < 0){
    //   perror("connection issue");
    //   break;
    // }
    // if(byteRec == 0){
    //   perror("client disconnected ");
    //   break;
    // }
    // cout<<"received : "<<string(buf,0,byteRec)<<endl;
//  }
  }while(true);
  //close socket
  close(sockfd);

  return 0;
}
