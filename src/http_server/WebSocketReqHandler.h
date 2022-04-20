#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/WebSocket.h>
#include <vector>
#include <memory>

using Poco::Net::WebSocket;

namespace http_server
{
    class WebSocketReqHandler : public Poco::Net::HTTPRequestHandler
    {
    public:
        ~WebSocketReqHandler();

        void handleRequest(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& res)override;
        void send(const std::string& buffer);
        void shutdown();

    private:
        std::unique_ptr<WebSocket> pWS_;
        int                        flags_{};
    };
}

