#include "Materials.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"

namespace Backrooms {
namespace {
using Microsoft::Xna::Framework::Color;
using Microsoft::Xna::Framework::Graphics::Texture2D;
using Microsoft::Xna::Framework::Graphics::GraphicsDevice;

std::uint32_t Noise(int x, int y, int salt) {
    std::uint32_t h=static_cast<std::uint32_t>(x)*0x9e3779b1U ^
                    static_cast<std::uint32_t>(y)*0x85ebca77U ^
                    static_cast<std::uint32_t>(salt)*0xc2b2ae3dU;
    h ^= h>>16; h *= 0x7feb352dU;
    h ^= h>>15; h *= 0x846ca68bU;
    return h ^ (h>>16);
}

Color Pixel(Material material, int x, int y) {
    const int id=static_cast<int>(material);
    const int grain=static_cast<int>(Noise(x,y,id)%17)-8;
    const int blotch=static_cast<int>(Noise(x/18,y/18,id+37)%15)-7;
    int r=0,g=0,b=0;
    switch (material) {
    case Material::Wallpaper: {
        const int stripe=(x%32==0 || x%32==1) ? -14 :
                         (x%32==3 || x%32==4) ? 7 : 0;
        const int stain=(Noise(x/13,y/20,51)%11==0) ? -13 : 0;
        r=206+grain/2+blotch+stripe+stain;
        g=193+grain/2+blotch+stripe+stain;
        b=127+grain/3+blotch+stripe+stain;
        break;
    }
    case Material::Carpet: {
        const int fiber=(x%3==0 && Noise(x,y,61)%3==0) ? 11 : 0;
        const int patch=static_cast<int>(Noise(x/11,y/11,62)%17)-8;
        r=103+grain+blotch+patch+fiber;
        g=98+grain+blotch+patch+fiber;
        b=78+grain+blotch+patch+fiber/2;
        break;
    }
    case Material::CeilingTile: {
        const int seam=(x<3 || y<3) ? -33 :
                       (x<5 || y<5) ? -11 : 0;
        r=203+grain+blotch+seam;
        g=201+grain+blotch+seam;
        b=182+grain+blotch+seam;
        break;
    }
    case Material::ConcreteWall: {
        const int seam=(y%64==0 || x%64==0) ? -18 : 0;
        r=117+grain+blotch+seam;
        g=121+grain+blotch+seam;
        b=118+grain+blotch+seam;
        break;
    }
    case Material::ConcreteFloor:
        r=83+grain+blotch; g=87+grain+blotch; b=85+grain+blotch; break;
    case Material::IndustrialCeiling:
        r=100+grain/2+blotch; g=109+grain/2+blotch; b=106+grain/2+blotch; break;
    case Material::TunnelWall: {
        const int rust=(Noise(x/9,y/12,73)%13==0) ? 20 : 0;
        r=101+grain+blotch+rust;
        g=89+grain+blotch-rust/3;
        b=74+grain+blotch-rust/2;
        break;
    }
    case Material::TunnelFloor:
        r=67+grain+blotch; g=68+grain+blotch; b=63+grain+blotch; break;
    case Material::TunnelCeiling:
        r=77+grain+blotch; g=75+grain+blotch; b=69+grain+blotch; break;
    case Material::Fluorescent: {
        const int edge=(x<5 || x>122 || y<5 || y>122) ? -30 : 0;
        r=247+grain/4+edge; g=242+grain/4+edge;
        b=204+grain/4+edge; break;
    }
    case Material::Count: break;
    }
    return Color(static_cast<std::uint8_t>(std::clamp(r,0,255)),
                 static_cast<std::uint8_t>(std::clamp(g,0,255)),
                 static_cast<std::uint8_t>(std::clamp(b,0,255)));
}
}

Materials::Materials(GraphicsDevice& device) {
    constexpr int size=128;
    for (int id=0;id<kMaterialCount;++id) {
        const Material material=static_cast<Material>(id);
        std::vector<Color> pixels;
        pixels.reserve(size*size);
        for (int y=0;y<size;++y) for (int x=0;x<size;++x)
            pixels.push_back(Pixel(material,x,y));
        textures_[id]=std::make_unique<Texture2D>(device,size,size);
        textures_[id]->SetData(pixels.data(),static_cast<int>(pixels.size()));
    }
}

} // namespace Backrooms
