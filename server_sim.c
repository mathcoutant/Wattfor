#include <stddef.h>
#include <stdio.h>    
#include <stdlib.h>    
#include <getopt.h>
#include <pthread.h>


void print_help(int argc, char **argv){
  printf("usage: %s [options]\n",argv[0]);
  printf("  options:\n");
  printf("    -p --port=4096 port sur lequel écouter\n");
  printf("    -c --clients=100 le nombre de clients maximum que le serveur peut gérer\n");
  printf("    --freq1=20 le nombre de messages par seconde à envoyer sur le premier port\n");
  printf("    --freq2=20 le nombre de messages par seconde à envoyer sur le deuxième port\n");
  printf("    --size1=40 la taille des messages en octets à envoyer sur le premier port\n");
  printf("    --size2=217 la taille des messages en octets à envoyer sur le deuxième port\n");
  printf("    --simulated=100 le nombre d'objet dont on simule le déplacement\n");
  printf("    --freq_sim=20 la fréquence à laquelle la simulation se joue\n");
}

void* rcv_thread_function(void* arg){
  
}

void* sim_thread_function(void* arg){

}

int main(int argc, char **argv){

  int port = 4096; // le premier port utilisé par le serveur, le second est port+1
  size_t total_clients = 100; // le nombre de clients que doit gérer le serveur
  int freq1 = 20; // les fréquences des messages envoyés par le serveur sur chaque port 
  int freq2 = 20; 
  size_t size1 = 40;
  size_t size2 = 217; // les tailles des messages envoyés par le serveur sur chaque port
  size_t n_sim_object = 100; // le nombre d'objet dont on simule la position
  int sim_freq = 20; // la fréquence à laquelle on simule le déplacement des objets
  
  static int help_flag; // un flag pour savoir si quelqu'un a entré l'option -h/--help
  
  pthread_t first_rcv_thread; // threads qui vont recevoir le traffic
  pthread_t second_rcv_thread; 
  pthread_t sim_thread; // thread qui simule le déplacement des objets
  
  // un array qui contient les options du programme
  // si l'option est disponible en format court, la valeur de retour (dernière colonne)
  // vaut le caractère du format court
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

  // boucle dans laquelle on récupère les options
  while (1) {

    // index modifié par getopt_long(), permet de savoir quelle est l'option longue récupérée
    int option_index = 0;
    int c = getopt_long(argc, argv, "n:p:h",long_options, &option_index);
    
    // si il n'y a plus d'options à récupérer, on quitte la boucle !
    if (c == -1)
      break;

    switch (c) {
      // cas des options longues sans équivalents courts
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

      // le cas où getopt_long ne reconnait pas une option
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


  printf("Serveur lancé avec les paramètres suivants :\n");
  printf("port : %i\n",port);
  printf("total clients : %li\n", total_clients);
  printf("fréquence des messages descendants sur le port %i : %i\n",port,freq1);
  printf("taille des messages descendants sur le port %i : %li\n", port, size1);
  printf("fréquence des messages descendants sur le port %i : %i\n", port + 1, freq2);
  printf("taille des messages descendants sur le port %i : %i\n", port + 1, freq2);
  printf("nombre d'objets simulés : %li\n", n_sim_object);
  printf("la fréquence de simulation (en déplacements simulés par seconde) est de : %i\n", sim_freq);
 
  pthread_create(&first_rcv_thread, NULL, rcv_thread_function, NULL);
  pthread_create(&second_rcv_thread, NULL, rcv_thread_function, NULL);
  pthread_create(&sim_thread, NULL, sim_thread_function, NULL);
  pthread_join(sim_thread,NULL);
}
