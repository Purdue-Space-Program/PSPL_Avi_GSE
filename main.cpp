#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <vector>

#include <sys/socket.h>

#include <nlohmann/json.hpp>
#include <client/cpp/synnax.h>

#include "channels/generator.hpp"
#include "channels/synnax.hpp"
#include "channels/commands.hpp"
#include "channels/telem.hpp"
#include "net/cms.hpp"

std::atomic<bool> thread_shutdown(false);

int main()
{
    initialize_config();
    while (true) {
        bool synnax_connected = connect_to_synnax();

        auto cmd_ret = get_cmdnet_sock();

        // auto telem_ret = get_telem_sock();

        // if (synnax_connected && synnax_client && telem_ret && cmd_ret) {
        if (synnax_connected && synnax_client && cmd_ret) {
            std::cout << "Connected to all targets!" << '\n';

            std::filesystem::path telem_config = "pspl_ops_config/cms/telem.json";
            add_telem_channels(*synnax_client, std::filesystem::absolute(telem_config));

            std::filesystem::path command_config = "pspl_ops_config/cms/commands.json";
            add_command_channels(*synnax_client, std::filesystem::absolute(command_config));

            // spin up tasks
            std::vector<std::thread> threads;

            threads.emplace_back(
                forward_commands, 
                std::ref(*synnax_client), 
                cmd_ret.value(), 
                command_config,
                std::ref(thread_shutdown)
            );
            // threads.emplace_back(
            //     telem_proxy, 
            //     std::ref(*synnax_client), 
            //     telem_ret.value(), 
            //     telem_config,
            //     std::ref(thread_shutdown)
            // );

            for (auto &t : threads) t.join();

            std::cout << "Shutdown. Attempting restart..." << '\n';

            std::this_thread::sleep_for(std::chrono::seconds(15));
        } else {
            // std::cout << "Synnax: " << synnax_connected << '\n';
            // std::cout << "CMDNet: " << cmd_ret.has_value() << '\n';
            // std::cout << "Telem: " << telem_ret.has_value() << '\n';
        }

        if (cmd_ret) {
            close(cmd_ret.value());
        }
        // if (telem_ret) {
        //     close(telem_ret.value());
        // }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}
