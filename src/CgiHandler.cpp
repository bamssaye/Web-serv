#include "../inc/CgiHandler.hpp"
#include "../inc/Request.hpp"

std::string to_string(size_t n) {
    std::stringstream ss;
    ss << n;
    return ss.str();
}

CgiHandler::CgiHandler(Request& request)
{
    _initEnv(request);
}

CgiHandler::~CgiHandler() {}

void CgiHandler::_initEnv(Request& request) {
    const std::map<std::string, std::string>& headers = request.getHeaders();

    if (headers.find("Auth-Scheme") != headers.end() && !headers.at("Auth-Scheme").empty())
        _env["AUTH_TYPE"] = headers.at("Authorization");

    _env["REDIRECT_STATUS"] = "200";
    _env["GATEWAY_INTERFACE"] = "CGI/1.1";
    _env["SCRIPT_NAME"] = request.getPath();
    _env["SCRIPT_FILENAME"] = request.getPath();
    _env["REQUEST_METHOD"] = request.getMethod();
    _env["CONTENT_LENGTH"] = to_string(request.getBody().length());
    _env["CONTENT_TYPE"] = headers.count("Content-Type") ? headers.at("Content-Type") : "";
    _env["PATH_INFO"] = request.getPath();
    _env["PATH_TRANSLATED"] = request.getPath();
    _env["QUERY_STRING"] = request.getQuery();
    _env["REMOTE_IDENT"] = headers.count("Authorization") ? headers.at("Authorization") : "";
    _env["REMOTE_USER"] = headers.count("Authorization") ? headers.at("Authorization") : "";
    _env["REQUEST_URI"] = request.getPath() + "?" + request.getQuery();
    _env["SERVER_PROTOCOL"] = "HTTP/1.0";
    _env["SERVER_SOFTWARE"] = "Weebserv/1.0";

    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        std::string key = "HTTP_" + it->first;
        for (size_t i = 0; i < key.size(); ++i) {
            key[i] = std::toupper(key[i]);
            if (key[i] == '-')
                key[i] = '_';
        }
        _env[key] = it->second;
    }

    std::cout << _env["REMOTE_ADDR"] << "                                     " << _env["REMOTE_USER"] << std::endl;

    _env.insert(request.loc_config.cgi_params.begin(), request.loc_config.cgi_params.end());
}

char **CgiHandler::_getEnvAsCstrArray() const {
    char **env = new char*[_env.size() + 1];
    int i = 0;
    for (std::map<std::string, std::string>::const_iterator it = _env.begin(); it != _env.end(); ++it) {
        std::string entry = it->first + "=" + it->second;
        env[i] = new char[entry.size() + 1];
        std::strcpy(env[i], entry.c_str());
        i++;
    }
    env[i] = NULL;
    return env;
}

std::string CgiHandler::executeCgi(Request& request) {
    pid_t pid;
    int saveStdin = dup(STDIN_FILENO);
    int saveStdout = dup(STDOUT_FILENO);
    FILE *fIn = tmpfile();
    FILE *fOut = tmpfile();
    int fdIn = fileno(fIn);
    int fdOut = fileno(fOut);
    std::string output;
    char **env;

    try {
        env = _getEnvAsCstrArray();
    } catch (...) {
        std::cerr << "Environment allocation failed." << std::endl;
        return "Status: 500\r\n\r\n";
    }


    write(fdIn, request.getBody().c_str(), request.getBody().size());
    lseek(fdIn, 0, SEEK_SET);

    pid = fork();
    if (pid == -1) {
        std::cerr << "Fork failed." << std::endl;
        return "Status: 500\r\n\r\n";
    } else if (pid == 0) {
        dup2(fdIn, STDIN_FILENO);
        dup2(fdOut, STDOUT_FILENO);
        std::string cgi_path = request.getCgipass();
        if (cgi_path.empty()) {
            std::cerr << "CGI path       is empty." << std::endl;
            write(STDOUT_FILENO, "Status: 500\r\n\r\n", 16);
            exit(1);
        }
        char * const argv[] = {
            const_cast<char*>(cgi_path.c_str()),
            const_cast<char*>(_env["SCRIPT_NAME"].c_str()),
            NULL 
        };

        execve(cgi_path.c_str(), argv, env);
        std::cerr << "Execve failed." << std::endl;
        write(STDOUT_FILENO, "Status: 500\r\n\r\n", 16);
        exit(1);
    } else {
        waitpid(pid, NULL, 0);
        lseek(fdOut, 0, SEEK_SET);
        char buffer[CGI_BUFSIZE];
        int bytes;

        while ((bytes = read(fdOut, buffer, CGI_BUFSIZE - 1)) > 0) {
            buffer[bytes] = '\0';
            output += buffer;
        }
    }

    dup2(saveStdin, STDIN_FILENO);
    dup2(saveStdout, STDOUT_FILENO);
    fclose(fIn);
    fclose(fOut);
    close(saveStdin);
    close(saveStdout);

    for (size_t i = 0; env[i]; i++)
        delete[] env[i];
    delete[] env;


    return output;
}


  bool check_cgi_executable(Request& request) {
        if (request.getPath().empty() || request.loc_config.cgi.empty()) {
            return false;
        }
        std::string extension = request.getPath().substr(request.getPath().find_last_of('.') + 1);
        if (request.loc_config.cgi.count(extension) == 0)
            return false;
        if (request.loc_config.cgi.at(extension).empty())
            return false;
        request.setCgipass(request.loc_config.cgi.at(extension));

        return true;
    }



std::string parseCgiOutput(const std::string& rawOutput, Request& request) {
    std::string headersPart;
    std::string bodyPart;
    std::string valid_output;
    std::string status = "200 OK";

    size_t separatorPos = rawOutput.find("\r\n\r\n");

    if (separatorPos != std::string::npos) {
        headersPart = rawOutput.substr(0, separatorPos);
        bodyPart = rawOutput.substr(separatorPos + 4);
    } else {
        headersPart = "";
        bodyPart = rawOutput;
    }


    std::stringstream ss(headersPart);
    std::string line;
    std::string cookie;


    while (std::getline(ss, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }
        
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 1);

        value.erase(0, value.find_first_not_of(" \t"));

        if (key == "Status") {
            status = value;

        } else {
            request.setHeadr(key, value);
            if (key == "Set-Cookie") {
                cookie  += key + ": " + value + "\r\n";
            }
        }

    }
    valid_output  = "HTTP/1.0 " + status + "\r\n";
    valid_output += "Content-type: " + request.getHeadr("Content-type") + "\r\n";
    valid_output += cookie + "Content-Length: " + to_string(bodyPart.size()) + "\r\n\r\n";
    valid_output += bodyPart;

    return valid_output;
}
