#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/WebSocket.h>
#include <memory>
#include <vector>

using Poco::Net::WebSocket;

namespace http_server
{
    class WebSocketReqHandler : public Poco::Net::HTTPRequestHandler
    {
    public:
        ~WebSocketReqHandler()override;

        void handleRequest(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& res)override;
        void send(const std::string& buffer);
        void shutdown();

        void handleSearch();
        void handleChat();

    private:
        std::unique_ptr<WebSocket>               pWS_;
        int                                      flags_{129}; // Magic number 
        static std::vector<WebSocketReqHandler*> connectedSockets_;
        static Poco::FastMutex                   mtx_;
    };
}

