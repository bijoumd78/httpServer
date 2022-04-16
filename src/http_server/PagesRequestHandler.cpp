#include "PagesRequestHandler.h"
#include "Logger.h"
#include "UserLogin.h"
#include "HandleImageProcessing.h"
#include <Poco/RegularExpression.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Format.h>
#include <Poco/String.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/Delegate.h>
#include <Poco/Thread.h>
#include <sstream>
#include <string>


using namespace Poco::Net;

namespace http_server
{
    void PagesRequestHandler::handleRequest(HTTPServerRequest& req, HTTPServerResponse& res)
    {
        try
        {
            if (const auto requestPath{ req.getMethod() }; requestPath == Poco::Net::HTTPRequest::HTTP_GET)
            {
                httpGet(req, res);
            }
            else if (requestPath == Poco::Net::HTTPRequest::HTTP_POST)
            {
                httpPost(req, res);
            }
            else if (requestPath == Poco::Net::HTTPRequest::HTTP_PUT)
            {
                httpPut(req, res);
            }
            else if (requestPath == Poco::Net::HTTPRequest::HTTP_DELETE)
            {
                httpDelete(req, res);
            }

            if (!res.sent())
            {
                throw HttpRequestException(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST, "Bad request");
            }
        }
        catch (const HttpRequestException& e)
        {
            if (!res.sent())
            {
                res.setStatusAndReason(e.status, e.what());
                res.send();
            }
        }
        catch (const Poco::Exception& e)
        {
            if (!res.sent())
            {
                res.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_INTERNAL_SERVER_ERROR, e.what());
                res.send();
            }
        }
        catch (...)
        {
            if (!res.sent())
            {
                res.setStatusAndReason(Poco::Net::HTTPServerResponse::HTTP_INTERNAL_SERVER_ERROR, "Internal server error");
                res.send();
            }
        }
    }



    void http_server::PagesRequestHandler::httpGet(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& res)
    {
        Common::Logging::Logger::log("information", "source", -1, "HttpGet is called");

        // Prepare response
        res.setChunkedTransferEncoding(true);
        Poco::Path path(req.getURI());
        const auto& extension = Poco::toLower(path.getExtension());
        const auto& tmp = root + path.toString();

        try {
            if (extension.empty())
            {
                const std::string tmp1{ tmp + "login.html" };
                res.sendFile(tmp1, "text/html");
                res.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
            }
            else if (extension == "json")
            {
                res.sendFile(tmp, "application/json");
                res.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
            }
            else if (extension == "html")
            {
                res.sendFile(tmp, "text/html");
                res.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
            }
            else if (extension == "jpg" || extension == "jpeg")
            {
                res.sendFile(tmp, "image/jpeg");
                res.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
            }
            else if (extension == "png")
            {
                res.sendFile(tmp, "image/png");
                res.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
            }
            else if (extension == "js")
            {
                res.sendFile(tmp, "application/javascript");
                res.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
            }
            else if (extension == "css")
            {
                res.sendFile(tmp, "text/css");
                res.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
            }
            else if (extension == "xml")
            {
                res.sendFile(tmp, "application/xml");
                res.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
            }
            else if (extension == "ico")
            {
                //TODO: Not needed!
                //res.sendFile(tmp, "image/x-icon");
                //res.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
            }
        }
        catch (const Poco::Exception& e)
        {
            Common::Logging::Logger::log("information", "source", -1, e.what());
            res.setStatusAndReason(HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST, "Page not found");
        }
    }


    void http_server::PagesRequestHandler::httpPost(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& res)
    {
        Common::Logging::Logger::log("information", "source", -1, "HttpPost is called");

        // Handle data from HTTP form 
        FormPartHandler formHandler;

        if (HTMLForm form(req, req.stream(), formHandler); !form.empty())
        {
            std::vector<std::string> loginParams;

            std::for_each(form.begin(), form.end(), [&loginParams](const std::pair<std::string, std::string>& e) {loginParams.push_back(e.second); });

#if 0
            for (const auto&[key, value] : form)
            {
                std::stringstream ss;
                ss << key << ": " << value;
                Common::Logging::Logger::log("information", "source", -1, ss.str());
            }
#endif

            HandleUserLogin::UserLoginDB db;

            if (loginParams.size() == 2)
            {
                HandleUserLogin::User_ l_user{ loginParams[0], loginParams[1] };

                // Search for authorized users in the database
                if (const auto isFound = db.searchUser(l_user); !isFound)
                {
                    // Send new user registration page
                    res.setChunkedTransferEncoding(true);
                    const std::string tmp{ root + "/registration.html" };
                    res.sendFile(tmp, "text/html");
                    res.setStatus(HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
                    return;
                }
            }
            else if ((loginParams.size() == 4) && (Poco::icompare(loginParams[3], "newAccount") == 0))
            {
                // At this point, I assume the user password has been confirmed
                HandleUserLogin::User_ l_user{ loginParams[0], loginParams[1] };

                // We need to make sure the user does not already exist.
                // Each new user needs to be unique (No duplicated record)
                if (const auto isFound = db.searchUser(l_user); !isFound)
                {
                    // Insert the new user into the database
                    db.insertUser(l_user);
                }
                else
                {
                    Common::Logging::Logger::log("warning", "UserLogin", -1, "User already exists.");
                }

                // Return the login page
                res.setChunkedTransferEncoding(true);
                const std::string tmp{ root + "/login.html" };
                res.sendFile(tmp, "text/html");
                res.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
                return;
            }
        }

        try {
            // Prepare response
            Poco::Path path(req.getURI());
            const auto& extension = Poco::toLower(path.getExtension());
            const auto& tmp = root + path.toString();

            if (extension == "html")
            {
                res.setChunkedTransferEncoding(true);
                res.sendFile(tmp, "text/html");
                res.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
            }

            if (Poco::icompare(path.getFileName(), "aiform") == 0)
            {
                std::stringstream ss;
                ss << "Starting image processing <Horse2Zebra>: " << formHandler.fileName();
                Common::Logging::Logger::log("information", "source POST AI method", -1, ss.str());

                // Transfrom horse image to zebra image
                AI::ImageProcessingUnit::InputArg inputArgs;
                inputArgs.xmlModel_ = "C:/Users/bijou/Desktop/PocoServer/PocoServer/src/AI_Image_Processing_Unit/ai_models/traced_zebra_model.pt";
                inputArgs.inputImagePath_ = root + "/Download/" + formHandler.fileName();
                inputArgs.outputImagePath_ = root + "/Download/out_" + formHandler.fileName();

                // Stall the computation for 500 ms for image upload to complete
                Poco::File file(inputArgs.inputImagePath_);
                do
                {
                    Poco::Thread::sleep(500);
                } while (!file.isFile());


                AI::ImageProcessing::HandleImageProcessing<AI::ImageProcessingUnit::InputArg> proc;
                AI::ImageProcessingUnit::RunCycleganAPI horseZebraImg;
                proc.processEvent += Poco::delegate(&horseZebraImg, &AI::ImageProcessingUnit::RunCycleganAPI::processImage);
                proc.processImage(inputArgs);
                proc.processEvent -= Poco::delegate(&horseZebraImg, &AI::ImageProcessingUnit::RunCycleganAPI::processImage);

                // Send reponse
                res.setChunkedTransferEncoding(true);
                res.setContentType("text/html");
                std::ostream& ostr = res.send();
                // Fill outstream with html form
                fillOstream("Download/" + formHandler.fileName(), "Download/out_" + formHandler.fileName(), ostr, true);
                res.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
            }
            else if (Poco::icompare(path.getFileName(), "aiform2") == 0)
            {
                std::stringstream ss;
                ss << "Starting image processing <HighRes>: " << formHandler.fileName();
                Common::Logging::Logger::log("information", "source POST AI method", -1, ss.str());

                // Transfrom image to high resolution images
                AI::ImageProcessingUnit2::InputArgList inputArgs;
                inputArgs.deviceName_ = "CPU";
                inputArgs.xmlModel_ = "C:/Users/bijou/Desktop/PocoServer/PocoServer/src/AI_Image_Processing_Unit2/ai_models/OpenVINO_IR/FP32/super_resolution.xml";
                inputArgs.inputImagePath_ = root + "/Download/" + formHandler.fileName();
                inputArgs.outputImagePath_ = root + "/Download/out_" + formHandler.fileName();

                // Stall the computation for 500 ms for image upload to complete
                Poco::File file(inputArgs.inputImagePath_);
                do
                {
                    Poco::Thread::sleep(500);
                } while (!file.isFile());

                AI::ImageProcessing::HandleImageProcessing<AI::ImageProcessingUnit2::InputArgList> proc;
                AI::ImageProcessingUnit2::RunHighResNet highResImg;
                proc.processEvent += Poco::delegate(&highResImg, &AI::ImageProcessingUnit2::RunHighResNet::processImage);
                proc.processImage(inputArgs);
                proc.processEvent -= Poco::delegate(&highResImg, &AI::ImageProcessingUnit2::RunHighResNet::processImage);

                // Send reponse
                res.setChunkedTransferEncoding(true);
                res.setContentType("text/html");
                std::ostream& ostr = res.send();
                // Fill outstream with html form
                fillOstream("Download/" + formHandler.fileName(), "Download/out_" + formHandler.fileName(), ostr, false);
                res.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
            }
            else
            {
                //TODO:
            }
        }
        catch (const Poco::Exception& e)
        {
            Common::Logging::Logger::log("information", "source", -1, e.what());
            res.setStatusAndReason(HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST, "Page not found");
        }
    }

    void http_server::PagesRequestHandler::httpPut(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& res)
    {
        Common::Logging::Logger::log("information", "source", -1, "HttpPut is called");

        res.send().flush();
        res.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
    }

    void http_server::PagesRequestHandler::httpDelete(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& res)
    {
        Common::Logging::Logger::log("information", "source", -1, "HttpDelete is called");

        res.send().flush();
        res.setStatus(Poco::Net::HTTPServerResponse::HTTP_OK);
    }

    void PagesRequestHandler::fillOstream(std::string_view inputImgPath, std::string_view outputImgPath, std::ostream& ostr, bool isHourseZebra)const
    {
        ostr <<
            "<!DOCTYPE html>\n"
            "<html lang = \"en-US\">\n"
            "<head>\n"
            "<title>AI Power Image Processing</title>\n"
            "<meta charset = \"utf-8\">\n"
            "<meta name = \"viewport\" content = \"width=device-width, initial-scale=1, shrink-to-fit=no\">\n";
        if (isHourseZebra) {
            ostr << "<link rel=\"stylesheet\" type=\"text/css\" href=\"css/ai2.css\">\n";
        }
        else
        {
            ostr << "<link rel=\"stylesheet\" type=\"text/css\" href=\"css/ai23.css\">\n";
        }
        ostr <<
            "<link rel = \"stylesheet\" type = \"text/css\" href = \"css/style.css\">\n"
            "<script src = \"https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js\"></script>\n"
            "<script src = \"javascripts/app.js\"></script>\n"
            "</head>\n"
            "<body>\n"
            "<header>\n"
            "<div class = \"container\">\n"
            "<div class = \"title-header\">\n"
            "<div class = \"logo\">\n"
            "<a href = \"index.html\">\n"
            "<img src = \"images/logo.png\" alt = \"Logo\">\n"
            "<img src = \"images/coffee-cup-icon.png\" alt = \"Logo\">\n"
            "</a>\n"
            "</div>\n"
            "<div class = \"title\">ClydeBank Coffee Shop</div>\n"
            "</div>\n"
            "</div>\n"
            "</header>\n"
            "<nav>\n"
            "<div class = \"container\">\n"
            "<ul>\n"
            "<li class = \"active\"><a href = \"index.html\">Menu</a></li>\n"
            "<li><a href = \"webSocketChat.html\">WebSocketChat</a></li>\n"
            "<li><a href = \"ai.html\">AI Image Processing</a></li>\n"
            "<li><a href = \"events.html\">Events</a></li>\n"
            "<li><a href = \"contact.html\">Contact</a></li>\n"
            "</ul>\n"
            "</div>\n"
            "</nav>\n"
            "<section>\n"
            "<nav>\n"
            "<div class=\"navcontainer\">\n"
            "<ul>\n"
            "</form>\n"
            "<h2>Load Horse Image</h2>\n"
            "<form id = \"horseZebra\" method = \"POST\" action = \"/aiform\" enctype = \"multipart/form-data\">\n"
            "<label for = \"file\">Choose JPG image to upload</label>\n"
            "<input type = \"file\" name = \"file\" size = \"31\"  accept = \"image/jpeg\">\n"
            "<input type = \"submit\" id = \"btnHorseZebra\" value = \"Upload\">\n"
            "</form>\n"
            "</ul>\n"
            "<br>\n"
            "</br>\n"
            "<ul>\n"
            "</form>\n"
            "<h2>High Resolution Image</h2>\n"
            "<form id = \"highRes\" method = \"POST\" action = \"/aiform2\" enctype = \"multipart/form-data\">\n"
            "<input type = \"file\" name = \"file\" size = \"31\"  accept = \"image/png, image/jpeg\">\n"
            "<input type = \"submit\" id = \"btnHighRes\" value = \"Upload\">\n"
            "</form>\n"
            "</ul>\n"
            "</div>\n"
            "</nav>\n"
            "<article>\n";
        ostr <<
            "<img src=";
        ostr << inputImgPath;
        ostr << " alt = \"Input image\" width=\"400\" height=\"400\">\n";
        ostr <<
            "<img src=";
        ostr << outputImgPath;
        ostr << " alt = \"Output image\">\n"
            "</article>\n"
            "</section>\n"
            "<footer>\n"
            "<p>&copy; All rights reserved</p>\n"
            "</footer>\n"
            "</body>\n"
            "</html>\n";
    }
}
