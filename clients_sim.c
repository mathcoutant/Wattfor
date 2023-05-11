#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

# define MYMSGLEN 2000
#define BUF_SIZE 1500 // Taille MTU

enum type_msg {INIT, INIT_OK, MSG_MONTANT, MSG_DESCENDANT, END, NB_TYPE_MESSAGE};

struct t_msg {
  enum type_msg typ;
  char data[BUF_SIZE-sizeof(enum type_msg)];
};

struct thread_args {
  int frequence;
  int taile;
};

char* host;
int port;
int nb_clients;
int frequence_messages_montants_1;
int taille_messages_montants_1;
int frequence_messages_montants_2;
int taille_messages_montants_2;

int main (int argc, char *argv[]) {

  // REMPLIR LES VARIABLES AVEC LES OPTIONS

  pthread_t tid = 0;
  struct thread_args args1 = {frequence_messages_montants_1, taille_messages_montants_1};
  struct thread_args args2 = {frequence_messages_montants_2, taille_messages_montants_2};

  for(int i = 0; i < nb_clients; i++){
    pthread_create(&tid, NULL, thread_function, &args1);
    pthread_create(&tid, NULL, thread_function, &args2);
  }
  pthread_join(tid, NULL); // Pour éviter que main() ne sorte et tue tous les threads lancés
}

void* thread_function(void* arg){
  struct thread_args args = *(struct thread_args*)arg;

  int sock;
  struct sockaddr_in server;
  char message[MYMSGLEN], server_reply[MYMSGLEN];

  //Create socket
  sock = socket (AF_INET, SOCK_DGRAM, 0) ;
  if (sock == -1) {
      printf("Could not create socket\n");
      return(-1);
    }

  printf("socket created\n");
  
  //Prepare the server sockaddr_in structure

  server.sin_addr.s_addr = inet_addr(host) ;
  server.sin_family = AF_INET ;
  server.sin_port = htons(port) ;

  //Bind

  //INIT messages

  //Création du thread d'ecoute

  //envoie des messages
}