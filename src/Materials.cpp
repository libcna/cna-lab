#include "Materials.hpp"

#include <algorithm>
#include <cmath>
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

float SmoothNoise(int x, int y, int cellSize, int salt) {
    const int count=128/cellSize;
    const int gx=x/cellSize,gy=y/cellSize;
    const float fx=static_cast<float>(x%cellSize)/cellSize;
    const float fy=static_cast<float>(y%cellSize)/cellSize;
    const auto value=[&](int ix,int iy) {
        return static_cast<float>(Noise(ix%count,iy%count,salt)%1024)/1023.0f;
    };
    const float a=value(gx,gy)*(1-fx)+value(gx+1,gy)*fx;
    const float b=value(gx,gy+1)*(1-fx)+value(gx+1,gy+1)*fx;
    return a*(1-fy)+b*fy;
}

Color Pixel(Material material, int x, int y) {
    const int id=static_cast<int>(material);
    const int grain=static_cast<int>(Noise(x,y,id)%17)-8;
    const int blotch=static_cast<int>(std::lround(
        (SmoothNoise(x,y,16,id+37)-0.5f)*18.0f));
    int r=0,g=0,b=0;
    switch (material) {
    case Material::Wallpaper: {
        const int row=y%64;
        const int bend=(row<32 ? row : 64-row)/4;
        const int stripeDistance=std::abs((x+bend)%32-16);
        const int stripe=stripeDistance<2 ? -6 :
                         stripeDistance<4 ? 2 : 0;
        const float stainNoise=SmoothNoise(x,y,16,51);
        const int stain=stainNoise>0.64f ?
            static_cast<int>((0.64f-stainNoise)*42.0f) : 0;
        r=206+grain/2+blotch/2+stripe+stain;
        g=193+grain/2+blotch/2+stripe+stain;
        b=127+grain/3+blotch/2+stripe+stain;
        break;
    }
    case Material::Carpet: {
        const int fiber=(x%3==0 && Noise(x,y,61)%3==0) ? 11 : 0;
        const int patch=static_cast<int>(std::lround(
            (SmoothNoise(x,y,16,62)-0.5f)*22.0f));
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
        const float rustNoise=SmoothNoise(x,y,16,73);
        const int rust=rustNoise>0.62f ?
            static_cast<int>((rustNoise-0.62f)*60.0f) : 0;
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
