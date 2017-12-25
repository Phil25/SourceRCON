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
#define LOG(str) {}
#endif

srcon::srcon(std::string address, int port, std::string pass)
: srcon(address, port, pass, SRCON_DEFAULT_TIMEOUT){}

srcon::srcon(std::string address, int port, std::string pass, int timeout)
: address(address), pass(pass), port(port), id(0), connected(false){
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockfd < 0){
		LOG("Error opening socket (" << sockfd << ").");
		return;
	}

	LOG("Socket (" << sockfd << ") opened.");

	LOG("Connecting...");
	if(!connect(timeout)){
		LOG("Connection not established.");
		return;
	}

	LOG("Connection established!");
	connected = true;

	send(pass, SERVERDATA_AUTH);
	unsigned char buffer[14];
	::recv(sockfd, buffer, 14, SERVERDATA_RESPONSE_VALUE);
	::recv(sockfd, buffer, 14, SERVERDATA_RESPONSE_VALUE);
}

bool srcon::connect(int timeout){
	struct sockaddr_in server;
	bzero((char*)&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(address.c_str());
	server.sin_port = htons(port);

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

std::string srcon::send(std::string data, int type){
	LOG("Sending: " << data);
	if(!connected)
		return "Connection has not been established.";

	int packet_len = data.length() +14;
	unsigned char packet[packet_len];
	pack(packet, data, packet_len, type);
	if(::send(sockfd, packet, packet_len, 0) < 0)
		return "Sending failed!";

	return type == SERVERDATA_EXECCOMMAND ? recv() : "";
}

bool srcon::is_connected(){
	return connected;
}

std::string srcon::recv(size_t size){
	unsigned char* buffer = new unsigned char[size];
	get_packet(buffer, size);
	size_t len = get_message_len(buffer);
	std::string resp(&buffer[12], &buffer[12] +len);
	delete [] buffer;
	return resp;
}

void srcon::pack(unsigned char packet[], std::string data, int packet_len, int type){
	int data_len = packet_len -14;
	bzero(packet, packet_len);
	packet[0] = data_len +10;
	packet[4] = srcon::id++;
	packet[8] = type;
	for(int i = 0; i < data_len; i++)
		packet[12 +i] = data.c_str()[i];
}

int srcon::get_packet(unsigned char* buffer, size_t size){
	//Avoid recv buffer size limit, keep reading until 0 terminator
	int bytes = 0;
	do
		bytes += ::recv(sockfd, buffer +bytes, size, 0);
	while(buffer[bytes -1] != 0);
	return bytes;
}

size_t srcon::get_message_len(unsigned char* buffer){
	return	static_cast<size_t>(
				static_cast<unsigned char>(buffer[0])		|
				static_cast<unsigned char>(buffer[1]) << 8	|
				static_cast<unsigned char>(buffer[2]) << 16	|
				static_cast<unsigned char>(buffer[3]) << 24
			) -11;
}
