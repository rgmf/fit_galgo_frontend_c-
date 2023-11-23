#ifndef _ES_RGMF_NET_HTTP_CLIENT_H
#define _ES_RGMF_NET_HTTP_CLIENT_H 1

#include <string>
#include <sstream>

class HttpClient {
private:
    std::string host;
    int port;
    std::stringstream headers;

public:
    /**
     * Constructs an HTTP client with the provided host, path, and port.
     *
     * @param host The host to send the request to.
     * @param port The port to use for the request.
     */
    HttpClient(const std::string& host, int port);

    /**
     * Adds a header that will be append to the requests.
     *
     * @param header HTTP header's name.
     * @param value HTTP header's value.
     */
    void addHeader(const std::string& header, const std::string& value);

    /**
     * Sends an HTTP GET request to the specified host and path.
     *
     * @param path The path of the resource to request.
     *
     * @return string The response received from the server.
     * @throws HttpClientException if an error occurs while sending the
     *         request.
     */
    std::string sendGetRequest(const std::string& path);

    /**
     * Sends an HTTP POST request to the specified host and path and adds
     * headers.
     *
     * @param path The path of the resource to request.
     * @param body The body of the HTTP POST request.
     *
     * @return string The response received from the server.
     * @throws HttpClientException if an error occurs while sending the
     *         request.
     */
    std::string sendPostRequest(
	const std::string& path, const std::string& body);

    std::string sendFileRequest(const std::string& token);
};

#endif // _ES_RGMF_NET_HTTP_CLIENT_H
