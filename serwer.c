#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <math.h>

#define MAXLINE 4096
#define LISTENQ 5

// Funkcja do obsługi błędów
int err (char* s){
   printf ("%s \n", s);
   printf ("Errno: %d \n", errno);
   fprintf (stderr, "%s \n", strerror(errno));
   exit(-1);
}

// Funkcja do liczenia wyniku
// strchr() wyszukuje w łańcuchu c znak ' ' i zwraca wskaźnik do pierwszego
// znalezionego znaku lub wartość ujemną kiedy go nie znajdzie
double* wynik (int a, int b, int c) {
    double* x;
    x = (double*)malloc(3*sizeof(double));
    double delta, x1, x2 = 0;
    x[2] = 1;
    delta = (pow(b, 2.0))-4*a*c;
    if (delta < 0) { x[2] = -1 ; return x;}
    else
    {
        double pidelta = pow(delta, (0.5));
        x[0] = (-b-pidelta)/(2*a);
        x[1] = (-b+pidelta)/(2*a);
        printf ("delta: %f \n", delta);
        printf ("pidelta: %f \n", pidelta);
        printf ("a: %d \n", a);
        printf ("b: %d \n", b);
        printf ("c: %d \n", c);
        return x;
    }
}

void sig_chld(int signo){
	pid_t pid;
	int stat;
	pid = wait(&stat);
	printf("potomek zamknięty: pid = %d",pid);
	return;
}

int main( int argc, char* argv[] ) {

    int a, b, c =0;

   // numer portu wprowadzamy pzy uruchomieniu serwera
   //kompilacja: gcc serwer.c -o serwer -w -lm
   //wywołanie: ./serwer 1234
   if (argc!=2) printf("Brak nr portu. Uruchomienie: ./serwer1 nr_portu \n");

   int port = atoi(argv[1]);
   int connfd, cli_len, i, n, sbuf_len;
   double* res;

   char* buffer[MAXLINE+1];
   struct sockaddr_in servaddr, cliaddr;

   int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
   if (sockfd == -1)  err("Błąd Socketa");

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr = htons(INADDR_ANY);
   servaddr.sin_port = htons(port);

   bind (sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
   if (listen(sockfd, LISTENQ) != 0) err("Błąd nasłuchu");

   int pid;
   
	signal(SIGCHLD,sig_chld);

   cli_len = sizeof(cliaddr);

   // ustanowienie połączenia z klientem
   connfd = accept(sockfd, (struct sockaddr*) &cliaddr, &cli_len);
    if (connfd < 0) err("Błąd połączenia");

   if(pid = fork() == 0){
	close(sockfd);
   while(1) {

    // odczyt danych od klienta przez bufor
    n = read(connfd, buffer, MAXLINE);
    if (n < 0) err("Błąd odczytu z socketa");

	a = atoi(buffer);
	bzero(&buffer, MAXLINE+1);
	printf("pierwsza odebrana liczba :%d\n",a);
	n = read(connfd, buffer, MAXLINE);
    if (n < 0) err("Błąd odczytu z socketa");

	b = atoi(buffer);
	bzero(&buffer, MAXLINE+1);
	printf("druga odebrana liczba: %d\n",b);
	n = read(connfd, buffer, MAXLINE);
    if (n < 0) err("Błąd odczytu z socketa");

    c = atoi(buffer);
    res = malloc(3*sizeof(double));
    res = wynik(a, b, c); // wyliczenie wyniku funkcją wynik
    if (res[2] > 0) printf("pierwiastki równania: x1 = %f, x2 = %f\n",(double)res[0], (double)res[1]);
	bzero(&buffer, MAXLINE+1);


    // zapisanie komunikatu z wynikiem do bufora
    if (res[2] > 0) sprintf(buffer, "Wynik z serwera: x1 = %.2f, x2 = %.2f", (double)res[0] , (double)res[1]);
	else sprintf(buffer, "delta ujemna, podaj inne współczynniki\n");

    // wysłanie wyniku do klienta przez bufor
    n = write(connfd, buffer, MAXLINE);
    if (n < 0) err("Błąd zapisu do socketa");
    n = 0;
    bzero(&buffer, MAXLINE+1);
  }
   close(connfd);
   exit(0);
   }else{
	close(connfd);
   }	

   close(connfd);
   close(sockfd);
}
