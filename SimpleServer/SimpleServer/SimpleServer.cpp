// RestServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

class Global
{
public:
    void OnInitialize(string_t address)
    {
        m_address = address;

        {
            uri_builder uri(address);
            uri.append_path(U("v1/hello"));
            auto hl = new http_listener(uri.to_uri().to_string());
            hl->support(web::http::methods::GET, [](web::http::http_request request)
            {
                ucout << request.to_string() << std::endl;

                auto paths = uri::split_path(uri::decode(request.relative_uri().path()));

                if (paths.empty())
                {
                    request.reply(status_codes::OK, L"Hello world");
                    return;
                }

                const int32_t bottles = std::stol(paths[0]);
                if (bottles > 100)
                {
                    request.reply(status_codes::BadRequest);
                    return;
                }

                string_t res = paths[0] + U(" bottles of beer!");
                request.reply(status_codes::OK, res);
                return;
            });

            m_listeners.emplace_back(hl);
        }

        {
            uri_builder uri(address);
            uri.append_path(U("v1/request"));
            auto hl = new http_listener(uri.to_uri().to_string());
            hl->support(web::http::methods::POST, [=](web::http::http_request request)
            {
                ucout << request.to_string() << std::endl;

                request.extract_string(true).then([request](utility::string_t body)
                {
                    ucout << U("[BODY]") << std::endl;
                    ucout << body << std::endl;
                    request.reply(status_codes::OK, U("{ result: 0 }"), U("application/json"));
                });
            });

            m_listeners.emplace_back(hl);
        }

        for (auto eachListner : m_listeners)
        {
            eachListner->open().wait();
        }
    }

    void OnFinalize()
    {
        for (auto eachListner : m_listeners)
        {
            eachListner->close().wait();
            delete eachListner;
        }
    }

private:
    string_t m_address;
    std::vector<http_listener *> m_listeners;
};

static Global g_Global;

int _tmain(int argc, _TCHAR* argv[])
{
    string_t address = U("http://localhost:");
    string_t port = U("8080");

    address.append(port);

    ucout << U("Server is Running... ") << address << std::endl;
    ucout << U("Press ENTER to exit.") << std::endl;

    g_Global.OnInitialize(address);

    std::string line;
    std::getline(std::cin, line);

    g_Global.OnFinalize();

    return 0;
}