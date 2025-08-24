#include "../inc/Library.hpp"
#include "../inc/Response.hpp"

std::string Library::_extratexcFilename(std::string filename){
    std::string::size_type dot = filename.find_last_of('.');
    if (dot == std::string::npos)
        return "";
    std::string exc = filename.substr(dot);
    return exc;
}
std::string Library::_extratFilename(std::string filename){
    std::string::size_type dot = filename.find_last_of('.');
    if (dot == std::string::npos)
        return filename;
    std::string exc = filename.substr(0, dot);
    return exc;
}
std::string Library::getUploadFilename(std::string& UriPath, std::string filename, std::string rd){
    std::string file;
    
    file = UriPath + "/";
    file += _extratFilename(filename);
    file += _toString(rd);
    file += _extratexcFilename(filename);
    return file;
}
std::string Library::geneFileName(size_t cliId, size_t lastActive){
    std::ostringstream bu;
    bu << "tmp/";
    bu <<   cliId;
    bu <<  lastActive;
    return bu.str();
}

std::string Library::getJsonResponse(std::map<std::string, std::string> &query, std::map<std::string, std::string> &con){
    std::ostringstream que;
    std::ostringstream body;
    body << "{";
    if (!query.empty()){
        for (std::map<std::string, std::string>::iterator it = query.begin(); it != query.end(); ) {
            body << "\"" << it->first << "\": \"" << it->second << "\",";
            ++it;
        }
    }
    for (std::map<std::string, std::string>::iterator it = con.begin(); it != con.end(); ) {
        body << "\"" << it->first << "\": \"" << it->second << "\"";
        ++it;
        if (it != con.end()) {
            body << ", ";
        }
    }
    body << "}";
    return body.str();
}

long long Library::FileSize(std::string filePath){
    std::ifstream file;
    file.open(filePath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        return -1;
    }
    file.seekg(0, std::ios::end);
    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);
    file.close();
    return size;
}

std::string Library::ErrorResponse(int code, std::map<int, std::string> error_pages){
        std::string body;
        Response re;
        std::map<int, std::string> map = re.getCodeMessageMap();
        std::map<int, std::string>::const_iterator it = map.find(code);

        std::ostringstream res;
        if (error_pages.find(code) != error_pages.end()) {
            std::ifstream file(error_pages.at(code).c_str());
            if (!file.is_open()) {
                body = re.getCodeMessageHtml(code);
            } else {
                std::ostringstream buffer;
                buffer << file.rdbuf();
                body = buffer.str();
                file.close();
            }
        } else {
            body = re.getCodeMessageHtml(code);
        }
        res << "HTTP/1.0 " << code << " " << it->second << "\r\n";
        res << "Content-Type: text/html\r\n";
        res << "Content-Length: " << body.size() << "\r\n";
        res << re.Connectionstatus("close");
        res << body;
        return res.str();
}

bool Library::isHexChar(char c){
    return (std::isxdigit(static_cast<unsigned char>(c)));
}

std::string Library::DecodeUrl(const std::string& str){
    std::ostringstream rzlt;
    for (size_t i = 0; i < str.length(); ++i) {
        char c = str[i];
        if (c == '%') {
            if (i + 2 >= str.length())
                return std::string("");
            char c1 = str[i + 1];
            char c2 = str[i + 2];
            if (!isHexChar(c1) || !isHexChar(c2))
                return std::string("");
            rzlt << static_cast<char>(std::strtol(str.substr(i + 1, 2).c_str(), NULL, 16));
            i += 2;
        } else if (c == '+') {
            rzlt << ' ';
        } else {
            rzlt << c;
        }
    }
    return rzlt.str();
}

int Library::stoi(const std::string i){
    long int num = 0;
    size_t j = 0;
    if (i == "-" || i == "+" || i.size() > 11)
        return -1;
    if (i[j] == '+' || i[j] == '-'){
        if (i[j] == '-') {return -1;}
        j++;}
    while (j < i.size()){
        if (!std::isdigit(i[j])){
            return -1;}
        num = num * 10 +  (i[j] - 48);
        j++;
    }
    if (num > 2147483647 || num < -2147483648)
        return -1;
    return num;
}

void Library::printMsg(std::string m){ std::cout << m << std::endl;}
void Library::printMsgErr(std::string m){ std::cerr << m << std::endl;}