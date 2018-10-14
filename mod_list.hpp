#pragma once

#ifndef Q_MOC_RUN // A Qt workaround, for those of you who use Qt
#   include <SimpleJSON/stringify/jss_fusion_adapted_struct.hpp>
#   include <SimpleJSON/parse/jsd_fusion_adapted_struct.hpp>
#endif

#include <string>
#include <vector>
#include <optional>
#include <filesystem>

struct FileDescription : public JSON::Stringifiable <FileDescription>
                       , public JSON::Parsable <FileDescription>
{
    std::string href;
    std::string name;
    std::string size;
    std::string mcVersion;
    long long timeEpoch;
    std::string timeHuman;

    std::pair <std::string, std::string> download() const;
    void download(std::filesystem::path const& to);
};

struct Mod : public JSON::Stringifiable <Mod>
           , public JSON::Parsable <Mod>
{
    std::string id;
    std::optional <std::string> mcVersion;
    std::optional <std::string> name;
    std::optional <std::string> installedName;
    std::optional <long long> newestInstalled;

    bool isNewer(long long epoch);
    void readName();
    std::string getProjectUrl() const;
    std::string getProjectUrlFiles() const;
    std::vector <FileDescription> getFiles(int page = 1) const;
};

struct ModList : public JSON::Stringifiable <ModList>
               , public JSON::Parsable <ModList>
{
    std::vector <Mod> mods;

    void load(std::filesystem::path const& path);
    void save(std::filesystem::path const& path) const;
};

BOOST_FUSION_ADAPT_STRUCT
(
    FileDescription,
    href, name, size, timeEpoch, timeHuman
)

BOOST_FUSION_ADAPT_STRUCT
(
    Mod,
    id, name, installedName, newestInstalled, mcVersion
)

BOOST_FUSION_ADAPT_STRUCT
(
    ModList,
    mods
)
