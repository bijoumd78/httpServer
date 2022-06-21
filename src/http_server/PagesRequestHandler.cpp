#include "PagesRequestHandler.h"
#include "Logger.h"
#include "UserLogin.h"
#include "RedisCache.h"
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

                // Search user in the cache first and then in the database
                if (const auto& result = rediscache::RedisCache::get(loginParams[0]); 
                    (result != std::nullopt) && result.value() == db.gethashKey(loginParams[1]))
                {
                    // Found user
                    Common::Logging::Logger::log("information", "source", -1, "Cache hit !");
                }
                else if (const auto isFound = db.searchUser(l_user); !isFound)
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

                // Each new user needs to be unique (No duplicated record)
                if (const auto isFound = db.searchUserEmail(l_user); !isFound)
                {
                    // Insert the new user into the database
                    db.insertUser(l_user);

                    // Put the user in the cache
                    Common::Logging::Logger::log("information", "source", -1, "Cache miss !");
                    if (!rediscache::RedisCache::set(l_user.email, db.gethashKey(l_user.password)))
                    {
                        Common::Logging::Logger::log("warning", "source", -1, "Failed to cache username and password");
                    }
                }
                else
                {
                    // Try again. User email already exists.
                    res.setChunkedTransferEncoding(true);
                    const std::string tmp{ root + "/try_again_registration.html" };
                    res.sendFile(tmp, "text/html");
                    res.setStatus(HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
                    return;
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
            R"(<!DOCTYPE html>
            <html lang = "en-US">
            <head>
            <title>AI Power Image Processing</title>
            <meta charset = "utf-8">
            <meta name = "viewport" content = "width=device-width, initial-scale=1, shrink-to-fit=no">)";
        if (isHourseZebra) {
            ostr << R"(<link rel="stylesheet" type="text/css" href="css/ai2.css">)";
        }
        else
        {
            ostr << R"(<link rel="stylesheet" type="text/css" href="css/ai23.css">)";
        }
        ostr <<
            R"(<link rel = "stylesheet" type = "text/css" href = "css/style.css">
            <script src = "https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js"></script>
            <script src = "javascripts/app.js"></script>
            </head>
            <body>
            <header>
            <div class = "container">
            <div class = "title-header">
            <div class = "logo">
            <a href = "index.html">
            <img src = "images/logo.png" alt = "Logo">
            <img src = "images/coffee-cup-icon.png" alt = "Logo">
            </a>
            </div>
            <div class = "title">ClydeBank Coffee Shop</div>
            </div>
            </div>
            </header>
            <nav>
            <div class = "container">
            <ul>
            <li class = "active"><a href = "index.html">Menu</a></li>
            <li><a href = "webSocketChat.html">WebSocketChat</a></li>
            <li><a href= "chat.html">Chat</a></li>
            <li><a href = "ai.html">AI Image Processing</a></li>
            <li><a href = "events.html">Events</a></li>
            <li><a href = "contact.html">Contact</a></li>
            </ul>
            </div>
            </nav>
            <section>
            <nav>
            <div class="navcontainer">
            <ul>
            </form>
            <h2>Load Horse Image</h2>
            <form id = "horseZebra" method = "POST" action = "/aiform" enctype = "multipart/form-data">
            <label for = "file">Choose JPG image to upload</label>
            <input type = "file" name = "file" size = "31"  accept = "image/jpeg">
            <input type = "submit" id = "btnHorseZebra" value = "Upload">
            </form>
            </ul>
            <br>
            </br>
            <ul>
            </form>
            <h2>High Resolution Image</h2>
            <form id = "highRes" method = "POST" action = "/aiform2" enctype = "multipart/form-data">
            <input type = "file" name = "file" size = "31"  accept = "image/png, image/jpeg">
            <input type = "submit" id = "btnHighRes" value = "Upload">
            </form>
            </ul>
            </div>
            </nav>
            <article> <img src=)";
        ostr << inputImgPath;
        ostr << R"( alt = "Input image" width="400" height="400"> <img src=)";
        ostr << outputImgPath;
        ostr << R"( alt = "Output image">
            </article>
            </section>
            <footer>
            <p>&copy; All rights reserved</p>
            </footer>
            </body>
            </html>)";
    }
}
