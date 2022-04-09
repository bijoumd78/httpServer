#include "RequestHandlerFactory.h"
#include "PagesRequestHandler.h"
#include "WebSocketReqHandler.h"

namespace http_server
{
    Poco::Net::HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest& req)
    {
        if (req.hasToken("Connection", "upgrade") && Poco::icompare(req.get("Upgrade", ""), "websocket") == 0)
        {
            return new WebSocketReqHandler;
        }
        else
        {
            return new http_server::PagesRequestHandler;
        }
    }

}