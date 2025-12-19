#include "header/common.h"
#include "header/grpcHelper.h"
#include "header/server.h"

int main(int argc, char** argv) {
    const char* port_env = std::getenv("SERVER_PORT");
    std::string port = port_env ? port_env : "50051";
    std::string server_address = "0.0.0.0:" + port;

    std::signal(SIGINT, HandleSignal);   // Ctrl+C
    std::signal(SIGTERM, HandleSignal);  // Docker stop
    
    RunServer(server_address);
    return 0;
}
