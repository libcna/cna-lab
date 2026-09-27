#include "Materials.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"

namespace Backrooms {
namespace {
using Microsoft::Xna::Framework::Color;
using Microsoft::Xna::Framework::Graphics::Texture2D;
using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
using Microsoft::Xna::Framework::Graphics::SurfaceFormat;

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
        // Small faded lozenges and pinstripes suggest old office wallpaper.
        // The 16-pixel motif is deliberately much smaller than a wall panel.
        const float localX=std::abs(std::remainder(x-8.0f,16.0f));
        const float localY=std::abs(std::remainder(y-8.0f,16.0f));
        const float diamond=localX/4.5f+localY/6.0f;
        const int motif=(localX<0.8f ? -5 : 0)+
                        (std::abs(diamond-1.0f)<0.17f ? -6 : 0)+
                        (localX<1.3f && localY<1.5f ? -4 : 0);
        const int panel=((x/16)&1) ? -1 : 1;
        const float stainNoise=SmoothNoise(x,y,32,51);
        const int stain=stainNoise>0.60f ?
            static_cast<int>((0.60f-stainNoise)*32.0f) : 0;
        r=211+grain/3+blotch/3+motif+panel+stain;
        g=200+grain/3+blotch/3+motif+panel+stain;
        b=138+grain/4+blotch/3+motif/2+panel+stain;
        break;
    }
    case Material::Carpet: {
        const auto fiberNoise=Noise(x,y,61);
        const int fiber=fiberNoise%7==0 ? 8 :
                        fiberNoise%19==0 ? -6 : 0;
        const int wear=static_cast<int>(std::lround(
            (SmoothNoise(x,y,32,62)-0.5f)*11.0f));
        const float dirtNoise=SmoothNoise(x,y,32,93);
        const int dirt=dirtNoise>0.57f ?
            static_cast<int>((0.57f-dirtNoise)*38.0f) : 0;
        r=139+grain/2+fiber+wear+dirt;
        g=128+grain/2+fiber+wear+dirt;
        b=100+grain/3+fiber/2+wear+dirt;
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
        textures_[id]=std::make_unique<Texture2D>(
            device,size,size,true,SurfaceFormat::Color);
        int width=size;
        for (int level=0;level<textures_[id]->getLevelCountProperty();++level) {
            textures_[id]->SetData(level,nullptr,pixels.data(),0,
                                   static_cast<int>(pixels.size()));
            if (width==1) break;
            const int nextWidth=width/2;
            std::vector<Color> next(nextWidth*nextWidth);
            for (int y=0;y<nextWidth;++y) for (int x=0;x<nextWidth;++x) {
                const auto& a=pixels[(2*y)*width+2*x];
                const auto& b=pixels[(2*y)*width+2*x+1];
                const auto& c=pixels[(2*y+1)*width+2*x];
                const auto& d=pixels[(2*y+1)*width+2*x+1];
                const auto average=[](int a,int b,int c,int d) {
                    return static_cast<std::uint8_t>((a+b+c+d+2)/4);
                };
                next[y*nextWidth+x]=Color(
                    average(a.getRProperty(),b.getRProperty(),
                            c.getRProperty(),d.getRProperty()),
                    average(a.getGProperty(),b.getGProperty(),
                            c.getGProperty(),d.getGProperty()),
                    average(a.getBProperty(),b.getBProperty(),
                            c.getBProperty(),d.getBProperty()));
            }
            pixels=std::move(next);
            width=nextWidth;
        }
    }
}

} // namespace Backrooms
