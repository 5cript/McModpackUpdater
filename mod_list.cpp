#include "mod_list.hpp"
#include "download.hpp"

#include <SimpleJSON/stringify/jss.hpp>
#include <SimpleJSON/parse/jsd.hpp>
#include <SimpleJSON/parse/jsd_convenience.hpp>
#include <SimpleJSON/utility/beauty_stream.hpp>
#include <SimpleJSON/utility/fill_missing.hpp>

#include <curl/curl.h>
#include <attendee/attendee.hpp>
#include <fstream>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <iostream>

using namespace std::string_literals;

//#####################################################################################################################
std::pair <std::string, std::string> FileDescription::download() const
{
    auto url = "https://minecraft.curseforge.com" + href + "/download";

    int code;
    std::string content;
    std::string redirect = url;
    do {
        auto [result, codeIn, redirectIn, contentStr] = ::followRedirect(redirect);
        code = codeIn;

        if (result != 0)
            throw std::runtime_error(url + " could not be downloaded: " + curl_easy_strerror(static_cast <CURLcode>(result)));

        if (code == 200)
            content = std::move(contentStr);
        else
            redirect = std::move(redirectIn);
    } while (code >= 300 && code < 400);
    return {content, redirect};
}
//---------------------------------------------------------------------------------------------------------------------
void FileDescription::download(std::filesystem::path const& to)
{
    auto [content, href] = download();
    auto pos = href.find_last_of('/');

    name = std::string{std::begin(href) + pos + 1, std::end(href)};
    name = attendee::response::url_decode(name);
    std::ofstream writer{to / name, std::ios_base::binary};
    writer << content;
}
//#####################################################################################################################
void Mod::readName()
{
    std::string href = "https://www.curseforge.com/minecraft/mc-mods/" + id;
    auto [result, content] = download(href);

    if (result != 0)
        throw std::runtime_error(href + " could not be downloaded: " + curl_easy_strerror(static_cast <CURLcode>(result)));

    auto getName = [&]()
    {
        auto p1 = content.find("<div class=\"project-header__details\">");
        auto p2 = content.find("<h2 class=\"name\">", p1) + 17;
        auto p3 = content.find("</h2>", p2);
        return std::string{std::begin(content) + p2, std::begin(content) + p3};
    };

    name = getName();
}
//---------------------------------------------------------------------------------------------------------------------
std::string Mod::getProjectUrl() const
{
    std::string href = "https://www.curseforge.com/minecraft/mc-mods/" + id;
    auto [result, content] = download(href);

    if (result != 0)
        throw std::runtime_error(href + " could not be downloaded: " + curl_easy_strerror(static_cast <CURLcode>(result)));

    auto pos = content.find("<p class=\"infobox__cta\">");
    pos = content.find("<a href=\"", pos) + 9;
    auto pos2 = content.find("\"", pos);

    return std::string{std::begin(content) + pos, std::begin(content) + pos2};
}
//---------------------------------------------------------------------------------------------------------------------
std::string Mod::getProjectUrlFiles() const
{
    return getProjectUrl() + "/files";
}
//---------------------------------------------------------------------------------------------------------------------
bool Mod::isNewer(long long epoch)
{
    if (!newestInstalled)
        return true;
    else
        return newestInstalled.value() < epoch;
}
//---------------------------------------------------------------------------------------------------------------------
std::vector <FileDescription> Mod::getFiles(int page) const
{
    std::string url = getProjectUrlFiles();
    if (page > 1)
        url += "?page="s + std::to_string(page);

    auto [result, content] = download(url);

    if (result != 0)
        throw std::runtime_error(url + " could not be downloaded: " + curl_easy_strerror(static_cast <CURLcode>(result)));

    auto extract = [&content](int trPos, std::string term, char const* end)
    {
        auto pos = content.find(term, trPos) + term.length();
        if (pos == std::string::npos)
            return std::string{""};
        return std::string{std::begin(content) + pos, std::begin(content) + content.find(end, pos)};
    };

    auto tbody = content.find("<tbody>");
    std::vector <FileDescription> list;
    for (int trPos = content.find("<tr class=\"project-file-list-item\">", tbody); static_cast <std::size_t> (trPos) != std::string::npos; ++trPos)
    {
        FileDescription descr;
        descr.href = extract(trPos, R"(<a class="overflow-tip twitch-link" href=")", "\"");
        //descr.name = extract(trPos, R"(data-name=")", "\"");
        descr.size = extract(trPos, R"(project-file-size">)", "<");
        descr.timeEpoch = std::stoll(extract(trPos, R"(data-epoch=")", "\""));
        descr.timeHuman = extract(trPos, R"(time-processed=")", "\"");
        descr.mcVersion = extract(trPos, R"(<span class="version-label">)", "<");

        list.push_back(descr);

        // next
        trPos = content.find("<tr class=\"project-file-list-item\">", trPos + 1);
        if (trPos == 0 || static_cast <std::size_t> (trPos) == std::string::npos)
            break;
    }
    return list;
}
//#####################################################################################################################
void ModList::load(std::filesystem::path const& path)
{
    auto stream = std::ifstream{path.string(), std::ios_base::binary};
    auto tree = JSON::parse_json(stream);
    JSON::fill_missing <decltype(*this)> ("", tree);
    JSON::parse(*this, "", tree);
}
//---------------------------------------------------------------------------------------------------------------------
void ModList::save(std::filesystem::path const& path) const
{
    namespace io = boost::iostreams;
    io::filtering_ostream out;
    out.push(JSON::BeautifiedStreamWrapper{});
    out.push(io::file_sink(path.string()));
    JSON::stringify(out, "modlist", *this);
}
//#####################################################################################################################
