#include "./inc/server.hpp"

long getFileSize(const std::string &filename) {
    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return -1; // error
    return file.tellg();
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_config_file>" << std::endl;
        return 1;
    }
    // std::string path = argv[1];
    // long size = getFileSize(path);
    // if (size >= 0)
    //     std::cout << "File size: " << size << " bytes" << std::endl;
    // else
    //     std::cout << "Cannot open file!" << std::endl;
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
