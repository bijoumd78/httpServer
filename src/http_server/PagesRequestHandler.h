#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/PartHandler.h>
#include <Poco/Net/MessageHeader.h>
#include <Poco/CountingStream.h>
#include <Poco/StreamCopier.h>
#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>
#include <fstream>

namespace http_server
{
class PagesRequestHandler : public Poco::Net::HTTPRequestHandler
{
public:
  void handleRequest( Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse &res)override;

protected:
    void httpGet(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& res);
    void httpPost(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& res);
    void httpPut(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& res);
    void httpDelete(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& res);

private:
	// Utility function
	void fillOstream(std::string_view inputImgPath, std::string_view outputImgPath, std::ostream& ostr, bool isHourseZebra)const;

    // WWW root
    const std::string root{ "wwwroot/ClydeBank-Coffee-Shop" };

};

class HttpRequestException : public std::exception
{
public:
    HttpRequestException(Poco::Net::HTTPResponse::HTTPStatus status, const char* msg) : 
        std::exception(msg), 
        status(status)
    {
    }

    const Poco::Net::HTTPResponse::HTTPStatus status;
};

class FormPartHandler : public Poco::Net::PartHandler
{
public:
	FormPartHandler() = default;

	void handlePart(const Poco::Net::MessageHeader& header, std::istream& stream) override
	{
		type_ = header.get("Content-Type", "(unspecified)");

		// Generate a UUID for each file
		Poco::UUIDGenerator& generator = Poco::UUIDGenerator::defaultGenerator();
		Poco::UUID uuid(generator.create()); // time based

		if (header.has("Content-Disposition"))
		{
			std::string disp;
			Poco::Net::NameValueCollection params;
			Poco::Net::MessageHeader::splitParameters(header["Content-Disposition"], disp, params);
			fileName_ = params.get("filename", "(unnamed)");
			uuidFileName_ = uuid.toString() + "_" + fileName_;
		}

		// Save uploaded file to the download directory
		Poco::CountingInputStream istr(stream);
		std::ofstream fileStream;
		fileStream.open(root_ +"/Download/" + uuidFileName_, std::ios::out | std::ios::binary);
		Poco::StreamCopier::copyStream(istr, fileStream);
		length_ = istr.chars();
	}

	size_t length() const
	{
		return length_;
	}

	const std::string& fileName() const
	{
		return fileName_;
	}

	const std::string& uuidFileName() const
	{
		return uuidFileName_;
	}

	const std::string& contentType() const
	{
		return type_;
	}

private:
	size_t length_{};
	std::string type_;
	std::string fileName_;
	std::string uuidFileName_;
	const std::string root_{ "wwwroot/ClydeBank-Coffee-Shop" };
};

}