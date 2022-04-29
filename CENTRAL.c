#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#define ECHOMAX 255
//#define qnts_peers 2

int main(void) {
    int sock;
    /* Estrutura: familia + endereco IP + porta */
    struct sockaddr_in central, client;
    int sock_size = sizeof(central);

    //salva as portas dos peers relacionados
    int qnts_peers = 2;
    int port_Peer[qnts_peers];
    int i_peer; //indice peer

    /* Cria o socket para enviar e receber datagramas
     parametros(familia, tipo, protocolo) */
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        printf("ERRO na Criacao do Socket!\n");
    
    char linha[ECHOMAX];
    char aux_exit[ECHOMAX];

    
    FILE *fp; //pipe
    char path[1035]; 

    /* Construcao da estrutura do endereco local  */
    bzero((char *)&central, sock_size);
    central.sin_family = AF_INET;
    central.sin_addr.s_addr = htonl(INADDR_ANY); /* endereco IP local */
    central.sin_port = htons(6000); /* porta local  */
    
    /* Bind para o endereco local
     parametros(descritor socket, estrutura do endereco local, comprimento do endereco) */
    if(-1 != bind(sock, (struct sockaddr *)&central, sock_size)){
        printf("Esperando %i peers se 'inscreverem'...\n", qnts_peers);
        
        for(int i = 0; i < qnts_peers; i++){
            recvfrom(sock, linha, ECHOMAX, 0, (struct sockaddr *)&client, &sock_size); //recebe
            printf("\nRecebido: %s de %i\n", linha, ntohs(client.sin_port)); 
            
            if (strcmp(linha, "IN") == 0){
                port_Peer[i] = ntohs(client.sin_port);
                printf("Peer %i entrou, PORTA:%i\n", (i+1), ntohs(client.sin_port) );
                memset(linha, '\0', ECHOMAX);
            }else {
                printf("[ERRO] Recebido %s em vez de IN\n", linha);
                i--;
            }
        }
        printf("\nTodos os Peers foram inscritos!");  

        do{
            do{
                printf("\n\nDigite qual Peer vai mandar o comado (1 a %i) ou 0 para add novo peer\n", qnts_peers);
                scanf("%d", &i_peer);
                
                /* se quer add */
                if (i_peer==0){
                    recvfrom(sock, linha, ECHOMAX, 0, (struct sockaddr *)&client, &sock_size); //recebe
                    printf("\nRecebido: %s de %i\n", linha, ntohs(client.sin_port)); 
                    
                    if (strcmp(linha, "IN") == 0){
                        port_Peer[qnts_peers] = ntohs(client.sin_port);
                        qnts_peers++;
                        printf("Peer adicionado: %i - PORTA: %i\n", qnts_peers, ntohs(client.sin_port) );
                        printf("Total de %i peers na rede\n", qnts_peers);
                        memset(linha, '\0', ECHOMAX);
                    }else {
                        printf("[ERRO] Não foi possivel add um novo peer. Tente novamente\n");
                    }
                }

            }while(i_peer<1 || i_peer>qnts_peers); //repete até ser algo entre 1 e qntos peers existentes 
            
            i_peer--; //arruma o índice
            for(int i = 0; i < qnts_peers; i++){
                if(i==i_peer){
                    client.sin_port = htons(port_Peer[i_peer]); //para mandar comandos
                    sendto(sock, "C", ECHOMAX, 0, (struct sockaddr *)&client, sock_size);
                }else{
                    client.sin_port = htons(port_Peer[i]); //para receber comandos
                    sendto(sock, "S", ECHOMAX, 0, (struct sockaddr *)&client, sock_size);
                }
            }
            
            printf("-------- RECEBENDO COMANDOS -------\n");    
            
            recvfrom(sock, linha, ECHOMAX, 0, (struct sockaddr *)&client, &sock_size); //recebe
            strcpy(aux_exit, linha);    //copia para auxiliar de saída
            printf("%s\n", linha);

            /* Abre o comando para leitura */
            fp = popen(linha, "r");
            
            /* Lê o output linha a linha e envia. */
            while (fgets(path, sizeof(path), fp) != NULL) {
                printf("%s", path);
                sendto(sock, path, ECHOMAX, 0, (struct sockaddr *)&client, sock_size);
            }
            sendto(sock, "OUT", ECHOMAX, 0, (struct sockaddr *)&client, sock_size);

            /* close */
            pclose(fp);

            memset(linha, '\0', ECHOMAX); //limpa memoria 
        }while(strcmp(aux_exit, "exit"));
    }else puts("Porta ocupada");
    close(sock);
    return 0;
}
