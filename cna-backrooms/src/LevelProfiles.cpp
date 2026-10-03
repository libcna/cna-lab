#include "LevelProfiles.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "System/IO/File.hpp"
#include "System/Text/Json/JsonDocument.hpp"

namespace Backrooms {
namespace {
using System::Text::Json::JsonElement;

float Number(const JsonElement& object,const char* name,double minimum,double maximum) {
    const double value=object.GetProperty(name).GetDouble();
    if (!std::isfinite(value) || value<minimum || value>maximum)
        throw std::invalid_argument(std::string(name)+" is outside the supported range");
    return static_cast<float>(value);
}

LevelDefinition::Rgb Color(const JsonElement& object,const char* name) {
    const auto channels=object.GetProperty(name).EnumerateArray();
    if (channels.size()!=3)
        throw std::invalid_argument(std::string(name)+" must have three RGB channels");
    LevelDefinition::Rgb result;
    for (int i=0;i<3;++i) {
        const int value=channels[i].GetInt32();
        if (value<0 || value>255)
            throw std::invalid_argument(std::string(name)+" RGB channel is outside 0..255");
        result[i]=static_cast<std::uint8_t>(value);
    }
    return result;
}

RegionKind Kind(const std::string& name,int level) {
    constexpr std::array<std::string_view,7> names{
        "open_office","columns","rooms","halls","irregular","storage","tunnels"};
    for (int i=0;i<7;++i) if (name==names[i]) {
        const bool allowed=level==0 ? i<=4 : level==1 ? i!=4 && i!=6 : i==3 || i==4 || i==6;
        if (!allowed) throw std::invalid_argument("room kind does not belong to this level family: "+name);
        return static_cast<RegionKind>(i);
    }
    throw std::invalid_argument("unknown room kind: "+name);
}
}

LevelCatalog ParseLevelCatalog(const std::string& json) {
    if (json.size()>65536) throw std::invalid_argument("level definitions exceed 64 KiB");
    const auto document=System::Text::Json::JsonDocument::Parse(json);
    const auto root=document->getRootElementProperty();
    if (root.GetProperty("format").GetInt32()!=kLevelProfileVersion)
        throw std::invalid_argument("unsupported level-definition format");
    const auto entries=root.GetProperty("levels").EnumerateArray();
    if (entries.size()!=3) throw std::invalid_argument("level definitions must contain exactly three families");
    LevelCatalog result;
    std::array<bool,3> seen{};
    for (const auto& entry:entries) {
        const int id=entry.GetProperty("id").GetInt32();
        if (id<0 || id>2 || seen[id]) throw std::invalid_argument("duplicate or invalid level id");
        seen[id]=true;
        auto& level=result.levels[id];
        level.name=entry.GetProperty("name").GetString();
        if (level.name.empty() || level.name.size()>80)
            throw std::invalid_argument("level name must contain 1..80 bytes");
        level.ceilingHeight=Number(entry,"ceiling_height",2.4,5);
        level.doorwayHeight=Number(entry,"doorway_height",1.95,4.5);
        if (level.doorwayHeight>level.ceilingHeight-0.1f)
            throw std::invalid_argument("doorway height must leave a ceiling lintel");
        const auto fog=entry.GetProperty("fog");
        level.fogStart=Number(fog,"start",0,80);
        level.fogEnd=Number(fog,"end",10,100);
        if (level.fogEnd<level.fogStart+5)
            throw std::invalid_argument("fog end must exceed start by at least five metres");
        level.fog=Color(fog,"color");
        const auto light=entry.GetProperty("lighting");
        level.ambient=Number(light,"ambient",0.15,0.85);
        level.lightStrength=Number(light,"fluorescent_strength",0.1,0.9);
        level.wallBounce=Number(light,"wall_bounce",0,0.6);
        level.ceilingBounce=Number(light,"ceiling_bounce",0,0.8);
        const int rarity=entry.GetProperty("entity_rarity").GetInt32();
        if (rarity<250 || rarity>100000)
            throw std::invalid_argument("entity rarity must be between 250 and 100000");
        level.entityRarity=static_cast<unsigned>(rarity);
        const auto colors=entry.GetProperty("tints");
        level.wall=Color(colors,"wall");level.pillar=Color(colors,"pillar");
        level.trim=Color(colors,"trim");level.floor=Color(colors,"floor");
        level.ceiling=Color(colors,"ceiling");level.structure=Color(colors,"structure");
        level.fluorescent=Color(colors,"fluorescent");
        const auto rooms=entry.GetProperty("regions").EnumerateArray();
        if (rooms.empty() || rooms.size()>7)
            throw std::invalid_argument("region weights must contain 1..7 room kinds");
        std::array<bool,7> used{};
        int total=0;
        for (std::size_t i=0;i<rooms.size();++i) {
            const auto kind=Kind(rooms[i].GetProperty("kind").GetString(),id);
            const int index=static_cast<int>(kind);
            const int weight=rooms[i].GetProperty("weight").GetInt32();
            if (used[index] || weight<0 || weight>100)
                throw std::invalid_argument("duplicate room kind or invalid weight");
            used[index]=true;total+=weight;
            level.regions[i]={kind,weight};
        }
        if (total!=100) throw std::invalid_argument("region weights must total 100");
    }
    // Source bytes are part of the reproducibility record. This is a compact
    // diagnostic fingerprint, not a cryptographic integrity guarantee.
    result.sourceHash=14695981039346656037ULL;
    for (const unsigned char byte:json) {
        result.sourceHash^=byte;result.sourceHash*=1099511628211ULL;
    }
    return result;
}

LevelCatalog LoadLevelCatalog(const std::filesystem::path& path) {
    if (!System::IO::File::Exists(path.string())) {
        std::clog << "[levels] " << path << " unavailable; using built-in defaults\n";
        return DefaultLevelCatalog();
    }
    try {
        auto result=ParseLevelCatalog(System::IO::File::ReadAllText(path.string()));
        std::clog << "[levels] format " << kLevelProfileVersion << ", world "
                  << kFormatVersion << ", source " << std::hex << result.sourceHash
                  << std::dec << " from " << path << '\n';
        return result;
    } catch (const std::exception& e) {
        throw std::invalid_argument("invalid level definitions in "+path.string()+": "+e.what());
    }
}
}
