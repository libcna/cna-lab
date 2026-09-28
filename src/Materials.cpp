#include "Materials.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "System/IO/File.hpp"
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

float SmoothNoise(int x, int y, int cellSize, int salt, int size) {
    const int count=size/cellSize;
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

Color Pixel(Material material, int x, int y, int size) {
    const int id=static_cast<int>(material);
    const int grain=static_cast<int>(Noise(x,y,id)%17)-8;
    const int blotch=static_cast<int>(std::lround(
        (SmoothNoise(x,y,16,id+37,size)-0.5f)*18.0f));
    int r=0,g=0,b=0;
    switch (material) {
    case Material::Wallpaper: {
        // Small faded lozenges and pinstripes suggest old office wallpaper.
        // This fallback matches the approximate scale of the optional bitmap.
        const float localX=std::abs(std::remainder(x-32.0f,64.0f)/4.0f);
        const float localY=std::abs(std::remainder(y-32.0f,64.0f)/4.0f);
        const float diamond=localX/4.5f+localY/6.0f;
        const int motif=(localX<0.8f ? -5 : 0)+
                        (std::abs(diamond-1.0f)<0.17f ? -6 : 0)+
                        (localX<1.3f && localY<1.5f ? -4 : 0);
        const int panel=((x/16)&1) ? -1 : 1;
        const float stainNoise=SmoothNoise(x,y,128,51,size);
        const int stain=stainNoise>0.55f ?
            static_cast<int>((0.55f-stainNoise)*31.0f) : 0;
        const float streak=SmoothNoise(x,0,64,137,size);
        const int waterline=streak>0.58f && y>size*3/4 ?
            static_cast<int>((0.58f-streak)*15.0f) : 0;
        r=211+grain/3+blotch/3+motif+panel+stain+waterline;
        g=200+grain/3+blotch/3+motif+panel+stain+waterline;
        b=138+grain/4+blotch/3+motif/2+panel+stain+waterline;
        break;
    }
    case Material::Carpet: {
        const auto fiberNoise=Noise(x,y,61);
        const int fiber=fiberNoise%5==0 ? 9 :
                        fiberNoise%13==0 ? -7 : 0;
        const int tuft=static_cast<int>(Noise(x/2,y/2,163)%7)-3;
        const int wear=static_cast<int>(std::lround(
            (SmoothNoise(x,y,size/4,62,size)-0.5f)*9.0f));
        const float dirtNoise=SmoothNoise(x,y,size/4,93,size);
        const int dirt=dirtNoise>0.57f ?
            static_cast<int>((0.57f-dirtNoise)*45.0f) : 0;
        r=173+grain/2+fiber+tuft+wear+dirt;
        g=160+grain/2+fiber+tuft+wear+dirt;
        b=122+grain/3+fiber/2+tuft+wear+dirt;
        break;
    }
    case Material::CeilingTile: {
        constexpr int tilePixels=128;
        const int localX=x%tilePixels,localY=y%tilePixels;
        const int seam=(localX<2 || localY<2) ? 6 :
                       (localX<5 || localY<5) ? -12 : 0;
        const auto panel=Noise(x/tilePixels,y/tilePixels,193);
        const int age=static_cast<int>(panel%7)-3;
        const int yellowing=panel%11==0 ? 5 : 0;
        const int pores=Noise(x,y,108)%19==0 ? -13 :
                        Noise(x,y,109)%23==0 ? 5 : 0;
        r=209+grain/2+blotch/3+seam+pores+age;
        g=207+grain/2+blotch/3+seam+pores+age-yellowing/2;
        b=187+grain/2+blotch/3+seam+pores+age-yellowing;
        break;
    }
    case Material::ConcreteWall: {
        const int seam=(y%64==0 || x%64==0) ? -18 : 0;
        r=117+grain+blotch+seam;
        g=121+grain+blotch+seam;
        b=118+grain+blotch+seam;
        break;
    }
    case Material::ConcreteCeiling: {
        const int mottling=static_cast<int>(std::lround(
            (SmoothNoise(x,y,128,271,size)-0.5f)*9));
        const int pore=Noise(x,y,273)%71==0 ? -15 : 0;
        const int formwork=(x%256<2 || y%256<2) ? -7 : 0;
        r=161+grain/2+mottling+pore+formwork;
        g=159+grain/2+mottling+pore+formwork;
        b=148+grain/2+mottling+pore+formwork;
        break;
    }
    case Material::ConcreteFloor:
        r=83+grain+blotch; g=87+grain+blotch; b=85+grain+blotch; break;
    case Material::GalvanizedMetal: {
        const int age=static_cast<int>(std::lround(
            (SmoothNoise(x,y,16,311,size)-0.5f)*7));
        const int pit=Noise(x,y,313)%47==0 ? -12 : 0;
        r=176+grain/3+age+pit;g=179+grain/3+age+pit;b=170+grain/3+age+pit;
        break;
    }
    case Material::IndustrialCeiling:
        r=100+grain/2+blotch; g=109+grain/2+blotch; b=106+grain/2+blotch; break;
    case Material::TunnelWall: {
        const float rustNoise=SmoothNoise(x,y,size/4,73,size);
        const int rust=rustNoise>0.62f ?
            static_cast<int>((rustNoise-0.62f)*52.0f) : 0;
        r=149+grain/2+blotch/3+rust;
        g=145+grain/2+blotch/3-rust/2;
        b=132+grain/2+blotch/3-rust/2;
        break;
    }
    case Material::TunnelFloor:
        r=77+grain+blotch; g=78+grain+blotch; b=72+grain+blotch; break;
    case Material::TunnelCeiling:
        r=86+grain+blotch; g=83+grain+blotch; b=76+grain+blotch; break;
    case Material::Wood: {
        const int grainLine=static_cast<int>(std::lround(
            5.0f*std::sin(y*0.39f+SmoothNoise(x,y,16,140,size)*2.0f)));
        r=172+grain/2+blotch/2+grainLine;
        g=138+grain/2+blotch/2+grainLine;
        b=93+grain/3+blotch/2+grainLine;
        break;
    }
    case Material::Cardboard: {
        const int tape=(x>=54 && x<=72) ? 15 : 0;
        const int edge=(x<3 || y<3 || x>124 || y>124) ? -12 : 0;
        r=173+grain/3+blotch/2+tape+edge;
        g=150+grain/3+blotch/2+tape+edge;
        b=109+grain/3+blotch/2+tape+edge;
        break;
    }
    case Material::PaintedTrim:
        r=224+grain/3;g=219+grain/3;b=197+grain/3;break;
    case Material::Upholstery: {
        const int weave=((x+y)%2==0 ? 3 : -2);
        const int worn=static_cast<int>(std::lround(
            (SmoothNoise(x,y,32,184,size)-0.5f)*12));
        r=158+grain/3+weave+worn;
        g=143+grain/3+weave+worn;
        b=108+grain/3+weave+worn;
        break;
    }
    case Material::Fluorescent: {
        const int edge=(x<5 || x>122 || y<5 || y>122) ? -30 : 0;
        r=250+grain/4+edge; g=249+grain/4+edge;
        b=229+grain/4+edge; break;
    }
    case Material::Count: break;
    }
    return Color(static_cast<std::uint8_t>(std::clamp(r,0,255)),
                 static_cast<std::uint8_t>(std::clamp(g,0,255)),
                 static_cast<std::uint8_t>(std::clamp(b,0,255)));
}
}

Materials::Materials(GraphicsDevice& device,const std::filesystem::path& assetDirectory) {
    for (int id=0;id<kMaterialCount;++id) {
        const Material material=static_cast<Material>(id);
        int size=material==Material::Carpet ? 1024 :
                 (material==Material::Wallpaper || material==Material::CeilingTile ||
                  material==Material::ConcreteCeiling || material==Material::TunnelWall) ? 512 : 128;
        std::vector<Color> pixels;
        const auto wallpaperPath=assetDirectory/"wallpaper-v1.png";
        if (material==Material::Wallpaper && System::IO::File::Exists(wallpaperPath.string())) {
            try {
                auto stream=System::IO::File::OpenRead(wallpaperPath.string());
                auto decoded=Texture2D::FromStream(device,stream,1024,1024,true);
                pixels.resize(1024*1024);
                decoded.GetData(pixels.data(),static_cast<int>(pixels.size()));
                size=1024;
                std::cerr << "Material ready: wallpaper loaded from " << wallpaperPath << '\n';
            } catch (const std::exception& error) {
                pixels.clear();
                std::cerr << "Material: wallpaper decode failed; using procedural fallback: "
                          << error.what() << '\n';
            }
        }
        if (pixels.empty()) {
            if (material==Material::Wallpaper)
                std::cerr << "Material: using procedural wallpaper fallback\n";
            pixels.reserve(size*size);
            for (int y=0;y<size;++y) for (int x=0;x<size;++x)
                pixels.push_back(Pixel(material,x,y,size));
        }
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
