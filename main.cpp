#include "./inc/server.hpp"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_config_file>" << std::endl;
        return 1;
    }
	try{
        Parser Parser(argv[1]);
        Parser.parse();
		Parser.displayConfigs();
        std::vector<ServerConfig> server = Parser.getServers();
        ServerConfig ser = server[0];
        Server serve(ser);
    }catch(std::exception& e){
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
