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
    if (argc < 2  || argc > 4) {
      	usage();
        return 1;
    }

    if (argc == 3)
    {
    	if (!strcmp(argv[2], "-e"))
	    {
    		usage();
    		return 1;
	    }
	    echo = true;

    }
	else if (argc == 4)
	{
		if (!strcmp(argv[3], "-b"))
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

	int optval = 1;
	int res_sock = ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (res_sock == -1) {
		perror("setsockopt");
		return 1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	ssize_t res_bind = ::bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	if (res_bind == -1) {
		perror("bind");
		return 1;
	}

	int res_lis = listen(sock, 5);
	if (res_lis == -1) {
		perror("listen");
		return 1;
	}

	while (true) {
		struct sockaddr_in addr;
		socklen_t len = sizeof(addr);
		int newsd = ::accept(sock, (struct sockaddr *)&addr, &len);
		if (newsd == -1) {
			perror("accept");
			break;
		}
		thread t(recvThread, newsd);
		t.detach();
	}
	::close(sock);


}