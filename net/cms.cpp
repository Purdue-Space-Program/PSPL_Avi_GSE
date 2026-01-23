#include "cms.hpp"

extern "C" {
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>
}

#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

int CMDNET_PORT = -1;
int TELEM_PORT = -1;
std::string CMDNET_PORT_STR;
std::string TELEM_PORT_STR;
std::string FC_HOSTNAME;

int initialize_net_config() {
    std::filesystem::path net_config = "pspl_ops_config/cms/network.json";

    std::cout << "Initializing network config using: " << net_config << '\n';

    std::ifstream config_file(net_config);
    if (!config_file.is_open()) {
        return -1;
    }

    nlohmann::json config_json;
    config_file >> config_json;

    CMDNET_PORT = config_json["fc"].at("command_port").get<int>();
    TELEM_PORT  = config_json["fc"].at("telem_port").get<int>();
    CMDNET_PORT_STR = std::to_string(CMDNET_PORT);
    TELEM_PORT_STR  = std::to_string(TELEM_PORT);
    FC_HOSTNAME = config_json["fc"].at("ip").get<std::string>();

    return 0;
}

std::optional<int> get_cmdnet_sock() {
    addrinfo hints{};
    addrinfo *res = nullptr;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(FC_HOSTNAME.data(), CMDNET_PORT_STR.data(), &hints, &res);
    if (ret != 0) return {};

    int internal_sock = -1;

    for (struct addrinfo *rp = res; rp != nullptr; rp = rp->ai_next) {
        internal_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (internal_sock == -1) continue;

        if (connect(internal_sock, rp->ai_addr, rp->ai_addrlen) != -1) {
            freeaddrinfo(res);

            struct timeval tv;
            tv.tv_sec = 5;  // 5 seconds
            tv.tv_usec = 0;
            setsockopt(internal_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

            int one = 1;
            setsockopt(internal_sock, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one));

            return internal_sock;
        }

        close(internal_sock);
        internal_sock = -1;
    }
    freeaddrinfo(res);

    return {};
}

std::optional<int> get_telem_sock() {
    addrinfo hints{};
    addrinfo *res = nullptr;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(FC_HOSTNAME.data(), TELEM_PORT_STR.data(), &hints, &res);
    if (ret != 0) return {};

    int internal_sock = -1;

    for (struct addrinfo *rp = res; rp != nullptr; rp = rp->ai_next) {
        internal_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (internal_sock == -1) continue;

        if (connect(internal_sock, rp->ai_addr, rp->ai_addrlen) != -1) {
            freeaddrinfo(res);
            return internal_sock;
        }

        close(internal_sock);
        internal_sock = -1;
    }
    freeaddrinfo(res);

    return {};
}
