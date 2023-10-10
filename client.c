//client.c

#include "pse.h"

void ReplaceSpace(char *str) {
    if (str == NULL) {
        return; // Vérification du pointeur valide
    }
    int length = strlen(str);
    for (int i = 0; i < length; i++) {
        if (str[i] == ' ') {
            str[i] = '_'; // Remplacer l'espace par un tiret bas
        }
    }
}

int main(int argc, char *argv[]) {
	
	if (argc!=3)//On vérifie que l'utilisateur a passé deux arguments
	{
		erreur("Il faut spécifier l'adresse du serveur et le port : %s port",argv[0]);
	}
	
	//création du socket 
	int soc;
	soc = socket(AF_INET,SOCK_STREAM,0);
	
	//Connexion au serveur
	int canal;
	struct sockaddr_in *adrserv; // Déclaration d'un pointeur vers une structure sockaddr_in qui représente l'adresse client 
	adrserv = resolv(argv[1], argv[2]); // argv[1]=adresse du serveur; argv[2]=port 
	canal = connect(soc, (struct sockaddr*) adrserv, sizeof(struct sockaddr_in)); // Établissement de la connexion avec le serveur, sizeof(struct sockaddr_in)=taille de la structure

	if (canal==0){ //la fonction connect renvoie 0 si la connexion à reussie et -1 sinon
		printf("Connexion réussie\n");}
	else {
		printf("Connexion échouée\n");
		return 0;}
	
	
	//SessionClient
	char choix[]="0";
	while(strcmp(choix,"fin")!=0){
		printf("\n\nTaper 0 pour vous inscrire\nTaper 1 pour vous connecter\nTaper fin pour quitter\n\nChoix : ");
		scanf("%s", choix);
		send(soc, choix, sizeof(choix), 0); //envoyer le choix au socket 'soc'
		
		//s'inscrire
		if (strcmp(choix,"0")==0){
			char pseudo[LIGNE_MAX];
			printf("\nInscription\npseudo : ");
			scanf("%s", pseudo);
			
			char password[LIGNE_MAX];
			printf("password : ");
			scanf("%s", password);
			
			send(soc, pseudo, sizeof(pseudo), 0); //envoyer le pseudo au socket 'soc'
			send(soc, password, sizeof(password), 0);  //envoyer le password au socket 'soc'
		}
		
		//se connecter
		if (strcmp(choix,"1")==0){
			char pseudo[LIGNE_MAX];
			printf("\nConnection\npseudo : ");
			scanf("%s", pseudo);
			
			char password[LIGNE_MAX];
			printf("password : ");
			scanf("%s", password);
			
			send(soc, pseudo, sizeof(pseudo), 0); //envoyer le pseudo au socket 'soc'
			send(soc, password, sizeof(password), 0);  //envoyer le password au socket 'soc'
			
			// Validation du serveur
			char buffer[LIGNE_MAX];  // Tampon de réception des données
			read(soc, buffer, sizeof(buffer));
			if(strcmp(buffer,"1")==0){
				printf("\n\nAuthentification réussi !\n");
				char choix2[LIGNE_MAX]="0";
				while(strcmp(choix2,"fin")!=0){
					printf("\n\nConnecté en tant que : %s\n\nTaper 0 pour poster un tweet\nTaper 1 pour afficher les derniers tweets\nTaper fin pour vous déconnecter\n\nChoix : ",pseudo);
					scanf("%s", choix2);
					getchar(); // Consomme le caractère de nouvelle ligne restant
					send(soc, choix2, sizeof(choix2), 0); //envoyer le choix au socket 'soc'
					
					//ÉCRIRE UN TWEET
					if (strcmp(choix2,"0")==0){
						char tweet[LIGNE_MAX];
						printf("\nÉcrivez votre tweet\n");
						fgets(tweet, sizeof(tweet), stdin);
						if (tweet[0] != '\n'){    // Lecture de la ligne depuis l'entrée standard (clavier)
							send(soc, tweet, sizeof(tweet), 0); //envoyer le tweet au socket 'soc'
						}
						else{
							printf("Le tweet ne peut pas être vide...\n");
							char vide[LIGNE_MAX]="XXXXXXXX";
							send(soc, vide, sizeof(vide), 0); //signifier que le tweet est vide
						}
					}
					
					//VOIR LES DERNIERS TWEETS
					if (strcmp(choix2,"1")==0){
						printf("Affichage des 5 tweets les plus récents\n");
						char pseudo [5][LIGNE_MAX];  
						char tweet [5][LIGNE_MAX]; 
						char data [LIGNE_MAX];
						for(int i=0;i<10;i++){
							if (i % 2 == 0){
								read(soc, data, sizeof(data));
								strcpy(pseudo[i/2], data);
							}
							else{
								read(soc, data, sizeof(data));
								strcpy(tweet[(i-1)/2], data);
							}
						}
						for (int i = 0; i < 5; i++) {
    						printf("\n%d.\nTweet de %s\n%s\n\n",i+1, pseudo[i],tweet[i]);
						}
					}
					
					
				}
			}
			else{
				printf("\nErreur de mot de passe ou de pseudo !\n");
			}
		}
		
	}
	
	
	//char ligne[LIGNE_MAX];  // Déclaration d'un tableau de caractères pour stocker une ligne de texte
	
	//écrire un message au serveur
	//fgets(ligne, LIGNE_MAX, stdin);  // Lecture d'une ligne de texte depuis l'entrée standard (clavier) et stockage dans le tableau 'ligne'
	//ecrireLigne(soc, ligne);  //envoyer la ligne de texte au socket 'soc'
		
	
	return 1;
	}
	
	
	
