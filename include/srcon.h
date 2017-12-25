#ifndef SCRON_H
#define SCRON_H


#include <string>


#define SERVERDATA_AUTH 3
#define SERVERDATA_EXECCOMMAND 2
#define SERVERDATA_AUTH_RESPONSE 2
#define SERVERDATA_RESPONSE_VALUE 0

#define SRCON_DEFAULT_TIMEOUT 4


class srcon{
	std::string address, pass;
	int sockfd;
	unsigned int port, id;
	bool connected;

public:
	srcon(std::string address, int port, std::string password);
	srcon(std::string address, int port, std::string password, int timeout);
	std::string send(std::string message, int type=SERVERDATA_EXECCOMMAND);
	bool is_connected();

private:
	bool connect(int timeout=SRCON_DEFAULT_TIMEOUT);
	std::string recv(size_t length=4096);
	void pack(unsigned char packet[], std::string data, int packet_len, int type);
	int get_packet(unsigned char*, size_t);
	size_t get_message_len(unsigned char*);
};


#endif
