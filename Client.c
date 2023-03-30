#include "MQTT_frames.h"

pthread_mutex_t mutex;
int fd;
f_PingRequest reqPing_frame;
f_PingResponse resPing_frame;

f_Connect create_ConnectF(char username[]){
    f_Connect frame;

    frame.bFrameType = 0x10;
    frame.wMsgLen = sizeof(f_Connect);
    frame.wProtlNameLen = 0x0004;
    strcpy(frame.sProtName,"MQTT");
    frame.bVersion = 0x02;
    frame.bConnectFlags = 0x04;
    frame.bKeepAlive = staticKeepAlive;
    frame.wClientIdLen = strlen(username);
    strcpy(frame.sClientID, username);

    return frame;
}

f_PingRequest create_PReq(){
    f_PingRequest frame;

    frame.bFrameType = 0xC0;
    frame.bkeepAlive = 0x00;

    return frame;
}

int menu(){
   int option;
   printf("======================MENU========================\n");
   printf("1. Publish\n2. Subscribe\n3. Disconnect\n");
   printf("==================================================\n");
   printf("- ");
   scanf("%i",&option);

   return option;
}

void connectServ(){
   struct sockaddr_in server;
   struct hostent *lh;//localhost

   if ((fd = socket(AF_INET, SOCK_STREAM, 0))==-1){
      printf("failed to create socket\n");
      exit(-1);
   }

   if ((lh=gethostbyname("localhost")) == NULL){
      printf("failed to get hostname\n");
      exit(-1);
   }//obtain localhost

   server.sin_family = AF_INET;
   server.sin_port = htons(PORT);
   server.sin_addr = *((struct in_addr *)lh->h_addr);
   //bzero(&(server.sin_zero),8);

   if(connect(fd,(struct sockaddr *)&server,sizeof(struct sockaddr))==-1){
      printf("failed to connect\n");
      exit(-1);
   }
}

void timer_handler (int signum)
{
   int numbytes;
   if(pthread_mutex_lock(&mutex) == 0){
      send(fd,(char *)&reqPing_frame,sizeof(f_PingRequest),0);
      printf("Ping Sent\n");

      if ((numbytes=recv(fd,(char *)&resPing_frame,sizeof(f_PingResponse),0)) == -1){
         printf("failed to recieve\n");
      }
      if(resPing_frame.bFrameType == 0xD0){
         printf("Keep alive successful\n");
      }else{
         printf("Keep alive failed\n");
      }
      pthread_mutex_unlock(&mutex);
   }
}

int main(int argc, char *argv[])
{
   int numbytes;
   //option;
   
   //Frames
   f_Connect conn_frame;
   f_ConnAcknowledge connack_frame;
   reqPing_frame = create_PReq();

   //Timer Variables
   struct sigaction sa;
	struct itimerval timer;

   connectServ();//Connection to server

   //Create frame with user inputs
   system("clear");
   conn_frame = create_ConnectF("Wilie");
	
	//Install timer_handler as the signal handler for SIGVTALRM.
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &timer_handler;
	sigaction (SIGVTALRM, &sa, NULL);
	
	//Configure the timer to expire after seconds
	timer.it_value.tv_sec = 5;
	timer.it_value.tv_usec = 0;
	
	//and every certain seconds after that
	timer.it_interval.tv_sec = 5;
	timer.it_interval.tv_usec = 0;

   if(pthread_mutex_init (&mutex, NULL) != 0){
      printf("Failed to initialize mutex");
   }//Initialize mutex variables

   if(pthread_mutex_lock(&mutex) == 0){//Request frame
      send(fd,(char *)&conn_frame,sizeof(f_Connect),0);
      puts("Sent connect frame\n");
      if ((numbytes=recv(fd,(char *)&connack_frame,sizeof(f_ConnAcknowledge),0)) == -1){
         printf("failed to recieve\n");
      }//Recieve response frame
      
      if(connack_frame.bFrameType == 0x20 &&  connack_frame.bReturnCode == 0x00){
         printf("Connect frame successful\n");
         setitimer(ITIMER_VIRTUAL, &timer, NULL);//Timer start
      }
      else{
         printf("Connect frame failed");
         exit(-1);
      }
      pthread_mutex_unlock(&mutex);
   }
   //==================================================

   /*while(option != 3){
         option = menu();

         switch (option){
         case 1:
            
            break;

         case 2:

            break;
         
         case 3:

            break;

         case 4:

            break;
         default:
            printf("Option not valid");
            break;
         }
   }*/
   while(1);
   return 0;
}