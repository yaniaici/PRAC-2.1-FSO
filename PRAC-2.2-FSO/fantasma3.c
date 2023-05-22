#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdlib.h>		/* per exit() */
#include <unistd.h>		/* per getpid() */
#include "winsuport2.h"		/* incloure definicions de funcions propies */
#include <pthread.h>
#include "memoria.h"
#include "semafor.h"

int df[] = { -1, 0, 1, 0 };	/* moviments de les 4 direccions possibles */
int dc[] = { 0, -1, 0, 1 };	/* dalt, esquerra, baix, dreta */

void coco_a_la_direccio(int fantasma, int fila, int columna, int direccio, int quantitat_fantasmes, int *fila_coco, int *columna_coco, int bustia);
int coco_visible(int bustia, int numFantasmes, int fantasmaActual);

/*
*/
int main(int n_args, char *arg[])
{
	int id_fantasma = 	atoi(arg[1]);
	int fila 		= 	atoi(arg[2]);
	int columna 	=	atoi(arg[3]);
	int direccio 	= 	atoi(arg[4]);
	int id_win 		=	atoi(arg[5]);
	int n_fil 		=	atoi(arg[6]);
	int n_col 		=	atoi(arg[7]);
	int retard 		= 	atoi(arg[8]);
	int id_fi2		= 	atoi(arg[9]);
	int id_sem		= 	atoi(arg[10]);
	int id_bustia	= 	atoi(arg[11]);
	int numfantasmes=	atoi(arg[12]);
	int id_filacoco	=	atoi(arg[13]);
	int id_colucoco	=	atoi(arg[14]);
	int *fi2 		=	(int *)map_mem(id_fi2);
	void *p_win		=	map_mem(id_win);
	int *fila_coco 	=	(int*)map_mem(id_filacoco);
	int *colu_coco	=	(int*)map_mem(id_colucoco);
	char caracter 	= '.';
	char fant = id_fantasma + '1';
	win_set(p_win, n_fil, n_col);
	win_escricar(fila, columna, '1', NO_INV);
	//MOURE FANTASMA
	int p, nd, i, vk, vd[3], fantasma_que_veu_al_coco;
	int seguent_fila, seguent_columna, seguent_caracter;
	unsigned int attr;
	unsigned int sattr;
	p = 0;
	do
	{
		nd = 0;
		if ((p % 2) == 0)
		{
			fantasma_que_veu_al_coco = coco_visible(id_bustia,numfantasmes,id_fantasma);
			if (caracter == '+')
			{
				attr = INVERS;
			}
			else
			{
				attr = NO_INV;
			}

			if (fantasma_que_veu_al_coco != id_fantasma)
			{
				for (i = -1; i <= 1; i++)
				{
					vk = (direccio + i) % 4;
					if (vk < 0) vk += 4;
					seguent_fila = fila + df[vk];
					seguent_columna = columna + dc[vk];
					waitS(id_sem);
					seguent_caracter = win_quincar(seguent_fila, seguent_columna);
					signalS(id_sem);
					if (seguent_caracter == ' ' || seguent_caracter == '.' || seguent_caracter == '0')
					{
						vd[nd] = vk;
						nd++;
					}
				}
			}
			if (nd == 0 && fantasma_que_veu_al_coco != id_fantasma)
			{
				direccio = (direccio + 2) % 4;
			}
			else
			{
				if (fantasma_que_veu_al_coco != id_fantasma)
				{
					if (nd == 1)
					{
						direccio = vd[0];
					}
					else
					{
						direccio = vd[rand() % nd];
					}
					seguent_fila = fila + df[direccio];
					seguent_columna = columna + dc[direccio];
				}
				else
				{
					if (*fila_coco < fila) seguent_fila = fila - 1;
					else if (*fila_coco > fila) seguent_fila = fila + 1;
					else seguent_fila = fila;
					if (*colu_coco < columna) seguent_columna = columna - 1;
					else if (*colu_coco > columna) seguent_columna = columna + 1;
					else seguent_columna = columna;
				}
				seguent_caracter = win_quincar(seguent_fila, seguent_columna);
				if (seguent_caracter == '.' || seguent_caracter == ' ' || seguent_caracter == '0' || seguent_caracter == '+')
				{
					if (fantasma_que_veu_al_coco == id_fantasma) sattr = INVERS;
					else sattr = NO_INV;
					waitS(id_sem);
					win_escricar(fila, columna, caracter, attr);
					win_escricar(seguent_fila, seguent_columna, fant, sattr);
					signalS(id_sem);
					fila = seguent_fila;
					columna = seguent_columna;
					caracter = seguent_caracter;
				}
			}
			if (caracter == '0')
			{
				*fi2 = 1;
			}
			coco_a_la_direccio(id_fantasma,fila,columna, direccio, numfantasmes, fila_coco, colu_coco, id_bustia);
		}
		win_retard(retard);
		p++;
	}while(!(*fi2));
	//FI MOURE FANTASMA

	return 0;
}

int coco_visible(int bustia, int numFantasmes, int fantasmaActual)
{
	int i, fantasmaTrobat, e;
	char c, missatge[3];
	i = 0; 
	fantasmaTrobat = -1;
	while (i < numFantasmes && fantasmaTrobat != fantasmaActual)
	{
		receiveM(bustia,&missatge);
		sscanf(missatge, "%i%c", &e, &c);
		sendM(bustia,missatge, sizeof(missatge));
		if (c == 's' && fantasmaTrobat != fantasmaActual)
		{
			fantasmaTrobat = fantasmaActual;
		}
		i++;
	}
	return fantasmaTrobat;
}

void coco_a_la_direccio(int fantasma, int fila, int columna, int direccio, int quantitat_fantasmes, int *fila_coco, int *columna_coco, int bustia)
{
	char c = win_quincar(fila,columna), missatge[3], nc;
	int i, e;
	do{
		fila += df[direccio];
		columna += dc[direccio];
		c = win_quincar(fila,columna);
	}while (c != '+' && c != '0');
	if (c == '0')
	{
		nc = 's';
		*fila_coco = fila;
		*columna_coco = columna;
	}
	else nc = 'n';
	for (i = 0; i < quantitat_fantasmes; i++)
	{
		receiveM(bustia,&missatge);
		sscanf(missatge, "%i%c", &e, &c);
		if (e == fantasma)
			sprintf(missatge, "%i%c", e, nc);
		sendM(bustia,missatge, sizeof(missatge));
	}
}