#include <stddef.h>
#include <stdio.h>    
#include <stdlib.h>    
#include <getopt.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>

#define MYMSGLEN 2000
#define BUF_SIZE 1500

enum type_msg {INIT, INIT_OK, MSG_MONTANT, MSG_DESCENDANT, END, NB_TYPE_MESSAGE};

struct rcv_thread_arg{
  int sock; 
  int freq;
  size_t total_clients;
  size_t size; 
};

struct send_thread_arg{
  int sock;
  int freq;
  size_t total_clients;
  size_t size;
  struct sockaddr** tab_connected_clients;
};

struct msg{
  enum type_msg type;
  char data[BUF_SIZE-sizeof(enum type_msg)];
};

struct sim_thread_arg{
  size_t n_object;
  int freq;
};

struct sim_object{
  double pos[3];
  double speed[3];
};

void print_help(int argc, char **argv){
  printf("usage: %s [options]\n",argv[0]);
  printf("  options:\n");
  printf("    -p --port=4096 port sur lequel ecouter\n");
  printf("    -c --clients=100 le nombre de clients maximum que le serveur peut gerer\n");
  printf("    --freq1=20 le nombre de messages par seconde a envoyer sur le premier port\n");
  printf("    --freq2=20 le nombre de messages par seconde a envoyer sur le deuxieme port\n");
  printf("    --size1=40 la taille des messages en octets a envoyer sur le premier port\n");
  printf("    --size2=217 la taille des messages en octets a envoyer sur le deuxieme port\n");
  printf("    --simulated=100 le nombre d'objet dont on simule le deplacement\n");
  printf("    --freq_sim=20 la frequence a laquelle la simulation se joue\n");
}


void* send_thread_function(void* arg){

}

void* rcv_thread_function(void* arg){
  struct rcv_thread_arg args = *(struct rcv_thread_arg*)arg;
  int sock = args.sock;
  int freq = args.freq;
  size_t total_clients = args.total_clients;
  size_t size = args.size;



  int total_connected_clients = 0;
  struct sockaddr ** tab_connected_clients = malloc(sizeof(struct sockaddr *) * total_clients);
  while (total_connected_clients < total_clients) {
    struct msg msg;
    struct sockaddr peer_addr;
    recvfrom(sock, &msg, sizeof(msg), 0, &peer_addr, NULL);
    if(msg.type == INIT){
      tab_connected_clients[total_connected_clients] = &peer_addr;
      total_connected_clients++;
      struct msg reply;
      reply.type = INIT_OK;
      sendto(sock, &reply, sizeof(reply), 0, &peer_addr, NULL);
    }
  }
  pthread_t send_thread;
  struct send_thread_arg thread_arg;
  thread_arg.sock = sock;
  thread_arg.freq = freq;
  thread_arg.size = size;
  thread_arg.tab_connected_clients = tab_connected_clients;
  pthread_create(&send_thread, NULL, send_thread_function, &thread_arg);

  struct msg msg;
  while(1){
    recvfrom(sock, &msg, sizeof(msg), 0, NULL, NULL);
  }
}

void calculate_square_distance(int x1, int y1, int z1, int x2, int y2, int z2){
    // Effectue le calcul de distance entre deux points et fait quelque chose avec le résultat
    int distance_carre = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1);
    // Faire quelque chose avec distance_carre,TODO peut etre mettre l'ecriture dans un fichier pour eviter de polluer le terminal
    printf("Distance au carre : %d\n", distance_carre);
}
void* sim_thread_function(void* arg){
  int nb_objets_simules = arg.simulated;
  int frequence_simulation = arg.freq_sim;
  int pos_x_y_z[nb_objets_simules];
  int speed_x_y_z[nb_objets_simules];

  //Initialisation avec des valeurs aleatoires
  srand(time(NULL));
  for (int i = 0; i<nb_objets_simules; i++){
    pos_x_y_z[i] = rand() % 100;
    speed_x_y_z[i] = rand() % 100;
  }

  while (1){
    //Calcul du temps ecoule depuis la derniere simulation d'objets, TODO je suis un peu perdue sur ce qu'il faut faire ici
    int dt = calcul_temps_ecoule_depuis_derniere_simulation_objets();
    for (int i = 0; i <nb_objets_simules; i++){
      pos_x_y_z[i]+=dt*speed_x_y_z[i];
    }

    for (int i=0; i <nb_objets_simules; i++){
      for (int j= i+1; j< nb_objets_simules; j==){
	calculate_square_distance(
				  pos_x_y_z[i], pos_x_y_z[i+1], pos_x_y_z[i+2],
				  pos_x_y_z[j], pos_x_y_z[j+1], pos_x_y_z[j+2]
				  );
      }
    }
    //Pause pour respecter la frequence adequate de simulation des deplacements d'objet
    usleep(1/frequence_simulation);
  }
  return NULL;
}

int main(int argc, char **argv){

  int port = 4096; // le premier port utilise par le serveur, le second est port+1
  size_t total_clients = 100; // le nombre de clients que doit gerer le serveur
  int freq1 = 20; // les frequences des messages envoyes par le serveur sur chaque port 
  int freq2 = 20; 
  size_t size1 = 40;
  size_t size2 = 217; // les tailles des messages envoyes par le serveur sur chaque port
  size_t n_sim_object = 100; // le nombre d'objet dont on simule la position
  int sim_freq = 20; // la frequence a laquelle on simule le deplacement des objets
  
  static int help_flag; // un flag pour savoir si quelqu'un a entre l'option -h/--help
  
  pthread_t first_rcv_thread; // threads qui vont recevoir le traffic
  pthread_t second_rcv_thread; 
  pthread_t sim_thread; // thread qui simule le deplacement des objets
  
  // un array qui contient les options du programme
  // si l'option est disponible en format court, la valeur de retour (derniere colonne)
  // vaut le caractere du format court
    static struct option long_options[] = {
      {"port",     required_argument, 0,  'p' },
      {"clients",  required_argument, 0,  'n' },
      {"freq1",    required_argument, 0,   0 },
      {"freq2",    required_argument, 0,   0 },
      {"size1",    required_argument, 0,   0 },
      {"size2",    required_argument, 0,   0 },
      {"simulated",required_argument, 0,   0 },
      {"freq_sim", required_argument, 0,   0 },
      {"help",     no_argument      , &help_flag,   1 },
      {0,          0,                 0,   0 },
    };

  // boucle dans laquelle on recupere les options
  while (1) {

    // index modifie par getopt_long(), permet de savoir quelle est l'option longue recuperee
    int option_index = 0;
    int c = getopt_long(argc, argv, "n:p:h",long_options, &option_index);
    
    // si il n'y a plus d'options a recuperer, on quitte la boucle !
    if (c == -1)
      break;

    switch (c) {
      // cas des options longues sans equivalents courts
      case 0:
        switch (option_index) {
          case 2:
            freq1 = atoi(optarg);
            if(freq1 <= 0){
              printf("the frequency must an int superior to 0\n");
              exit(1);
            }
            break;
          case 3:
            freq2 = atoi(optarg);
            if(freq2 <= 0){
              printf("the frequency must an int superior to 0\n");
              exit(1);
            }
            break;
          case 4:
            size1 = atoi(optarg);
            if(size1 <= 0){
              printf("the size of the packets must be an int superior to 0\n");
            }
            break;
          case 5:
            size2 = atoi(optarg);
            if(size2 <= 0){
              printf("the size of the packets must be an int superior to 0\n");
            }
            break;
          case 6:
            n_sim_object = atoi(optarg); 
            break;
          case 7:
            sim_freq = atoi(optarg); 
            if(freq2 <= 0){
              printf("the frequency must an int superior to 0\n");
              exit(1);
            }
            break;
          default:
            break;
        }
        break;

      case 'n':
        total_clients = atoi(optarg); 
        break;

      case 'p':
        port = atoi(optarg);
        break;

      case 'h':
        help_flag = 1;
        break;

      // le cas ou getopt_long ne reconnait pas une option
      case '?':
        print_help(argc,argv);
        exit(1);

      default:
        break;
    }
  } 

  if(help_flag){
    print_help(argc,argv);
    exit(0);
  }


  printf("Serveur lance avec les parametres suivants :\n");
  printf("port : %i\n",port);
  printf("total clients : %li\n", total_clients);
  printf("frequence des messages descendants sur le port %i : %i\n",port,freq1);
  printf("taille des messages descendants sur le port %i : %li\n", port, size1);
  printf("frequence des messages descendants sur le port %i : %i\n", port + 1, freq2);
  printf("taille des messages descendants sur le port %i : %i\n", port + 1, freq2);
  printf("nombre d'objets simules : %li\n", n_sim_object);
  printf("la frequence de simulation (en deplacements simules par seconde) est de : %i\n", sim_freq);
 


  struct sockaddr_in server_addr1;
  server_addr1.sin_family = AF_INET;
  server_addr1.sin_addr.s_addr = inet_addr("localhost");
  server_addr1.sin_port = htons(port);
  

  int sock1 = socket(PF_INET,SOCK_DGRAM,0);
  if(bind(sock1,(struct sockaddr *)&server_addr1,sizeof(server_addr1)) == -1){
    perror("bind");
    exit(1);
  }


  struct rcv_thread_arg first_thread_arg;
  first_thread_arg.sock = sock1;
  first_thread_arg.freq = freq1;
  first_thread_arg.size = size1;
  first_thread_arg.total_clients = total_clients;
  pthread_create(&first_rcv_thread, NULL, rcv_thread_function, &first_thread_arg);

  struct sockaddr_in server_addr2;
  server_addr2.sin_family = AF_INET;
  server_addr2.sin_addr.s_addr = inet_addr("localhost");
  server_addr2.sin_port = htons(port + 1);
  

  int sock2 = socket(PF_INET,SOCK_DGRAM,0);
  if(bind(sock2,(struct sockaddr *)&server_addr2,sizeof(server_addr2)) == -1){
    perror("bind");
    exit(1);
  }

  struct rcv_thread_arg second_thread_arg;
  second_thread_arg.sock = sock2;
  second_thread_arg.freq = freq2;
  second_thread_arg.size = size2;
  second_thread_arg.total_clients = total_clients;
  pthread_create(&second_rcv_thread, NULL, rcv_thread_function, &second_thread_arg);

  struct sim_thread_arg sim_arg;
  sim_arg.freq = sim_freq;
  sim_arg.n_object = n_sim_object;
  pthread_create(&sim_thread, NULL, sim_thread_function, &sim_arg);
  pthread_join(sim_thread,NULL);
}
