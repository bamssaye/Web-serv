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
        Server serve(Parser.getServerConfig());
    }catch(std::exception& e){
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
