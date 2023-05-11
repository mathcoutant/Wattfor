#include <stddef.h>
#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <getopt.h>

int main(int argc, char **argv){

  int port; // le premier port utilisé par le serveur, le second est port+1
  size_t n_clients; // le nombre de clients que doit gérer le serveur
  int freq1, freq2; // les fréquences des messages envoyés par le serveur sur chaque port
  size_t size1, size2; // les tailles des messages envoyés par le serveur sur chaque port

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
      {0,          0,                 0,   0 }
    };

  // boucle dans laquelle on récupère les options
  while (1) {

    // index modifié par getopt_long(), permet de savoir quelle est l'option longue récupérée
    int option_index = 0;
    int c = getopt_long(argc, argv, "n:p:",long_options, &option_index);
    
    // si il n'y a plus d'options à récupérer, on quitte la boucle !
    if (c == -1)
      break;

    switch (c) {
      // cas des options longues sans équivalents courts
      case 0:
        switch (option_index) {
          case 2:
            freq1 = atoi(optarg);
            break;
          case 3:
            freq2 = atoi(optarg);
            break;
          case 4:
            size1 = atoi(optarg);
            break;
          case 5:
            size2 = atoi(optarg);
            break;
        }
        break;

      case 'n':
        n_clients = atoi(optarg); 
        break;

      case 'p':
        port = atoi(optarg);
        break;

      default:
        break;
    }
  }
}
