#include "Logger.h"
#include "ILogger.h"
#include <Poco/File.h>
#include <Poco/RegularExpression.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/Path.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/StringTokenizer.h>
#include <fstream>

namespace Common::Logging {

	std::vector<ILogger*> Logger::channels_{};
	Poco::FastMutex Logger::mtx_;

	void Logger::addChannel(ILogger* channelPtr)
	{
		Poco::FastMutex::ScopedLock lock(mtx_);
		channels_.push_back(channelPtr);
	}

	void Logger::removeAllChannel()
	{
		Poco::FastMutex::ScopedLock lock(mtx_);
		if (!channels_.empty())
		{
			channels_.clear();
		}		
	}

	std::vector<std::string> Logger::searchFileLogs(const std::string& dir, const std::string& pattern)
	{
		Poco::FastMutex::ScopedLock lock(mtx_);

		if (Poco::File Dir(dir); !Dir.isDirectory())
		{
			throw std::invalid_argument("Search directory does not exist");
		}

		std::vector<std::string> result;
		Poco::RegularExpression re(pattern, Poco::RegularExpression::RE_CASELESS);

		Poco::DirectoryIterator it(dir);
		Poco::DirectoryIterator end;
		while (it != end)
		{
			Poco::Path p(it->path());
			std::ifstream infile(p.toString());
			std::string line;

			while (std::getline(infile, line))
			{
				Poco::RegularExpression::Match mtch;
				if (re.match(line, mtch))
				{
					std::vector<std::string> vec;
					vec.push_back(Poco::DateTimeFormatter::format(it->getLastModified(), Poco::DateTimeFormat::SORTABLE_FORMAT));
					vec.push_back(p.getFileName());
					Poco::StringTokenizer tok(line, " ", Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY);
					vec.push_back(tok[0] + " " + tok[1]);
					vec.push_back(tok[2].substr(1, tok[2].length() - 3));
					vec.push_back(tok[3]);
					vec.push_back(tok[4]);
					vec.push_back(Poco::cat(std::string(" "), tok.begin() + 5, tok.end()));
					result.push_back(Poco::cat(std::string(", "), vec.begin(), vec.end()));
				}
			}
			++it;
		}
		return result;
	}

	std::vector<std::string> Logger::searchDB(const std::string & pattern)
	{
		return std::vector<std::string>();
	}

	void Logger::log(std::string_view level, std::string_view source, int transaction, std::string_view msg)
	{
		Poco::FastMutex::ScopedLock lock(mtx_);

		if (!channels_.empty())
		{
			for (const auto& e : channels_)
			{
				switch (ILogger::getLoggingLevel(level.data()))
				{
				case Level::fatal:
					e->logFatal(source, transaction, msg);
					break;
				case Level::error:
					e->logError(source, transaction, msg);
					break;
				case Level::warning:
					e->logWarning(source, transaction, msg);
					break;
				case Level::information:
					e->logInfo(source, transaction, msg);
					break;
				case Level::debug:
					e->logDebug(source, transaction, msg);
					break;
				case Level::trace:
					e->logTrace(source, transaction, msg);
					break;
				default:
					throw Poco::InvalidArgumentException("Unknown logging level");
				}
			}
		}
	}
}
