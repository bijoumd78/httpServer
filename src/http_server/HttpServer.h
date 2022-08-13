#pragma once
#include <Poco/Util/ServerApplication.h>

namespace http_server
{

    class HttpServer : public Poco::Util::ServerApplication
    {
    protected:
        void initialize(Application& self)override;
        void defineOptions(Poco::Util::OptionSet& options) override;
        int main(const std::vector<std::string>& args) override;

    private:
        void handleHelp(const std::string& name, const std::string& value);
        void handlePort(const std::string& name, const std::string& value);
        void handleConfig(const std::string& name, const std::string& value);

        void displayHelp()const;

        bool isHelpRequested_{ false };
        std::string port_{ "5849" };
        std::string configFile_{ "Config.json" };
        bool isLoggingToConsoleEnabled_;
        bool isLoggingToFileEnabled_;
        bool isLoggingToDBEnabled_;
    };


}