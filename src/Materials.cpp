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
        // A repeating, faded wallpaper motif: wavy vertical stems and broad
        // ornamental loops. Its contrast remains legible at corridor distance.
        constexpr float pi=3.14159265358979323846f;
        const float wave=2.7f*std::sin(y*2.0f*pi/64.0f);
        const float stem=std::abs(std::remainder(x+wave-16.0f,32.0f));
        const float localY=std::remainder(static_cast<float>(y),64.0f);
        const float loop=std::sqrt(std::pow(stem/10.5f,2.0f)+
                                   std::pow(localY/24.0f,2.0f));
        const int motif=(stem<1.7f ? -14 : stem<3.3f ? -6 : 0)+
                        (std::abs(loop-1.0f)<0.10f ? -8 : 0);
        const int panel=((x/16)&1) ? -2 : 2;
        const float stainNoise=SmoothNoise(x,y,32,51);
        const int stain=stainNoise>0.60f ?
            static_cast<int>((0.60f-stainNoise)*32.0f) : 0;
        r=211+grain/3+blotch/3+motif+panel+stain;
        g=198+grain/3+blotch/3+motif+panel+stain;
        b=133+grain/4+blotch/3+motif/2+panel+stain;
        break;
    }
    case Material::Carpet: {
        const int thread=((x+y)%4==0 ? 5 : 0)-
                         ((x-y+128)%7==0 ? 3 : 0);
        const int wear=static_cast<int>(std::lround(
            (SmoothNoise(x,y,32,62)-0.5f)*11.0f));
        const float dirtNoise=SmoothNoise(x,y,32,93);
        const int dirt=dirtNoise>0.68f ?
            static_cast<int>((0.68f-dirtNoise)*42.0f) : 0;
        r=139+grain/2+thread+wear+dirt;
        g=128+grain/2+thread+wear+dirt;
        b=100+grain/3+thread/2+wear+dirt;
        break;
    }
    case Material::CeilingTile: {
        const int seam=(x<3 || y<3) ? -64 :
                       (x<6 || y<6) ? -13 : 0;
        const int pores=Noise(x,y,108)%19==0 ? -13 :
                        Noise(x,y,109)%23==0 ? 5 : 0;
        r=209+grain/2+blotch/3+seam+pores;
        g=207+grain/2+blotch/3+seam+pores;
        b=187+grain/2+blotch/3+seam+pores;
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
        const float rustNoise=SmoothNoise(x,y,32,73);
        const int rust=rustNoise>0.62f ?
            static_cast<int>((rustNoise-0.62f)*38.0f) : 0;
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
