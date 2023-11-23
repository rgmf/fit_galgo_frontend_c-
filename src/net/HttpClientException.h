#ifndef _ES_RGMF_NET_HTTP_CLIENT_EXCEPTION_H
#define _ES_RGMF_NET_HTTP_CLIENT_EXCEPTION_H 1

#include <string>
#include <exception>

class HttpClientException: public std::exception {
private:
    std::string errorMessage;
    
public:
    HttpClientException(const std::string& message);
    virtual const char* what() const throw() override;
};

#endif // _ES_RGMF_NET_HTTP_CLIENT_EXCEPTION_H
