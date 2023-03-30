#include "MQTT_frames.h"

pthread_mutex_t mutex;
r_Client clients[BACKLOG];

f_PingRequest reqPing_frame;
f_PingResponse resPing_frame;

f_ConnAcknowledge create_ConnAck(uint8_t ret_code){
    f_ConnAcknowledge frame;

    frame.bFrameType = 0x20;
    frame.bRemainLen = 0x02;
    frame.bReservedVal = 0x00;
    frame.bReturnCode = ret_code;
   
    return frame;
}

f_PingResponse create_PRes(){
    f_PingResponse frame;

    frame.bFrameType = 0xD0;
    frame.bresponse = 0x00;

    return frame;
}

void timer_handler (int signum)
{
   for(int f = 0; f < BACKLOG; f++){
      if(clients[f].fd != 0){
         clients[f].iKeepAlive = clients[f].iKeepAlive - 0x01;
         printf("Keep Alive: %x\n",clients[f].iKeepAlive);
         if(clients[f].iKeepAlive == 0){
            close(clients[f].fd);
            clients[f].fd = 0;
         }
      }
   }
}

void *timer_count(void *param){
   //Timer Variables
   struct sigaction sa;
	struct itimerval timer;

   //Install timer_handler as the signal handler for SIGVTALRM.
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &timer_handler;
	sigaction (SIGVTALRM, &sa, NULL);
	
	//Configure the timer to expire after seconds
	timer.it_value.tv_sec = 1;
	timer.it_value.tv_usec = 0;
	
	//and every certain seconds after that
	timer.it_interval.tv_sec = 1;
	timer.it_interval.tv_usec = 0;

   setitimer(ITIMER_VIRTUAL, &timer, NULL);//Timer start
   printf("Timer set\n");

   while(1);
}

void *handle_client(void *param) {
   int read_size;

    while(1){
      if((read_size = recv(clients[0].fd ,(char *)&reqPing_frame , sizeof(f_PingRequest) , 0)) > 0){
         printf("Ping Request: %x-%x\n", reqPing_frame.bFrameType, reqPing_frame.bkeepAlive);
         clients[0].iKeepAlive = clients[0].iKeepAliveMax;
         send(clients[0].fd, &resPing_frame, sizeof(f_PingResponse), 0);
      }
    }

   if(close(clients[0].fd) < 0) {
      perror("Close socket failed\n");
   }

   pthread_exit(NULL);
}

int main(int argc, char **argv) {
   int sockfd, newfd, numbytes;
   struct sockaddr_in host_addr, client_addr;
   pthread_t thread,thread2;
   f_Connect conn_frame;
   f_ConnAcknowledge connack_frame;
   socklen_t sin_size;
   resPing_frame = create_PRes();

   if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("Socket failed");
      exit(-1);
   }
   puts("--Socket created\n");

   host_addr.sin_family = AF_INET;
   host_addr.sin_port = htons(PORT);
   host_addr.sin_addr.s_addr = INADDR_ANY;
   memset(&(host_addr.sin_zero), '\0', 8);

   if(bind(sockfd, (struct sockaddr *)&host_addr, sizeof(struct sockaddr)) == -1) {
      perror("Bind failed");
      exit(-1);
   }
   puts("--Address binded\n");

   if(listen(sockfd, BACKLOG) == -1) {
      perror("Listen failed");
      exit(-1);
   }
   puts("--Listening...\n");

   sin_size = sizeof(struct sockaddr_in);
   
   if(pthread_create(&thread, NULL, handle_client, NULL) < 0) {
      perror("Thread creation failed");
      exit(-1);
   }

   if(pthread_create(&thread2, NULL, timer_count, NULL) < 0) {
      perror("Thread creation failed");
      exit(-1);
   }

   if((newfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size)) > 0){  
      if ((numbytes=recv(newfd,(char *)&conn_frame,sizeof(f_Connect),0)) == -1){
         printf("failed to recieve\n");
      }
      if(conn_frame.bFrameType == 0x10){//================Add more parameters to assure its correct
         for(int i = 0; i < BACKLOG; i++){
            if(clients[i].fd == 0){
               clients[i].fd = newfd;
               strcpy(clients[i].sUsername, conn_frame.sClientID);
               clients[i].iKeepAlive = conn_frame.bKeepAlive;
               clients[i].iKeepAliveMax = conn_frame.bKeepAlive;
               clients[i].bFashion = false;
               clients[i].bFood = false;
               clients[i].bMusic = false;
               printf("New client found!\n\n");
               connack_frame = create_ConnAck(0x00);
               if(send(newfd, &connack_frame, sizeof(f_ConnAcknowledge), 0) < 0) {
                  perror("Send failed\n");
               }
               break;
            }
            if(i == BACKLOG-1 && clients[i].fd != 0){
               printf("Client list full\n\n"); 
               connack_frame = create_ConnAck(0x03);
               if(send(newfd, &connack_frame, sizeof(f_ConnAcknowledge), 0) < 0) {
                  perror("Send failed\n");
               }
            }
         }
      }else{
         connack_frame = create_ConnAck(0x01);
         if(send(newfd, &connack_frame, sizeof(f_ConnAcknowledge), 0) < 0) {
            perror("Send failed\n");
         }
      }   
   }

   pthread_join(thread, NULL);

      if(close(sockfd) < 0) {
      perror("Close socket failed\n");
      exit(-1);
   }

   return 0;
}