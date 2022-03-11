#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

// defines importantes
#define READERS 12              // numero total de leitores
#define MAX_READERS 10          // numero maximo de leitores
#define WRITERS 1               // numero de escritores

// definição de variaveis importantes
pthread_mutex_t     acesso_dados;       // controla o acesso a dados       
pthread_mutex_t     mutex_rc;           // rc
sem_t               reader_semaphore;   // semaforo que controla a quantidade de leitores
int                 leitores_ativos;    // variavel que controla o numero de leitores ativos atualmente               


// funcoes
int read_data_base();
int realiza_acao_com_data();
int think_up_data();
void escreve_no_arquivo(int);
void reader();
void writer(); 

int main() {
    pthread_t readerthreads[READERS]; 
    pthread_t writerthreads[WRITERS];
    

    // inicia as threads
    pthread_mutex_init(&acesso_dados, NULL); 
    pthread_mutex_init(&mutex_rc, NULL);

    // inicia os semaforos
    sem_init(&reader_semaphore, 0, MAX_READERS); // inicia o semaforo que controla a quantidade de leitores maxima, a variavel MAX_READERS pode ser modificada para alterar o numero de leitores permitidos por vez


    // para evitar qualquer problema na implementação o arquivo eh criado caso nao exista e limpo caso ja esteja escrito (w+). Em seguida eh preenchido com um integer gerado aleatoriamente, para que nao seja iniciado vazio
  FILE *p = fopen("registro.txt", "w+");
  escreve_no_arquivo(think_up_data());


    // cria as threads para os escritores
  for (int i = 0; i < WRITERS; i++)
    pthread_create(&writerthreads[i], NULL, (void *) writer, NULL);

    // cria as threads para os leitores
  for (int i = 0; i < READERS; i++)
    pthread_create(&readerthreads[i], NULL, (void *) reader, NULL);

    // completa as threads dos escritores
  for (int i = 0; i < WRITERS; i++)
    pthread_join(writerthreads[i], NULL);

    // completa as threads dos leitores
  for (int i = 0; i < READERS; i++)
    pthread_join(readerthreads[i], NULL);

  return 0;
}

int use_data_read(int data) {
  data = data * 2 - 1;
  sleep(1);
  printf("Dado apos modificacao = %d.\n", data);
  return data;
}

int read_data_base() {
  sleep(1);
  int data = 0;
  FILE *p = fopen("registro.txt", "r+");
  fscanf(p, "%d", &data);
  fclose(p);
  printf("Dado lido = %d\n", data);
  printf("%d leitores ativos.\n", leitores_ativos);

  return data;
}

int think_up_data() {
  sleep(1);
  
  return ((rand() % 100) + leitores_ativos) ;
}

void escreve_no_arquivo(int data) {
  sleep(1);
  printf("Estamos escrevendo dados.\n");  
  FILE *p = fopen("registro.txt", "w");
  fprintf(p, "%d", data);
  fclose(p);
}

void reader() {
  int data;

  while (1) {
    sem_wait(&reader_semaphore);  // esse semaforo controla a quantidade de leitores, aqui eh feito um down nele

    pthread_mutex_lock(&mutex_rc);  // bloqueia o acesso a variavel mutex_rc (rc)
    leitores_ativos++;

    if (leitores_ativos == 1)  // se esse for o primeiro leitor, bloqueia o acesso a todos os writers
      pthread_mutex_lock(&acesso_dados);


    // libera o acesso a variavel mutex_rc (rc)
    pthread_mutex_unlock(&mutex_rc);

    // le o arquivo e guarda a informação na variavel data
    data = read_data_base();

    // bloqueia novamente o acesso a variavel mutex_rc (rc)
    pthread_mutex_lock(&mutex_rc);

    // decrementa o numero de leitores ativos, uma vez que a leitura foi concluida
    leitores_ativos--;


    // caso esse seja o ultimo leitor o acesso ao arquivo é liberado a todos
    if (!leitores_ativos) 
      pthread_mutex_unlock(&acesso_dados);
    
    // libera o acesso a variavel mutex_rc (rc)
    pthread_mutex_unlock(&mutex_rc); 


    // chama a função use_data_read para utilizar o dado que lido do arquivo
    use_data_read(data);

    // ao final faz um up no semaforo que controla a quantidade de leitores
    sem_post(&reader_semaphore);
  }
}

void writer() {
  int data;

  while (1) {
    data = think_up_data();     // gera um valor aleatorio para a variavel data
    pthread_mutex_lock(&acesso_dados);    // bloqueia o acesso aos dados
    escreve_no_arquivo(data);   // escreve no arquivo a informação gerada
    pthread_mutex_unlock(&acesso_dados);  // libera o acesso aos dados
  }
}