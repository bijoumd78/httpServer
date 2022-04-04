#pragma once
#include <Poco/Mutex.h>
#include <string_view>
#include <vector>
#include <memory>

namespace Common::Logging {

	// Forward enum declaration
	enum class Level;
	class ILogger;

	class Logger final
	{
	public:
		static void addChannel(ILogger* channelPtr);

		template<typename T>
		static void removeChannel();

		static void removeAllChannel();
		static std::vector<std::string> searchFileLogs(const std::string& dir, const std::string& pattern);
		static std::vector<std::string> searchDB(const std::string& pattern);

		static void log(std::string_view level, std::string_view source, int transaction, std::string_view message);

	private:
		static std::vector<ILogger*> channels_;
		static Poco::FastMutex mtx_;
	};

	template<typename T>
	void Logger::removeChannel()
	{
		Poco::FastMutex::ScopedLock lock(mtx_);

		if (!channels_.empty())
		{
			auto itr = std::find_if(std::begin(channels_), std::end(channels_), 
				[](Common::Logging::ILogger* logger) { return typeid(*logger) == typeid(T); });

			if (itr != channels_.end())
			{
				channels_.erase(itr);
			}
		}
	}
}

