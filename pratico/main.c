/*
 * Compile:
 *
 *   gcc sctptest.c -o server -lsctp -Wall
 *   ln -s server client
 *
 * Invoke:
 *
 *   ./client
 *   ./server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MY_PORT_NUM 6000
#define SIZE 1024

static void die(const char *s) {
        perror(s);
        exit(1);
}

static void server(void) {
        int listen_fd, conn_fd, flags, ret, in;
        struct sctp_sndrcvinfo sndrcvinfo;
        struct sockaddr_in servaddr = {
                .sin_family = AF_INET,
                .sin_addr.s_addr = htonl(INADDR_ANY),
                .sin_port = htons(MY_PORT_NUM),
        };
        struct sctp_initmsg initmsg = {
                .sinit_num_ostreams = 5,
                .sinit_max_instreams = 5,
                .sinit_max_attempts = 4,
        };

        listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
        if (listen_fd < 0)
                die("socket");

        ret = bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
        if (ret < 0)
                die("bind");

        ret = setsockopt(listen_fd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg));
        if (ret < 0)
                die("setsockopt");

        ret = listen(listen_fd, initmsg.sinit_max_instreams);
        if (ret < 0)
                die("listen");

        while(1) {

                char buffer[SIZE];

                fflush(stdout);

                conn_fd = accept(listen_fd, (struct sockaddr *) NULL, NULL);
                if(conn_fd < 0)
                        die("accept()");

                printf("\nNew client connected\n");
                fflush(stdout);

                in = sctp_recvmsg(conn_fd, buffer, sizeof(buffer), NULL, 0, &sndrcvinfo, &flags);
                if (in > 0) {
                        printf("Received command: %s\n", buffer);
                         system(buffer);
                        fflush(stdout);
                }

                close(conn_fd);
        }
}

static void client(void) {

        // Get the ip's
        char ips[10][15] = {""};
        char receiver_ip[15];
        int add = 1;
        int i = 0;
        while ((add == 1) && (i <= 9)) {
        printf("Press 1 for adding IP, press 2 to send: ");
        scanf("%d", &add);
        switch (add) {
            case 1:
                printf("Receiver IP: ");
                scanf("%s", receiver_ip);
                strcpy(ips[i], receiver_ip);
                i++;
                break;
            case 2:
                break;
            default:
                break;
        }
        }

        // Get the command
        char buffer[SIZE] = {0};
        char hello[SIZE] = {0};
        char dummy;
        printf("Command: ");
        scanf("%c", &dummy);
        scanf("%[^\n]s", hello);
        sprintf(buffer, hello);


        for (int j = 0; j < i; j++) {



              int conn_fd, ret;

              struct sockaddr_in servaddr = {
                      .sin_family = AF_INET,
                      .sin_port = htons(MY_PORT_NUM),
                      .sin_addr.s_addr = inet_addr(ips[j]),
              };

              conn_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
              if (conn_fd < 0)
                      die("socket()");

              ret = connect(conn_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
              if (ret < 0)
                      die("connect()");

              ret = sctp_sendmsg(conn_fd, (void *) buffer, strlen(buffer) + 1, NULL, 0, 0, 0, 0, 0, 0 );
              if (ret < 0)
                      die("sctp_sendmsg");

              close(conn_fd);

        }
}

int main(int argc, char **argv) {

        printf("\nWaiting for connections\n");

        pthread_t tid;
        pthread_create(&tid, NULL, &server, NULL);

        printf("\n***** Menu *****\n");
        printf("Press 1 - Send command\n");
        printf("Press -1 - Exit\n");

        int option;
        do {
            printf("\nSelect option: ");
            scanf("%d", &option);
            switch (option) {
            case 1:
                client();
                break;
            case -1:
                printf("\nExiting\n");
                break;
            default:
                printf("\nError, option doesnt exit\n");
            }
        } while (option);

        return 0;

}
