#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define ECHOMAX 255

void mandarcomando(int sock, int sock_size, int *end_ss);

int main(int argc, char **argv){
    int sock;
    /* Estrutura: familia + endereco IP + porta */
    struct sockaddr_in client, server;
    
    /* Criando Socket */
    if ((sock = socket(AF_INET,SOCK_DGRAM,0)) < 0)
        printf("Socket Falhou!!!\n");
    
    int sock_size = sizeof(client);
    char linha[ECHOMAX];
    char aux_exit[ECHOMAX];
    int menu;

 
    /* Consistencia */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Central IP> \n", argv[0]);
        exit (1);
    }
    
    /* Construindo a estrutura de endereco local ----> Se não criar o SO criará */
    bzero((char *)&client, sock_size);
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = htonl(INADDR_ANY); /* endereco IP local */
    client.sin_port = htons(0); /* porta local (0=auto assign) */
    bind(sock, (struct sockaddr *)&client, sock_size);
    
    /* Construindo a estrutura de endereco do destino
     A funcao bzero eh usada para colocar zeros na estrutura server */
    bzero((char *)&server, sock_size);
    server.sin_family=AF_INET;
    /* endereco IP de destino */
    server.sin_addr.s_addr=inet_addr(argv[1]); /* host local */
    server.sin_port=htons(6000); /* porta do servidor */
    
    do{
        printf("Digite 'IN' pra entrar na rede\n");
        fgets (linha, ECHOMAX, stdin);
        linha[strcspn(linha, "\n")] = 0;
        sendto(sock, linha, ECHOMAX, 0, (struct sockaddr *)&server, sock_size);
    }while(strcmp(linha, "IN"));
    
    printf("Parabéns! Você entrou\n");

    do {
        char menu[3];
        printf("\nAguarde a central...\n");

        recvfrom(sock, menu, ECHOMAX, 0,(struct sockaddr *)&server, &sock_size);

        switch(menu[0]){ //'C' manda comandos,   'S' serve
            case('C'):
                printf("--------- COMANDANDO --------- \n");
                printf("Digite o comando que deseja executar: "); 
                fgets (linha, ECHOMAX, stdin);
                linha[strcspn(linha, "\n")] = 0;
                sendto(sock, linha, ECHOMAX, 0, (struct sockaddr *)&server, sock_size);
                strcpy(aux_exit, linha);
                
                //faz o que ele mesmo mandou
                system(linha);
                
                //Le as saída do outro
                do{            
                    recvfrom(sock, linha, ECHOMAX, 0,(struct sockaddr *)&server, &sock_size);
                    if(strcmp(linha, "OUT") != 0)
                        printf("%i | %s", ntohs(server.sin_port), linha);  //printa id porta antes de printar a resposta
                }while(strcmp(linha, "OUT"));    
                memset(linha, '\0', ECHOMAX);
                break;
            
            case('S'):
                printf("--------- SERVINDO ---------\n");
                
                break;
            default:
                printf("Digitado %s\n", menu);
        }
        memset(menu, '\0', 3);

    } while(strcmp(aux_exit,"exit"));
    close(sock);
    return 0;
}