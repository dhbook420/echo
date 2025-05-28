#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <iostream>
#include <thread>
#include <cstring>


using namespace std;

void usage() {
    cout << "\nsyntax : echo-server <port> [-e[-b]]\nsample : echo-server 1234 -e -b\n";
}

bool echo = false;
bool broadcast = false;

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
		if (echo) {
			res = ::send(sd, buf, res, 0);
			if (res == 0 || res == -1) {
				fprintf(stderr, "send return %zd", res);
				perror("send");
				break;
			}
		}
	}
	printf("disconnected\n");
	fflush(stdout);
	::close(sd);
}




int main(int argc, char *argv[]) {
    if (argc < 3  || argc > 5) {
      	usage();
        return 1;
    }

    if (argc == 4)
    {
    	if (!memcmp (argv[3], "-e", 2))
	    {
    		usage();
    		return 1;
	    }
	    echo = true;

    }
	else if (argc == 5)
	{
		if (!memcmp (argv[4], "-b", 2))
		{
			usage();
			return 1;
		}
    	broadcast = true;
	}

    int     port = atoi(argv[1]);

    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

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