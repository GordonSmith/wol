#ifdef WIN32
#define _SCL_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x0501
#endif

#include <iostream>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <boost/algorithm/string.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
using boost::asio::ip::udp;

void echoHelp(const po::options_description & options) {
            std::cout << "Usage:  wol -b BROADCAST_TARGET [-p BROADCAST_PORT] MAC_ADDRESS" << std::endl;
            std::cout << "Broadcasts a \"MagicPacket\" to specified MAC address (via broadcast IP/PORT) to wake machine from sleep." << std::endl << std::endl;
            std::cout << options << std::endl;
}

bool parseOptions(int ac, char* av[], po::variables_map & vm) {
    po::options_description general("Options");
    general.add_options()
        ("help,?", "Display this message")
        ("broadcast,b", po::value<std::string>(), "Broadcast Target")
        ("port,p", po::value<std::string>()->default_value("3389"), "Broadcast Port")
        ;

    po::options_description hidden("Hidden");
    hidden.add_options()
        ("mac,m", po::value<std::string>(), "MAC Address")
        ;

    po::options_description all;
    all.add(general).add(hidden);

    po::positional_options_description positional_options;
    positional_options.add("mac", 1);

    po::store(po::command_line_parser(ac, av).options(all).positional(positional_options).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        echoHelp(general);
        return false;
    }

    if (!vm.count("broadcast")) {
        std::cout << "Error:  required \"BROADCAST_TARGET\" missing." << std::endl << std::endl;
        echoHelp(general);
        return false;
    }

    if (!vm.count("mac")) {
        std::cout << "Error:  required \"MAC_ADDRESS\" missing." << std::endl << std::endl;
        echoHelp(general);
        return false;
    }

    return true;
}

#define MAC_ADDRESS_LENGTH 6
typedef boost::array<char, 17 * MAC_ADDRESS_LENGTH> MagicPacket;

const MagicPacket & formatMagicPacket(const std::string & mac_address, MagicPacket & packet) {
    packet.fill(0);
    typedef std::vector<std::string> MacAddressParts;
    MacAddressParts parts;
    boost::split(parts, mac_address, boost::is_any_of(":"));
    if (parts.size() == MAC_ADDRESS_LENGTH) {
        for(int i = 0; i < MAC_ADDRESS_LENGTH; ++i) {
            packet[i] = static_cast<unsigned char>(0xFF);
        }

        for(int i = 1; i <= 16; ++i) {
            for(int j = 0; j < MAC_ADDRESS_LENGTH; ++j) {
                packet[i * MAC_ADDRESS_LENGTH + j] = static_cast<unsigned char>(strtol(parts[j].c_str(), NULL, 16));
            }
        }
    }
    return packet;
}

bool broadcastMagicPacket(const std::string & host, const std::string & service, const std::string & mac) {
    boost::asio::io_service io_service;

    boost::system::error_code error;
    boost::asio::ip::udp::socket socket(io_service);

    socket.open(udp::v4(), error);
    if (!error) {
        socket.set_option(boost::asio::socket_base::broadcast(true));

        udp::resolver resolver(io_service);
        udp::resolver::query query(udp::v4(), host, service);
        udp::endpoint senderEndpoint = *resolver.resolve(query, error);

        if (!error) {
            MagicPacket magicPacket;
            socket.send_to(boost::asio::buffer(formatMagicPacket(mac, magicPacket)), senderEndpoint);
        }

        socket.close();
    }

    if (error)
        return false;

    return true;
}

int main(int ac, char* av[]) {
    try {
        po::variables_map vm;
        if (!parseOptions(ac, av, vm))
            return 1;

        if (!broadcastMagicPacket(vm["broadcast"].as<std::string>(), vm["port"].as<std::string>(), vm["mac"].as<std::string>()))
            return 2;
    }
    catch(const std::exception& e) {
        std::cerr << "Error:  " << e.what() << "\n";
        return 3;
    }
    catch(...) {
        std::cerr << "Exception of unknown type!\n";
        return 4;
    }

    return 0;
}
