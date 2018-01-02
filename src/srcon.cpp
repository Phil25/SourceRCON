#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include "../include/srcon.h"

//#define DEBUG 1
#ifdef DEBUG
#define LOG(str) std::clog << str << std::endl;
#else
#define LOG(str)
#endif

srcon::srcon(const std::string address, const int port, const std::string pass, const int timeout)
: srcon(srcon_addr{address, port, pass}, timeout){}

srcon::srcon(const srcon_addr addr, const int timeout):
	addr(addr),
	sockfd(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)),
	id(0),
	connected(false)
{
	if(sockfd == -1){
		LOG("Error opening socket.");
		return;
	}

	LOG("Socket (" << sockfd << ") opened, connecting...");
	if(!connect(timeout)){
		LOG("Connection not established.");
		close(sockfd);
		return;
	}

	LOG("Connection established!");
	connected = true;

	send(addr.pass, SERVERDATA_AUTH);
	unsigned char buffer[SRCON_HEADER_SIZE];
	::recv(sockfd, buffer, SRCON_HEADER_SIZE, SERVERDATA_RESPONSE_VALUE);
	::recv(sockfd, buffer, SRCON_HEADER_SIZE, SERVERDATA_RESPONSE_VALUE);
}

srcon::~srcon(){
	if(get_connected())
		close(sockfd);
}

bool srcon::connect(const int timeout) const{
	struct sockaddr_in server;
	bzero((char*)&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(addr.addr.c_str());
	server.sin_port = htons(addr.port);

	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);

	int status = -1;
	if((status = ::connect(sockfd, (struct sockaddr *)&server, sizeof(server))) == -1)
		if(errno != EINPROGRESS)
			return false;

	status = select(sockfd +1, NULL, &fds, NULL, &tv);
	fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) & ~O_NONBLOCK);
	return status != 0;
}

std::string srcon::send(const std::string data, const int type){
	LOG("Sending: " << data);
	if(!get_connected())
		return "Connection has not been established.";

	int packet_len = data.length() +SRCON_HEADER_SIZE;
	unsigned char packet[packet_len];
	pack(packet, data, packet_len, srcon::id++, type);
	if(::send(sockfd, packet, packet_len, 0) < 0)
		return "Sending failed!";

	return type == SERVERDATA_EXECCOMMAND ? recv() : "";
}

std::string srcon::recv(const size_t size) const{
	unsigned char* buffer = new unsigned char[size];
	if(get_packet(buffer, size) <= SRCON_HEADER_SIZE)
		return "";
	size_t len = get_message_len(buffer);
	std::string resp(&buffer[12], &buffer[12] +len);
	delete [] buffer;
	return resp;
}

void srcon::pack(unsigned char packet[], const std::string data, int packet_len, int id, int type) const{
	int data_len = packet_len -SRCON_HEADER_SIZE;
	bzero(packet, packet_len);
	packet[0] = data_len +10;
	packet[4] = id;
	packet[8] = type;
	for(int i = 0; i < data_len; i++)
		packet[12 +i] = data.c_str()[i];
}

int srcon::get_packet(unsigned char* buffer, size_t size) const{
	//Avoid recv buffer size limit, keep reading until 0 terminator
	int bytes = 0;
	do
		bytes += ::recv(sockfd, buffer +bytes, size, 0);
	while(buffer[bytes -1] != 0);
	LOG("Received " << bytes << " bytes");
	return bytes;
}

size_t srcon::get_message_len(unsigned char* buffer) const{
	return	static_cast<size_t>(
				static_cast<unsigned char>(buffer[0])		|
				static_cast<unsigned char>(buffer[1]) << 8	|
				static_cast<unsigned char>(buffer[2]) << 16	|
				static_cast<unsigned char>(buffer[3]) << 24
			) -11;
}
