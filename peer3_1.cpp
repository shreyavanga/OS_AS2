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
#include <openssl/sha.h>
#include<arpa/inet.h>
#include<pthread.h>

using namespace std;

pthread_mutex_t locks;

    struct clientDetails {

        string user_id;
        string passwd;
        string ipaddr;
        int port;
        char* oldfile;
        char* newfile;
        int fd;
        int chunks;
    };


    struct peerData{
      int port;
      int clientport;
    };



    struct clientFiles{
      string group_id;
      vector<string>files;
      string hash;
      vector<string>users;
      vector<string>owner;
      //pending user_id
      vector<string>pending_requests;
      int nochunks;
    };

    struct file{

      string filename;
      string sha;
      int file_size;
      // vector<string>users;
    };

//key = group_id
map<string,struct clientFiles *>filesclient;
//key = filename , values = sha,file_size
map<string,struct file *>gidfile;
//key = user_id
map<string,struct clientDetails *>detailclient;


void *print_message_fucntion(void *ptr){
  char *message;
  int c;
  message = (char *)ptr;
  printf("%s \n",message );
  //cin>>c;
}




string sha256_hash_string (unsigned char hash[SHA256_DIGEST_LENGTH])
{
   stringstream ss;
   for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}
string sha256(const string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

string sha256_file(FILE *file,int file_size, int *chunks, string filepath)
{
    if(!file) return NULL;
    string finalHash="";
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    const int bufSize = 512;
    unsigned char *buffer =(unsigned char*) malloc(bufSize+1);
    int bytesRead = 0;
    if(!buffer) return NULL;
    int i=0;
    int count = 0;
    while((bytesRead = fread(buffer, sizeof(char), bufSize, file))){
          count++;
          SHA256_Update(&sha256, buffer, bytesRead);
          SHA256_Final(hash, &sha256);
          string outputBuffer = sha256_hash_string(hash);
          string finalAnswer = outputBuffer.substr(0, 20);
          finalHash += finalAnswer;
          memset ( buffer , '\0', 512);
      }

    *chunks = count;
    fclose(file);
    free(buffer);
    return finalHash;
}

void* FileRecFunc(void* threadarg){

      char host[NI_MAXHOST];
      struct clientDetails *th;
      char svc[NI_MAXSERV];
      memset(host,0,NI_MAXHOST);
      memset(svc,0,NI_MAXSERV);
      cout<<"entered in FileRecFunc"<<endl;
      th = (struct clientDetails *)threadarg;
      int clientSocket = th->fd;
      char filename[100];
      int c;
      // if(recv(clientSocket,&c,sizeof(c),0)<0)
      // {
      //   perror("couldnt receive data");
      // //  exit(1);
    //  }
      int chunknum;
      recv(clientSocket,&chunknum,sizeof(chunknum),0);
      cout<<"chunknum receievd at server is "<<chunknum<<endl;
      int ack=1;
      send(clientSocket,&ack,sizeof(ack),0);
      recv(clientSocket,filename, sizeof(filename),0);
      cout<<"filename = "<<filename<<endl;
  //cout<<"chunk no = "<<c<<endl;

    //while receiving,display message
      char buf[256]={0};
      FILE *fp;
      fp = fopen(filename,"rb");
      fseek(fp,(c-1)*512,SEEK_SET);
      // int size = ftell(fp);
      // rewind(fp);
      int size=512;
      int byteRec =0;
      while( size>0 && (byteRec = fread(buf,sizeof(char),256,fp))>0 ){
          int serversend = send(clientSocket,buf,byteRec,0);

          size = size-byteRec;
          cout<<"buff content = "<<buf<<endl;
            memset(buf,'\0',256);
      }
      int ack1;
  	  recv(clientSocket,&ack1,sizeof(ack1),0);
	// fclose(fp);
	// close(clientSocket);

    // int n;
    // while((n=fread(buf,sizeof(char),1024,fp))>0 && size>0){
    //   serversend = send(clientSocket,buf,n,0);
    //     if(serversend<0){
    //       perror("couldnt send data to server");
    //       continue;
    //     }
    //     memset(buf,'\0',1024);
    //     size = size - n;
    // }
      fclose(fp);
      close(clientSocket);
}



void* ChunkDownload(void * clientDetails){
  cout<<"entered ChunkDownload fucntion"<<endl;
	struct clientDetails* cd;
	cd=(struct clientDetails*)clientDetails;
	int sockfd=socket(AF_INET,SOCK_STREAM,0);
  if(sockfd < 0){
    perror("error while creating socket");
    exit(1);
  }
  struct sockaddr_in hint;
  hint.sin_family = AF_INET;
  cout<<"cd - >port = "<<cd->port;
  hint.sin_port = htons(cd->port);
  hint.sin_addr.s_addr = INADDR_ANY;
  //inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);
  //connect to server on Socket
  char buf[256] = {0};
  char* filetobedownloaded=cd->oldfile;
  cout<<"port is "<<cd->port<<endl;
  cout<<"filename is "<<cd->oldfile<<endl;
  int connRes = connect(sockfd,(struct sockaddr*)&hint,sizeof(hint));
  if(connRes<0){
    perror("couldnt connect with the server");
  //  exit(1);
  }

  int chunkno = cd->chunks;
  cout<<" accessing chunk no "<<chunkno<<endl;
  if(send(sockfd,&chunkno,sizeof(chunkno),0)<0){
    perror("error while sending file");
  //  exit(1);
  }

  int ack;
  recv(sockfd,&ack,sizeof(ack),0);
  send(sockfd,filetobedownloaded,sizeof(filetobedownloaded),0);
  pthread_mutex_lock(&locks);
  FILE *fp = fopen(cd->newfile,"rab+");
  cout<<"filename = "<<cd->newfile<<endl;
  int chunksize = 512;
  rewind(fp);
  fseek(fp,(cd->chunks -1)*512,SEEK_SET);
  int n;
  while( chunksize > 0 && (n=recv(sockfd,buf,256,0)) > 0)
	{
      cout<<" n  = "<<n<<endl;
		  int byter = fwrite(buf,sizeof(char),n,fp);
      cout<<"buytes receive = "<<byter<<endl;
		  cout<<"Buffer"<<buf<<endl;
      memset(buf,'\0',256);

		  chunksize=chunksize-n;
		  cout<<"CHUNKSIZE "<<chunksize<<endl;
	}

	fclose(fp);
  cout<<"fiile closed"<<endl;
	int ack2=1;
	send(sockfd,&ack2,sizeof(ack2),0);
	cout<<"releasing lock"<<endl;
	pthread_mutex_unlock(&locks);
	cout<<"lock released"<<endl;

	close(sockfd);
}


void* clientThreadFunc(void* threadarg){
  struct peerData *pd;
  pd = (struct peerData*)threadarg;

  //create a Socket
  int sockfd = socket(AF_INET,SOCK_STREAM,0);
  if(sockfd < 0){
    perror("error while creating socket");
  //  exit(1);
  }
  //create a hint structure for server we're connecting to
  // int port;
  // cout<<"enter client port"<<endl;
  // cin>>port;
  // string ipAddress = "127.0.0.1";
  struct sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(pd->port);
  hint.sin_addr.s_addr = INADDR_ANY;
  //inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);
  //connect to server on Socket
  int netw=1;
  struct sockaddr_in client;
  client.sin_family = AF_INET;
  client.sin_port = htons(pd->clientport);
  client.sin_addr.s_addr = INADDR_ANY;
  char *ipinput = new char[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(hint.sin_addr),ipinput,INET_ADDRSTRLEN);
  string ipip = ipinput;
  string ipport = to_string(ntohs(hint.sin_port));
  string socketRec = ipip+":"+ipport;

  cout<<"Socket received = "<<socketRec<<endl;

  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &netw, sizeof(netw)))
  {
    cout<<"set socket opt"<<endl;
    pthread_exit(NULL);
  }

  if(bind(sockfd,(sockaddr*)&client,sizeof(client)) == -1){
    perror("error occured while binding");
    exit(1);
  }

  int connRes = connect(sockfd,(struct sockaddr*)&hint,sizeof(hint));
  if(connRes<0){
    perror("couldnt connect with the server");
      pthread_exit(NULL);

  //  exit(1);
  }
  //send data
  char buf[4096];
  memset(buf,0,4096);
  FILE *fp;
  //fp = fopen("/home/ubuntu/Desktop/Desktop/testing.txt","wb");
  // int file_size;
  // int byteRec = recv(sockfd,&file_size,sizeof(file_size),0);
  // cout<<"bytes Recv: "<<byteRec<<endl;
  int n;
  // char user[100];
  char user[100];
  string input;


  cin.ignore();
  int choice = 0;
  while(1){

  cout<<"enter command"<<endl;

//  cin>>input;
  getline(cin,input);
  //cout<<input<<endl;
  n = input.length();
  char input_char[1024];
  strcpy(input_char,input.c_str());
  //cout<<input_char<<endl;
  // char *tokens = strtok(input_char," ");
  // while(tokens!=NULL){
  //   printf("%s\n",tokens);
  //   tokens = strtok(NULL," ");
  // }
  char *temp1[1024];
  int k =0;
  temp1[k++] = strtok(input_char," ");

  //push the rest of the text into the array till EOF
  char *temp;
  while((temp1[k] = strtok(NULL," ")) != NULL)
        k++;
  temp1[k] = NULL;
  cout<<temp1[0]<<endl;

        // cout<<a<<endl;
  int serversend;

  string tempuser_id,temppasswd,tempfinal;
  /*   if(strcmp(temp1[0],"download_file")==0){

         char buf[256] = {0};
         char download_filename[1024];
         cout<<"enter filename to download"<<endl;
         cin>>download_filename;
         int csend;
         //tracker ka kaam hai filesize return karvana
         //-----trackers duty
         //csend = send(sockfd,download_filename,sizeof(download_filename),0);
         //int file_size=27;
      //   int byteRec = recv(sockfd,&file_size,sizeof(file_size),0);
         //--------trackers duty ends here
         FILE *fout;
         fout = fopen(download_filename, "r");
         //char buf[512];
         fseek(fout, 0L, SEEK_END);
     int file_size = ftell(fout);
     rewind(fout);
     fclose(fout);
     int chunks=0;
       cout<<"file_size = "<<file_size<<endl;
         cout<<"file_size "<<file_size<<endl;
         char newFile[100];
         cout<<"enter filename"<<endl;
         cin>>newFile;
         FILE *fp;
         fp = fopen(newFile,"wb+");
         memset(buf,'\0',256);
         int n,temporary_size;
         temporary_size = file_size;
         while(temporary_size>0){
           if(temporary_size >= 256)
            fwrite(buf,sizeof(char),256,fp);
            else
              fwrite(buf,sizeof(char),temporary_size,fp);
            temporary_size = temporary_size -256;
         }
         fclose(fp);
        int port;
         int clientnum=3;
         pthread_t clientAvail[clientnum];
         int arr[3] = {6000,7000,8000};
         int ch[3] = {3,2,1};
         int i=0;
         while(clientnum>0){
           struct clientDetails *cd = (struct clientDetails*)malloc(sizeof(struct clientDetails));
           cd->ipaddr = "127.0.0.1";

           cd->oldfile = download_filename;
           cd->newfile = newFile;
           cd->port = arr[i];
           cd->chunks = ch[i];
           pthread_create(&clientAvail[i],NULL,&DownloadChunk, (void*)cd);
           i++;
           clientnum--;
         }
         for(int i=0;i<3;i++)
          pthread_join(clientAvail[i],NULL);


       }*/
    if(strcmp(temp1[0],"create_user") == 0){
      //create a structureof clientdetials
      struct clientDetails client1;

    //  char *temp;
      temp = temp1[1];

      tempuser_id=temp;
      //string tempstring = string(temp1[1]) ;
      string cmd;
      temp = temp1[0];
      cmd = temp;
      client1.user_id = tempuser_id;
      temp = temp1[2];
      temppasswd  = temp;
      client1.passwd = temppasswd;
      tempfinal =cmd+":"+ tempuser_id+":"+temppasswd;
      client1.ipaddr = ipip;
      client1.port = stoi(ipport);
      cout<<"final string in create_user = "<<tempfinal<<endl;

      serversend = send(sockfd,(char*)tempfinal.c_str(),1024,0);
      int byteRec = recv(sockfd,buf,4096,0);
      if(byteRec < 0){
        perror("connection issue");
          pthread_exit(NULL);
      }
      if(byteRec == 0){
        perror("client disconnected ");
        pthread_exit(NULL);
      }
      string receive = string(buf,0,byteRec);
      cout<<"received- : "<<receive<<endl;

    /*  char user[100];
      string input;
      cout<<"enter command"<<endl;
      //cin.ignore();
      getline(cin,input);
      cout<<"input = "<<input<<endl;
      n = input.length();
      char input_char[1024];
      strcpy(input_char,input.c_str());
      //cout<<input_char<<endl;
      // char *tokens = strtok(input_char," ");
      // while(tokens!=NULL){
      //   printf("%s\n",tokens);
      //   tokens = strtok(NULL," ");
      // }
      char *temp1[1024];
      int k =0;
      temp1[k++] = strtok(input_char," ");

      //push the rest of the text into the array till EOF
        char *temp;
      while((temp1[k] = strtok(NULL," ")) != NULL)
        k++;
        temp1[k] = NULL;
        cout<<temp1[0]<<endl;


    /*  if(strcmp(temp1[0],"login")==0){
       //login
       cout<<"enter user_id and passwd"<<endl;
     //  string tempfinal;
       // cin>>tempuser_id;
       // cin>>temppasswd;

       temp = temp1[1];

       tempuser_id=temp;
       //string tempstring = string(temp1[1]) ;
       string cmd;
       temp = temp1[0];
       cmd = temp;
     //  client1.user_id = tempuser_id;
       temp = temp1[2];
       temppasswd  = temp;
     //   client1.passwd = temppasswd;
        tempfinal =cmd+":"+ tempuser_id+":"+temppasswd;
        cout<<"final string = "<<tempfinal<<endl;

        //tempfinal = tempuser_id+":"+temppasswd;
          serversend = send(sockfd,(char*)tempfinal.c_str(),1024,0);
          // int byteRec = recv(sockfd,buf,4096,0);
          // if(byteRec < 0){
          //   perror("connection issue");
          //   exit(1);
          // }
          // if(byteRec == 0){
          //   perror("client disconnected ");
          //   exit(1);
          // }
          // string receive = string(buf,0,byteRec);
          // cout<<"received- : "<<receive<<endl;
          choice =1;
     }*/



    }
    else if(strcmp(temp1[0],"login")==0){
      //login
      cout<<"enter user_id and passwd"<<endl;
    //  string tempfinal;
      // cin>>tempuser_id;
      // cin>>temppasswd;

      temp = temp1[1];

      tempuser_id=temp;
      //string tempstring = string(temp1[1]) ;
      string cmd;
      temp = temp1[0];
      cmd = temp;
    //  client1.user_id = tempuser_id;
      temp = temp1[2];
      temppasswd  = temp;
    //   client1.passwd = temppasswd;
      tempfinal =cmd+":"+ tempuser_id+":"+temppasswd;
      cout<<"final string = "<<tempfinal<<endl;

       //tempfinal = tempuser_id+":"+temppasswd;
      serversend = send(sockfd,(char*)tempfinal.c_str(),1024,0);
      int byteRec = recv(sockfd,buf,4096,0);
      if(byteRec < 0){
           perror("connection issue");
            pthread_exit(NULL);
      }
      if(byteRec == 0){
           perror("client disconnected ");
             pthread_exit(NULL);
      }
      string receive = string(buf,0,byteRec);
      cout<<"received- : "<<receive<<endl;
      choice = 1;

    }
    // while(choice){
    //   char user[100];
    //   string input;
    //   cout<<"enter command"<<endl;
    //   //cin.ignore();
    //   getline(cin,input);
    //   cout<<"input = "<<input<<endl;
    //   n = input.length();
    //   char input_char[1024];
    //   strcpy(input_char,input.c_str());
    //   //cout<<input_char<<endl;
    //   // char *tokens = strtok(input_char," ");
    //   // while(tokens!=NULL){
    //   //   printf("%s\n",tokens);
    //   //   tokens = strtok(NULL," ");
    //   // }
    //   char *temp1[1024];
    //   int k =0;
    //   temp1[k++] = strtok(input_char," ");
    //
    //   //push the rest of the text into the array till EOF
    //     char *temp;
    //   while((temp1[k] = strtok(NULL," ")) != NULL)
    //     k++;
    //     temp1[k] = NULL;
    //     cout<<temp1[0]<<endl;

      else if((strcmp(temp1[0],"create_group") == 0) && choice==1){


      temp = temp1[1];
      string tempgroup = temp;
      string cmd = temp1[0];
      tempfinal =cmd+":"+ tempgroup;
      cout<<"final string = "<<tempfinal<<endl;

        //tempfinal = tempuser_id+":"+temppasswd;
      serversend = send(sockfd,(char*)tempfinal.c_str(),1024,0);
      int byteRec = recv(sockfd,buf,4096,0);
      if(byteRec < 0){
            perror("connection issue");
            exit(1);
      }
      if(byteRec == 0){
            perror("client disconnected ");
            exit(1);
      }
      string receive = string(buf,0,byteRec);
      cout<<"received- : "<<receive<<endl;

   }
   else if(strcmp(temp1[0],"leave_group") == 0 && choice==1){
      temp = temp1[1];
      string tempgroup = temp;
      string cmd = temp1[0];
      tempfinal =cmd+":"+ tempgroup;
      cout<<"final string = "<<tempfinal<<endl;

      //tempfinal = tempuser_id+":"+temppasswd;
      serversend = send(sockfd,(char*)tempfinal.c_str(),1024,0);
      int byteRec = recv(sockfd,buf,4096,0);
      if(byteRec < 0){
          perror("connection issue");
          exit(1);
      }
      if(byteRec == 0){
          perror("client disconnected ");
          exit(1);
      }
      string receive = string(buf,0,byteRec);
      cout<<"received- : "<<receive<<endl;
   }
   else if(strcmp(temp1[0],"logout")==0 && choice==1){
      temp = temp1[1];
      string tempgroup = temp;
      string cmd = temp1[0];
      tempfinal =cmd+":"+ tempgroup;
      cout<<"final string = "<<tempfinal<<endl;

      //tempfinal = tempuser_id+":"+temppasswd;
      serversend = send(sockfd,(char*)tempfinal.c_str(),1024,0);
      int byteRec = recv(sockfd,buf,4096,0);
      if(byteRec < 0){
          perror("connection issue");
          exit(1);
      }
      if(byteRec == 0){
        perror("client disconnected ");
          exit(1);
      }
      string receive = string(buf,0,byteRec);
      cout<<"received- : "<<receive<<endl;
   }
   else if(strcmp(temp1[0],"list_groups")==0 && choice==1){
    // temp = temp1[1];
    // string tempgroup = temp;
      string cmd = temp1[0];
      tempfinal =cmd;
      cout<<"final string = "<<tempfinal<<endl;

      //tempfinal = tempuser_id+":"+temppasswd;
         serversend = send(sockfd,(char*)tempfinal.c_str(),1024,0);
        // int byteRec = recv(sockfd,buf,4096,0);
        // if(byteRec < 0){
        //   perror("connection issue");
        //   exit(1);
        // }
        // if(byteRec == 0){
        //   perror("client disconnected ");
        //   exit(1);
        // }
        // string receive = string(buf,0,byteRec);
        // cout<<"received- : "<<receive<<endl;


   }
   else if(strcmp(temp1[0],"upload_file") ==0 && choice==0){
     struct clientFiles *file_entry;
     file_entry = (struct clientFiles*)malloc(sizeof(clientFiles));

     temp = temp1[2];
     string tempgroup = temp;
     string cmd = temp1[0];
      tempfinal =cmd+":"+ tempgroup;
      cout<<"final string = "<<tempfinal<<endl;
      string filepath = temp1[1];
      cout<<filepath<<endl;
      FILE *fp,*fout;
      fp = fopen(temp1[1], "r");
      char buf[512];
      fseek(fp, 0L, SEEK_END);
  int file_size = ftell(fp);
  rewind(fp);
  int chunks=0;
    cout<<"file_size = "<<file_size<<endl;
    string finalHash = sha256_file(fp,file_size,&chunks,filepath);
    cout<<"finalHash = "<<finalHash<<endl;
  fclose(fp);
    file_entry->group_id = tempgroup;
    file_entry->hash = finalHash;
    file_entry->nochunks = chunks;
    file_entry->files.push_back(filepath);
    filesclient[tempgroup] = file_entry;
    int n = finalHash.length();
    fout = fopen("hash.txt","w");
    char buff[n+1]; int len=1024;
    strcpy(buff,finalHash.c_str());
  fprintf(fout,"%s",buff);
    fclose(fout);

    fp = fopen("hash.txt","r");
    int bytesRead = 0;
    if(!fp){
      while((bytesRead = fread(buf, sizeof(char), 1024, fp))){
        if(bytesRead < 0){
          cout<<"nothing to read"<<endl;
          break;
        }
          serversend = send(sockfd,(char*)tempfinal.c_str(),1024,0);

    }




    }

 }
 else if(strcmp(temp1[0],"download_file")==0 && choice==0){

      char buf[256] = {0};
      char download_filename[1024];
      cout<<"enter filename to download"<<endl;
      cin>>download_filename;
      int csend;
      //tracker ka kaam hai filesize return karvana
      //-----trackers duty
      //csend = send(sockfd,download_filename,sizeof(download_filename),0);
      //int file_size=27;
   //   int byteRec = recv(sockfd,&file_size,sizeof(file_size),0);
      //--------trackers duty ends here
      FILE *fout;
      fout = fopen(download_filename, "r");
      //char buf[512];
      fseek(fout, 0L, SEEK_END);
  int file_size = ftell(fout);
  rewind(fout);
  fclose(fout);
  int chunks=0;
    cout<<"file_size = "<<file_size<<endl;
      cout<<"file_size "<<file_size<<endl;
      char newFile[100];
      cout<<"enter filename"<<endl;
      cin>>newFile;
      FILE *fp;
      fp = fopen(newFile,"wb+");
      memset(buf,'\0',256);
      int n,temporary_size;
      temporary_size = file_size;
      while(temporary_size>0){
        if(temporary_size >= 256)
         fwrite(buf,sizeof(char),256,fp);
         else
           fwrite(buf,sizeof(char),temporary_size,fp);
         temporary_size = temporary_size -256;
      }
      fclose(fp);
     int port;
      int clientnum=3;
      pthread_t clientAvail[clientnum];
      int arr[3] = {6000,7000,8000};
      int ch[3] = {3,2,1};
      int i=0;
      while(clientnum>0){
        struct clientDetails *cd = (struct clientDetails*)malloc(sizeof(struct clientDetails));
        cd->ipaddr = "127.0.0.1";

        cd->oldfile = download_filename;
        cd->newfile = newFile;
        cd->port = arr[i];
        cd->chunks = ch[i];
        pthread_create(&clientAvail[i],NULL,&ChunkDownload, (void*)cd);
        i++;
        clientnum--;
      }
      for(int i=0;i<3;i++)
       pthread_join(clientAvail[i],NULL);


    //}

   // char buf[256] = {0};
   // char download_filename[1024];
   // cout<<"enter filename to download"<<endl;
   // cin>>download_filename;
   // int csend;
   // csend = send(sockfd,download_filename,sizeof(download_filename),0);
   // int file_size;
   // int byteRec = recv(sockfd,&file_size,sizeof(file_size),0);
   // cout<<"file_size "<<file_size<<endl;
   // char newFile[100];
   // cout<<"enter filename"<<endl;
   // cin>>newFile;
   // FILE *fp;
   // fp = fopen(newFile,"wb+");
   // memset(buf,'\0',256);
   // int n,temporary_size;
   // temporary_size = file_size;
   // while(temporary_size>0){
   //   if(temporary_size >= 256)
   //    fwrite(buf,sizeof(char),256,fp);
   //    else
   //      fwrite(buf,sizeof(char),temporary_size,fp);
   //    temporary_size = temporary_size -256;
   // }
   // fclose(fp);
   // int port;
   // int clientnum=3;
   // pthread_t clientAvail[clientnum];
   // int arr[3] = {6000,7000,8000};
   // int ch[3] = {3,2,1};
   // int i=0;
   // while(clientnum>0){
   //   struct clientDetails *cd = (struct clientDetails*)malloc(sizeof(struct clientDetails));
   //   cd->ipaddr = "127.0.0.1";
   //   cd->port = arr[i];
   //   cd->chunks = ch[i];
   //   cd->oldfile = download_filename;
   //   cd->newfile = newFile;
   //   pthread_create(&clientAvail[i],NULL,&ChunkDownload, (void*)cd);
   //   i++;
   //   clientnum--;
   // }
   // for(int i=0;i<3;i++)
   //  pthread_join(clientAvail[i],NULL);


 }
 else if(strcmp(temp1[0],"join_group")==0 && choice==1){
   temp = temp1[1];
   string tempgroup = temp;
   string cmd = temp1[0];
    tempfinal =cmd+":"+ tempgroup;
    cout<<"final string = "<<tempfinal<<endl;

    //tempfinal = tempuser_id+":"+temppasswd;
      serversend = send(sockfd,(char*)tempfinal.c_str(),1024,0);
      int byteRec = recv(sockfd,buf,4096,0);
      if(byteRec < 0){
        perror("connection issue");
        exit(1);
      }
      if(byteRec == 0){
        perror("client disconnected ");
        exit(1);
      }
      string receive = string(buf,0,byteRec);
      cout<<"received- : "<<receive<<endl;
 }
 else if(strcmp(temp1[0],"list_requests")==0 && choice==1){
   temp = temp1[1];
   string tempgroup = temp;
   string cmd = temp1[0];
    tempfinal =cmd+":"+ tempgroup;
    cout<<"final string = "<<tempfinal<<endl;

    //tempfinal = tempuser_id+":"+temppasswd;
      serversend = send(sockfd,(char*)tempfinal.c_str(),1024,0);
      // int byteRec = recv(sockfd,buf,4096,0);
      // if(byteRec < 0){
      //   perror("connection issue");
      //   exit(1);
      // }
      // if(byteRec == 0){
      //   perror("client disconnected ");
      //   exit(1);
      // }
      // string receive = string(buf,0,byteRec);
      // cout<<"received- : "<<receive<<endl;


 }
 else if(strcmp(temp1[0],"accept_requests")==0 && choice==1){
   temp = temp1[1];
   string tempgroup = temp;
   temp = temp1[2];
   string tempuser = temp;
   string cmd = temp1[0];
    tempfinal =cmd+":"+tempgroup+":"+tempuser;
    cout<<"final string = "<<tempfinal<<endl;

    //tempfinal = tempuser_id+":"+temppasswd;
      serversend = send(sockfd,(char*)tempfinal.c_str(),1024,0);
      int byteRec = recv(sockfd,buf,4096,0);
      if(byteRec < 0){
        perror("connection issue");
        exit(1);
      }
      if(byteRec == 0){
        perror("client disconnected ");
        exit(1);
      }
      string receive = string(buf,0,byteRec);
      cout<<"received- : "<<receive<<endl;


 }
 else if(strcmp(temp1[0],"upload_file")==0 && choice==1){
   // struct file *file_entry;
   // //cout<<"list groups"<<endl;
   // file_entry = (struct file*)malloc(sizeof(file));
    temp = temp1[1];
    string filepath = temp;
    //file_entry.filename = filepath;
    temp = temp1[2];
    string groupid = temp;
  //  file_entry.
    string cmd = temp1[0];
    tempfinal = cmd +":"+filepath+":"+groupid;
    cout<<"final string = "<<tempfinal<<endl;
    int file_size;
    int chunks = 0;
    FILE *fp,*fout;
    fp = fopen(temp1[1], "r");
    char buf[512];
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
    chunks=0;
    cout<<"file_size = "<<file_size<<endl;
    string finalHash = sha256_file(fp,file_size,&chunks,filepath);
    cout<<"finalHash = "<<finalHash<<endl;
    tempfinal = tempfinal + ":"+ to_string(file_size) +":"+finalHash;
    fclose(fp);

  //  string sha = calsha(filepath);
    //tempfinal = tempuser_id+":"+temppasswd;

      serversend = send(sockfd,(char*)tempfinal.c_str(),4096,0);
      int byteRec = recv(sockfd,buf,4096,0);
      if(byteRec < 0){
        perror("connection issue");
        exit(1);
      }
      if(byteRec == 0){
        perror("client disconnected ");
        exit(1);
      }
      string receive = string(buf,0,byteRec);
      cout<<"received- : "<<receive<<endl;


 }
 else if(strcmp(temp1[0],"download_file")==0 && choice == 1){

   string cmd = temp1[0];
    string groupid = temp1[1];
    string filename = temp1[2];
    // if(strcmp(temp1[3],NULL)==0){
    //   cout<<"enter destination path"<<endl;
    // }
    // else{
    int byteRec;
      string dest = temp1[3];
      string tempfinal = cmd + ":" + groupid + ":" + filename + ":" + dest;
      cout<<"final str = "<<tempfinal<<endl;

      serversend = send(sockfd,(char*)tempfinal.c_str(),4096,0);
       byteRec = recv(sockfd,buf,4096,0);
      if(byteRec < 0){
        perror("connection issue");
        exit(1);
      }
      if(byteRec == 0){
        perror("client disconnected ");
        exit(1);
      }
      string receive = string(buf,0,byteRec);
      cout<<"received- : "<<receive<<endl;
      memset(buf,0,4096);

      int ack =1;
      serversend = send(sockfd,&ack,sizeof(ack),0);


      //--------receieve number of users in that group
       byteRec = recv(sockfd,buf,4096,0);
      if(byteRec < 0){
        perror("connection issue");
        exit(1);
      }
      if(byteRec == 0){
        perror("client disconnected ");
        exit(1);
      }
       receive = string(buf,0,byteRec);
      cout<<"received- : "<<receive<<endl;
      memset(buf,0,4096);

      serversend = send(sockfd,&ack,sizeof(ack),0);

      int n = stoi(receive);
      cout<<"no of users in trackers "<<n<<endl;
      //vector<pair <string, string>> ip_port;

      //for loop for accessing
      for(int i=0;i<n;i++){

        byteRec = recv(sockfd,buf,4096,0);
       if(byteRec < 0){
         perror("connection issue");
         exit(1);
       }
       if(byteRec == 0){
         perror("client disconnected ");
         exit(1);
       }
       string receive = string(buf,0,byteRec);
       cout<<"received- : "<<receive<<endl;
       memset(buf,0,4096);

       serversend = send(sockfd,&ack,sizeof(ack),0);

       if(byteRec>0){
         //tokenize the string received into ip and port

         char *temp_ip_port[1024];
         int m =0;
         temp_ip_port[m++] = strtok((char*)receive.c_str(),":");

         //push the rest of the text into the array till EOF
         //char *temp;
         while((temp_ip_port[m] = strtok(NULL,":")) != NULL)
               m++;
         temp_ip_port[m] = NULL;
         cout<<" ip of client = "<<temp_ip_port[0]<<endl;
         cout<<"port of client = "<<temp_ip_port[1]<<endl;


       }

      }

      // --------for loop ends here

  //  }




 }
 else{
   cout<<"login first or wrong command"<<endl;
   // pthread_exit(NULL);
 }
}      //while true ends here
//}

  close(sockfd);

}

void* serverThreadFunc(void* threadarg){
  int opt = 1;
  struct peerData *pd;
  pd = (struct peerData*)threadarg;
  // int PORT;
  // cout<<"enter port";
  // cin>>PORT;
  //create a socket
  int server_sock;
  server_sock = socket(PF_INET,SOCK_STREAM,0);
  if(server_sock<0){
    perror("Socket creation failed");
    exit(1);
  }
  struct sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(pd->port);
  //hint.sin_addr.s_addr = INADDR_ANY;
  int addrlen = sizeof(hint);
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
  // struct sockaddr_in client;
  // socklen_t clientSize = sizeof(client);
  char host[NI_MAXHOST];
  char svc[NI_MAXSERV];

  int clientSocket;
  while((clientSocket = accept(server_sock,(struct sockaddr*)&hint,(socklen_t*)&addrlen))){
    pthread_t newThread;
    struct clientDetails th;
    th.fd = clientSocket;
    cout<<"out from here"<<endl;
    int* ptrTopass = new int(clientSocket);
    pthread_create(&newThread, NULL, FileRecFunc, (void*)&th);
    cout<<"thread created"<<endl;
    pthread_detach(newThread);
    // cout<<"out from here"<<endl;
  }
  //close the listening socket

// memset(host,0,NI_MAXHOST);
// memset(svc,0,NI_MAXSERV);
//   //while receiving,display message
//   char buf[4096];
//   FILE *fp;
//   fp = fopen("/home/ubuntu/Desktop/Desktop/ss.txt","rb");
//   fseek(fp,0,SEEK_END);
//   int size = ftell(fp);
//   rewind(fp);
//   int serversend = send(clientSocket,&size,sizeof(60),0);
//   int n;
//   while((n=fread(buf,sizeof(char),1024,fp))>0 && size>0){
//     serversend = send(clientSocket,buf,n,0);
//       if(serversend<0){
//         perror("couldnt send data to server");
//         continue;
//       }
//       memset(buf,'\0',1024);
//       size = size - n;
//   }
//   fclose(fp);
//   // while(true){
//   //   memset(buf,0,4096);
//   //   int byteRec = recv(clientSocket,buf,4096,0);
//   //   if(byteRec < 0){
//   //     perror("connection issue");
//   //     break;
//   //   }
//   //   if(byteRec == 0){
//   //     perror("client disconnected ");
//   //     break;
//   //   }
//   //   cout<<"received : "<<string(buf,0,byteRec)<<endl;
//   //   send(clientSocket,buf,byteRec+1,0);
//   // }
  //close socket
    close(clientSocket);
close(server_sock);

}


int main(){
  pthread_t clientThread,serverThread;
  struct peerData pdserver;
    struct peerData pdclient;
  cout<<"port no for server"<<endl;
  cin>>pdserver.port;
  cout<<"port number for client to connect tracker"<<endl;
  cin>>pdclient.port;
  pdclient.clientport = pdserver.port;
  // string msg1 = "Thread 1";
  // char *message1 = const_cast<char*>(msg1.c_str());
  // string msg2 = "Thread 2";
  // char *message2 = const_cast<char*>(msg2.c_str());
  int iret1,iret2;

  if(pthread_mutex_init(&locks,NULL)!=0){
    cout<<"mutex init failed"<<endl;
  }

  /*create independent threads each of which will execute fucntion*/
//  iret2 = pthread_create(&serverThread, NULL, serverThreadFunc, (void*)&pdserver);
  iret1 = pthread_create(&clientThread, NULL, clientThreadFunc, (void*)&pdclient);


  /* Wait till threads are complete before main continues. Unless we  */
     /* wait we run the risk of executing an exit which will terminate   */
     /* the process and all threads before the threads have completed.   */

     pthread_join (clientThread, NULL);
  //  pthread_join (serverThread , NULL);
     printf("thread1 %d\n", iret1);
    //   printf("thread2 %d\n", iret2);
        exit(0);

}
