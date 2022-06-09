#include "HttpServer.h"
#include "RequestHandlerFactory.h"
#include "Logger.h"
#include "RedisCache.h"
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocketImpl.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Data/SQLite/Connector.h>

namespace
{
    class ServerSocketImpl : public Poco::Net::ServerSocketImpl
    {
    public:
        using Poco::Net::SocketImpl::init;
    };

    class Socket : public Poco::Net::Socket
    {
    public:
        explicit Socket(const std::string& address)
            : Poco::Net::Socket(new ServerSocketImpl())
        {
            const Poco::Net::SocketAddress socket_address(address);
            auto socket = static_cast<ServerSocketImpl*>(impl());
            socket->init(socket_address.af());
            socket->setReuseAddress(true);
            socket->setReusePort(false);
            socket->bind(socket_address, false);
            socket->listen();
        }
    };

} // anonymous namespace

namespace http_server
{
    void HttpServer::defineOptions(Poco::Util::OptionSet& options)
    {
        Application::defineOptions(options);

        options.addOption(
            Poco::Util::Option("help", "h", "Display help information")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<http_server::HttpServer>(this, &http_server::HttpServer::handleHelp)));

        options.addOption(
            Poco::Util::Option("port", "p", "Enter http port")
            .required(false)
            .repeatable(false)
            .argument("value")
            .callback(Poco::Util::OptionCallback<http_server::HttpServer>(this, &http_server::HttpServer::handlePort)));

        options.addOption(
            Poco::Util::Option("config-file", "f", "Load a config file")
            .required(false)
            .repeatable(false)
            .argument("value")
            .callback(Poco::Util::OptionCallback<http_server::HttpServer>(this, &http_server::HttpServer::handleConfig)));
    }

    int HttpServer::main(const std::vector<std::string>& args)
    {
        if (isHelpRequested_)
        {
            displayHelp();
            stopOptionsProcessing();
        }
        else
        {
            // Register SQLite connector (User database)
            Poco::Data::SQLite::Connector::registerConnector();

            // Application Logging registration
            Common::Logging::ConsoleLogger pConsolechannel(configFile_);
            Common::Logging::Logger::addChannel(&pConsolechannel);

            Common::Logging::FileLogger pFilechannel(configFile_);
            Common::Logging::Logger::addChannel(&pFilechannel);

            //Common::Logging::DatabaseLogger pDatabasechannel(configFile_);
            //Common::Logging::Logger::addChannel(&pDatabasechannel);

            // Connect to redis server
            rediscache::RedisCache rc(configFile_);
            Poco::Net::HTTPServerParams::Ptr parameters = new Poco::Net::HTTPServerParams();
            parameters->setTimeout(100000);
            parameters->setMaxQueued(100);
            parameters->setMaxThreads(12);

            const Poco::Net::ServerSocket socket(Socket("localhost:" + port_));

            Poco::Net::HTTPServer server(new RequestHandlerFactory(), socket, parameters);
            Common::Logging::Logger::log("information", "source", -1, "Poco Restful Web Service started and running.");
            Common::Logging::Logger::log("information", "source", -1, "Type http://localhost:" + port_ + " to use it or type CRLT+C to finish it.");

            // Start sever thread
            server.start();
            waitForTerminationRequest();
            server.stopAll();
        }
        return Poco::Util::Application::EXIT_OK;
    }

    void HttpServer::handleHelp(const std::string&, const std::string&)
    {
        isHelpRequested_ = true;
    }

    void HttpServer::handlePort(const std::string&, const std::string& value)
    {
        port_ = value;
    }

    void HttpServer::handleConfig(const std::string&, const std::string& value)
    {
        configFile_ = value;
    }

    void HttpServer::displayHelp()const
    {
        Poco::Util::HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("/port portNumber /config-file fileName");
        helpFormatter.setHeader("A Generic Httpt server");
        helpFormatter.format(std::cout);
    }
}