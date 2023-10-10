//serveur.c

#include "pse.h"
#define BDD "bdd_pseudo_password.log"
#define TWEETS "bdd_tweets.log"

int SessionClient(int canal);
void *threadSessionClient(void *arg);
int ecrireLigne2(char* buffer);

int fdBDD_CON;
int fdTWEETS;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;//mutex ecrire dans la bdd login
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;//mutex ecrire dans la bdd tweet

int main(int argc, char *argv[]) { //argc=le nombre d'argument et argv[./monprogramme, argument1, argument2...]

	fdBDD_CON = open(BDD, O_CREAT|O_RDWR|O_APPEND, 0600);
	fdTWEETS = open(TWEETS, O_CREAT|O_RDWR|O_APPEND, 0600);

	if (argc!=2)//On vérifie que l'utilisateur a passé deux arguments
	{
		erreur("Vous n'avez pas spécifié le port : %s port",argv[0]);
	}
	
	//On récupère le port qui a été passé en argument par l'utilisateur
	short port;
	port = (short)atoi(argv[1]);
	
	//création d'un socket
	int soc1;
	soc1=socket(AF_INET,SOCK_STREAM,0);
	
	//Lier le socket à une adresse réseau (celle de la machine hôte) et un port
	struct sockaddr_in monadr;           // Déclaration d'une structure sockaddr_in pour représenter l'adresse
	monadr.sin_family = AF_INET;         // Spécification de la famille d'adresses (IPv4)
	monadr.sin_addr.s_addr = INADDR_ANY; // Indique que le socket sera lié à toutes les interfaces réseau disponibles sur la machine
	monadr.sin_port = htons(port);       //sert à spécifier sur quel port sera lié le socket, htons sert à mettre le bon format
	bind(soc1, (struct sockaddr*)&monadr, sizeof(monadr)); // Association du socket "soc1" avec l'adresse "monadr"

	//Se mettre à l'écoute des connexions
	listen(soc1,5); //5 clients peuvent se connecter en même temps
	
	// Accepter une connexion entrante
	struct sockaddr_in adrcli;  // Déclaration d'une structure sockaddr_in pour stocker l'adresse du client
	unsigned int lgadr = sizeof(adrcli);  // Variable pour stocker la taille de la structure adrcli
	
	while(1){
	int canal = accept(soc1, (struct sockaddr*)&adrcli, &lgadr);  // Accepter une connexion sur le socket soc1 et récupérer le nouveau socket canal pour la communication avec le client
	pthread_t idThread; //pour y récupérer l'identifiant
	pthread_create(&idThread, NULL, threadSessionClient, &canal);//pour créer le thread
		}		
	return 1;
	}
	
void *threadSessionClient(void *arg) {
  	int *p;
  	int canal;
	p = (int *)arg;
	canal = *p;
  	SessionClient(canal);
  	pthread_exit(NULL);
  }
	
int SessionClient(int canal){
	//choix se connecter ou s'inscrire
	//Lire une ligne envoyée par le client
	char choix[LIGNE_MAX];
	while(strcmp(choix,"fin")!=0){
		ssize_t nbLus = read(canal, choix, sizeof(choix));
		if (nbLus == 0) {
			printf("Un client s'est déconnecté\n");
			return 0;
		}
		//S'INSCRIRE
		if (strcmp(choix,"0")==0)
		{
			char pseudo[LIGNE_MAX];
			nbLus = read(canal, pseudo, sizeof(pseudo)); //lit la ligne dans le canal et la stocke dans pseudo	
			if (nbLus == 0) {
				printf("Un client s'est déconnecté\n");
				return 0;
			}	
			char password[LIGNE_MAX];
			nbLus = read(canal, password, sizeof(password)); //lit la ligne dans le canal et la stocke dans password
			if (nbLus == 0) {
				printf("Un client s'est déconnecté\n");
				return 0;
			}
			pthread_mutex_lock(&mutex1);
			ecrireLigne(fdBDD_CON, pseudo); 
			ecrireLigne(fdBDD_CON, password);	
			pthread_mutex_unlock(&mutex1);
		}
		//SE CONNECTER
		else if (strcmp(choix,"1")==0)
		{
			fdBDD_CON = open(BDD, O_CREAT|O_RDWR|O_APPEND, 0600);
			char pseudo[LIGNE_MAX];
			nbLus=read(canal, pseudo, sizeof(pseudo)); //lit la ligne dans le canal et la stocke dans pseudo
			if (nbLus == 0) {
				printf("Un client s'est déconnecté\n");
				return 0;
			}		
			char password[LIGNE_MAX];
			nbLus=read(canal, password, sizeof(password)); //lit la ligne dans le canal et la stocke dans password
			if (nbLus == 0) {
				printf("Un client s'est déconnecté\n");
				return 0;
			}			
			lseek(fdBDD_CON, 0, SEEK_SET); // Revenir au début du fichier
			char buffer[LIGNE_MAX];//tampon pour lire le fichier
			char ligne[LIGNE_MAX];//tampon pour lire un ligne
			int lg;
			int ligneIndex =0;
			int validation1 = 0; //validation du pseudo
			int validation2 = 0; //validation du password
			while ((lg = read(fdBDD_CON, buffer, sizeof(buffer))) > 0) {
				for (int i = 0; i < lg; i++) {
					char c = buffer[i];
					if (c == '\n') {
						ligne[ligneIndex] = '\0'; // Ajouter le caractère de fin de chaîne
						//printf("Ligne : %s // pseudo : %s // password : %s\n", ligne,pseudo,password);
						//vérification du mot du pseudo et du password
						if(strcmp(pseudo,ligne)==0 && validation1==0){
							//printf("\n\nC'est le même pseudo\n\n");
							validation1++;
						}
						else if(strcmp(password,ligne)==0 && validation1==1){
							//printf("\n\nC'est le même password\n\n");
							printf("\n\nAuthentification réussie de %s\n\n",pseudo);
							validation2=1;
						}
						else {
							validation1=0;
						}
							
						ligneIndex = 0; // Réinitialiser l'indice de la ligne
					}
					else {
						ligne[ligneIndex] = c;
						ligneIndex++;
					}
				}
			}
				
			if (validation2==1) 
			{
				char validation[] = "1";  // Authentification réussie
				send(canal, validation, sizeof(validation), 0);
				
				char choix2[LIGNE_MAX]="0";//MENU POUR LE CLIENT CONNECTÉE 
				while(strcmp(choix2,"fin")!=0){
					nbLus = read(canal, choix2, sizeof(choix2)); //lit la ligne dans le canal et la stocke dans choix					
					if (nbLus == 0) {
						printf("%s a déconnecté le client brusquement\n",pseudo);
						return 0;
					}	
					//ÉCRIRE UN TWEET
					if (strcmp(choix2,"0")==0)
					{
						char tweet[LIGNE_MAX];
						nbLus = read(canal, tweet, sizeof(tweet)); //lit la ligne dans le canal et la stocke dans tweet	
						if (nbLus == 0) {
							printf("%s a déconnecté le client brusquement\n",pseudo);
							return 0;
						}	
						char pseudo2[LIGNE_MAX+1];
						sprintf(pseudo2, "%s\n", pseudo);
						printf("tweet : %s\n",tweet);	
						pthread_mutex_lock(&mutex2);
						if(strcmp(tweet,"XXXXXXXX")!=0){
							ecrireLigne2(tweet);
							ecrireLigne2(pseudo2); 
						}
						pthread_mutex_unlock(&mutex2);
						fdTWEETS = open(TWEETS, O_CREAT|O_RDWR|O_APPEND, 0600);
						}
					
					//AFFICHER LES TWEETS LES PLUS RECENTS
					if (strcmp(choix2,"1")==0)
					{	
						lseek(fdTWEETS, 0, SEEK_SET); // Revenir au début du fichier
						char buffer[LIGNE_MAX];//tampon pour lire le fichier
						char ligne[LIGNE_MAX];//tampon pour lire un ligne
						int lg;
						int ligneIndex =0;
						int nb =0; //Nombre de tweets a afficher
						while ((lg = read(fdTWEETS, buffer, sizeof(buffer))) > 0) {
							for (int i = 0; i < lg; i++) {
								char c = buffer[i];
								if (c == '\n') {
									nb++;
									ligne[ligneIndex] = '\0'; // Ajouter le caractère de fin de chaîne
									//printf("Ligne : %s\n", ligne);										
									ligneIndex = 0; // Réinitialiser l'indice de la ligne
									if(nb<11){
										send(canal, ligne, sizeof(ligne), 0); //envoie de la ligne au client
									}
								}
								else {
									ligne[ligneIndex] = c;
									ligneIndex++;
								}
							}
						}
						
						
						
						
					}
					
				}
				printf("\n\nDéconnexion de %s\n\n",pseudo);
			}
			else
			{
				char validation[] = "0";  // Erreur de mot de passe ou de pseudo
				send(canal, validation, sizeof(validation), 0);
			}
		}
	}
	return 0;
}
	

int ecrireLigne2(char* ligne) {
    int fd = open(TWEETS, O_RDWR);
    if (fd == -1) {
        perror("Erreur lors de l'ouverture du fichier");
        return -1;
    }
    char buffer[LIGNE_MAX];
    char** lignes = NULL;
    int nbLignes = 0;
    int capacite = 0;
    int i;

    // Lire le contenu existant du fichier
    while (1) {
        ssize_t lg = read(fd, buffer, sizeof(buffer));
        if (lg == -1) {
            // Gestion de l'erreur de lecture
            perror("Erreur lors de la lecture du fichier");
            return -1;
        } else if (lg == 0) {
            // Fin de fichier atteinte
            break;
        } else {
            // Stocker la ligne dans le tableau
            char* nouvelleLigne = malloc(lg);
            memcpy(nouvelleLigne, buffer, lg);
            if (nbLignes >= capacite) {
                capacite = (capacite == 0) ? 1 : capacite * 2;
                lignes = realloc(lignes, capacite * sizeof(char*));
            }
            lignes[nbLignes++] = nouvelleLigne;
        }
    }

    // Sauvegarder la position actuelle du curseur
    off_t positionCourante = lseek(fd, 0, SEEK_CUR);

    // Positionner le curseur au début du fichier
    lseek(fd, 0, SEEK_SET);

    // Écrire la nouvelle ligne
    write(fd, ligne, strlen(ligne));

    // Réécrire le contenu existant
    for (i = 0; i < nbLignes; i++) {
        write(fd, lignes[i], strlen(lignes[i]));
        free(lignes[i]);
    }

    // Restaurer la position du curseur
    lseek(fd, positionCourante, SEEK_SET);

    free(lignes);
	
	close(fd);
    return 0;
}



