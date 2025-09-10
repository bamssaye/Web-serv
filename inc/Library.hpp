#pragma once

#include "webserv.h"

class Library {
    static std::string                         _extratexcFilename(std::string filename);
    static std::string                         _extratFilename(std::string filename);
public:
    static long long                            FileSize(std::string filePath);
    static std::string                          getUploadFilename(std::string& UriPath, std::string filename, std::string rd);
    static std::string                          geneFileName(size_t cliId, size_t lastActive);
    static std::string                          getJsonResponse(std::map<std::string, std::string> &query, std::map<std::string, std::string> &con);
    static std::string                          ErrorResponse(int code, std::map<int, std::string> error_pages);
    static bool                                 isHexChar(char c);
    static std::string                          DecodeUrl(const std::string& str);
    static int                                  stoi(const std::string i);
    static void                                 printMsg(std::string m);
    static void                                 printMsgErr(std::string m);
};

