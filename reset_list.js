var fs = require("fs");
var content = JSON.parse(fs.readFileSync("modlist.json"));

for (mod in content.mods)
{
	delete content.mods[mod].installedName;
	delete content.mods[mod].newestInstalled;
}

fs.writeFileSync("modlist.json", JSON.stringify(content, null, "\t"));