#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LEN_LIST 124699 //taille de la liste de mots (words.txt)
//Quand ces valeurs sont modifiées: penser à modifier la police dans graphics.txt
#define L_WIDTH 1 //Largeur de la police de caractères des lettres dans 'graphics.txt'
#define L_HEIGHT 1 //Hauteur

FILE *wordsList;
FILE *graphics;

//Tableau contenant les lettres de l'alphabet sous forme d'ASCII Art
//Tout les gros caractères de graphics.txt font 6 lignes de haut et 7 caractères de large (8 avec le caractère '\0')
char lettersGraphicsTab[27][L_HEIGHT][L_WIDTH+1];

//Tableau de référence des emplacements (cuseurs) des différents éléments graphiques dans graphics.txt
int graphicsElementsCursors[3];

//Structure de la grille de jeu
struct Grid {
    char line[6][10];   //'@'pour siginifier '.' (car '@'-'A'+1=0)
    char isCorrect[6][10]; //?: non-évalué, N: non, Y: oui, S: oui mais mal placé
    char sutom[10];     //mot mystère
    long sutomIndex;       //index du mot mystère dans l'index
    int length;     //longueur du mot mystère
    int step;   //étape du jeu
};

struct Grid G;

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

//Débarasse le mot des accents et le met en majuscule
void cleanWord(unsigned char *word) {
    for (int i=0;i<strlen(word);i++) {
        if (*(word+i) > 'Z') {
            *(word+i) -= 'a'-'A';
        }
    }
}

int isInvalid(char* word) {
    if (strlen(word) == G.length) {
        for (int i=0;i<G.length;i++) {
            if (word[i] < 'A' || word[i] > 'Z') {
                return 3; //caractère non autorisés
            }
        }
        return 0; //mot valide
    }
    else if (strlen(word) < G.length) {
        return 1; //trop court
    }
    else {
        return 2; //trop long
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
    chooseWord(G.sutom);
    /*for (int i=0;i<strlen(G.sutom);i++) {     //debug: affiche le code ASCII de chaque char du mot
        printf("%d/",G.sutom[i]);
    }*/
    cleanWord(G.sutom);
    G.length = strlen(G.sutom);
    G.step = 0;
    for (int i=0;i<6;i++) {
        for (int j=0;j<G.length;j++) {
           G.line[i][j] = '@';
           G.isCorrect[i][j] = '?';
        }
        G.line[i][G.length] = '\0';
    }
    G.line[0][0] = G.sutom[0];
    G.isCorrect[0][0] = 'Y';
}

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

void printGrid() {
    printf("%c",201);
    for (int i=0;i<L_WIDTH+2;i++) { printf("%c",205); }
    for (int i=0;i<G.length-1;i++) { printf(""); for (int i=0;i<L_WIDTH+2;i++) { printf("%c",205); }}
    printf("%c\n",187);
    for (int i=0;i<5;i++) {
        printBigWord(i);
        printf(" ");
    }
    printBigWord(5);
    printf("%c",200);
    for (int i=0;i<L_WIDTH+2;i++) { printf("%c",205); }
    for (int i=0;i<G.length-1;i++) { printf(""); for (int i=0;i<L_WIDTH+2;i++) { printf("%c",205); }}
    printf("%c\n",188);
}

//Affiche le logo SUTOM
void printLogo() {
    char graphicsTemp[100] = "";
    while (fgets(graphicsTemp,100,graphics) != NULL) {
        printf("%s",graphicsTemp);
    }
}

//Stockage des éléments graphiques en mémoire
void getGraphics() {
    char tempChar;
    rewind(graphics);
    //Commentaires début fichier
    do {
        tempChar = fgetc(graphics);
        if (tempChar == '/') {
            do {
                tempChar = fgetc(graphics);
            } while (tempChar != '\n');
        }
    } while (tempChar != '$');
    //LOGO
    do {
        tempChar = fgetc(graphics);
        //printf("%c",tempChar);
    } while (tempChar != '$');
    //ACCUEIL
    do {
        tempChar = fgetc(graphics);
        //printf("%c",tempChar);
    } while (tempChar != '$');
    //Lettres /!\ A cet endroit dans le fichier: placer le jeu de caractère souhaité ! Donc les lettres simples ou les gros caractères
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
        getGraphics();
        //printLogo();
        
        initGrid(&G);
        
        printf("Mot %-6d : %s\n",G.sutomIndex,G.sutom);
        char proposition[30];
        strcpy(proposition,G.sutom);
        int invalidity = 0;
        for (G.step;G.step<6;G.step++) {
            do {
                printGrid(&G);
                isInvalid(proposition);
                if (invalidity) {
                    switch (invalidity)
                    {
                    case 1:
                        printf("Le mot est trop court !\n");
                        break;
                    case 2:
                        printf("Le mot est trop long !\n");
                        break;
                    case 3:
                        printf("Le mot contient des caract%cres invalides (espace, apostrophe, tiret, etc.).\n",138);
                        break;
                    }
                } 
                printf("> Proposez un mot:\t");
                fgets(proposition,20,stdin);
                proposition[strcspn(proposition,"\n")] = 0; //supprime le caractère \n pris en compte par fgets()
                cleanWord(proposition);
                invalidity = isInvalid(proposition);
            } while (invalidity);
            cleanWord(proposition);
            strcpy(G.line[G.step],proposition);
            //TODO: correction
            printGrid(&G);
            printf("%s\n",proposition);
        }
        

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