#include "RedisSubscribe.h"
#include <Poco/Redis/RedisEventArgs.h>
#include <Poco/Redis/AsyncReader.h>
#include <Poco/Exception.h>
#include <Poco/Delegate.h>

namespace redissubscribe
{
    using namespace Poco::Redis;

    bool RedisSubscribe::connected_ = false;
    Poco::Redis::Client RedisSubscribe::redis_;
    std::vector<std::string> RedisSubscribe::receivedMessages_;

    RedisSubscribe::RedisSubscribe(std::string_view configFile) :
        pConfig_{ std::make_unique<Common::Logging::Configuration>(configFile) }
    {
        host_ = pConfig_->getRedisHost();
        port_ = pConfig_->getRedisPort();

        std::stringstream ss;

        if (!connected_)
        {
            try
            {
                Poco::Timespan t(10, 0); // Connect within 10 seconds
                redis_.connect(host_, port_, t);
                connected_ = true;
                ss << "Connected to [" << host_ << ':' << port_ << ']';
                Common::Logging::Logger::log("information", "RedisPublish", -1, ss.str());
            }
            catch (const Poco::Exception& e)
            {
                ss << "Couldn't connect to [" << host_ << ':' << port_ << ']' << e.message() << ". ";
                Common::Logging::Logger::log("error", "RedisPublish", -1, ss.str());
            }
        }
    }

    RedisSubscribe::~RedisSubscribe()
    {
        if (connected_)
        {
            redis_.disconnect();
            connected_ = false;
            std::stringstream ss;
            ss << "Disconnected from [" << host_ << ':' << port_ << ']';
            Common::Logging::Logger::log("information", "RedisPublish", -1, ss.str());
        }
    }

    class RedisSubscriber
    {
    public:
        void onMessage(const void* pSender, RedisEventArgs& args)
        {
            if (!args.message().isNull())
            {
                auto arrayType = dynamic_cast<Type<Array>*>(args.message().get());
                if (arrayType != nullptr)
                {
                    const Array& array = arrayType->value();
                    if (array.size() == 3)
                    {
                        if (BulkString type = array.get<BulkString>(0); type.value().compare("unsubscribe") == 0)
                        {
                            Poco::Int64 n = array.get<Poco::Int64>(2);
                            // When 0, no subscribers anymore, so stop reading ...
                            if (n == 0) args.stop();
                        }

                        // Get message
                        if (Poco::icompare(array.get<BulkString>(0).value(), "message" ) == 0)
                        {
                            message_ = array.get<BulkString>(2).value();
                        }
                    }
                    else
                    {
                        // Wrong array received. Stop the reader
                        args.stop();
                    }
                }
                else
                {
                    // Invalid type of message received. Stop the reader ...
                    args.stop();
                }
            }
        }

        void onError(const void* pSender, RedisEventArgs& args)
        {
            // No need to call stop, AsyncReader stops automatically when an
            // exception is received.
            Common::Logging::Logger::log("error", "RedisSubscribe", -1, args.exception()->className());
        }

        static std::string getMessage()
        {
            return message_;
        }

    private:
        static std::string message_;
    };

    std::string RedisSubscriber::message_;

    void RedisSubscribe::subscribe(std::string_view topic)
    {
        if (!connected_)
        {
            Common::Logging::Logger::log("error", "RedisSubscribe", -1, "Not connected to Redis server.");
            return;
        }

        RedisSubscriber subscriber;

        Array subscribe;
        subscribe.add("SUBSCRIBE").add(topic.data());

        redis_.execute<void>(subscribe);
        redis_.flush();

        std::string stop{};
        for (;;)
        {
            AsyncReader reader(redis_);
            reader.redisResponse += Poco::delegate(&subscriber, &RedisSubscriber::onMessage);
            reader.redisException += Poco::delegate(&subscriber, &RedisSubscriber::onError);
            reader.start();

            stop = RedisSubscriber::getMessage();
            
            if (Poco::icompare(stop, "") != 0)
            {
                receivedMessages_.push_back(stop);
                // Remove duplicate message
                // TODO: The first message gets dropped for some unknown reason
                // Need to be investigated further. Hence, we send the message twice, so they might be duplicate message
                if(auto last = std::unique(receivedMessages_.begin(), receivedMessages_.end()); last != receivedMessages_.end())
                {
                    receivedMessages_.erase(last, receivedMessages_.end());
                }
            }

            if (Poco::icompare(stop, "break") == 0)
            {
                Array unsubscribe;
                unsubscribe.add("UNSUBSCRIBE");
                redis_.execute<void>(unsubscribe);
                redis_.flush();
                break;
            } 
        }
    }

    std::vector<std::string> RedisSubscribe::getMessages()
    {
        return receivedMessages_;
    }
}
