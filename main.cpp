#include "./inc/server.hpp"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_config_file>" << std::endl;
        return 1;
    }
	
    Server server(argv[1]);

    return 0;
}
