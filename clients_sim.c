#include <stddef.h>
#include <stdio.h>    
#include <stdlib.h>    
#include <getopt.h>
#include <pthread.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>

#define BUF_SIZE 1500 // Taille MTU
#define S_TO_US 1000000

enum type_msg
{
  INIT,
  INIT_OK,
  MSG_MONTANT,
  MSG_DESCENDANT,
  END,
  NB_TYPE_MESSAGE
};

struct msg
{
  enum type_msg type;
  char data[BUF_SIZE - sizeof(enum type_msg)];
};

struct send_thread_args
{
  char * host;
  int port;
  int frequence;
  int taile;
};

struct receive_thread_args
{
  int sock;
};

void print_help(int argc, char **argv)
{
  printf("usage: %s [options]\n", argv[0]);
  printf("  options:\n");
  printf("    -H --host='102.10.12.189' le nom du host du serveur\n");
  printf("    -p --port=4096 port sur lequel ecouter\n");
  printf("    -n --nClients=100 le nombre de clients que le programme doit simuler\n");
  printf("    --freq1=20 le nombre de messages par seconde a envoyer sur le premier port\n");
  printf("    --freq2=20 le nombre de messages par seconde a envoyer sur le deuxieme port\n");
  printf("    --size1=40 la taille des messages en octets a envoyer sur le premier port\n");
  printf("    --size2=217 la taille des messages en octets a envoyer sur le deuxieme port\n");
}

int open_socket(char * hostname, int port){

  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int sfd, s, j;
  size_t len;
  ssize_t nread;
  char buf[BUF_SIZE];

  /* Obtain address(es) matching host/port */

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
  hints.ai_flags = 0;
  hints.ai_protocol = 0;          /* Any protocol */

  char str_port[6];
  sprintf(str_port, "%d", port);
  s = getaddrinfo(hostname, str_port, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  /* getaddrinfo() returns a list of address structures.
   *               Try each address until we successfully connect(2).
   *                             If socket(2) (or connect(2)) fails,
   *                             we (close the socket
   *                                           and) try the next
   *                                           address. */

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype,
        rp->ai_protocol);
    if (sfd == -1)
      continue;

    if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
      break;                  /* Success */

    close(sfd);
  }

  if (rp == NULL) {               /* No address succeeded */
    fprintf(stderr, "Could not connect\n");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(result);           /* No longer needed */
  return sfd;
}
void *thread_receiving_function(void *arg) {
  struct receive_thread_args args = *(struct receive_thread_args *)arg;

  struct msg msg;
  while (1)
  {
    recvfrom(args.sock, &msg, sizeof(msg), 0, NULL, NULL);
    if (msg.type == END)
    {
      printf("The server ended the communication.\n");
      exit(0);
    }
  }
}

void *thread_sending_function(void *arg)
{
  struct send_thread_args args = *(struct send_thread_args *)arg;

  // Create socket
  int sock = open_socket(args.host,args.port);

  // INIT messages
  struct msg init_msg;
  init_msg.type = INIT;
  //if (sendto(sock, &init_msg, sizeof(enum type_msg) + sizeof(strlen(init_msg.data)), 0, (struct sockaddr*)&(args.server_addr), sizeof(args.server_addr)) < 0)
  if (write(sock,&init_msg, sizeof(enum type_msg)) < 0)
  {
    perror("Failed to send init message.\n");
    exit(1);
  }

  // Server response
  struct msg init_ok_msg;
  struct sockaddr peer_addr;
  printf("Wait for init_ok\n");
  int nread = read(sock, &init_ok_msg, sizeof(init_ok_msg));
  if( nread < 0){
    perror("read");
    exit(1);
  }
  if (init_ok_msg.type != INIT_OK)
  {
    printf("The server response was not of type INIT_OK.\n");
    exit(1);
  }

  // Creating the listening thread
  pthread_t tid = 0;
  struct receive_thread_args recv_args = {sock};
  pthread_create(&tid, NULL, thread_receiving_function, &recv_args);

  // Sending messages
  struct msg msg;
  msg.type = MSG_MONTANT;
  memset(&msg.data,'A',args.taile);
  // TODO : construction de la data
  while (1) {
    usleep(S_TO_US * 1.0 / args.frequence);
    send(sock, &msg, sizeof(enum type_msg) + strlen(msg.data), 0);
  }
}

int main(int argc, char *argv[])
{
  int port = 4096;
  char host[30] = "127.0.0.1";
  int nb_clients = 10;
  int freq1 = 20;
  size_t size1 = 40;
  int freq2 = 20;
  size_t size2 = 114;

  static int help_flag = 0;

  static struct option long_options[] = {
      {"port", required_argument, 0, 'p'},
      {"clients", required_argument, 0, 'n'},
      {"freq1", required_argument, 0, 0},
      {"freq2", required_argument, 0, 0},
      {"size1", required_argument, 0, 0},
      {"size2", required_argument, 0, 0},
      {"help", no_argument, &help_flag, 1},
      {"host", required_argument, 0, 'H'},
      {0, 0, 0, 0},
  };

  // boucle dans laquelle on recupere les options
  while (1)
  {

    // index modifie par getopt_long(), permet de savoir quelle est l'option longue recuperee
    int option_index = 0;
    int c = getopt_long(argc, argv, "n:p:hH:", long_options, &option_index);

    // si il n'y a plus d'options a recuperer, on quitte la boucle !
    if (c == -1)
      break;

    switch (c)
    {
    // cas des options longues sans equivalents courts
    case 0:
      switch (option_index)
      {
      case 2:
        freq1 = atoi(optarg);
        if (freq1 <= 0)
        {
          printf("the frequency must be an int superior to 0\n");
          exit(1);
        }
        break;
      case 3:
        freq2 = atoi(optarg);
        if (freq2 <= 0)
        {
          printf("the frequency must be an int superior to 0\n");
          exit(1);
        }
        break;
      case 4:
        size1 = atoi(optarg);
        if (size1 <= 0)
        {
          printf("the size of the packets must be an int superior to 0\n");
        }
        break;
      case 5:
        size2 = atoi(optarg);
        if (size2 <= 0)
        {
          printf("the size of the packets must be an int superior to 0\n");
        }
        break;
      default:
        break;
      }
      break;

    case 'n':
      nb_clients = atoi(optarg);
      break;

    case 'p':
      port = atoi(optarg);
      break;

    case 'h':
      help_flag = 1;
      break;

    case 'H':
      strcpy(host, optarg);
      break;

    // le cas où getopt_long ne reconnait pas une option
    case '?':
      print_help(argc, argv);
      exit(1);

    default:
      break;
    }
  }

  if (help_flag)
  {
    print_help(argc, argv);
    exit(0);
  }

  // Print the options
  printf("Client lance avec les parametres suivants :\n");
  printf("host : %s\n", host);
  printf("port : %i\n", port);
  printf("nombre de clients : %i\n", nb_clients);
  printf("fréquence des messages montants sur le port %i : %i\n", port, freq1);
  printf("taille des messages montants sur le port %i : %li\n", port, size1);
  printf("fréquence des messages montants sur le port %i : %i\n", port + 1, freq2);
  printf("taille des messages montants sur le port %i : %li\n", port + 1, size2);


  // Threads
  pthread_t tid = 0;
  struct send_thread_args args1 = {host, port, freq1, size1};
  struct send_thread_args args2 = {host, port+1, freq2, size2};
  for (int i = 0; i < nb_clients; i++)
  {
    pthread_create(&tid, NULL, thread_sending_function, &args1);
    pthread_create(&tid, NULL, thread_sending_function, &args2);
  }
  pthread_join(tid, NULL); // Pour eviter que main() ne sorte et tue tous les threads lances
}
