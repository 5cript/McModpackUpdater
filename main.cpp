#include "mod_list.hpp"

#include <iostream>
#include <filesystem>

using namespace std::string_literals;

int main()
{
    ModList list;
    list.load("modlist.json");

    std::cout << "Enter command:\n";
    std::cout << "\tu = update\n";
    std::cout << "\tl = list new\n";
    std::cout << "\tq = quit\n";

    char command;
    std::cin >> command;
    if (command == 'q')
        return 0;

    if (command == 'u')
    {
        std::filesystem::create_directory("mods_old");
    }

    int i = 1;
    for (auto& mod: list.mods)
    {
        auto files = mod.getFiles();
        if (files.empty())
        {
            std::cout << "WARNING! no files found for " << mod.id;
            continue;
        }

        std::cout << i << "/" << list.mods.size() << ") ";
        auto newest = files.front();
        if (mod.isNewer(newest.timeEpoch))
        {
            if (mod.mcVersion && mod.mcVersion.value() != newest.mcVersion)
            {
                bool foundOne = false;
                for (auto const file : files)
                {
                    if (file.mcVersion == mod.mcVersion.value())
                    {
                        foundOne = true;
                        newest = file;
                        break;
                    }
                }
                if (!foundOne)
                {
                    std::cout << mod.id << " - ERROR! MC version switch needs manual intervention.";
                    continue;
                }
            }
            if (command == 'l')
            {
                std::cout << newest.name;
            }
            else if (command == 'u')
            {
                if (!mod.name)
                    mod.readName();
                if (mod.installedName)
                    std::filesystem::rename("mods/"s + mod.installedName.value(), "mods_old/"s + mod.installedName.value());
                newest.download("mods");
                std::cout << mod.name.value() << " updated";
                mod.installedName = newest.name;
                mod.newestInstalled = newest.timeEpoch;
                mod.mcVersion = newest.mcVersion;
            }
        }
        else
        {
            if (mod.name)
                std::cout << mod.name.value() << " up to date";
            else
                std::cout << mod.id << " up to date";
        }
        std::cout << "\n";
        ++i;
    }

    if (command == 'u')
    {
        list.save("modlist.json");
    }

    return 0;
}
