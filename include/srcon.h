#ifndef SCRON_H
#define SCRON_H


#include <string>


#define SERVERDATA_AUTH 3
#define SERVERDATA_EXECCOMMAND 2
#define SERVERDATA_AUTH_RESPONSE 2
#define SERVERDATA_RESPONSE_VALUE 0

#define SRCON_DEFAULT_TIMEOUT 4
#define SRCON_HEADER_SIZE 14


struct srcon_addr{
	std::string addr;
	int port;
	std::string pass;
};

class srcon{
	const srcon_addr addr;
	const int sockfd;
	unsigned int id;
	bool connected;

public:
	srcon(const srcon_addr addr, const int timeout=SRCON_DEFAULT_TIMEOUT);
	srcon(const std::string address, const int port, const std::string password, const int timeout=SRCON_DEFAULT_TIMEOUT);
	virtual ~srcon();

	std::string send(const std::string message, int type=SERVERDATA_EXECCOMMAND);

	inline bool get_connected() const{
		return connected;
	}

	inline bool is_connected() const{
		return get_connected();
	}

	inline srcon_addr get_addr() const{
		return addr;
	}

private:
	bool connect(int timeout=SRCON_DEFAULT_TIMEOUT) const;
	std::string recv(size_t length=4096) const;
	void pack(unsigned char packet[], std::string data, int packet_len, int id, int type) const;
	int get_packet(unsigned char*, size_t) const;
	size_t get_message_len(unsigned char*) const;
};


#endif
