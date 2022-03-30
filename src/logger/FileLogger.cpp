#include "FileLogger.h"
#include "Poco/Logger.h"
#include <filesystem>

namespace Common::Logging {

    FileLogger::FileLogger(std::string_view configFile):
        pFC_{ new FormattingChannel(pPF_, pFile_) },
        pConfig_{ std::make_unique<Configuration>(configFile) }
    {
        setFileProperties(pFile_, pPF_, pConfig_.get(), pFC_);
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
        const AutoPtr<Configuration>& configPtr, const AutoPtr<FormattingChannel>& formattingChannelPtr) const
    {
        using namespace std::filesystem;
        // Make sure the logging directory exit or create it.
        auto log_dir = current_path();
        log_dir /= configPtr->getFilePath();
        if (!exists(log_dir)) { create_directory(log_dir.parent_path()); }

        filePtr->setProperty("path", log_dir.u8string());
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
