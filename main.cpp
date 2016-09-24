/*
 ** main.cpp
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include <iostream>

#include "ConnectionInfo.h"

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10  // how many pending connections queue will hold

#define BUFFER_SIZE 1000

// Server -> Client
// Important: every message muy end with a \r\n
#define WELCOME_MESSAGE "Escape character is '^]'.\n\r220 mx0b-00164701.email.com ESMTP cm-m004676\n\r"
#define OUT_HELO  "250 mx0b-01164601.email.com Hello mail-abc-gw.email.edu [10.10.10.1], pleased to meet you\n\r"
#define OUT_FROM "250 2.1.0 Sender ok\n\r"
#define OUT_RCPT "250 2.1.5 Recipient ok\n\r"
#define OUT_DATA "354 Enter mail, end with \".\" on a line by itself\r\n"
#define OUT_QUIT "221 2.0.0 mx0b-00164701.email.com Closing connection\r\n"
#define MESSAGE_ACCEPTED_FOR_DELIVERY "250 2.0.0 25uc3w81mk-1 Message accepted for delivery\n\r"

// Client -> Server 
#define IN_HELO "HELO local.mail.com"
#define IN_FROM "MAIL FROM: validuser@email.com"
#define IN_RCPT "RCPT TO: valid@email.com"
#define IN_DATA "DATA"
#define IN_QUIT "QUIT"

// Protoypes
int dealWithClient(int fd, ConnectionInfo& conn);
int sendAndExpect(int fd, ConnectionInfo& conn, char* exp_input, char* reply);

using namespace std;

void sigchld_handler(int s) {
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while (waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

/**
 * This gets a line from the input buffer.
 * It calls recv() until a \r\n is found. It returns a string with the current line
 * and it also changes the buffer. On next read, must start from index. 
 * 
 * @param fd file descriptor of this buffer
 * @param buffer
 * @param buffer_length
 * @return 
 */
string recvLine(int fd, ConnectionInfo& conn) {

	int n = 0;

	string ret;
	bool done = false;
	cout << "Estoy en recvline" << endl;

	while (!done) {
		int size = recv(fd, conn.getBuffer() + n, BUFFER_SIZE - n, 0);

		if (size == 0){
			cout << "Comm closed by remote host" << endl;
			return "";
		} else if (size == -1) {
			perror("receive");
			return "";
		} else {
			cout << "Received " << size << " bytes, " << "n=" << n << ", size=" << size << endl;

			for (int k = n; k < (n + size); k++) {
				cout << conn.getBuffer()[k];
				if (conn.getBuffer()[k] == '\n') {
					cout << "done" << endl;
					ret = string(conn.getBuffer(), k);
					conn.setIndex(k);
					done = true;
				}
			}
			cout << endl;
		}
		// we increase our index to the right
		n += size;
	}
	cout << "time to go! result: " << ret << endl;
	return ret;
}

int main(void) {
	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP


	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof (int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	cout << "Server waiting for connections on port " << PORT << endl;

	while (1) { // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *) &their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		char buffer[BUFFER_SIZE];
		int size;

		if (!fork()) { // this is the child process

			ConnectionInfo ci;
			dealWithClient(new_fd, ci);

		}
		close(new_fd); // parent doesn't need this
	}

	return 0;
}

/**
 * This simulates a talk to the client. If the response received is not the expected,
 * it returns a number !=0. See errors list for details.
 * 
 * @param new_fd
 * @param conn
 * @return 
 */
int dealWithClient(int fd, ConnectionInfo& conn) {

	string reply;

	// Let's greet our new host
	send(fd, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE), 0);


	// 1) wait for HELO
	if (sendAndExpect(fd, conn, IN_HELO, OUT_HELO) != 0) {
		return -1;
	}

	// 2) wait for FROM
	if (sendAndExpect(fd, conn, IN_FROM, OUT_FROM) != 0) {
		return -1;
	}

	// 3) wait for RCPT
	if (sendAndExpect(fd, conn, IN_RCPT, OUT_RCPT) != 0) {
		return -1;
	}

	// 4) wait for DATA
	if (sendAndExpect(fd, conn, IN_DATA, OUT_DATA) != 0) {
		return -1;
	}
	
	// Wait for a line with a dot...
	

	// 5) wait for QUIT
	if (sendAndExpect(fd, conn, IN_QUIT, OUT_QUIT) != 0) {
		return -1;
	}
	close(fd);
	exit(0);
}

int sendAndExpect(int fd, ConnectionInfo& conn, char* exp_input, char* reply) {

	cout << "Waiting for: " << exp_input << endl;

	// Read a line 
	string rec = recvLine(fd, conn);

	if (rec.compare(exp_input) == 0) {
		send(fd, reply, strlen(reply), 0);
	} else {
		cout << "Not received expected input (" << exp_input << ")" << endl;
		cout << "Instead, received: " << reply;
		close(fd);
		return -1;
	}
	return 0;
}

