#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include "memoria.h"
#include "winsuport2.h"

#define MIN_FIL 7
#define MAX_COL 25
#define MAX_FIL 80
#define MIN_COL 10

#define MAX_THREADS 10 // 9 Fantasmes + comecocos

typedef struct
{            /* per un objecte (menjacocos o fantasma) */
    int f;   /* posicio actual: fila */
    int c;   /* posicio actual: columna */
    int d;   /* direccio actual: [0..3] */
    char a;  /* caracter anterior en pos. actual */
    float r; /* per indicar un retard relati */
} objecte;

int df[] = {-1, 0, 1, 0}; /* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1}; /* dalt, esquerra, baix, dreta */
int *fi1_p;
int *fi2_p;
int fi2;
objecte *mc;
int *retard;
objecte *f1;

int main(int n_args, const char *ll_args[])
{

    void *p_win;
    int id_fi1 = atoi(ll_args[1]);
    int id_fi2 = atoi(ll_args[2]);
    int p_id = atoi(ll_args[3]);
    int id_f1 = atoi(ll_args[4]);
    int id_mc = atoi(ll_args[5]);
    int id_retard = atoi(ll_args[6]);
    int id_win = atoi(ll_args[7]);
    int n_fil = atoi(ll_args[8]);
    int n_col = atoi(ll_args[9]);

    fi1_p = map_mem(id_fi1);
    fi2_p = map_mem(id_fi2);
    mc = map_mem(id_mc);
    retard = map_mem(id_retard);
    f1 = map_mem(id_f1);

    objecte fantasmes = f1[p_id];

    p_win = map_mem(id_win);
    win_set(p_win, n_fil, n_col);

    objecte seg;
    int ret = 0;
    int k, vk, nd, vd[3];
    do
    {
        nd = 0;                   // numero de direccions disponibles
        for (k = -1; k <= 1; k++) /* provar direccio actual i dir. veines */
        {
            vk = (fantasmes.d + k) % 4; /* direccio veina */
            if (vk < 0)
                vk += 4; /* corregeix negatius */

            seg.f = fantasmes.f + df[vk]; /* calcular posicio en la nova dir.*/
            seg.c = fantasmes.c + dc[vk];

            seg.a = win_quincar(seg.f, seg.c); /* calcular caracter seguent posicio */

            if ((seg.a == ' ') || (seg.a == '.') || (seg.a == '0'))
            {
                vd[nd] = vk; /* memoritza com a direccio possible */
                nd++;
            }
        }
        if (nd == 0)
        {                                        /* si no pot continuar, */
            fantasmes.d = (fantasmes.d + 2) % 4; /* canvia totalment de sentit */
        }
        else
        {
            if (nd == 1)
            {                        /* si nomes pot en una direccio */
                fantasmes.d = vd[0]; /* li assigna aquesta */
            }
            else
            {                                  /* altrament */
                fantasmes.d = vd[rand() % nd]; /* segueix una dir. aleatoria */
            }
            seg.f = fantasmes.f + df[fantasmes.d]; /* calcular seguent posicio final */
            seg.c = fantasmes.c + dc[fantasmes.d];
            seg.a = win_quincar(seg.f, seg.c);                           /* calcular caracter seguent posicio */
            win_escricar(fantasmes.f, fantasmes.c, fantasmes.a, NO_INV); /* esborra posicio anterior */

            fantasmes.f = seg.f;
            fantasmes.c = seg.c;
            fantasmes.a = seg.a;                                        /* actualitza posicio */
            win_escricar(fantasmes.f, fantasmes.c, '1' + p_id, NO_INV); /* redibuixa fantasma */

            if (fantasmes.a == '0')
            {
                ret = 1; /* ha capturat menjacocos */
                fprintf(stderr, "Fantasma %d capturat\n", p_id);
            }
        }

        *fi2_p = ret;

        win_retard((mc->r) * (int)*retard);

    } while (!(*fi1_p) && !(*fi2_p));

    return (fi2);
}