#include "WebSocketReqHandler.h"
#include "Logger.h"

#include <Poco/Net/NetException.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/AbstractHTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/TextConverter.h>
#include <Poco/Latin1Encoding.h>
#include <Poco/UTF8Encoding.h>
#include <Poco/String.h>
#include <Poco/Path.h>
#include <algorithm>

using Poco::Net::WebSocketException;
using Poco::Net::HTTPResponse;
using Poco::Timespan;

namespace http_server
{
    std::vector<WebSocketReqHandler *> WebSocketReqHandler::connectedSockets_;
    Poco::FastMutex                    WebSocketReqHandler::mtx_;

    WebSocketReqHandler::~WebSocketReqHandler()
    {
        shutdown();
    }

    void WebSocketReqHandler::handleRequest(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& res)
    {
        try
        {
            if (!pWS_)
            {
                pWS_ = std::make_unique<WebSocket>(req, res);
                constexpr const int MAXPAYLOAD{ 1024 };
                pWS_->setMaxPayloadSize(MAXPAYLOAD);
                Timespan ts(600, 0);
                pWS_->setReceiveTimeout(ts);
                pWS_->setSendTimeout(ts);

                Poco::FastMutex::ScopedLock lock(mtx_);
                connectedSockets_.push_back(this);
            }

            Poco::Path path(req.getURI());

            if (Poco::icompare(path.toString(), "\\search") == 0)
            {
                handleSearch();
            }
            else if (Poco::icompare(path.toString(), "\\chat") == 0)
            {
                handleChat();
            }
            else
            {
                throw Poco::Exception("Unknown webSocket endpoint");
            }
        }
        catch (const WebSocketException& exc)
        {
            Common::Logging::Logger::log("error", "WebSocketReqHandler", -1, exc.displayText());
            switch (exc.code())
            {
            case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
                res.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
                // fallthrough
            case WebSocket::WS_ERR_NO_HANDSHAKE:
            case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
            case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
                res.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
                res.setContentLength(0);
                res.send();
                break;
            default:
                throw Poco::Exception("unknown WebSocket exception");
            }
        }
    }

    void WebSocketReqHandler::handleSearch()
    {
        Common::Logging::Logger::log("information", "WebSocketReqHandler", -1, "Search webSocket connection established.");
        Poco::Buffer<char> buf(0);
        auto n = pWS_->receiveFrame(buf, flags_);
        if (!buf.empty())
        {
            std::string tmp;
            tmp.resize(n);
            std::copy(buf.begin(), buf.end(), begin(tmp));
            // Both file repository and database are searchable
            if (auto& result = Common::Logging::Logger::search<Common::Logging::FileLogger/*DatabaseLogger*/>(tmp); !result.empty()) {
                for (auto& e : result)
                {
                    // Remove comma for now
                    std::replace_copy_if(e.begin(), e.end(), e.begin(), [](char c) {return c == ','; }, ' ');
                    pWS_->sendFrame(e.c_str(), static_cast<int>(e.length()));
                }
            }
        }
        Common::Logging::Logger::log("information", "WebSocketReqHandler", -1, "Search webSocket connection closed.");
    }

    void WebSocketReqHandler::handleChat()
    {
        Common::Logging::Logger::log("information", "WebSocketReqHandler", -1, "Chat webSocket connection established.");   

        while ((flags_ & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE)
        {
            Poco::Buffer<char> buf(0);
            auto n = pWS_->receiveFrame(buf, flags_);
            if (!buf.empty())
            {
                std::string tmp;
                tmp.resize(n);
                std::copy(buf.begin(), buf.end(), begin(tmp));

                // Sent message to all connected parties
                if (!connectedSockets_.empty())
                {
                    for (const auto& e : connectedSockets_)
                    {
                        e->send(tmp);
                    }

                }
            }
        }

        for (auto it = connectedSockets_.begin(); it != connectedSockets_.end(); ++it)
        {
            if (*it == this)
            {
                Common::Logging::Logger::log("information", "WebSocketReqHandler", -1, "Chat webSocket connection closed");
                Poco::FastMutex::ScopedLock lock(mtx_);
                connectedSockets_.erase(it);
                break;
            }
        }
    }

    void WebSocketReqHandler::send(const std::string& buffer)
    {
        pWS_->sendFrame(buffer.data(), static_cast<int>(buffer.size()), flags_);
    }

    void WebSocketReqHandler::shutdown()
    {
        if (pWS_)
        {
            pWS_->shutdown();
        }
    }
}
