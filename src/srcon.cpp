#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <thread>

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

	if(type != SERVERDATA_EXECCOMMAND)
		return "";

	unsigned long halt_id = id;
	send("", SERVERDATA_RESPONSE_VALUE);
	return recv(halt_id);
}

std::string srcon::recv(unsigned long halt_id) const{
	unsigned int bytes = 0;
	unsigned char* buffer = nullptr;
	std::string response;
	bool can_sleep = true;
	while(1){
		delete [] buffer;
		buffer = read_packet(bytes, can_sleep);
		if(byte32_to_int(buffer) == halt_id)
			break;
		int offset = bytes -SRCON_HEADER_SIZE +3;
		if(offset == -1)
			continue;
		std::string part(&buffer[8], &buffer[8] +offset);
		response += part;
	}
	delete [] buffer;
	buffer = read_packet(bytes, can_sleep);
	delete [] buffer;
	return response;
}

unsigned char* srcon::read_packet(unsigned int& size, bool& can_sleep) const{
	size_t len = read_packet_len();
	if(can_sleep && len > SRCON_SLEEP_THRESHOLD){
		std::this_thread::sleep_for(std::chrono::milliseconds(SRCON_SLEEP_MILLISECONDS));
		can_sleep = false;
	}
	unsigned char* buffer = new unsigned char[len]{0};
	unsigned int bytes = 0;
	do bytes += ::recv(sockfd, buffer, len -bytes, 0);
	while(bytes < len);
	size = len;
	return buffer;
}

size_t srcon::read_packet_len() const{
	unsigned char* buffer = new unsigned char[4]{0};
	::recv(sockfd, buffer, 4, 0);
	const size_t len = byte32_to_int(buffer);
	delete [] buffer;
	return len;
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

size_t srcon::byte32_to_int(unsigned char* buffer) const{
	return	static_cast<size_t>(
				static_cast<unsigned char>(buffer[0])		|
				static_cast<unsigned char>(buffer[1]) << 8	|
				static_cast<unsigned char>(buffer[2]) << 16	|
				static_cast<unsigned char>(buffer[3]) << 24 );
}
