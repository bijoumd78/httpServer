#pragma once
#include "ILogger.h"
#include "Configuration.h"
#include <Poco/FileChannel.h>
#include <Poco/FormattingChannel.h>
#include <Poco/PatternFormatter.h>
#include <filesystem>

namespace Common::Logging {

    using Poco::FormattingChannel;
    using Poco::PatternFormatter;
    using Poco::FileChannel;

    class FileLogger final : public ILogger
    {
    public:
        explicit FileLogger(std::string_view configFile);
        ~FileLogger() override= default;

        // Prevent copy construction and assignment operation.
        FileLogger(const FileLogger&) = delete;
        FileLogger& operator=(const FileLogger&) = delete;
       

        // Allow move construction and assignment operation.
        FileLogger& operator=(FileLogger&&) = default;
        FileLogger(FileLogger&&) = default;

        // Search log files
        static std::vector<std::string> search(const std::string& pattern);

        void logFatal(std::string_view source, const int transaction, std::string_view msg) override;
        void logError(std::string_view source, const int transaction, std::string_view msg) override;
        void logWarning(std::string_view source, const int transaction, std::string_view msg) override;
        void logInfo(std::string_view source, const int transaction, std::string_view msg) override;
        void logDebug(std::string_view source, const int transaction, std::string_view msg) override;
        void logTrace(std::string_view source, const int transaction, std::string_view msg) override;

    private:
        void setFileProperties(AutoPtr<FileChannel>& filePtr, AutoPtr<PatternFormatter>& patternFormatterPtr,
            const AutoPtr<Configuration>& configPtr, const AutoPtr<FormattingChannel>& formattingChannelPtr, const std::filesystem::path& logFile)const;

        AutoPtr<FileChannel> pFile_{ new FileChannel };
        AutoPtr<PatternFormatter> pPF_{ new PatternFormatter };
        AutoPtr<FormattingChannel> pFC_;
        std::unique_ptr<Configuration> pConfig_;
        static std::string logDir_;
    };

}

