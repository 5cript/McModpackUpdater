#include "download.hpp"

#include <attendee/attendee.hpp>

//#####################################################################################################################
std::pair <int, std::string> download(std::string const& link)
{
    attendee::request request;
    std::string content;
    auto response = request
        .get(link)
        .sink(content)
        .perform()
    ;

    auto result = response.result();
    return std::pair{result, content};
}
//---------------------------------------------------------------------------------------------------------------------
std::tuple <int, int, std::string, std::string> followRedirect(std::string const& link)
{
    attendee::request request;
    std::string content;
    auto response = request
        .get(link)
        .sink(content)
        .perform()
    ;

    auto result = response.result();
    return {result, response.code(), response.redirect_url(), content};
}
//#####################################################################################################################
