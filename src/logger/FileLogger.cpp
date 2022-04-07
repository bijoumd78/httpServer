#include "FileLogger.h"
#include <Poco/Logger.h>
#include <Poco/File.h>
#include <Poco/RegularExpression.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Path.h>
#include <fstream>

namespace Common::Logging {

	std::string FileLogger::logDir_{};

    FileLogger::FileLogger(std::string_view configFile):
        pFC_{ new FormattingChannel(pPF_, pFile_) },
        pConfig_{ std::make_unique<Configuration>(configFile) }
    {
		// Make sure the logging directory exit or create it.
		auto logFile = std::filesystem::current_path();
		logFile /= pConfig_->getFilePath();
		logDir_ = logFile.parent_path().u8string();
		if (!exists(logFile)) { create_directory(logFile.parent_path()); }
        setFileProperties(pFile_, pPF_, pConfig_.get(), pFC_, logFile);
    }

	std::vector<std::string> FileLogger::search(const std::string & pattern)
	{
		if (Poco::File Dir(logDir_); !Dir.isDirectory())
		{
			throw std::invalid_argument("Search directory does not exist");
		}

		std::vector<std::string> result;
		Poco::RegularExpression re(pattern, Poco::RegularExpression::RE_CASELESS);

		Poco::DirectoryIterator it(logDir_);
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

	void FileLogger::logFatal(std::string_view source, const int transaction, std::string_view msg)
    {
        Poco::Logger::root().fatal("[FATAL]:    " + std::string{ source.data() + std::string{" "} + std::to_string(transaction) + std::string{" "} + msg.data() });
    }

    void FileLogger::logError(std::string_view source, const int transaction, std::string_view msg)
    {
        Poco::Logger::root().error("[ERROR]:    " + std::string{ source.data() + std::string{" "} + std::to_string(transaction) + std::string{" "} + msg.data() });
    }

    void FileLogger::logWarning(std::string_view source, const int transaction, std::string_view msg)
    {
        Poco::Logger::root().warning("[WARNING]:  " + std::string{ source.data() + std::string{" "} + std::to_string(transaction) + std::string{" "} + msg.data() });
    }

    void FileLogger::logInfo(std::string_view source, const int transaction, std::string_view msg)
    {
        Poco::Logger::root().information("[INFO]:     " + std::string{ source.data() + std::string{" "} + std::to_string(transaction) + std::string{" "} + msg.data() });
    }

    void FileLogger::logDebug(std::string_view source, const int transaction, std::string_view msg)
    {
        Poco::Logger::root().debug("[DEBUG]:    " + std::string{ source.data() + std::string{" "} + std::to_string(transaction) + std::string{" "} + msg.data() });
    }

    void FileLogger::logTrace(std::string_view source, const int transaction, std::string_view msg)
    {
        Poco::Logger::root().trace("[TRACE]:    " + std::string{ source.data() + std::string{" "} + std::to_string(transaction) + std::string{" "} + msg.data() });
    }

    void FileLogger::setFileProperties(AutoPtr<FileChannel>& filePtr, AutoPtr<PatternFormatter>& patternFormatterPtr,
        const AutoPtr<Configuration>& configPtr, const AutoPtr<FormattingChannel>& formattingChannelPtr, const std::filesystem::path& logFile) const
    {
        filePtr->setProperty("path", logFile.u8string());
        filePtr->setProperty("rotation", configPtr->getFileRotation());
        filePtr->setProperty("archive", configPtr->getFileArchive());
        filePtr->setProperty("compress", configPtr->getFileCompress());
        filePtr->setProperty("purgeAge", configPtr->getFilePurgeAge());
        filePtr->setProperty("purgeCount", configPtr->getFilePurgeCount());

        patternFormatterPtr->setProperty("pattern", configPtr->getFilePattern());
        patternFormatterPtr->setProperty("times", configPtr->getFileTimes());
        Poco::Logger::root().setLevel(configPtr->getFileLoggingLevel());
        Poco::Logger::root().setChannel(formattingChannelPtr);
    }

}
