# C++ Source RCON implementation

Uses Valve's [Source RCON protocol](https://developer.valvesoftware.com/wiki/Source_RCON_Protocol) for communicating with remote Source engine servers.

# Features

* Implemented as a C++ class
* Exposes a very simple interface
* Non-threaded
* Connection timeout (default 4 seconds)

**Notes about this implementation**:

* This is not a standalone application.
* It works on UNIX systems **only** as of now.
* It does **NOT** support multi-packet responses yet.

# Example usage

Prints `test`.

```c++
srcon client = srcon("127.0.0.1", 27015, "password");
std::string response = client.send("echo test");
std::cout << response << std::endl;
```

# Constructors

1. `srcon(srcon_addr addr, int timeout=SRCON_DEFAULT_TIMEOUT)`
	* `addr` — [`srcon_addr` struct](#address-structure)
	* `timeout` — connection timeout in seconds (defaults to 4)
1. `srcon(std::string address, int port, std::string pass, int timeout=SRCON_DEFAULT_TIMEOUT)`
	* `address` — address of the server
	* `port` — port of the server
	* `pass` — rcon password of the server
	* `timeout` — connection timeout in seconds (defaults to 4)

# Address Structure

`srcon_addr` struct consists of:

1. `std::string addr` — address of the server
1. `int port` — port of the server
1. `std::string pass` — rcon password of the server
