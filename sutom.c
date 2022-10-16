#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LEN_LIST 124701 //taille de la liste de mots (words.txt)
//Quand ces valeurs sont modifiées: penser à modifier la police dans graphics.txt
#define L_WIDTH 1 //Largeur de la police de caractères des lettres dans 'graphics.txt'
#define L_HEIGHT 1 //Hauteur

FILE *wordsList;
FILE *graphics;

//Tableau contenant les lettres de l'alphabet sous forme d'ASCII Art
//Tout les gros caractères de graphics.txt font 6 lignes de haut et 7 caractères de large (8 avec le caractère '\0')
char lettersGraphicsTab[27][L_HEIGHT][L_WIDTH+1];

//Tableau de référence des emplacements (curseurs) des différents éléments graphiques dans graphics.txt
long graphicsElementsCursors[3];     //0 = Logo SUTOM; 1 = Accueil; 2 = Lettres

//Structure de la grille de jeu
struct Grid {
    char line[6][10];   //'@'pour siginifier '.' (car '@'-'A'+1=0)
    char isCorrect[6][10]; //?: non-évalué, N: non, Y: oui, S: oui mais mal placé
    char sutom[10];     //mot mystère
    char sutomLetters[10]; //liste des lettres du mot mystère
    int lettersByQuantity[10]; //quantité pour chaque lettre (2*A, 1*B, 3*E,...)
    int nbSutomLetters; //nombre de lettres différentes dans le mot mystère
    long sutomIndex;       //index du mot mystère dans l'index
    int length;     //longueur du mot mystère
    int step;   //étape du jeu
};

struct Grid G;  //Grille comme variable globale

//Renvoie l'index d'un char présent dans un tableau, si absent: -1
int indexCharInTab(char tab[], int lenTab, char c) {
    for (int i=0;i<lenTab;i++) {
        if (c == tab[i]) {
            return i;
        }
    }
    return -1;
}

//Vide le buffer d'entrée
void flushstdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

//Clear le terminal (repositionne le curseur)
void clearscr() {
    printf("\e[1;1H\e[2J");
}

//Stocke dans word le mot numero line
void getWord(char *word, long line) {
    rewind(wordsList);  //place la tête de lecture en tête de fichier
    if (wordsList != NULL && line > 0) {
        char temp;
        int cpt = 0;
        for (int i=0;i<line-1;i++) {
            do {
                temp = getc(wordsList);
            } while (temp != '\n');
        }
        temp = getc(wordsList);
        while (temp != '\n') {
            *(word+cpt) = temp;
            temp = getc(wordsList);
            cpt++;
        }
        *(word+cpt) = '\0';
    }
}

//Retourne un entier long (max 2^32) allant de 1 à 336531. rand() vaut max 32767
long rangeAleaInt() {
    long aleaInt;
    do {
        aleaInt = ((rand()+1)*(rand()+1))%LEN_LIST;
    } while (aleaInt>LEN_LIST);
    return aleaInt;
}

//Choisi le mot mystère
void chooseWord(char *word) {
    G.sutomIndex = rangeAleaInt();
    getWord(word,G.sutomIndex);
}

//Met le mot en majuscule
void cleanWord(unsigned char *word) {
    for (int i=0;i<strlen(word);i++) {
        if (*(word+i) > 'Z') {
            *(word+i) -= 'a'-'A';
        }
    }
}

//Vérifie la validité d'un mot proposé
int isInvalid(char* word) {
    if (strlen(word) == G.length) {
        for (int i=0;i<G.length;i++) {
            if (word[i] < 'A' || word[i] > 'Z') {
                return 3; //caractères non autorisés
            }
        }
        if (*(word) == G.sutom[0]) {
            rewind(wordsList);
            char tempWord[30];
            char temp = '0';
            int cpt;
            while (!feof(wordsList)) {
                cpt = 0;
                temp = getc(wordsList);
                while (temp != '\n' && temp != EOF) {
                    *(tempWord+cpt) = temp;
                    cpt++;
                    temp = getc(wordsList);
                }
                *(tempWord+cpt) = '\0';
                //printf("%s / %d\n",tempWord, feof(wordsList));
                if (!strcmp(tempWord,word)) {
                    return 0; //mot valide: présent dans le dictionnaire
                }
            }
            return 4; //mot absent du dictionnaire
        }
        else {
            return 5; //commence pas par la bonne lettre
        }
    }
    else if (strlen(word) < G.length) {
        return 1; //trop court
    }
    else {
        return 2; //trop long
    }
}

//Si le mot est valide, correction du mot
void correction() {
    int lettersToMark[10];       //nombre de lettres moins celles déjà marquées, initialement: =G.lettersByQuantity
    int markYellow;     //lettre à marquer en jaune ou pas: 0 = non

    for (int i=0;i<G.nbSutomLetters;i++) {  //copie pour affectation de lettersToMark
        lettersToMark[i] = G.lettersByQuantity[i];
    }
    for (int i=0;i<G.length;i++) {  //parcours pour les lettres rouges
        if (*(G.line[G.step]+i) == G.sutom[i]) {    //lettre correspondante
            G.isCorrect[G.step][i] = 'Y';
            lettersToMark[indexCharInTab(G.sutomLetters,G.nbSutomLetters,*(G.line[G.step]+i))]--; //une lettre de moins est à marquer. Exemple: un 'E' à été détécté comme correspondant, si il y a 2 'E' dans le mot mystère, il en reste alors 1 à potentiellement marquer en jaune ou rouge
        }
    }
    for (int i=1;i<G.length;i++) {  //parcours pour les autres lettres
        markYellow = 0;
        if (*(G.line[G.step]+i) != G.sutom[i]) {    //si la lettre ne correspond pas
            for (int j=0;j<G.nbSutomLetters;j++) {      //est-ce que lettre est présente dans le mot
                printf("Lettre examen: %c / Lettre liste lettres: %c / Correspondance: %d\n",*(G.line[G.step]+i),G.sutomLetters[j],*(G.line[G.step]+i) == G.sutomLetters[j]);
                if (*(G.line[G.step]+i) == G.sutomLetters[j]) {
                    markYellow = (lettersToMark[indexCharInTab(G.sutomLetters,G.nbSutomLetters,*(G.line[G.step]+i))] > 0);  //markYellow = 1 uniquement si il y a encore des lettres à marquer (exemple: si il y a un seul 'E' dans le mot mystère et qu'il a déjà été marqué, alors markYellow=0)
                    break;
                }
            }
            if (markYellow) {   //on vérifie si lettre pas déjà présente dans liste à mettre en jaune
                G.isCorrect[G.step][i] = 'S';
                lettersToMark[indexCharInTab(G.sutomLetters,G.nbSutomLetters,*(G.line[G.step]+i))]--;
            }
            else {
                G.isCorrect[G.step][i] = 'N';
            }
        }
    }
}

//Retourne la taille de la liste de mots
long getListSize() {
    rewind(wordsList);
    long i = 0;
    char temp = 'a';
    while (temp!=EOF) {
        temp = getc(wordsList);
        if (temp=='\n') {
            i++;
        }
    }
    return i+1;
}

//Initialise la grille
void initGrid() {
    ////code normal:
    chooseWord(G.sutom);
    cleanWord(G.sutom);
    ////Code debug:
    //strcpy(G.sutom,"SOLIDITE\0");
    //G.sutomIndex = 0;

    G.length = strlen(G.sutom);
    G.step = 0;
    for (int i=0;i<6;i++) {
        for (int j=0;j<G.length;j++) {
           G.line[i][j] = '@';
           G.isCorrect[i][j] = '?';
        }
        G.line[i][G.length] = '\0';
    }
    //Init de la liste des lettres du mot mystère
    int nbLetters = 0;  //taille de la liste de lettres
    int presence;   //si la lettre du mot mystère en cours d'examen est déjà présente dans la liste des lettres
    for (int i=0;i<G.length;i++) {
        presence = 0;
        for (int j=0;j<nbLetters;j++) {     //pour chaque lettre déjà présente dans la liste on regarde si elle correspond à lettre en cours d'examen du mot mystère
            if (G.sutom[i] == G.sutomLetters[j]) {
                presence = 1;
                G.lettersByQuantity[j]++;   //lettre présente plus d'une fois
                break;
            }
        }
        if (!presence) {
            G.sutomLetters[nbLetters] = G.sutom[i];
            G.lettersByQuantity[nbLetters] = 1;     //lettre présente au moins une fois
            nbLetters++;
        }
    }
    //debug
    G.nbSutomLetters = nbLetters;
    for (int i=0;i<nbLetters;i++) {
        printf("%c,%d / ",G.sutomLetters[i],G.lettersByQuantity[i]);
    }
    printf("\n");
}

//Imprime une ligne de la grille, en tenant compte de la police présente dans graphics.txt
void printBigWord(int nbLine) {
    char aroundLetter[3] = {' ',' ',' '};
    printf(" ");
    for (int i=0;i<strlen(G.line[nbLine]);i++) {    //Première ligne (contour)
        switch (G.isCorrect[nbLine][i]) {
            case 'Y':
                aroundLetter[0] = 201;
                aroundLetter[1] = 205;
                aroundLetter[2] = 187;
                break;
            case 'N':
                aroundLetter[0] = aroundLetter[1] = aroundLetter[2] = ' ';
                break;
            case 'S':
                aroundLetter[0] = 218;
                aroundLetter[1] = 196;
                aroundLetter[2] = 191;
                break;
            case '?':
                aroundLetter[0] = aroundLetter[1] = aroundLetter[2] = ' ';
                break;
            default:
                aroundLetter[0] = aroundLetter[1] = aroundLetter[2] = '!';
                break;
        }
        printf("%c",aroundLetter[0]);
        for (int j=0;j<L_WIDTH;j++) {
            printf("%c",aroundLetter[1]);
        }
        printf("%c",aroundLetter[2]);
    }
    printf("\n");
    for (int i=0;i<L_HEIGHT;i++) {
        printf(" ");    //marge gauche
        for (int j=0;j<strlen(G.line[nbLine]);j++) {    //Deuxième ligne: 1ère ligne des lettres selon le schéma suivant: contour-caractères de la lettre-contour
            switch (G.isCorrect[nbLine][j]) {
                case 'Y':
                    aroundLetter[0] = 186;
                    break;
                case 'N':
                    aroundLetter[0] = ' ';
                    break;
                case 'S':
                    aroundLetter[0] = 179;
                    break;
                case '?':
                    aroundLetter[0] = ' ';
                    break;
                default:
                    aroundLetter[0] = '!';
                    break;
            }
            printf("%c%s%c", aroundLetter[0], lettersGraphicsTab[G.line[nbLine][j]-'A'+1][i], aroundLetter[0]);
        }
        printf("\n");
    }
    printf(" ");
    for (int i=0;i<strlen(G.line[nbLine]);i++) {
        switch (G.isCorrect[nbLine][i]) {
            case 'Y':
                aroundLetter[0] = 200;
                aroundLetter[1] = 205;
                aroundLetter[2] = 188;
                break;
            case 'N':
                aroundLetter[0] = aroundLetter[1] = aroundLetter[2] = ' ';
                break;
            case 'S':
                aroundLetter[0] = 192;
                aroundLetter[1] = 196;
                aroundLetter[2] = 217;
                break;
            case '?':
                aroundLetter[0] = aroundLetter[1] = aroundLetter[2] = ' ';
                break;
            default:
                aroundLetter[0] = aroundLetter[1] = aroundLetter[2] = '!';
                break;
        }
        printf("%c",aroundLetter[0]);
        for (int j=0;j<L_WIDTH;j++) {
            printf("%c",aroundLetter[1]);
        }
        printf("%c",aroundLetter[2]);
    }
    printf("\n");
}

//Affiche la grille
void printGrid() {
    printf("%c",201);
    for (int i=0;i<L_WIDTH+2;i++) { printf("%c",205); }
    for (int i=0;i<G.length-1;i++) { printf(""); for (int i=0;i<L_WIDTH+2;i++) { printf("%c",205); }}
    printf("%c\n",187);
    for (int i=0;i<5;i++) {
        printBigWord(i);
    }
    printBigWord(5);
    printf("%c",200);
    for (int i=0;i<L_WIDTH+2;i++) { printf("%c",205); }
    for (int i=0;i<G.length-1;i++) { printf(""); for (int i=0;i<L_WIDTH+2;i++) { printf("%c",205); }}
    printf("%c\n",188);
}

//Affiche le logo SUTOM
void printGraphicElement(int e) {
    int temp;
    fseek(graphics,graphicsElementsCursors[e],SEEK_SET);
    temp = fgetc(graphics);
    while(temp != '$') { 
        printf("%c", temp);
        temp = fgetc(graphics);
    }
}

//Stockage des éléments graphiques en mémoire
void getGraphics() {
    rewind(graphics);
    //Commentaires début fichier
    while(1) {
        if (fgetc(graphics) == '$') {
            if (fgetc(graphics) == '$') {
                break;
            }
        }
    }
    //LOGO
    graphicsElementsCursors[0] = ftell(graphics);
    while(fgetc(graphics) != '$') { ; }
    //ACCUEIL
    graphicsElementsCursors[1] = ftell(graphics);
    while(fgetc(graphics) != '$') { ; }
    //Lettres /!\ A cet endroit dans le fichier: placer le jeu de caractère souhaité ! Donc les lettres simples ou les gros caractères
    //Les lettres sont placées dans la RAM (tableau à deux dimension)
    graphicsElementsCursors[2] = ftell(graphics);
    fgetc(graphics);
    for (int i=0;i<L_HEIGHT;i++) {
        for (int j=0;j<28;j++) {
            fgets(lettersGraphicsTab[j][i],L_WIDTH+1,graphics);
        }
    }
}

int main() {
    srand(time(NULL));

    //Ouverture des fichiers (mots et graphismes)
    wordsList = fopen("words.txt","r+");
    graphics = fopen("graphics.txt","r");

    if (wordsList != NULL && graphics != NULL) {
        getGraphics();      //Parcours de graphics.txt pour obtenir les emplacements (curseur) des différents éléments graphiques
        printGraphicElement(0);     //Affichage du logo
        printGraphicElement(1);     //Affiche le message d'accueil
        
        flushstdin(); //Appuyer sur une touche pour entrer puis flush de STDIN au cas où l'utilisateur aurait entré un caractère autre que Entrée
        
        initGrid(&G);
        
        printf("Mot %-6d : %s\n",G.sutomIndex,G.sutom);
        char proposition[30];
        strcpy(proposition,G.sutom);
        int invalidity = 0;
        for (G.step;G.step<6;G.step++) {
            G.line[G.step][0] = G.sutom[0];
            G.isCorrect[G.step][0] = 'Y';
            do {    //Affichage de la grille et input du mot proposé: répété tant que le mot est invalide
                clearscr();
                printGrid(&G);
                if (invalidity) {
                    printf("\a");   //Caractère BEL (produit un son)
                    switch (invalidity)
                    {
                    case 1:
                        printf("Le mot est trop court !\n");
                        break;
                    case 2:
                        printf("Le mot est trop long !\n");
                        break;
                    case 3:
                        printf("Le mot contient des caract%cres invalides (espace, apostrophe, tiret, etc.) !\n",138);
                        break;
                    case 4:
                        printf("Le mot n'est pas pr%csent dans le dictionnaire !\n",130);
                        break;
                    case 5:
                        printf("Le mot doit commencer par un '%c' !\n", G.sutom[0]);
                        break;
                    default:
                        printf("ca pue du cu\n");
                        break;
                    }
                }
                else {
                    printf("\n");
                }
                printf("> Proposez un mot:\t");
                fgets(proposition,20,stdin);
                proposition[strcspn(proposition,"\n")] = '\0'; //supprime le caractère \n pris en compte par fgets()
                cleanWord(proposition);
                invalidity = isInvalid(proposition);
            } while (invalidity);
            strcpy(G.line[G.step],proposition);
            correction();
        }
        clearscr();
        printGrid(&G);
        

        getchar();
    }
    else {
        printf("Erreur: Fichier words.txt ou graphics.txt introuvables. Placez-les au m%cme niveau que l'ex%ccutable.\n",136,130);
        return 1;
    }
    return 0;
}

/*for (int i=0;i<20;i++) {
    alea = rangeAleaInt();
    getWord(fichier,mot,sutomIndex);
    cleanWord(fichier,mot);
    printf("Mot %-6d : %s\n",sutomIndex,mot);
}*/

/*
        //Test de distribution du générateur de nb aléa: semble uniforme (testé avec i=100000)
        int z1=0,z2=0,z3=0,z4=0,z5=0,z6=0,z7=0;
        printf("Mot %-6d : %s\n",game.sutomIndex,game.sutom);
        for (int i=0;i<100000;i++) {
            long aleeea = rangeAleaInt();
            printf("%d\n",aleeea);
            if (aleeea>300000) {
                z7++;
            }
            else if (aleeea>250000) {
                z6++;
            }
            else if (aleeea>200000) {
                z5++;
            }
            else if (aleeea>150000) {
                z4++;
            }
            else if (aleeea>100000) {
                z3++;
            }
            else if (aleeea>50000) {
                z2++;
            }
            else {
                z1++;
            }
        }
        printf("\n0-50: %d\n50-100: %d\n100-150: %d\n150-200: %d\n200-250: %d\n250-300: %d\n300    : %d\n",z1,z2,z3,z4,z5,z6,z7);*/