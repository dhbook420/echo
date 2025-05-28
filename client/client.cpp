#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <iostream>
#include <thread>
#include <cstring>


using namespace std;

void usage() {
    cout << "\nsyntax : echo-client <ip> <port>\nsample : echo-client 192.169.10.2 1234\n";
}

void recvThread(int sd) {
    printf("connected\n");
    fflush(stdout);
    static const int BUFSIZE = 65536;
    char buf[BUFSIZE];
    while (true) {
        ssize_t res = ::recv(sd, buf, BUFSIZE - 1, 0);
        if (res == 0 || res == -1) {
            fprintf(stderr, "recv return %zd", res);
            perror("recv");
            break;
        }
        buf[res] = '\0';
        printf("%s", buf);
        fflush(stdout);
    }
    printf("disconnected\n");
    fflush(stdout);
    close(sd);
    exit(0);
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
      usage();
      return 1;
    }

    char     *ip = argv[1];
    int     port = atoi(argv[2]);

    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
      perror("connect");
      close(sock);
      return 1;
    }

    thread th(recvThread, sock);
    th.detach();

    while (true) {
        std::string s;
        std::getline(std::cin, s);
        s += "\r\n";
        ssize_t res = ::send(sock, s.data(), s.size(), 0);
        if (res == 0 || res == -1) {
            fprintf(stderr, "send return %zd", res);
            perror("send");
            break;
        }
    }
    ::close(sock);
    return 0;
}