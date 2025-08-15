#include "../inc/Parser.hpp"

LocationConfig::LocationConfig() : autoindex(false), return_code(0) {}
ServerConfig::ServerConfig() : host(0), port(80), client_max_body_size(1048576) {}

Parser::Parser(const std::string& filename) : _filename(filename) {}
Parser::~Parser() {}

void Parser::parse() {
    std::string content = this->readFile();
    std::vector<std::string> tokens = this->tokenize(content);

    size_t i = 0;
    while (i < tokens.size()) {
        if (tokens[i] == "server") {
            ServerConfig serverConf;
            i++;
            if (i >= tokens.size() || tokens[i] != "{") {
                throw std::runtime_error("Syntax error: expected '{' after 'server'");
            }
            i++;

            while (i < tokens.size() && tokens[i] != "}") {
                this->parseServerDirective(tokens, i, serverConf);
            }

            if (i >= tokens.size() || tokens[i] != "}") {
                 throw std::runtime_error("Syntax error: server block not closed with '}'");
            }
            i++;
            _servers.push_back(serverConf);
        } else {
            throw std::runtime_error("Syntax error: unknown directive outside server block: " + tokens[i]);
        }
    }
    if (_servers.empty()) {
        throw std::runtime_error("Error: No server is configured in the file.");
    }

}

bool check_duplicate_location(const std::vector<LocationConfig>& locations, const std::string& path) {
    for (size_t i = 0; i < locations.size(); ++i) {
        if (locations[i].path == path) {
            return true;
        }
    }
    return false;
}

const std::vector<ServerConfig>& Parser::getServers() const {
    return this->_servers;
}

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
        if (colon != std::string::npos) {
            config.host = custom_inet_addr(val.substr(0, colon));
            std::string portStr = val.substr(colon + 1);
            validatePort(portStr);
            config.port = std::atoi(portStr.c_str());
        } else {
            validatePort(val);
            config.port = std::atoi(val.c_str());
        }
        // validateHost(config.host);
    } else if (key == "server_name") {
        while (i < tokens.size() && tokens[i] != ";") config.server_names.push_back(tokens[i++]);
    } else if (key == "root") {
        config.root = tokens[i++];
        validateRoot(config.root);
    } else if (key == "client_max_body_size") {
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
        if (locConf.cgi_pass.empty() && !locConf.cgi_extensions.empty()) {
            throw std::runtime_error("Error: cgi_pass must be set if cgi_extensions are defined.");
        }
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
        locConf.cgi_pass = tokens[i++];
    } else if (key == "allowed_methods") {
        std::vector<Methods> methods;
        while (i < tokens.size() && tokens[i] != ";") {
            std::string method = tokens[i++];
            if (method == "GET") methods.push_back(GET);
            else if (method == "POST") methods.push_back(POST);
            else if (method == "DELETE") methods.push_back(DELETE);
            else throw std::runtime_error("Error: Unsupported method: " + method);
        }
        validateMethods(methods);
        locConf.allowed_methods = methods;
    } else if (key == "return") {
        std::string codeStr = tokens[i++];
        if (!isNumber(codeStr)) throw std::runtime_error("Error: invalid redirect code.");
        int code = std::atoi(codeStr.c_str());
        validateReturnCode(code);
        locConf.return_code = code;
        locConf.return_url = tokens[i++];
    } else if (key == "upload_path") {
        locConf.upload_path = tokens[i++];
        validateRoot(locConf.upload_path);
    } else if (key == "cgi_extension") {
        while (i < tokens.size() && tokens[i] != ";") {
            locConf.cgi_extensions.push_back(tokens[i++]);
        }
    } else {
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
    if (host != "localhost" && host != "0.0.0.0") {

    }
}

void Parser::validateRoot(const std::string& path) {
    if (path.empty() || path[0] != '/') {
        throw std::runtime_error("Error: Path '" + path + "' must be an absolute path.");
    }
}

void Parser::validateMethods(const std::vector<Methods>& methods) {
    if (methods.empty()) {
        throw std::runtime_error("Error: The 'allowed_methods' directive cannot be empty.");
    }
    for (size_t i = 0; i < methods.size(); ++i) {
        if (methods[i] != GET && methods[i] != POST && methods[i] != DELETE) {
            throw std::runtime_error("Error: Unsupported HTTP method.");
        }
    }
}

void Parser::validateReturnCode(int code) {
    if (code < 300 || code >= 400) {
        std::stringstream ss;
        ss << "Error: Invalid redirect code: " << code << ". Must be between 300 and 399.";
        throw std::runtime_error(ss.str());
    }
}

void Parser::validateErrorPageCode(int code) {
    if (code < 400 || code >= 600) {
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
    const std::vector<ServerConfig>& servers = this->getServers();
    for (size_t i = 0; i < servers.size(); ++i) {
        const ServerConfig& s = servers[i];
        std::cout << "\n\033[1;34m## Server #" << i + 1 << " ##\033[0m\n";
        std::cout << "  \033[1mHost:\033[0m " << s.host << ":" << s.port << "\n";
        std::cout << "  \033[1mRoot:\033[0m " << s.root << "\n";
        std::cout << "  \033[1mServer Names:\033[0m ";
        for (size_t j = 0; j < s.server_names.size(); ++j) {
            std::cout << s.server_names[j] << " ";
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
            std::cout << "      - CGI Pass: " << l.cgi_pass << "\n";
            if (!l.upload_path.empty()) {
                std::cout << "      - Upload Path: " << l.upload_path << "\n";
            }
            if (!l.cgi_extensions.empty()) {
                std::cout << "      - CGI Extensions: ";
                for (size_t j = 0; j < l.cgi_extensions.size(); ++j) {
                    std::cout << l.cgi_extensions[j] << " ";
                }
            }
            std::cout << "\n";
        }
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
