#include <string>

#include "HttpClientException.h"

HttpClientException::HttpClientException(const std::string& message)
    : errorMessage(message) {}

const char* HttpClientException::what() const throw() {
    return errorMessage.c_str();
}
