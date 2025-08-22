#include "../inc/Parser.hpp"

// --- Copy Constructors and Assignment Operators (THE FIX) ---
LocationConfig::LocationConfig(const LocationConfig& other) {
    *this = other;
}

LocationConfig& LocationConfig::operator=(const LocationConfig& other) {
    if (this != &other) {
        path = other.path;
        root = other.root;
        index = other.index;
        cgi = other.cgi;
        autoindex = other.autoindex;
        allowed_methods = other.allowed_methods;
        return_code = other.return_code;
        return_url = other.return_url;
        upload_path = other.upload_path;
        cgi_params = other.cgi_params;
    }
    return *this;
}

ServerConfig::ServerConfig(const ServerConfig& other) {
    *this = other;
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other) {
    if (this != &other) {
        // host = other.host;
        // port = other.port;
        listen = other.listen;
        host_str = other.host_str;
        client_max_body_size = other.client_max_body_size;
        error_pages = other.error_pages;
        locations = other.locations;
    }
    return *this;
}
// ---------------------------------------------------------

LocationConfig::LocationConfig() : autoindex(false), return_code(0) {}
ServerConfig::ServerConfig() : client_max_body_size(1048576) {
    // listen.push_back(std::make_pair(0, 80));
    // Default listen on port 80
}
Parser::Parser(const std::string& filename) : _filename(filename) {}
Parser::~Parser() {}

void Parser::parse() {
    std::string content = this->readFile();
    std::vector<std::string> tokens = this->tokenize(content);

    size_t i = 0;
    size_t ser_size = 0;
    bool server_found = false;
    for(size_t y = 0; y < tokens.size(); ++y) {
        if (tokens[y] == "server") {
            ser_size++;
            if (ser_size > 1) {
                throw std::runtime_error("Error: Multiple server blocks found in the configuration file.");
            }
            if (i != 0) {
                throw std::runtime_error("Error: Server block must be the first directive in the configuration file.");
            }
        }
    }
    while (i < tokens.size()) {
        if (tokens[i] == "server") {
            server_found = true;
            i++;
            if (i >= tokens.size() || tokens[i] != "{") {
                throw std::runtime_error("Syntax error: expected '{' after 'server'");
            }
            i++;

            while (i < tokens.size() && tokens[i] != "}") {
                this->parseServerDirective(tokens, i, _server);
            }

            if (i >= tokens.size() || tokens[i] != "}") {
                 throw std::runtime_error("Syntax error: server block not closed with '}'");
            }
            i++;
        } else {
            throw std::runtime_error("Syntax error: unknown directive outside server block: " + tokens[i]);
        }
    }
    if (!server_found) {
        throw std::runtime_error("Error: No server is configured in the file.");
    }
    validateListenDirectives(_server.listen);

}

bool check_duplicate_location(const std::vector<LocationConfig>& locations, const std::string& path) {
    for (size_t i = 0; i < locations.size(); ++i) {
        if (locations[i].path == path) {
            return true;
        }
    }
    return false;
}

// const ServerConfig& Parser::getServers() const {
//     return this->_server;
// }

std::string Parser::readFile() {
    std::ifstream file(this->_filename.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Error: Unable to open configuration file: " + this->_filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<std::string> Parser::tokenize(const std::string& content) {
    std::vector<std::string> tokens;
    std::string current_token;
    for (size_t i = 0; i < content.length(); ++i) {
        char c = content[i];
        if (c == '#') {
            while (i < content.length() && content[i] != '\n') i++;
            continue;
        }
        if (std::isspace(c)) {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
        } else if (c == '{' || c == '}' || c == ';') {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
            tokens.push_back(std::string(1, c));
        } else {
            current_token += c;
        }
    }
    if (!current_token.empty()) {
        tokens.push_back(current_token);
    }
    return tokens;
}

void Parser::parseServerDirective(const std::vector<std::string>& tokens, size_t& i, ServerConfig& config) {
    std::string key = tokens[i++];

    if (key == "listen") {
        std::string val = tokens[i++];
        size_t colon = val.find(':');
        unsigned long host_addr = 0;
        unsigned short port_num = 80;
        std::string host_str_val;
        if (colon != std::string::npos && colon == val.length() - 1) {
            throw std::runtime_error("Error: Invalid listen directive, port number missing.");
        }
        else if (colon == 0) {
            throw std::runtime_error("Error: Invalid listen directive, host missing.");
        }
        if (colon != std::string::npos) {
            host_str_val = val.substr(0, colon);
            if (host_str_val == "localhost")
                host_addr = custom_inet_addr("127.0.0.1");
            else
                host_addr = custom_inet_addr(host_str_val);
            std::string portStr = val.substr(colon + 1);
            // validatePort(portStr);
            port_num = static_cast<unsigned short>(std::atoi(portStr.c_str()));
        } else {
            host_str_val = val;
            if (host_str_val == "localhost")
                host_addr = custom_inet_addr("127.0.0.1");
            else
                host_addr = custom_inet_addr(host_str_val);
            port_num = 80;
        }
        config.listen.push_back(std::make_pair(host_addr, port_num));
        config.host_str.push_back(host_str_val);
    }
    else if (key == "client_max_body_size") {
        std::string value = tokens[i++];
        char unit = ' ';
        if (!value.empty() && std::isalpha(value[value.size() - 1])) {
            unit = std::toupper(value[value.size() - 1]);
            value.erase(value.size() - 1);
        }
        if (!isNumber(value) || (unit != ' ' && unit != 'K' && unit != 'M')) throw std::runtime_error("Error: invalid client_max_body_size.");
        long size = std::atol(value.c_str());
        if (unit == 'K') size *= 1024;
        else if (unit == 'M') size *= 1024 * 1024;
        if (size > 1073741824) {
            throw std::runtime_error("Error: client_max_body_size exceeds 1GB limit.");
        }
        config.client_max_body_size = size;
    } else if (key == "error_page") {
        std::string codeStr = tokens[i++];
        if (!isNumber(codeStr)) throw std::runtime_error("Error: invalid error page code.");
        int code = std::atoi(codeStr.c_str());
        validateErrorPageCode(code);
        config.error_pages[code] = tokens[i++];
    } else if (key == "location") {
        if (check_duplicate_location(config.locations, tokens[i])) {
            throw std::runtime_error("Error: Duplicate location path: " + tokens[i]);
        }
        LocationConfig locConf;
        locConf.path = tokens[i++];
        if (i >= tokens.size() || tokens[i] != "{") throw std::runtime_error("Syntax error: expected '{' after location path.");
        i++;
        while(i < tokens.size() && tokens[i] != "}") parseLocationDirective(tokens, i, locConf);
        if (i >= tokens.size() || tokens[i] != "}") throw std::runtime_error("Syntax error: location block not closed with '}'");
        i++;
        config.locations.push_back(locConf);
        return;
    } else {
        throw std::runtime_error("Error: Unknown server directive: " + key);
    }

    if (i >= tokens.size() || tokens[i] != ";") {
        throw std::runtime_error("Syntax error: expected ';' after directive '" + key + "'");
    }
    i++;
}

void Parser::parseLocationDirective(const std::vector<std::string>& tokens, size_t& i, LocationConfig& locConf) {
    std::string key = tokens[i++];

    if (key == "root") {
        locConf.root = tokens[i++];
        validateRoot(locConf.root);
    } else if (key == "index") {
        locConf.index = tokens[i++];
    } else if (key == "autoindex") {
        std::string val = tokens[i++];
        validateAutoindex(val);
        locConf.autoindex = (val == "on");
    } else if (key == "cgi_pass") {
        if (i + 1 >= tokens.size() || tokens[i+1] == ";") {
            throw std::runtime_error("Syntax error: cgi_pass requires an extension and a path.");
        }
        std::string ext = tokens[i++];
        std::string path = tokens[i++];
        locConf.cgi[ext] = path;
    } else if (key == "allowed_methods") {
        std::vector<std::string> methods;
        while (i < tokens.size() && tokens[i] != ";") {
            std::string method = tokens[i++];
            if (method == "GET") methods.push_back("GET");
            else if (method == "POST") methods.push_back("POST");
            else if (method == "DELETE") methods.push_back("DELETE");
            else throw std::runtime_error("Error: Unsupported method: " + method);
        }
        validateMethods(methods);
        locConf.allowed_methods = methods;
    } else if (key == "return") {
        if (locConf.return_code != 0) {
            throw std::runtime_error("Error: 'return' directive must be unique within a location block.");
        }
        std::string codeStr = tokens[i++];
        if (!isNumber(codeStr)) throw std::runtime_error("Error: invalid redirect code.");
        int code = std::atoi(codeStr.c_str());
        validateReturnCode(code);
        locConf.return_code = code;
        locConf.return_url = tokens[i++];
    } else if (key == "upload_path") {
        locConf.upload_path = tokens[i++];
        validateRoot(locConf.upload_path);
    } else if (key == "cgi_params") {
        if (i + 1 >= tokens.size() || tokens[i+1] == ";") {
            throw std::runtime_error("Syntax error: cgi_param requires a key and a value.");
        }
        std::string param_key = tokens[i++];
        std::string param_value = tokens[i++];
        locConf.cgi_params[param_key] = param_value;
    }
    else {
        throw std::runtime_error("Error: Unknown location directive: " + key);
    }
    if (i >= tokens.size() || tokens[i] != ";") {
        throw std::runtime_error("Syntax error: expected ';' after directive '" + key + "'");
    }
    i++;
}

bool Parser::isNumber(const std::string& s) {
    for (size_t i = 0; i < s.length(); i++) {
        if (!std::isdigit(s[i])) return false;
    }
    return !s.empty();
}

void Parser::validatePort(const std::string& portStr) {
    if (!isNumber(portStr)) {
        throw std::runtime_error("Error: Port '" + portStr + "' is not a valid number.");
    }
    long port = std::atol(portStr.c_str());
    if (port < 1 || port > 65535) {
        throw std::runtime_error("Error: Port is outside valid range (1-65535).");
    }
}

void Parser::validateHost(const std::string& host) {
    (void)host;
}

void Parser::validateRoot(std::string& path) {
    (void)path;
}

void Parser::validateMethods(const std::vector<std::string>& methods) {
    if (methods.empty()) {
        throw std::runtime_error("Error: The 'allowed_methods' directive cannot be empty.");
    }
    for (size_t i = 0; i < methods.size(); ++i) {
        if (methods[i] != "GET" && methods[i] != "POST" && methods[i] != "DELETE") {
            throw std::runtime_error("Error: Unsupported HTTP method.");
        }
    }
}

void Parser::validateReturnCode(int code) {
    if (code != 301 && code != 302) {
        std::stringstream ss;
        ss << "Error: Invalid redirect code: " << code << ". Must be 301 or 302.";
        throw std::runtime_error(ss.str());
    }
}

void Parser::validateErrorPageCode(int code) {
    if (!((code >= 400 && code <= 404) || (code >= 500 && code <= 504))) {
        std::stringstream ss;
        ss << "Error: Invalid error code for error_page: " << code << ". Must be between 400 and 599.";
        throw std::runtime_error(ss.str());
    }
}

void Parser::validateAutoindex(const std::string& val) {
    if (val != "on" && val != "off") {
        throw std::runtime_error("Error: Value for autoindex must be 'on' or 'off'.");
    }
}

void Parser::displayConfigs() {
    const ServerConfig& s = this->_server;
    std::cout << "\n\033[1;34m## Server ##\033[0m\n";
    std::cout << "  \033[1mListen:\033[0m ";
    for (size_t i = 0; i < s.listen.size(); ++i)
    {
        std::cout << s.listen[i].first << ":" << s.listen[i].second <<

            (i < s.listen.size() - 1 ? ", " : "");
    }
    std::cout << "\n  \033[1mClient Max Body Size:\033[0m " << s.client_max_body_size << " bytes\n";
    std::cout << "  \033[1mError Pages:\033[0m ";
    for (std::map<int, std::string>::const_iterator it = s.error_pages.begin(); it != s.error_pages.end(); ++it) {
        std::cout << it->first << " -> " << it->second << "     ";
    }
    std::cout << "\n";
    std::cout << "  \033[1mLocations (" << s.locations.size() << "):\033[0m\n";
    for (size_t k = 0; k < s.locations.size(); ++k) {
        const LocationConfig& l = s.locations[k];
        std::cout << "    \033[1;32m-> Location Path:\033[0m " << l.path << "\n";
        std::cout << "      - Root: " << l.root << "\n";
        std::cout << "      - Index: " << l.index << "\n";
        std::cout << "      - Autoindex: " << (l.autoindex ? "on" : "off") << "\n";
        if (l.return_code != 0) {
            std::cout << "      - Redirect: " << l.return_code << " -> " << l.return_url << "\n";
        }
        std::cout << "      - Allowed Methods: ";
        for (size_t m = 0; m < l.allowed_methods.size(); ++m) {
            std::cout << l.allowed_methods[m] << " ";
        }
        std::cout << "\n";
        for (std::map<std::string, std::string>::const_iterator it = l.cgi.begin(); it != l.cgi.end(); ++it) {
            std::cout << "      - CGI: " << it->first << " -> " << it->second << "\n";
        }
        if (!l.upload_path.empty()) {
            std::cout << "      - Upload Path: " << l.upload_path << "\n";
        }
        if (!l.cgi_params.empty()) {
            std::cout << "\n      - CGI Parameters: ";
            for (std::map<std::string, std::string>::const_iterator it = l.cgi_params.begin(); it != l.cgi_params.end(); ++it) {
                std::cout << it->first << "=" << it->second << " ";
            }
        }
        std::cout << "\n";
    }
}

unsigned long Parser::custom_inet_addr(const std::string& ip_str) {
    std::stringstream ss(ip_str);
    std::string segment;
    std::vector<unsigned long> parts;

    while (std::getline(ss, segment, '.')) {
        if (!isNumber(segment)) {
            throw std::runtime_error("Invalid IP address: contains non-numeric characters.");
        }
        long part = std::atol(segment.c_str());
        if (part < 0 || part > 255) {
            throw std::runtime_error("Invalid IP address: an octet is out of range 0-255.");
        }
        parts.push_back(static_cast<unsigned long>(part));
    }

    if (parts.size() != 4) {
        throw std::runtime_error("Invalid IP address: must contain exactly 4 octets.");
    }

    unsigned long result = (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8) | (parts[3]);
    return result;
}
std::string Parser::ipToString(unsigned long ip) {
    std::stringstream ss;
    ss << ((ip >> 24) & 0xFF) << "."
       << ((ip >> 16) & 0xFF) << "."
       << ((ip >> 8) & 0xFF) << "."
       << (ip & 0xFF);
    return ss.str();
}


void Parser::validateListenDirectives(const std::vector<std::pair<unsigned long, unsigned short> >& listen_sockets) {
    std::vector<unsigned short> wildcard_ports;

    for (size_t i = 0; i < listen_sockets.size(); ++i) {
        if (listen_sockets[i].first == 0) {
            wildcard_ports.push_back(listen_sockets[i].second);
        }
    }

    if (wildcard_ports.empty()) {
        return; 
    }

    for (size_t i = 0; i < listen_sockets.size(); ++i) {
        if (listen_sockets[i].first != 0) {
            for (size_t j = 0; j < wildcard_ports.size(); ++j) {
                if (listen_sockets[i].second == wildcard_ports[j]) {
                    std::stringstream error_msg;
                    error_msg << "Configuration error: a specific IP ("
                              << ipToString(listen_sockets[i].first)
                              << ") cannot be bound to port " << listen_sockets[i].second
                              << " because that port is already bound to a wildcard address (0.0.0.0).";
                    throw std::runtime_error(error_msg.str());
                }
            }
        }
    }
}