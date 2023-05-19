/*****************************************************************************/
/*									                                         */
/*				     cocos2.c				                                 */
/*									                                         */
/*     Programa inicial d'exemple per a les practiques 2.1 i 2.2 de FSO.     */
/*     Es tracta del joc del menjacocos: es dibuixa un laberint amb una      */
/*     serie de punts (cocos), els quals han de ser "menjats" pel menja-     */
/*     cocos. Aquest menjacocos es representara amb el caracter '0', i el    */
/*     moura l'usuari amb les tecles 'w' (adalt), 's' (abaix), 'd' (dreta)   */
/*     i 'a' (esquerra). Simultaniament hi haura un conjunt de fantasmes,    */
/*     representats per numeros de l'1 al 9, que intentaran capturar al      */
/*     menjacocos. En la primera versio del programa, nomes hi ha un fan-    */
/*     tasma.								                                 */
/*     Evidentment, es tracta de menjar tots els punts abans que algun fan-  */
/*     tasma atrapi al menjacocos.					                         */
/*									                                         */
/*  Arguments del programa:						                             */
/*     per controlar la posicio de tots els elements del joc, cal indicar    */
/*     el nom d'un fitxer de text que contindra la seguent informacio:	     */
/*		n_fil1 n_col fit_tauler creq				                         */
/*		mc_f mc_c mc_d mc_r						                             */
/*		f1_f f1_c f1_d f1_r						                             */
/*									                                         */
/*     on 'n_fil1', 'n_col' son les dimensions del taulell de joc, mes una   */
/*     fila pels missatges de text a l'ultima linia. "fit_tauler" es el nom  */
/*     d'un fitxer de text que contindra el dibuix del laberint, amb num. de */
/*     files igual a 'n_fil1'-1 i num. de columnes igual a 'n_col'. Dins     */
/*     d'aquest fitxer, hi hauran caracter ASCCII que es representaran en    */
/*     pantalla tal qual, excepte el caracters iguals a 'creq', que es visua-*/
/*     litzaran invertits per representar la paret.			                 */
/*     Els parametres 'mc_f', 'mc_c' indiquen la posicio inicial de fila i   */
/*     columna del menjacocos, aixi com la direccio inicial de moviment      */
/*     (0 -> amunt, 1-> esquerra, 2-> avall, 3-> dreta). Els parametres	     */
/*     'f1_f', 'f1_c' i 'f1_d' corresponen a la mateixa informacio per al    */
/*     fantasma 1. El programa verifica que la primera posicio del menja-    */
/*     cocos o del fantasma no coincideixi amb un bloc de paret del laberint.*/
/*	   'mc_r' 'f1_r' son dos reals que multipliquen el retard del moviment.  */
/*     A mes, es podra afegir un segon argument opcional per indicar el      */
/*     retard de moviment del menjacocos i dels fantasmes (en ms);           */
/*     el valor per defecte d'aquest parametre es 100 (1 decima de segon).   */
/*									                                         */
/*  Compilar i executar:					  	                             */
/*     El programa invoca les funcions definides a 'winsuport.h', les        */
/*     quals proporcionen una interficie senzilla per crear una finestra     */
/*     de text on es poden escriure caracters en posicions especifiques de   */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser   */
/*     compilat amb la llibreria 'curses':				                     */
/*									                                         */
/*	   $ gcc -Wall cocos0.c winsuport.o -o cocos0 -lcurses		             */
/*	   $ ./cocos0 fit_param [retard]				                         */
/*									                                         */
/*  Codis de retorn:						  	                             */
/*     El programa retorna algun dels seguents codis al SO:		             */
/*	0  ==>  funcionament normal					                             */
/*	1  ==>  numero d'arguments incorrecte 				                     */
/*	2  ==>  fitxer de configuracio no accessible			                 */
/*	3  ==>  dimensions del taulell incorrectes			                     */
/*	4  ==>  parametres del menjacocos incorrectes			                 */
/*	5  ==>  parametres d'algun fantasma incorrectes			                 */
/*	6  ==>  no s'ha pogut crear el camp de joc			                     */
/*	7  ==>  no s'ha pogut inicialitzar el joc			                     */
/*****************************************************************************/

#include <stdio.h>	   /* incloure definicions de funcions estandard */
#include <stdlib.h>	   /* per exit() */
#include <unistd.h>	   /* per getpid() */
#include "winsuport.h" /* incloure definicions de funcions propies */
#include <pthread.h>
#include <stdint.h>

#define MIN_FIL 7 /* definir limits de variables globals */
#define MAX_FIL 25
#define MIN_COL 10
#define MAX_COL 80
#define MAX_THREADS 10

/* definir estructures d'informacio */
typedef struct
{			 /* per un objecte (menjacocos o fantasma) */
	int f;	 /* posicio actual: fila */
	int c;	 /* posicio actual: columna */
	int d;	 /* direccio actual: [0..3] */
	float r; /* per indicar un retard relati */
	char a;	 /* caracter anterior en pos. actual */
} objecte;

/* variables globals */
int n_fil1, n_col; /* dimensions del camp de joc */
char tauler[70];   /* nom del fitxer amb el laberint de joc */
char c_req;		   /* caracter de pared del laberint */
int fi1;
int fi2;
int minuts;
int segons;

/* VARIABLES OBJECTES */

objecte mc;				 /* informacio del menjacocos */
objecte f1[MAX_THREADS]; /* informacio del fantasma 1 */
int nFantasmes;

int df[] = {-1, 0, 1, 0}; /* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1}; /* dalt, esquerra, baix, dreta */

int cocos;	/* numero restant de cocos per menjar */
int retard; /* valor del retard de moviment, en mil.lisegons */

/* VARIABLES THREADS */

pthread_mutex_t s = PTHREAD_MUTEX_INITIALIZER;

/* funcio per realitzar la carrega dels parametres de joc emmagatzemats */
/* dins d'un fitxer de text, el nom del qual es passa per referencia a  */
/* 'nom_fit'; si es detecta algun problema, la funcio avorta l'execucio */
/* enviant un missatge per la sortida d'error i retornant el codi per-	*/
/* tinent al SO (segons comentaris al principi del programa).		    */
void carrega_parametres(const char *nom_fit)
{
	FILE *fit;
	nFantasmes = 0;

	fit = fopen(nom_fit, "rt"); /* intenta obrir fitxer */
	if (fit == NULL)
	{
		fprintf(stderr, "No s'ha pogut obrir el fitxer \'%s\'\n", nom_fit);
		exit(2);
	}

	if (!feof(fit))
		fscanf(fit, "%d %d %s %c\n", &n_fil1, &n_col, tauler, &c_req);
	else
	{
		fprintf(stderr, "Falten parametres al fitxer \'%s\'\n", nom_fit);
		fclose(fit);
		exit(2);
	}
	if ((n_fil1 < MIN_FIL) || (n_fil1 > MAX_FIL) ||
		(n_col < MIN_COL) || (n_col > MAX_COL))
	{
		fprintf(stderr, "Error: dimensions del camp de joc incorrectes:\n");
		fprintf(stderr, "\t%d =< n_fil1 (%d) =< %d\n", MIN_FIL, n_fil1, MAX_FIL);
		fprintf(stderr, "\t%d =< n_col (%d) =< %d\n", MIN_COL, n_col, MAX_COL);
		fclose(fit);
		exit(3);
	}

	if (!feof(fit))
		fscanf(fit, "%d %d %d %f\n", &mc.f, &mc.c, &mc.d, &mc.r);
	else
	{
		fprintf(stderr, "Falten parametres al fitxer \'%s\'\n", nom_fit);
		fclose(fit);
		exit(2);
	}
	if ((mc.f < 1) || (mc.f > n_fil1 - 3) ||
		(mc.c < 1) || (mc.c > n_col - 2) ||
		(mc.d < 0) || (mc.d > 3))
	{
		fprintf(stderr, "Error: parametres menjacocos incorrectes:\n");
		fprintf(stderr, "\t1 =< mc.f (%d) =< n_fil1-3 (%d)\n", mc.f, (n_fil1 - 3));
		fprintf(stderr, "\t1 =< mc.c (%d) =< n_col-2 (%d)\n", mc.c, (n_col - 2));
		fprintf(stderr, "\t0 =< mc.d (%d) =< 3\n", mc.d);
		fclose(fit);
		exit(4);
	}

	if (!feof(fit))
	{
		while (!feof(fit))
		{
			fscanf(fit, "%d %d %d %f\n", &f1[nFantasmes].f, &f1[nFantasmes].c, &f1[nFantasmes].d, &f1[nFantasmes].r);
			if ((f1[nFantasmes].f < 1) ||
				(f1[nFantasmes].f > n_fil1 - 3) ||
				(f1[nFantasmes].c < 1) ||
				(f1[nFantasmes].c > n_col - 2) ||
				(f1[nFantasmes].d < 0) ||
				(f1[nFantasmes].d > 3))
			{
				fprintf(stderr, "Error: parametres fantasma %d incorrectes:\n", nFantasmes);
				fprintf(stderr, "\t1 =< f1.f (%d) =< n_fil1-3 (%d)\n", f1[nFantasmes].f, (n_fil1 - 3));
				fprintf(stderr, "\t1 =< f1.c (%d) =< n_col-2 (%d)\n", f1[nFantasmes].c, (n_col - 2));
				fprintf(stderr, "\t0 =< f1.d (%d) =< 3\n", f1[nFantasmes].d);
				fclose(fit);
				exit(5);
			}
			nFantasmes++;
		}
	}
	else
	{
		fprintf(stderr, "Falten parametres al fitxer '%s'n", nom_fit);
		fclose(fit);
		exit(2);
	}
	fclose(fit); /* fitxer carregat: tot OK! */
	printf("Joc del MenjaCocos\n\tTecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
		   TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
	printf("prem una tecla per continuar:\n");
	getchar();
}

/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc(void)
{
	int r, i, j;
	char strin[50];
	i = 0;

	r = win_carregatauler(tauler, n_fil1 - 1, n_col, c_req);
	if (r == 0)
	{
		mc.a = win_quincar(mc.f, mc.c);
		if (mc.a == c_req)
			r = -6; /* error: menjacocos sobre pared */
		else
		{
			f1[i].a = win_quincar(f1[i].f, f1[i].c);
			if (f1[i].a == c_req)
				r = -7; /* error: fantasma sobre pared */
			else
			{
				cocos = 0; /* compta el numero total de cocos */
				for (i = 0; i < n_fil1 - 1; i++)
					for (j = 0; j < n_col; j++)
						if (win_quincar(i, j) == '.')
							cocos++;

			for(int z=1; z<nFantasmes;z++)
              {
                printf("Numero de fantasma a pintar %d", z);
                win_escricar(f1[z].f,f1[z].c,'1',NO_INV);
              }

				win_escricar(mc.f, mc.c, '0', NO_INV);
				win_escricar(f1[i].f, f1[i].c, '1', NO_INV);

				if (mc.a == '.')
					cocos--; /* menja primer coco */

				sprintf(strin, "Cocos: %d", cocos);
				win_escristr(strin);
			}
		}
	}
	if (r != 0)
	{
		win_fi();
		fprintf(stderr, "Error: no s'ha pogut inicialitzar el joc:\n");
		switch (r)
		{
		case -1:
			fprintf(stderr, "  nom de fitxer erroni\n");
			break;
		case -2:
			fprintf(stderr, "  numero de columnes d'alguna fila no coincideix amb l'amplada del tauler de joc\n");
			break;
		case -3:
			fprintf(stderr, "  numero de columnes del laberint incorrecte\n");
			break;
		case -4:
			fprintf(stderr, "  numero de files del laberint incorrecte\n");
			break;
		case -5:
			fprintf(stderr, "  finestra de camp de joc no oberta\n");
			break;
		case -6:
			fprintf(stderr, "  posicio inicial del menjacocos damunt la pared del laberint\n");
			break;
		case -7:
			fprintf(stderr, "  posicio inicial del fantasma damunt la pared del laberint\n");
			break;
		}
		exit(7);
	}
}

/* funcio per moure un fantasma una posicio; retorna 1 si el fantasma   */
/* captura al menjacocos, 0 altrament					*/
int *mou_fantasma(void *pos)
{
	objecte seg;
	int ret;
	int k, vk, nd, vd[3], p;
	int posicio = (intptr_t)pos;
	p = 0;

	if (posicio < 0)
	{
		fprintf(stderr, "Error: index incorrecte\n");
		exit(6);
	}
	do
	{
		nd = 0;
		ret = 0;

		if ((p % 2) == 0)
		{ // Ralentitza fantasma a 2*retard

			for (k = -1; k <= 1; k++)
			{ /* provar direccio actual i dir. veines */

				vk = (f1[posicio].d + k) % 4; /* direccio veina */
				if (vk < 0)
					vk += 4; /* corregeix negatius */

				seg.f = f1[posicio].f + df[vk]; /* calcular posicio en la nova dir.*/
				seg.c = f1[posicio].c + dc[vk];
				pthread_mutex_lock(&s);
				seg.a = win_quincar(seg.f, seg.c); /* calcular caracter seguent posicio */
				pthread_mutex_unlock(&s);

				if ((seg.a == ' ') || (seg.a == '.') || (seg.a == '0'))
				{
					vd[nd] = vk; /* memoritza com a direccio possible */
					nd++;
				}
			}

			if (nd == 0)
			{											 /* si no pot continuar, */
				f1[posicio].d = (f1[posicio].d + 2) % 4; /* canvia totalment de sentit */
			}
			else
			{
				if (nd == 1)
				{						   /* si nomes pot en una direccio */
					f1[posicio].d = vd[0]; /* li assigna aquesta */
				}
				else
				{									 /* altrament */
					f1[posicio].d = vd[rand() % nd]; /* segueix una dir. aleatoria */
				}
				seg.f = f1[posicio].f + df[f1[posicio].d]; /* calcular seguent posicio final */
				seg.c = f1[posicio].c + dc[f1[posicio].d];
				pthread_mutex_lock(&s);
				seg.a = win_quincar(seg.f, seg.c);								   /* calcular caracter seguent posicio */
				win_escricar(f1[posicio].f, f1[posicio].c, f1[posicio].a, NO_INV); /* esborra posicio anterior */
				pthread_mutex_unlock(&s);
				f1[posicio].f = seg.f;
				f1[posicio].c = seg.c;
				f1[posicio].a = seg.a; /* actualitza posicio */
				pthread_mutex_lock(&s);
				win_escricar(f1[posicio].f, f1[posicio].c, '1', NO_INV); /* redibuixa fantasma */
				pthread_mutex_unlock(&s);

				if (f1[posicio].a == '0')
					ret = 1; /* ha capturat menjacocos */
			}

			if (fi2 == 0)
				fi2 = ret;
		}
		win_retard(retard);
		p++;
	} while (!fi1 && !fi2);
}

/* funcio per moure el menjacocos una posicio, en funcio de la direccio de   */
/* moviment actual; retorna -1 si s'ha premut RETURN, 1 si s'ha menjat tots  */
/* els cocos, i 0 altrament */
int *mou_menjacocos(void *nul)
{
	char strin[120];
	objecte seg;
	int tec, ret;

	do
	{

		ret = 0;
		pthread_mutex_lock(&s);
		tec = win_gettec();
		pthread_mutex_unlock(&s);
		if (tec != 0)
			switch (tec) /* modificar direccio menjacocos segons tecla */
			{
			case TEC_AMUNT:
				mc.d = 0;
				break;
			case TEC_ESQUER:
				mc.d = 1;
				break;
			case TEC_AVALL:
				mc.d = 2;
				break;
			case TEC_DRETA:
				mc.d = 3;
				break;
			case TEC_RETURN:
				ret = -1;
				break;
			}
		seg.f = mc.f + df[mc.d]; /* calcular seguent posicio */
		seg.c = mc.c + dc[mc.d];
		pthread_mutex_lock(&s);
		seg.a = win_quincar(seg.f, seg.c); /* calcular caracter seguent posicio */
		pthread_mutex_unlock(&s);
		if ((seg.a == ' ') || (seg.a == '.'))
		{
			pthread_mutex_lock(&s);
			win_escricar(mc.f, mc.c, ' ', NO_INV); /* esborra posicio anterior */
			pthread_mutex_unlock(&s);
			mc.f = seg.f;
			mc.c = seg.c; /* actualitza posicio */
			pthread_mutex_lock(&s);
			win_escricar(mc.f, mc.c, '0', NO_INV); /* redibuixa menjacocos */
			pthread_mutex_unlock(&s);

			if (seg.a == '.')
			{
				cocos--;
				pthread_mutex_lock(&s);
				sprintf(strin, "Cocos: %d", cocos);
				pthread_mutex_unlock(&s);
				win_escristr(strin);
				if (cocos == 0)
					ret = 1;
			}
		}

		win_retard(retard);
		fi1 = ret;

	} while (!fi1 && !fi2);
}

void *counter(void *nul) // Funció auxiliar que ens serveix de comptador de temps
{
	char strin[35];
	segons = 0;
	minuts = 0;
	do
	{
		pthread_mutex_lock(&s);
		sprintf(strin, "Cocos: %03d  Temps: %02d:%02d", cocos, minuts, segons);
		win_escristr(strin);
		pthread_mutex_unlock(&s);
		win_retard(1000);
		segons++;
		if (segons == 60)
		{
			minuts++;
			segons = 0;
		}
	} while (!fi1 && !fi2);
}

/* programa principal				    */
int main(int n_args, const char *ll_args[])
{
	/* DECLARACIO THREADS */

	pthread_t t_coco;
	pthread_t tFantasmes[MAX_THREADS];
	pthread_t tCounter;

	int rc; /* variables locals */
	int i;

	srand(getpid()); /* inicialitza numeros aleatoris */

	if ((n_args != 2) && (n_args != 3))
	{
		fprintf(stderr, "Comanda: cocos0 fit_param [retard]\n");
		exit(1);
	}
	carrega_parametres(ll_args[1]);

	if (n_args == 3)
		retard = atoi(ll_args[2]);
	else
		retard = 100;

	rc = win_ini(&n_fil1, &n_col, '+', INVERS); /* intenta crear taulell */
	if (rc == 0)								/* si aconsegueix accedir a l'entorn CURSES */
	{
		inicialitza_joc();
		fi1 = 0;
		fi2 = 0;
		pthread_mutex_init(&s, NULL);
		pthread_create(&t_coco, NULL, (void* (*)(void*))mou_menjacocos, NULL);
		for (i = 0; i < nFantasmes; i++)
		{
			pthread_create(&tFantasmes[i], NULL, (void* (*)(void*))mou_fantasma, (void*)(intptr_t)i);

		}
		pthread_create(&tCounter, NULL, counter, 0);
		while (!fi1 && !fi2);
		pthread_join(t_coco, NULL);
		for (i = 0; i < nFantasmes; i++)
		{
			pthread_join(tFantasmes[i], NULL);
		}
		pthread_join(tCounter, NULL);
		pthread_mutex_destroy(&s);
		win_fi();

		if (fi1 == -1)
			printf("S'ha aturat el joc amb tecla RETURN!\n");
		else
		{
			if (fi1)
				printf("Ha guanyat l'usuari!\n");
			else
				printf("Ha guanyat l'ordinador!\n");
		}
	}
	else
	{
		fprintf(stderr, "Error: no s'ha pogut crear el taulell:\n");
		switch (rc)
		{
		case -1:
			fprintf(stderr, "camp de joc ja creat!\n");
			break;
		case -2:
			fprintf(stderr, "no s'ha pogut inicialitzar l'entorn de curses!\n");
			break;
		case -3:
			fprintf(stderr, "les mides del camp demanades son massa grans!\n");
			break;
		case -4:
			fprintf(stderr, "no s'ha pogut crear la finestra!\n");
			break;
		}
		exit(6);
	}
	return (0);
}
