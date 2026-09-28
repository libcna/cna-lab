#include "LevelProfiles.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "System/IO/File.hpp"

using namespace Backrooms;

namespace {
void Check(bool passed,const char* description) {
    if (passed) return;
    std::cerr << "level-profile test failed: " << description << '\n';
    std::exit(EXIT_FAILURE);
}

void RejectChanged(const std::string& source,const std::string& before,
                   const std::string& after) {
    std::string changed=source;
    const auto position=changed.find(before);
    Check(position!=std::string::npos,"mutation exists in fixture");
    changed.replace(position,before.size(),after);
    try { ParseLevelCatalog(changed); }
    catch (const std::exception&) { return; }
    Check(false,("accepted invalid field: "+after).c_str());
}
}

int main(int argc,char** argv) {
    try {
        const std::filesystem::path path=argc>1 ? argv[1] : "assets/levels.json";
        const auto source=System::IO::File::ReadAllText(path.string());
        const auto parsed=ParseLevelCatalog(source);
        Check(parsed.levels==DefaultLevelCatalog().levels,"packaged profiles match built-in defaults");
        Check(parsed.sourceHash!=0 && parsed.sourceHash==ParseLevelCatalog(source).sourceHash,
              "source fingerprint is reproducible");
        Check(LoadLevelCatalog(path).levels==parsed.levels,"native file-loading path");
        Check(LoadLevelCatalog("/tmp/cna-backrooms-missing-level-profile.json").levels==
              DefaultLevelCatalog().levels,"missing-file fallback");
        for (int level=0;level<3;++level) {
            const WorldConfig builtIn{31337,level},configured{31337,level,nullptr,&parsed};
            for (int x=-30;x<30;++x) for (int z=-30;z<30;++z) {
                Check(RegionAt(builtIn,x,z)==RegionAt(configured,x,z),"default region recipe unchanged");
                Check(VerticalEdge(builtIn,x,z)==VerticalEdge(configured,x,z),"default geometry unchanged");
            }
        }
        auto custom=parsed;
        custom.levels[0].regions={{{RegionKind::OpenOffice,100}}};
        const WorldConfig modified{31337,0,nullptr,&custom};
        Check(RegionAt(modified,37,-31)==RegionKind::OpenOffice,"authored region weights affect generation");
        RejectChanged(source,"\"format\": 1","\"format\": 2");
        RejectChanged(source,"\"id\": 1","\"id\": 0");
        RejectChanged(source,"\"id\": 2","\"id\": 8");
        RejectChanged(source,"\"ceiling_height\": 2.75","\"ceiling_height\": 1.8");
        RejectChanged(source,"\"doorway_height\": 2.4","\"doorway_height\": 3.2");
        RejectChanged(source,"\"end\": 85","\"end\": 18");
        RejectChanged(source,"\"ambient\": 0.42","\"ambient\": 1.0");
        RejectChanged(source,"[255, 250, 214]","[256, 250, 214]");
        RejectChanged(source,"[255, 250, 214]","[255, 214]");
        RejectChanged(source,"\"weight\": 18","\"weight\": 17");
        RejectChanged(source,"\"kind\": \"columns\"","\"kind\": \"tunnels\"");
        RejectChanged(source,"\"kind\": \"columns\"","\"kind\": \"open_office\"");
        RejectChanged(source,"\"entity_rarity\": 850","\"entity_rarity\": 0");
        RejectChanged(source,"\"format\": 1","\"format\": 1.5");
        std::cout << "level-profile tests passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "level-profile tests: " << e.what() << '\n';
        return 1;
    }
}
