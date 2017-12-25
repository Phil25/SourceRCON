# C++ Source RCON implementation

Uses Valve's [Source RCON protocol](https://developer.valvesoftware.com/wiki/Source_RCON_Protocol) for communicating with remote Source engine servers.

# Features

* Implemented as a C++ class
* Exposes a very simple interface
* Non-threaded
* Connection timeout (default 4 seconds)

**Notes about this implementation**:

* It works on UNIX systems **only**.
* It does **NOT** support multi-packet responses yet.

# Example usage

Prints `test`.

```c++
srcon client = srcon("127.0.0.1", "27015", "password");
std::string response = client.send("echo test");
std::cout << response << "std::endl;
```
