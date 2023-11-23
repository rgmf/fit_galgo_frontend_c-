#include <cstdint>
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <vector>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "HttpClient.h"
#include "HttpClientException.h"

using namespace std;

const int BUFFER_SIZE = 1024;

HttpClient::HttpClient(const string& host, int port)
{
    this->host = host;
    this->port = port;
}

void HttpClient::addHeader(const string& header, const string& value)
{
    this->headers << header << ": " << value << "\r\n";
}

string HttpClient::sendGetRequest(const string& path)
{
    // 1.- Create socket.
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
	throw HttpClientException("Failed to create socket.");
    }

    // 2.- Resolve host IP address.
    struct sockaddr_in server;
    if (inet_pton(AF_INET, this->host.c_str(), &(server.sin_addr)) <= 0) {
	throw HttpClientException("Failed to resolve host IP address.");
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(this->port);

    // 3.- Connect to the server.
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
	throw HttpClientException("Failed to connect to the server.");
    }

    // 4.- Send HTTP GET request.
    stringstream request;
    request << "GET " << path << " HTTP/1.1\r\n";
    request << "Host: " << this->host << "\r\n";
    request << this->headers.str();
    request << "Connection: close\r\n\r\n";
    if (send(sock, request.str().c_str(), request.str().length(), 0) < 0) {
	throw HttpClientException("Failed to send HTTP GET request.");
    }

    // 5.- Receive the response.
    stringstream response;
    char buffer[4096];
    int bytesRead;
    while ((bytesRead = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
	response.write(buffer, bytesRead);
    }

    // 6.- Close the socket connection.
    close(sock);

    // 7.- Return the response.
    return response.str();
}

string HttpClient::sendPostRequest(const string& path, const string& body)
{
    // 1.- Create socket.
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
	throw HttpClientException("Failed to create socket.");
    }

    // 2.- Resolve host IP address.
    struct sockaddr_in server;
    if (inet_pton(AF_INET, this->host.c_str(), &(server.sin_addr)) <= 0) {
	throw HttpClientException("Failed to resolve host IP address.");
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(this->port);

    // 3.- Connect to the server.
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
	throw HttpClientException("Failed to connect to the server.");
    }

    // 4.- Send HTTP POST request.
    stringstream request;
    request << "POST " << path << " HTTP/1.1\r\n";
    request << "Host: " << this->host << "\r\n";
    request << this->headers.str();
    request << "Connection: close" << "\r\n";
    request << "\r\n";
    request << body;
    if (send(sock, request.str().c_str(), request.str().length(), 0) < 0) {
	throw HttpClientException("Failed to send HTTP POST request.");
    }

    // 5.- Receive the response.
    stringstream response;
    char buffer[4096];
    int bytesRead;
    while ((bytesRead = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
	response.write(buffer, bytesRead);
    }

    // 6.- Close the socket connection.
    close(sock);

    // 7.- Return the response.
    return response.str();
}

string HttpClient::sendFileRequest(const string& token)
{
    string path = "/files/";

    // 1.- File size and opening.
    string filePath = "/home/roman/fit_tmp/garmin_monitor/Sleep/D9R82917.fit";
    //uintmax_t fileSize = filesystem::file_size(filePath);
    ifstream file(
        "/home/roman/fit_tmp/garmin_monitor/Sleep/D9R82917.fit",
        ios::binary);
    if (!file) {
        throw HttpClientException("Failed opening the file to send.");
    }

    // 2.- Create socket.
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        throw HttpClientException("Failed to create socket.");
    }

    // 3.- Resolve host IP address.
    struct sockaddr_in server;
    if (inet_pton(AF_INET, this->host.c_str(), &(server.sin_addr)) <= 0) {
        throw HttpClientException("Failed to resolve host IP address.");
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(this->port);

    // 4.- Connect to the server.
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        throw HttpClientException("Failed to connect to the server.");
    }

    // 5.- Send HTTP POST request.
    stringstream request;
    request << "POST " << path << " HTTP/1.1\r\n";
    request << "Host: " << this->host << "\r\n";
    request << "Connection: close\r\n";
    request << "Authorization: Bearer " << token << "\r\n";
    request << "Content-Type: multipart/form-data; boundary=------------------------itw0KMzIbtR21QTdgsI1vj\r\n";
    request << "\r\n";
    request << "--------------------------itw0KMzIbtR21QTdgsI1vj\r\n";
    request << "Content-Disposition: form-data; name=\"files\"; filename=\"D9R82917.fit\"\r\n";
    request << "Content-Type: application/octet-stream\r\n";
    request << "\r\n";

    // 6.- Add the file as part of form data and send request.
    request << file.rdbuf();
    if (send(sock, request.str().c_str(), request.str().length(), 0) < 0) {
        throw HttpClientException("Failed to send HTTP POST request.");
    }

    // 7.- Send the request's footer.
    stringstream footer;
    footer << "\r\n";
    footer << "--------------------------itw0KMzIbtR21QTdgsI1vj--\r\n";
    if (send(sock, footer.str().c_str(), footer.str().length(), 0) < 0) {
        throw HttpClientException("Failed to send HTTP POST footer request.");
    }

    // 8.- Receive the response.
    stringstream response;
    char responseBuffer[4096];
    int bytesRead;
    while ((bytesRead = recv(sock, responseBuffer, sizeof(responseBuffer), 0)) > 0) {
	response.write(responseBuffer, bytesRead);
    }

    // 9.- Close the socket connection and the file.
    close(sock);
    file.close();

    // 10.- Return the response.
    return response.str();
}

/*string HttpClient::sendFileRequest(const string& token)
{
    string path = "/files/";

    // 1.- File size and opening.
    string filePath = "/home/roman/fit_tmp/garmin_monitor/Sleep/D9R82917.fit";
    uintmax_t fileSize = filesystem::file_size(filePath);
    ifstream file(
        "/home/roman/fit_tmp/garmin_monitor/Sleep/D9R82917.fit",
        ios::binary);
    if (!file) {
        throw HttpClientException("Failed opening the file to send.");
    }

    // 2.- Create socket.
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        throw HttpClientException("Failed to create socket.");
    }

    // 3.- Resolve host IP address.
    struct sockaddr_in server;
    if (inet_pton(AF_INET, this->host.c_str(), &(server.sin_addr)) <= 0) {
        throw HttpClientException("Failed to resolve host IP address.");
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(this->port);

    // 4.- Connect to the server.
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        throw HttpClientException("Failed to connect to the server.");
    }

    // 5.- Send HTTP POST request.
    std::stringstream request;
    request << "POST " << path << " HTTP/1.1\r\n";
    request << "Host: " << this->host << "\r\n";
    request << "Authorization: Bearer " << token << "\r\n";
    //request << "Keep-Alive: 300\r\n";
    //request << "Connection: keep-alive\r\n";
    request << "Content-Type: multipart/form-data; boundary=Boundary\r\n";
    request << "Connection: close" << "\r\n\r\n";
    if (send(sock, request.str().c_str(), request.str().length(), 0) < 0) {
	throw HttpClientException("Failed to send HTTP POST request.");
    }

    // 6.- Send file as part of form data.
    std::stringstream formData;
    formData << "--Boundary\r\n";
    formData << "Content-Disposition: form-data; name=\"files\"; filename=\"D9R82917.fit\"\r\n";
    formData << "Content-Type: application/octet-stream\r\n\r\n";
    if (send(sock, formData.str().c_str(), formData.str().length(), 0) < 0) {
	throw HttpClientException("Failed to send form data.");
    }

    std::vector<char> buffer(BUFFER_SIZE, 0);
    while (file.read(buffer.data(), BUFFER_SIZE)) {
        send(sock, buffer.data(), BUFFER_SIZE, 0);
        memset(buffer.data(), 0, BUFFER_SIZE);
    }

    if (file.gcount() > 0) {
        send(sock, buffer.data(), file.gcount(), 0);
    }

    file.close();

    // Send the final boundary to indicate the end of the file
    send(sock, "\r\n--Boundary--\r\n", 16, 0);

    // 7.- Receive the response.
    std::stringstream response;
    char responseBuffer[4096];
    int bytesRead;
    while ((bytesRead = recv(sock, responseBuffer, sizeof(responseBuffer), 0)) > 0) {
	response.write(responseBuffer, bytesRead);
    }

    // 8.- Close the socket connection.
    close(sock);

    // 9.- Return the response.
    return response.str();
}
*/
