#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
  #include <cstdlib>
  #define MALLOC(T,n) (T*)std::malloc(sizeof(T)*(n))
  #define FREE(p) std::free(p)
#else
  #define MALLOC(T,n) (T*)malloc(sizeof(T)*(n))
  #define FREE(p) free(p)
#endif

typedef struct { int r, c; } Ponto;

typedef enum {
    LIMPAR,
    MOVER_N,
    MOVER_S,
    MOVER_L,
    MOVER_O,
    FICAR
} Acao;

typedef struct {
    Acao *v;
    int cap, ini, sz;
} Log;

typedef struct {
    int N, M, T;
    char **g;
    Ponto S;
    int sujeira_total;
} Mapa;

void log_init(Log *L, int cap){
    L->v = MALLOC(Acao, cap);
    L->cap = cap;
    L->ini = 0;
    L->sz = 0;
}

void log_push(Log *L, Acao a){
    int pos;
    if(L->sz < L->cap){
        pos = (L->ini + L->sz) % L->cap;
        L->v[pos] = a;
        L->sz++;
    }else{
        L->v[L->ini] = a;
        L->ini = (L->ini + 1) % L->cap;
    }
}

void log_free(Log *L){
    FREE(L->v);
}

const char* nome_acao(Acao a){
    switch(a){
        case LIMPAR: return "LIMPAR";
        case MOVER_N: return "MOVER NORTE";
        case MOVER_S: return "MOVER SUL";
        case MOVER_L: return "MOVER LESTE";
        case MOVER_O: return "MOVER OESTE";
        case FICAR: return "FICAR";
        default: return "?";
    }
}

int dentro(Mapa *M, int r, int c){
    return r>=0 && r<M->N && c>=0 && c<M->M;
}

int eh_sujo(Mapa *M, int r, int c){
    return dentro(M,r,c) && M->g[r][c]=='*';
}

int eh_bloqueio(Mapa *M, int r, int c){
    return !dentro(M,r,c) || M->g[r][c]=='#';
}

void imprimir_mapa(Mapa *M, Ponto pos){
    for(int i=0;i<M->N;i++){
        for(int j=0;j<M->M;j++){
            if(i==pos.r && j==pos.c) printf("R");
            else printf("%c", M->g[i][j]);
        }
        printf("\n");
    }
}

Acao decide_reflex(Mapa *M, Ponto pos, char *motivo){

    if(M->g[pos.r][pos.c]=='*'){
        strcpy(motivo,"Regra 1: celula atual suja");
        return LIMPAR;
    }

    if(eh_sujo(M,pos.r-1,pos.c)){ strcpy(motivo,"Regra 2: sujeira ao norte"); return MOVER_N; }
    if(eh_sujo(M,pos.r+1,pos.c)){ strcpy(motivo,"Regra 2: sujeira ao sul"); return MOVER_S; }
    if(eh_sujo(M,pos.r,pos.c+1)){ strcpy(motivo,"Regra 2: sujeira ao leste"); return MOVER_L; }
    if(eh_sujo(M,pos.r,pos.c-1)){ strcpy(motivo,"Regra 2: sujeira ao oeste"); return MOVER_O; }


//===================================================
if(pos.r % 2 == 0){
    if(!eh_bloqueio(M,pos.r,pos.c+1)){ strcpy(motivo,"Regra 3: zig-zag leste"); return MOVER_L; }
    if(!eh_bloqueio(M,pos.r+1,pos.c)){ strcpy(motivo,"Regra 3: zig-zag sul"); return MOVER_S; }
    if(!eh_bloqueio(M,pos.r,pos.c-1)){ strcpy(motivo,"Regra 3: zig-zag oeste"); return MOVER_O; }
}else{
    if(!eh_bloqueio(M,pos.r,pos.c-1)){ strcpy(motivo,"Regra 3: zig-zag oeste"); return MOVER_O; }
    if(!eh_bloqueio(M,pos.r+1,pos.c)){ strcpy(motivo,"Regra 3: zig-zag sul"); return MOVER_S; }
    if(!eh_bloqueio(M,pos.r,pos.c+1)){ strcpy(motivo,"Regra 3: zig-zag leste"); return MOVER_L; }
}

    if(!eh_bloqueio(M,pos.r-1,pos.c)){ strcpy(motivo,"Fallback norte"); return MOVER_N; }
    if(!eh_bloqueio(M,pos.r+1,pos.c)){ strcpy(motivo,"Fallback sul"); return MOVER_S; }

    strcpy(motivo,"Fallback ficar");
    return FICAR;
}

int aplicar_acao(Mapa *M, Ponto *pos, Acao a, int *limpezas, int *bloqueios){

    int nr=pos->r, nc=pos->c;

    if(a==LIMPAR){
        if(M->g[pos->r][pos->c]=='*'){
            M->g[pos->r][pos->c]='.';
            (*limpezas)++;
            M->sujeira_total--;
        }
        return 1;
    }

    if(a==MOVER_N) nr--;
    if(a==MOVER_S) nr++;
    if(a==MOVER_L) nc++;
    if(a==MOVER_O) nc--;

    if(a!=FICAR){
        if(eh_bloqueio(M,nr,nc)){
            (*bloqueios)++;
            return 0;
        }
        pos->r = nr;
        pos->c = nc;
    }

    return 1;
}

int main(){

    Mapa M;
    int passo=0, limpezas=0, bloqueios=0;
    int modo=0;
    char motivo[128];

    printf("Digite N M T:\n");
    scanf("%d %d %d",&M.N,&M.M,&M.T);

    M.g = MALLOC(char*, M.N);
    for(int i=0;i<M.N;i++)
        M.g[i] = MALLOC(char, M.M+1);

    M.sujeira_total = 0;

    printf("Digite o mapa:\n");

    for(int i=0;i<M.N;i++){
        scanf("%s", M.g[i]);
        for(int j=0;j<M.M;j++){
            if(M.g[i][j]=='S'){
                M.S.r=i;
                M.S.c=j;
                M.g[i][j]='.';
            }
            if(M.g[i][j]=='*') M.sujeira_total++;
        }
    }

    //========================================================================================
    printf("\nMapa carregado:\n");
    imprimir_mapa(&M, M.S);
    printf("Sujeira total: %d\n", M.sujeira_total);

    //========================================================================================

    printf("Modo passo-a-passo? (1=sim,0=nao): ");
    scanf("%d",&modo);

    Ponto pos = M.S;

    Log log;
    log_init(&log,64);

    clock_t inicio = clock();

    for(passo=0; passo<M.T && M.sujeira_total>0; passo++){

        Acao a = decide_reflex(&M,pos,motivo);
        int ok = aplicar_acao(&M,&pos,a,&limpezas,&bloqueios);
        log_push(&log,a);

        if(modo){
            printf("\nPasso %d\n",passo+1);
            printf("Acao: %s\n",nome_acao(a));
            printf("%s\n",motivo);
            printf("%s\n", ok ? "ok" : "bloqueado");
            imprimir_mapa(&M,pos);
            printf("Sujeira restante: %d\n",M.sujeira_total);
            getchar(); getchar();
        }
    }

    clock_t fim = clock();
    double tempo = (double)(fim-inicio)/CLOCKS_PER_SEC;

    int sujeira_inicial = limpezas + M.sujeira_total;
    double perc = (sujeira_inicial>0) ? (100.0*limpezas/sujeira_inicial) : 0;

    printf("\n===== RESULTADO FINAL =====\n");
    printf("Passos executados: %d / %d\n",passo,M.T);
    printf("Limpezas: %d\n",limpezas);
    printf("Bloqueios: %d\n",bloqueios);
    printf("Sujeira inicial: %d\n",sujeira_inicial);
    printf("Sujeira restante: %d\n",M.sujeira_total);
    printf("%% removida: %.2f%%\n",perc);
    printf("Tempo CPU: %.4f s\n",tempo);

    log_free(&log);

    for(int i=0;i<M.N;i++) FREE(M.g[i]);
    FREE(M.g);

    return 0;
}