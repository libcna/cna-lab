#include "BackroomsGame.hpp"
#include "Assets.hpp"
#include "LevelProfiles.hpp"
#include "Lighting.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <tuple>
#include <vector>

#include "CNA/Internal/Renderers/Common/IGraphicsRenderer.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/PreparingDeviceSettingsEventArgs.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPassCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerStateCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColorTexture.hpp"
#include "Microsoft/Xna/Framework/Input/ButtonState.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"

namespace Backrooms {
using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace {
using Clock = std::chrono::steady_clock;
using Mesh = std::vector<VertexPositionColorTexture>;
using Meshes = std::array<Mesh,kMaterialCount>;
constexpr float kCarpetRepeat=0.5f; // two-metre loop-pile bitmap footprint

Color FromRgb(const LevelDefinition::Rgb& value) {
    return Color(value[0],value[1],value[2]);
}

void QuadColors(Meshes& meshes, Material material, Vector3 a, Vector3 b,
                Vector3 c, Vector3 d, const std::array<Color,4>& colors,
                Vector2 uvA, Vector2 uvB, Vector2 uvC, Vector2 uvD) {
    auto& mesh=meshes[static_cast<int>(material)];
    mesh.emplace_back(a,colors[0],uvA); mesh.emplace_back(b,colors[1],uvB);
    mesh.emplace_back(c,colors[2],uvC);
    mesh.emplace_back(a,colors[0],uvA); mesh.emplace_back(c,colors[2],uvC);
    mesh.emplace_back(d,colors[3],uvD);
}

void Quad(Meshes& meshes, Material material, Vector3 a, Vector3 b, Vector3 c,
          Vector3 d, Color color, Vector2 uvA, Vector2 uvB, Vector2 uvC,
          Vector2 uvD) {
    QuadColors(meshes,material,a,b,c,d,{color,color,color,color},
               uvA,uvB,uvC,uvD);
}

void Flat(Meshes& meshes, Material material, float x0, float z0, float x1,
          float z1, float y, Color color, float repeat) {
    Quad(meshes,material,{x0,y,z0},{x1,y,z0},{x1,y,z1},{x0,y,z1},color,
         {x0*repeat,z0*repeat},{x1*repeat,z0*repeat},
         {x1*repeat,z1*repeat},{x0*repeat,z1*repeat});
}

void FlatShaded(Meshes& meshes, Material material, float x0, float z0,
                float x1, float z1, float y,
                const std::array<Color,4>& colors, float repeat) {
    QuadColors(meshes,material,{x0,y,z0},{x1,y,z0},{x1,y,z1},
               {x0,y,z1},colors,
               {x0*repeat,z0*repeat},{x1*repeat,z0*repeat},
               {x1*repeat,z1*repeat},{x0*repeat,z1*repeat});
}

void LightPanel(Meshes& meshes,float x0,float z0,float x1,float z1,
                float y,Color color,bool longAxisX) {
    // Map the entire diffuser once. World-aligned UVs wrapped the dark texture
    // edge through the face of fixtures and their former tube overlays.
    Quad(meshes,Material::Fluorescent,{x0,y,z0},{x1,y,z0},
         {x1,y,z1},{x0,y,z1},color,{0,0},
         longAxisX ? Vector2(1,0) : Vector2(0,1),{1,1},
         longAxisX ? Vector2(0,1) : Vector2(1,0));
}

Color Scale(Color color, float factor) {
    const auto scaled=[&](int channel) {
        return static_cast<std::uint8_t>(std::clamp(
            static_cast<int>(std::lround(channel*factor)),0,255));
    };
    return Color(scaled(color.getRProperty()),scaled(color.getGProperty()),
                 scaled(color.getBProperty()));
}

void WallFaceColors(Meshes& meshes, Material material, bool vertical, float boundary,
                    float a, float b, float y0, float y1,
                    const std::array<Color,4>& colors) {
    const float repeat=material==Material::Wallpaper ? 0.8f :
                       (material==Material::TunnelWall ||
                        material==Material::ConcreteWall) ? 0.5f : 0.6f;
    const float u0=a*repeat,u1=b*repeat;
    const float v0=1.0f-y0*repeat,v1=1.0f-y1*repeat;
    if (vertical)
        QuadColors(meshes,material,{boundary,y0,a},{boundary,y0,b},
                   {boundary,y1,b},{boundary,y1,a},colors,
                   {u0,v0},{u1,v0},{u1,v1},{u0,v1});
    else
        QuadColors(meshes,material,{a,y0,boundary},{b,y0,boundary},
                   {b,y1,boundary},{a,y1,boundary},colors,
                   {u0,v0},{u1,v0},{u1,v1},{u0,v1});
}

void WallFace(Meshes& meshes, Material material, bool vertical, float boundary,
              float a, float b, float y0, float y1, Color color,
              float lightStart=1.0f, float lightEnd=1.0f) {
    const auto shade=[&](float height) {
        if (material==Material::Wallpaper)
            return 0.83f+0.17f*std::clamp(height/3.0f,0.0f,1.0f);
        if (material==Material::ConcreteWall)
            return 0.90f+0.10f*std::clamp(height/4.1f,0.0f,1.0f);
        if (material==Material::TunnelWall)
            return 0.86f+0.14f*std::clamp(height/2.55f,0.0f,1.0f);
        return 1.0f;
    };
    const Color bottomStart=Scale(color,shade(y0)*lightStart);
    const Color bottomEnd=Scale(color,shade(y0)*lightEnd);
    const Color topStart=Scale(color,shade(y1)*lightStart);
    const Color topEnd=Scale(color,shade(y1)*lightEnd);
    WallFaceColors(meshes,material,vertical,boundary,a,b,y0,y1,
                   {bottomStart,bottomEnd,topEnd,topStart});
}

using WallLighting=std::array<std::array<std::array<float,3>,3>,2>;

void Partition(Meshes& meshes, Material material, Edge edge,
               OpeningSpan opening, bool vertical,
               float boundary, float along, float height, float doorHeight,
               Color wall, Color trim, const WallLighting& lights) {
    if (edge==Edge::Open) return;
    const bool office=material==Material::Wallpaper;
    const auto lightAt=[&](float coordinate,float y,int side) {
        const float t=std::clamp((coordinate-along)/5.0f,0.0f,1.0f);
        const auto& l=lights[side];
        const float heightT=std::clamp(y/height,0.0f,1.0f)*2;
        const int row=heightT<1 ? 0 : 1;
        const float fraction=heightT-row;
        const auto at=[&](int column) {
            return l[column][row]*(1-fraction)+l[column][row+1]*fraction;
        };
        return t<=0.5f ? at(0)+(at(1)-at(0))*2.0f*t :
                         at(1)+(at(2)-at(1))*(2.0f*t-1.0f);
    };
    const auto section=[&](float a,float b,float y0,float y1,Color tint,bool painted=false) {
        for (int side=0;side<2;++side) {
            const float face=boundary+(side==0 ? -kWallHalfThickness : kWallHalfThickness);
            const auto draw=[&](float from,float to) {
                const auto band=[&](float low,float high) {
                    if (office)
                        WallFaceColors(meshes,painted ? Material::PaintedTrim : material,
                            vertical,face,from,to,low,high,
                            {Scale(tint,lightAt(from,low,side)),
                             Scale(tint,lightAt(to,low,side)),
                             Scale(tint,lightAt(to,high,side)),
                             Scale(tint,lightAt(from,high,side))});
                    else WallFace(meshes,material,vertical,face,from,to,low,high,tint,
                                  lightAt(from,low,side),lightAt(to,low,side));
                };
                if (office && y0<height*0.5f && y1>height*0.5f) {
                    band(y0,height*0.5f);band(height*0.5f,y1);
                } else band(y0,y1);
            };
            const float middle=along+2.5f;
            if (a<middle && b>middle) { draw(a,middle);draw(middle,b); }
            else draw(a,b);
        }
    };
    const auto full=[&](float a,float b) {
        const float base=office ? 0.12f : 0.16f;
        section(a,b,0,base,trim,office);
        section(a,b,base,material!=Material::TunnelWall ? height : height-0.10f,wall);
        if (material==Material::TunnelWall) section(a,b,height-0.10f,height,trim);
    };
    if (edge==Edge::Solid) full(along,along+5);
    else {
        const float first=static_cast<float>(opening.start);
        const float last=static_cast<float>(opening.end);
        full(along,along+first);full(along+last,along+5);
        const auto cap=[&](float end) {
            const float light=(lightAt(end,height*0.5f,0)+lightAt(end,height*0.5f,1))*0.5f;
            const float base=office ? 0.12f : 0.16f;
            if (office) {
                // Painted returns keep the wallpaper motif off narrow reveals.
                const Color paint=Scale(wall,0.85f);
                for (int band=0;band<2;++band) {
                    const float low=band==0 ? base : height*0.5f;
                    const float high=band==0 ? height*0.5f : height;
                    const Color bottom=Scale(paint,0.5f*(lightAt(end,low,0)+lightAt(end,low,1)));
                    const Color top=Scale(paint,0.5f*(lightAt(end,high,0)+lightAt(end,high,1)));
                    WallFaceColors(meshes,Material::PaintedTrim,!vertical,end,
                        boundary-kWallHalfThickness,boundary+kWallHalfThickness,
                        low,high,{bottom,bottom,top,top});
                }
            } else WallFace(meshes,material,!vertical,end,
                        boundary-kWallHalfThickness,boundary+kWallHalfThickness,base,
                        material!=Material::TunnelWall ? height : height-0.10f,
                        Scale(wall,0.82f),light,light);
            WallFace(meshes,office ? Material::PaintedTrim : material,!vertical,end,
                     boundary-kWallHalfThickness,boundary+kWallHalfThickness,0,base,trim,light,light);
            if (material==Material::TunnelWall)
                WallFace(meshes,material,!vertical,end,
                         boundary-kWallHalfThickness,boundary+kWallHalfThickness,height-0.10f,height,
                         trim,light,light);
        };
        cap(along+first);cap(along+last);
        if (edge==Edge::Door) {
            section(along+first,along+last,doorHeight,height,wall);
            const auto underside=[&](float coordinate,int side) {
                return Scale(wall,(office ? 0.85f : 1.0f)*0.83f*
                                  lightAt(coordinate,doorHeight,side));
            };
            const float a=along+first,b=along+last;
            const Material returnMaterial=office ? Material::PaintedTrim : material;
            if (vertical)
                FlatShaded(meshes,returnMaterial,boundary-kWallHalfThickness,a,
                     boundary+kWallHalfThickness,b,doorHeight,
                     {underside(a,0),underside(a,1),underside(b,1),underside(b,0)},0.6f);
            else
                FlatShaded(meshes,returnMaterial,a,boundary-kWallHalfThickness,
                     b,boundary+kWallHalfThickness,doorHeight,
                     {underside(a,0),underside(b,0),underside(b,1),underside(a,1)},0.6f);
        }
    }
}

void BoxRange(Meshes& meshes, Material material, float x0, float z0,
              float x1, float z1, float y0, float y1, Color color) {
    WallFace(meshes,material,true,x0,z0,z1,y0,y1,color);
    WallFace(meshes,material,true,x1,z0,z1,y0,y1,color);
    WallFace(meshes,material,false,z0,x0,x1,y0,y1,color);
    WallFace(meshes,material,false,z1,x0,x1,y0,y1,color);
    Flat(meshes,material,x0,z0,x1,z1,y1,color,0.6f);
}

void PipeRun(Meshes& meshes, bool alongZ, float cross, float start,
             float end, float height, float radius, Color color,
             Material material=Material::IndustrialCeiling,bool caps=false,
             int facets=8) {
    constexpr float turn=6.28318530718f;
    for (int facet=0;facet<facets;++facet) {
        const float a=turn*facet/facets,b=turn*(facet+1)/facets;
        const float ha=std::cos(a)*radius,hb=std::cos(b)*radius;
        const float ya=height+std::sin(a)*radius;
        const float yb=height+std::sin(b)*radius;
        // Shared edge colors approximate a round, overhead-lit surface without
        // adding a normal/light shader or a large number of radial segments.
        const Color edgeA=Scale(color,0.86f+0.14f*std::sin(a));
        const Color edgeB=Scale(color,0.86f+0.14f*std::sin(b));
        const Vector3 p0=alongZ ? Vector3(cross+ha,ya,start) :
                                  Vector3(start,ya,cross+ha);
        const Vector3 p1=alongZ ? Vector3(cross+ha,ya,end) :
                                  Vector3(end,ya,cross+ha);
        const Vector3 p2=alongZ ? Vector3(cross+hb,yb,end) :
                                  Vector3(end,yb,cross+hb);
        const Vector3 p3=alongZ ? Vector3(cross+hb,yb,start) :
                                  Vector3(start,yb,cross+hb);
        const float circumference=turn*radius;
        QuadColors(meshes,material,p0,p1,p2,p3,{edgeA,edgeA,edgeB,edgeB},
             {start*0.3f,circumference*facet/facets},
             {end*0.3f,circumference*facet/facets},
             {end*0.3f,circumference*(facet+1)/facets},
             {start*0.3f,circumference*(facet+1)/facets});
    }
    if (caps) for (float along:{start,end}) {
        const Vector3 center=alongZ ? Vector3(cross,height,along) : Vector3(along,height,cross);
        const Color tint=Scale(color,along==start ? 0.65f : 0.78f);
        auto& mesh=meshes[static_cast<int>(material)];
        for (int facet=0;facet<facets;++facet) {
            const float a=turn*facet/facets,b=turn*(facet+1)/facets;
            const float ha=std::cos(a)*radius,hb=std::cos(b)*radius;
            const float ya=std::sin(a)*radius,yb=std::sin(b)*radius;
            const Vector3 pa=alongZ ? Vector3(cross+ha,height+ya,along) :
                                     Vector3(along,height+ya,cross+ha);
            const Vector3 pb=alongZ ? Vector3(cross+hb,height+yb,along) :
                                     Vector3(along,height+yb,cross+hb);
            mesh.emplace_back(center,tint,Vector2(0.5f,0.5f));
            mesh.emplace_back(pa,tint,Vector2(0.5f+ha/(2*radius),0.5f+ya/(2*radius)));
            mesh.emplace_back(pb,tint,Vector2(0.5f+hb/(2*radius),0.5f+yb/(2*radius)));
        }
    }
}

void PipeWallReturn(Meshes& meshes,bool alongZ,float cross,float straightEnd,
                    float wallSide,float direction,float height,float radius,
                    Color color) {
    constexpr float bendRadius=0.20f,halfPi=1.57079632679f,turn=6.28318530718f;
    constexpr int segments=2,facets=8;
    const auto point=[&](float angle,float radial) {
        const float c=std::cos(angle),s=std::sin(angle);
        const float across=cross-wallSide*bendRadius*(1-c)+radius*std::cos(radial)*c;
        const float along=straightEnd+direction*bendRadius*s+
                          radius*std::cos(radial)*wallSide*direction*s;
        const float y=height+radius*std::sin(radial);
        return alongZ ? Vector3(across,y,along) : Vector3(along,y,across);
    };
    for (int segment=0;segment<segments;++segment) {
        const float a=halfPi*segment/segments,b=halfPi*(segment+1)/segments;
        const float ua=(straightEnd+direction*bendRadius*a)*0.3f;
        const float ub=(straightEnd+direction*bendRadius*b)*0.3f;
        for (int facet=0;facet<facets;++facet) {
            const float ra=turn*facet/facets,rb=turn*(facet+1)/facets;
            const Color ca=Scale(color,0.86f+0.14f*std::sin(ra));
            const Color cb=Scale(color,0.86f+0.14f*std::sin(rb));
            QuadColors(meshes,Material::GalvanizedMetal,
                point(a,ra),point(b,ra),point(b,rb),point(a,rb),{ca,ca,cb,cb},
                {ua,radius*ra},{ub,radius*ra},{ub,radius*rb},{ua,radius*rb});
        }
    }
}

void PropBox(Meshes& meshes, Material material, float cx, float cz,
             int quarterTurn, float sink, float x0, float z0, float x1,
             float z1, float y0, float y1, Color color) {
    const auto rotate=[&](float x,float z) {
        switch (quarterTurn&3) {
        case 1: return std::pair{z,-x};
        case 2: return std::pair{-x,-z};
        case 3: return std::pair{-z,x};
        default: return std::pair{x,z};
        }
    };
    const auto a=rotate(x0,z0),b=rotate(x1,z1);
    const float left=cx+std::min(a.first,b.first);
    const float right=cx+std::max(a.first,b.first);
    const float near=cz+std::min(a.second,b.second);
    const float far=cz+std::max(a.second,b.second);
    WallFace(meshes,material,true,left,near,far,y0-sink,y1-sink,Scale(color,0.84f));
    WallFace(meshes,material,true,right,near,far,y0-sink,y1-sink,Scale(color,0.90f));
    WallFace(meshes,material,false,near,left,right,y0-sink,y1-sink,Scale(color,0.96f));
    WallFace(meshes,material,false,far,left,right,y0-sink,y1-sink,Scale(color,0.86f));
    Flat(meshes,material,left,near,right,far,y1-sink,color,0.6f);
}

void Furniture(Meshes& meshes, const CellProp& prop, double chunkX,
               double chunkZ, int level, float illumination, std::uint32_t variation) {
    if (prop.kind==PropKind::None) return;
    const float x=static_cast<float>(prop.x-chunkX);
    const float z=static_cast<float>(prop.z-chunkZ);
    const int turn=prop.quarterTurn;
    const float sink=prop.sink;
    const Material metal=Material::IndustrialCeiling;
    const std::array<Color,3> fabrics{{Color(231,213,167),Color(175,185,148),
                                      Color(213,199,173)}};
    const Color seat=fabrics[variation%fabrics.size()];
    const Color frame(202,205,194);
    const auto box=[&](Material mat,float x0,float z0,float x1,float z1,
                       float y0,float y1,Color color) {
        PropBox(meshes,mat,x,z,turn,sink,x0,z0,x1,z1,y0,y1,Scale(color,illumination));
    };
    if (prop.kind==PropKind::Chair || prop.kind==PropKind::EmbeddedChair) {
        constexpr float seatX=kChairHalfWidth-0.015f;
        constexpr float seatZ=kChairHalfDepth-0.025f;
        box(metal,-seatX,-seatZ,seatX,seatZ,0.41f,0.44f,frame);
        box(Material::Upholstery,-seatX,-seatZ,seatX,seatZ,0.44f,0.48f,seat);
        box(Material::Upholstery,-0.24f,0.24f,0.24f,0.285f,0.66f,0.98f,seat);
        for (float legX: {-0.235f,0.235f})
            for (float legZ: {-0.23f,0.23f})
                box(metal,legX-0.018f,legZ-0.018f,
                    legX+0.018f,legZ+0.018f,0,0.44f,frame);
        for (float postX: {-0.25f,0.25f})
            box(metal,postX-0.015f,0.255f,postX+0.015f,0.285f,0.46f,1.01f,frame);
        box(metal,-0.265f,0.255f,0.265f,0.285f,0.98f,1.01f,frame);
    } else if (prop.kind==PropKind::LowPartition) {
        box(Material::Wallpaper,-1.66f,-0.14f,1.66f,0.14f,
            0,1.34f,Color(214,205,171));
        box(Material::PaintedTrim,-1.70f,-0.16f,1.70f,0.16f,
            1.34f,1.41f,Color(151,143,112));
    } else if (prop.kind==PropKind::TallPartition) {
        box(Material::Wallpaper,-1.25f,-0.12f,1.25f,0.12f,
            0,2.16f,Color(241,232,199));
        box(Material::PaintedTrim,-1.28f,-0.14f,1.28f,0.14f,
            0,0.17f,Color(184,173,132));
        box(Material::PaintedTrim,-1.28f,-0.14f,1.28f,0.14f,
            2.16f,2.22f,Color(175,164,125));
    } else {
        const Color top=level==0 ? Color(255,245,223) : Color(201,212,204);
        box(level==0 ? Material::Wood : Material::IndustrialCeiling,
            -kTableHalfWidth,-kTableHalfDepth,kTableHalfWidth,kTableHalfDepth,
            0.73f,0.765f,top);
        for (float legX: {-0.65f,0.65f})
            for (float legZ: {-0.30f,0.30f})
                box(metal,legX-0.025f,legZ-0.025f,
                    legX+0.025f,legZ+0.025f,0,0.74f,frame);
        box(metal,-0.675f,-0.325f,0.675f,-0.30f,0.65f,0.73f,frame);
        box(metal,-0.675f,0.30f,0.675f,0.325f,0.65f,0.73f,frame);
    }
}

void CardboardBox(Meshes& meshes, float x0, float z0, float x1, float z1,
                  float y0, float y1, Color color) {
    const Material material=Material::Cardboard;
    const Vector2 uv0(0,1),uv1(1,1),uv2(1,0),uv3(0,0);
    Quad(meshes,material,{x0,y0,z0},{x0,y0,z1},
         {x0,y1,z1},{x0,y1,z0},Scale(color,0.90f),uv0,uv1,uv2,uv3);
    Quad(meshes,material,{x1,y0,z0},{x1,y0,z1},
         {x1,y1,z1},{x1,y1,z0},Scale(color,0.84f),uv0,uv1,uv2,uv3);
    Quad(meshes,material,{x0,y0,z0},{x1,y0,z0},
         {x1,y1,z0},{x0,y1,z0},color,uv0,uv1,uv2,uv3);
    Quad(meshes,material,{x0,y0,z1},{x1,y0,z1},
         {x1,y1,z1},{x0,y1,z1},Scale(color,0.93f),uv0,uv1,uv2,uv3);
    Quad(meshes,material,{x0,y1,z0},{x1,y1,z0},
         {x1,y1,z1},{x0,y1,z1},Scale(color,1.05f),
         {0,0},{1,0},{1,1},{0,1});
}

void StorageRack(Meshes& meshes, float x0, float z0, float x1, float z1,
                 std::uint32_t hash) {
    const Color frame(122,139,132),shelf(184,194,180);
    constexpr float post=0.065f;
    for (float x: {x0,x1-post}) for (float z: {z0,z1-post})
        BoxRange(meshes,Material::IndustrialCeiling,x,z,x+post,z+post,
                 0,2.45f,frame);
    const int shelfCount=hash%3==0 ? 4 : 3;
    const std::array<float,4> heights=shelfCount==4 ?
        std::array<float,4>{{0.52f,1.16f,1.80f,2.38f}} :
        hash%2 ? std::array<float,4>{{0.40f,1.36f,2.38f,0}} :
                 std::array<float,4>{{0.60f,1.49f,2.38f,0}};
    for (int index=0;index<shelfCount;++index) {
        const float y=heights[index];
        BoxRange(meshes,Material::IndustrialCeiling,x0,z0,x1,z1,
                 y,y+0.055f,shelf);
    }
    const auto brace=[&](bool reverse) {
        const float nearY=reverse ? 2.30f : 0.18f;
        const float farY=reverse ? 0.18f : 2.30f;
        if (x1-x0>=z1-z0)
            Quad(meshes,Material::IndustrialCeiling,
                 {x0+post,nearY-0.025f,z1-0.025f},
                 {x1-post,farY-0.025f,z1-0.025f},
                 {x1-post,farY+0.025f,z1-0.025f},
                 {x0+post,nearY+0.025f,z1-0.025f},frame,
                 {0,0},{1,0},{1,1},{0,1});
        else
            Quad(meshes,Material::IndustrialCeiling,
                 {x1-0.025f,nearY-0.025f,z0+post},
                 {x1-0.025f,farY-0.025f,z1-post},
                 {x1-0.025f,farY+0.025f,z1-post},
                 {x1-0.025f,nearY+0.025f,z0+post},frame,
                 {0,0},{1,0},{1,1},{0,1});
    };
    brace(false);brace(true);
    for (int row=0;row<shelfCount-1;++row) for (int slot=0;slot<2;++slot) {
        const unsigned bits=(hash>>(row*6+slot*3))&7U;
        if (bits>2) continue;
        const float width=0.36f+0.07f*bits;
        const float depth=0.36f+0.04f*((hash>>(20+slot*2))&3U);
        const float cx=x0+(x1-x0)*(slot ? 0.72f : 0.28f);
        const float cz=z0+(z1-z0)*(row%2 ? 0.66f : 0.34f);
        const float base=heights[row]+0.055f;
        CardboardBox(meshes,cx-width*0.5f,cz-depth*0.5f,
                     cx+width*0.5f,cz+depth*0.5f,base,
                     base+0.32f+0.055f*bits,
                     bits==0 ? Color(217,213,192) : Color(239,232,207));
    }
}

void FalseDoor(Meshes& meshes, bool vertical, float boundary, float along,
               int level, float illumination,int normal=1,float ceiling=10) {
    const float height=std::min(ceiling-0.10f,
                               level==1 ? 2.58f : level==2 ? 2.15f : 2.18f);
    const Material material=level==0 ? Material::PaintedTrim : Material::ConcreteWall;
    const Color panel=Scale(level==0 ? Color(197,189,157) : Color(85,96,94),illumination);
    const Color frame=Scale(level==0 ? Color(227,217,188) : Color(126,136,130),illumination);
    const float left=along+2.04f,right=along+2.96f;
    const float face=boundary+normal*(kWallHalfThickness+0.005f);
    WallFace(meshes,material,vertical,face,left,right,0.02f,height,panel);
    WallFace(meshes,material,vertical,face+normal*0.004f,left-0.075f,left,
             0,height+0.08f,frame);
    WallFace(meshes,material,vertical,face+normal*0.004f,right,right+0.075f,
             0,height+0.08f,frame);
    WallFace(meshes,material,vertical,face+normal*0.004f,left,right,
             height,height+0.08f,frame);
    const Color hardware=Scale(Color(228,232,219),illumination);
    const float handle=right-0.16f;
    WallFace(meshes,Material::GalvanizedMetal,vertical,face+normal*0.009f,
             handle-0.03f,handle+0.03f,0.94f,1.12f,hardware);
    const auto handleBox=[&](float a,float b,float d0,float d1,float y0,float y1) {
        const float near=std::min(face+normal*d0,face+normal*d1);
        const float far=std::max(face+normal*d0,face+normal*d1);
        if (vertical) BoxRange(meshes,Material::GalvanizedMetal,near,a,far,b,y0,y1,hardware);
        else BoxRange(meshes,Material::GalvanizedMetal,a,near,b,far,y0,y1,hardware);
    };
    handleBox(handle-0.025f,handle+0.025f,0.01f,0.06f,1.015f,1.06f);
    handleBox(handle-0.15f,handle+0.025f,0.05f,0.073f,1.02f,1.052f);
}

int FloorDiv(int value, int size) {
    return value>=0 ? value/size : (value-size+1)/size;
}


void ContactShadow(Meshes& meshes, BakedLighting& lighting,
                   Edge edge, OpeningSpan opening, bool vertical,
                   float boundary, float along, double chunkX,
                   double chunkZ, Color floorColor) {
    if (edge==Edge::Open) return;
    const auto strip=[&](float x0,float z0,float x1,float z1,
                         const std::array<float,4>& strength) {
        const auto color=[&](float x,float z,float scale) {
            return Scale(floorColor,scale*lighting.FloorSample(chunkX+x,chunkZ+z));
        };
        FlatShaded(meshes,Material::Carpet,x0,z0,x1,z1,0.004f,
            {color(x0,z0,strength[0]),color(x1,z0,strength[1]),
             color(x1,z1,strength[2]),color(x0,z1,strength[3])},kCarpetRepeat);
    };
    const auto section=[&](float start,float end) {
        if (end<=start) return;
        constexpr float inset=0.105f,outset=0.34f,dim=0.84f;
        float a=along+start;
        const float stop=along+end;
        while (a<stop-0.001f) {
            const float b=std::min(stop,(std::floor(a/1.25f)+1)*1.25f);
            if (vertical) {
                strip(boundary-outset,a,boundary-inset,b,{1,dim,dim,1});
                strip(boundary+inset,a,boundary+outset,b,{dim,1,1,dim});
            } else {
                strip(a,boundary-outset,b,boundary-inset,{1,1,dim,dim});
                strip(a,boundary+inset,b,boundary+outset,{dim,dim,1,1});
            }
            a=b;
        }
    };
    if (edge==Edge::Solid) section(0,5);
    else { section(0,static_cast<float>(opening.start));
           section(static_cast<float>(opening.end),5); }
}

void FloorContactShadow(Meshes& meshes, BakedLighting& lighting,
                         float x0, float z0, float x1, float z1,
                         double chunkX, double chunkZ, Color floor,
                         float dim=0.82f, Material material=Material::Carpet,
                         float repeat=kCarpetRepeat) {
    constexpr float feather=0.28f;
    const auto shadow=[&](float a,float b,float c,float d,
                         const std::array<float,4>& strength) {
        const auto color=[&](float x,float z,float scale) {
            return Scale(floor,scale*lighting.FloorSample(chunkX+x,chunkZ+z));
        };
        FlatShaded(meshes,material,a,b,c,d,0.005f,
            {color(a,b,strength[0]),color(c,b,strength[1]),
             color(c,d,strength[2]),color(a,d,strength[3])},repeat);
    };
    shadow(x0,z0,x1,z1,{dim,dim,dim,dim});
    shadow(x0-feather,z0,x0,z1,{1,dim,dim,1});
    shadow(x1,z0,x1+feather,z1,{dim,1,1,dim});
    shadow(x0,z0-feather,x1,z0,{1,1,dim,dim});
    shadow(x0,z1,x1,z1+feather,{dim,dim,1,1});
    shadow(x0-feather,z0-feather,x0,z0,{1,1,dim,1});
    shadow(x1,z0-feather,x1+feather,z0,{1,1,1,dim});
    shadow(x1,z1,x1+feather,z1+feather,{dim,1,1,1});
    shadow(x0-feather,z1,x0,z1+feather,{1,dim,1,1});
}

void OfficeObstacle(Meshes& meshes, BakedLighting& lighting,
                    float x0, float z0, float x1, float z1,
                    double chunkX, double chunkZ, float height,
                    Color wall, Color trim, Color floor) {
    const auto face=[&](bool vertical,float boundary,float a,float b,int normal) {
        const auto light=[&](float along,float y) {
            const double wx=chunkX+(vertical ? boundary+normal*0.04f : along);
            const double wz=chunkZ+(vertical ? along : boundary+normal*0.04f);
            return lighting.WallSample(wx,wz,vertical ? normal : 0,
                                             vertical ? 0 : normal,y);
        };
        const auto band=[&](float from,float to,float low,float high,
                            Material material,Color color) {
            WallFaceColors(meshes,material,vertical,boundary,from,to,low,high,
                {Scale(color,light(from,low)),Scale(color,light(to,low)),
                 Scale(color,light(to,high)),Scale(color,light(from,high))});
        };
        for (float from=a;from<b-0.001f;) {
            const float to=std::min(b,from+2.5f);
            band(from,to,0,0.12f,Material::PaintedTrim,trim);
            band(from,to,0.12f,height*0.5f,Material::Wallpaper,wall);
            band(from,to,height*0.5f,height,Material::Wallpaper,wall);
            from=to;
        }
    };
    face(true,x0,z0,z1,-1);face(true,x1,z0,z1,1);
    face(false,z0,x0,x1,-1);face(false,z1,x0,x1,1);
    FloorContactShadow(meshes,lighting,x0,z0,x1,z1,
                        chunkX,chunkZ,floor);
}

void OfficeAlcoveVisual(Meshes& meshes,BakedLighting& lighting,
                        const OfficeAlcove& alcove,double chunkX,double chunkZ,
                        float ceiling,Color wall,Color tile) {
    const float boundary=static_cast<float>(alcove.boundary-
                          (alcove.vertical ? chunkX : chunkZ));
    const float start=static_cast<float>(alcove.start-
                       (alcove.vertical ? chunkZ : chunkX));
    const float end=static_cast<float>(alcove.end-
                     (alcove.vertical ? chunkZ : chunkX));
    const float cross=boundary+alcove.inward*static_cast<float>(alcove.depth);
    const float roof=std::min(2.42f,ceiling-0.15f);
    const float center=(start+end)*0.5f;
    const float illumination=lighting.WallSample(
        chunkX+(alcove.vertical ? cross+alcove.inward*0.04f : center),
        chunkZ+(alcove.vertical ? center : cross+alcove.inward*0.04f),
        alcove.vertical ? alcove.inward : 0,alcove.vertical ? 0 : alcove.inward);
    const auto underside=Scale(tile,0.70f*illumination);
    const float near=std::min(boundary,cross),far=std::max(boundary,cross);
    if (alcove.vertical)
        Flat(meshes,Material::CeilingTile,near,start-0.10f,far,end+0.10f,
             roof,underside,0.4f);
    else
        Flat(meshes,Material::CeilingTile,start-0.10f,near,end+0.10f,far,
             roof,underside,0.4f);
    WallFace(meshes,Material::Wallpaper,alcove.vertical,cross,
             start-0.10f,end+0.10f,roof,ceiling,wall,illumination,illumination);
    if (alcove.falseDoor) {
        const float light=lighting.WallSample(
            chunkX+(alcove.vertical ? boundary+alcove.inward*0.14f : center),
            chunkZ+(alcove.vertical ? center : boundary+alcove.inward*0.14f),
            alcove.vertical ? alcove.inward : 0,alcove.vertical ? 0 : alcove.inward);
        FalseDoor(meshes,alcove.vertical,boundary,center-2.5f,0,
                  light,alcove.inward,roof);
    }
}

void StructuralColumn(Meshes& meshes, BakedLighting& lighting,
                      float x0,float z0,float x1,float z1,
                      double chunkX,double chunkZ,float height,
                      Color body,Color base,Color floor,Material floorMaterial) {
    constexpr float baseHeight=0.20f;
    const auto face=[&](bool vertical,float boundary,float a,float b,int normal) {
        const auto sample=[&](float coordinate,float y) {
            return lighting.WallSample(
                chunkX+(vertical ? boundary+normal*0.04f : coordinate),
                chunkZ+(vertical ? coordinate : boundary+normal*0.04f),
                vertical ? normal : 0,vertical ? 0 : normal,y);
        };
        const auto band=[&](float low,float high) {
            WallFaceColors(meshes,Material::TunnelWall,vertical,boundary,a,b,
                low,high,{Scale(body,sample(a,low)),Scale(body,sample(b,low)),
                          Scale(body,sample(b,high)),Scale(body,sample(a,high))});
        };
        band(baseHeight,height*0.5f);band(height*0.5f,height);
        const float start=sample(a,baseHeight),end=sample(b,baseHeight);
        WallFace(meshes,Material::IndustrialCeiling,vertical,boundary,a,b,
                 0,baseHeight,base,start,end);
    };
    face(true,x0,z0,z1,-1);face(true,x1,z0,z1,1);
    face(false,z0,x0,x1,-1);face(false,z1,x0,x1,1);
    FloorContactShadow(meshes,lighting,x0,z0,x1,z1,chunkX,chunkZ,
                       floor,0.80f,floorMaterial,0.5f);
}

void UtilityEquipment(Meshes& meshes, BakedLighting& lighting,
                      float x0,float z0,float x1,float z1,
                      float cellX,float cellZ,double chunkX,double chunkZ,
                      float height,Color floor,bool pressureRack,bool alongZ) {
    const float cx=(x0+x1)*0.5f,cz=(z0+z1)*0.5f;
    const bool vertical=alongZ;
    const int normal=(vertical ? cellX-cx : cellZ-cz)>0 ? 1 : -1;
    const float face=vertical ? (normal>0 ? x1 : x0) : (normal>0 ? z1 : z0);
    const float light=lighting.WallSample(
        chunkX+(vertical ? face+normal*0.04f : cx),
        chunkZ+(vertical ? cz : face+normal*0.04f),
        vertical ? normal : 0,vertical ? 0 : normal);
    if (pressureRack) {
        const Color frame=Scale(Color(189,199,183),light);
        const Color pipe=Scale(Color(213,217,196),light);
        for (float x:{x0+0.04f,x1-0.09f}) for (float z:{z0+0.04f,z1-0.09f})
            BoxRange(meshes,Material::GalvanizedMetal,x,z,x+0.05f,z+0.05f,
                     0,height-0.12f,frame);
        BoxRange(meshes,Material::GalvanizedMetal,x0,z0,x1,z1,0.17f,0.22f,frame);
        BoxRange(meshes,Material::GalvanizedMetal,x0,z0,x1,z1,1.25f,1.30f,frame);
        const float radius=std::min(0.44f,(vertical ? x1-x0 : z1-z0)*0.38f);
        const float cross=vertical ? cx : cz;
        const float start=(vertical ? z0 : x0)+0.08f;
        const float end=(vertical ? z1 : x1)-0.08f;
        for (const float y:{0.68f,1.72f}) {
            PipeRun(meshes,vertical,cross,start,end,y,radius,pipe,Material::GalvanizedMetal,true,16);
            for (const float along:{start+0.13f,end-0.13f})
                PipeRun(meshes,vertical,cross,along-0.035f,along+0.035f,y,
                        radius*1.08f,Scale(pipe,0.84f),Material::GalvanizedMetal,false,16);
        }
    } else {
        PropBox(meshes,Material::GalvanizedMetal,cx,cz,0,0,x0-cx,z0-cz,x1-cx,z1-cz,
                0,height,Scale(Color(181,194,177),light));
        const float near=(vertical ? z0 : x0)+0.13f;
        const float far=(vertical ? z1 : x1)-0.13f;
        const float plane=face+normal*0.004f;
        WallFace(meshes,Material::PaintedTrim,vertical,plane,near,far,
                 0.14f,height-0.14f,Scale(Color(182,193,173),light));
        const Color dark=Scale(Color(61,67,58),light);
        for (int vent=0;vent<4;++vent)
            WallFace(meshes,Material::GalvanizedMetal,vertical,plane+normal*0.004f,
                     near+0.12f,far-0.12f,height-0.54f+vent*0.065f,
                     height-0.515f+vent*0.065f,dark);
        WallFace(meshes,Material::GalvanizedMetal,vertical,plane+normal*0.006f,
                 far-0.12f,far-0.08f,0.91f,1.05f,dark);
    }
    FloorContactShadow(meshes,lighting,x0,z0,x1,z1,chunkX,chunkZ,
                       floor,0.82f,Material::TunnelFloor,0.5f);
}

void PortalVisual(Meshes& meshes, BakedLighting& lighting,
                  const WorldConfig& world, const PortalDefinition& portal,
                  double chunkX, double chunkZ, float ceiling,
                  Color wall, Color trim, Color floor) {
    const int level=world.level;
    const bool alongX=portal.alongX;
    const float cx=static_cast<float>((portal.cellX+0.5)*kCellSize-chunkX);
    const float cz=static_cast<float>((portal.cellZ+0.5)*kCellSize-chunkZ);
    constexpr float near=static_cast<float>(kPortalEntryDepth);
    constexpr float back=static_cast<float>(kPortalBackDepth-kWallHalfThickness);
    constexpr float end=static_cast<float>(kPortalBackDepth+kWallHalfThickness);
    constexpr float half=static_cast<float>(kPortalHalfWidth);
    constexpr float doorHeight=2.25f;
    const Material exterior=level==0 ? Material::Wallpaper :
                            level==1 ? Material::ConcreteWall : Material::TunnelWall;
    const float illumination=lighting.Sample(chunkX+cx,chunkZ+cz);
    const Color exteriorColor=Scale(wall,0.32f+0.68f*illumination);
    const auto box=[&](Material material,float d0,float s0,float d1,
                       float s1,float y0,float y1,Color tint) {
        if (alongX) BoxRange(meshes,material,cx+d0,cz+s0,cx+d1,cz+s1,y0,y1,tint);
        else BoxRange(meshes,material,cx+s0,cz+d0,cx+s1,cz+d1,y0,y1,tint);
    };
    const auto flat=[&](Material material,float d0,float s0,float d1,
                        float s1,float y,Color tint,float repeat) {
        if (alongX) Flat(meshes,material,cx+d0,cz+s0,cx+d1,cz+s1,y,tint,repeat);
        else Flat(meshes,material,cx+s0,cz+d0,cx+s1,cz+d1,y,tint,repeat);
    };
    const auto frames=PortalWalls(portal);
    for (int i=0;i<frames.count;++i) {
        const auto& bounds=frames.walls[i];
        const float x0=static_cast<float>(bounds.minX-chunkX);
        const float z0=static_cast<float>(bounds.minZ-chunkZ);
        const float x1=static_cast<float>(bounds.maxX-chunkX);
        const float z1=static_cast<float>(bounds.maxZ-chunkZ);
        const float height=i==2 ? doorHeight : ceiling;
        if (level==0)
            OfficeObstacle(meshes,lighting,x0,z0,x1,z1,chunkX,chunkZ,
                           height,wall,trim,floor);
        else BoxRange(meshes,exterior,x0,z0,x1,z1,0,height,exteriorColor);
    }
    // An ordinary service opening belongs to its level's architecture. Only
    // the short recessed interior has a different finish and dimmer lighting.
    box(exterior,near,-half,end,half,doorHeight,ceiling,exteriorColor);
    const Material casing=level==0 ? Material::PaintedTrim : Material::IndustrialCeiling;
    const Color frame=Scale(level==0 ? trim : Color(177,182,168),
                            0.45f+0.55f*illumination);
    box(casing,near-0.035f,-half-0.075f,near+0.015f,-half,0,doorHeight+0.075f,frame);
    box(casing,near-0.035f,half,near+0.015f,half+0.075f,0,doorHeight+0.075f,frame);
    box(casing,near-0.035f,-half,near+0.015f,half,doorHeight,doorHeight+0.075f,frame);
    const Material interior=level==2 ? Material::TunnelWall : Material::ConcreteWall;
    const Color innerFront(146,148,131),innerBack(75,77,66);
    const auto sideFace=[&](float side) {
        const Vector3 a=alongX ? Vector3(cx+near,0,cz+side) : Vector3(cx+side,0,cz+near);
        const Vector3 b=alongX ? Vector3(cx+back,0,cz+side) : Vector3(cx+side,0,cz+back);
        QuadColors(meshes,interior,a,b,{b.X,doorHeight,b.Z},{a.X,doorHeight,a.Z},
                   {Scale(innerFront,0.87f),Scale(innerBack,0.87f),innerBack,innerFront},
                   {near*0.6f,1},{back*0.6f,1},{back*0.6f,1-doorHeight*0.6f},
                   {near*0.6f,1-doorHeight*0.6f});
    };
    sideFace(-half+0.003f);sideFace(half-0.003f);
    WallFace(meshes,interior,alongX,(alongX ? cx : cz)+back-0.003f,
             (alongX ? cz : cx)-half,(alongX ? cz : cx)+half,
             0,doorHeight,innerBack);
    const Material interiorFloor=level==2 ? Material::TunnelFloor : Material::ConcreteFloor;
    const Color frontFloor(198,195,179),backFloor(103,105,92);
    if (alongX)
        FlatShaded(meshes,interiorFloor,cx+near,cz-half,cx+back,cz+half,0.012f,
                   {frontFloor,backFloor,backFloor,frontFloor},0.5f);
    else
        FlatShaded(meshes,interiorFloor,cx-half,cz+near,cx+half,cz+back,0.012f,
                   {frontFloor,frontFloor,backFloor,backFloor},0.5f);
    flat(Material::IndustrialCeiling,near,-half,back,half,doorHeight-0.005f,
         Color(116,121,105),0.6f);
    LightPanel(meshes,cx+(alongX ? 1.08f : -0.20f),
               cz+(alongX ? -0.20f : 1.08f),
               cx+(alongX ? 1.48f : 0.20f),
               cz+(alongX ? 0.20f : 1.48f),doorHeight-0.015f,
               Color(173,169,135),alongX);
}
}

BackroomsGame::BackroomsGame(std::uint64_t seed, bool streamTest,
                             int startLevel, double startX, double startZ,
                             double walkSpeed, double runSpeed,
                             double streamTestMetres,float verticalFovDegrees,
                             int multiSampleCount)
    : graphics_(this), streamTest_(streamTest),
      streamTestMetres_(streamTestMetres) {
    if (startLevel<0 || startLevel>2 || !std::isfinite(startX) ||
        !std::isfinite(startZ) || std::abs(startX)>1.0e8 ||
        std::abs(startZ)>1.0e8 || !std::isfinite(walkSpeed) ||
        !std::isfinite(runSpeed) || walkSpeed<=0 || runSpeed<=walkSpeed ||
        runSpeed>20.0 || !std::isfinite(streamTestMetres) ||
        streamTestMetres<400.0 || streamTestMetres>1.0e7 ||
        !std::isfinite(verticalFovDegrees) || verticalFovDegrees<45 ||
        verticalFovDegrees>90 ||
        (multiSampleCount!=0 && multiSampleCount!=4))
        throw std::invalid_argument("invalid start, movement speed, field of view, multisampling, or streaming distance");
    world_.seed = seed;
    world_.level=startLevel;
    world_.roomLayouts=&roomLayouts_;
    levelCatalog_=LoadLevelCatalog(FindAssetDirectory()/"levels.json");
    world_.levels=&levelCatalog_;
    if (Collides(world_,startX,startZ,0.31))
        throw std::invalid_argument("start position intersects generated geometry");
    x_=startX;
    z_=startZ;
    walkSpeed_=walkSpeed;
    runSpeed_=runSpeed;
    verticalFovDegrees_=verticalFovDegrees;
    yaw_=startLevel==2 ? 0.0f : 1.5707963f;
    entityDepthOnlyBlend_.setColorWriteChannelsProperty(ColorWriteChannels::None);
    graphics_.setPreferredBackBufferWidthProperty(1280);
    graphics_.setPreferredBackBufferHeightProperty(720);
    graphics_.setSynchronizeWithVerticalRetraceProperty(true);
    graphics_.setPreferMultiSamplingProperty(multiSampleCount>0);
    graphics_.PreparingDeviceSettings += [multiSampleCount](
        System::Object*,const PreparingDeviceSettingsEventArgs& args) {
        args.getGraphicsDeviceInformationEXT().getPresentationParametersProperty()
            .setMultiSampleCountProperty(multiSampleCount);
    };
    getWindowProperty().setTitleProperty("cna-backrooms");
}

BackroomsGame::~BackroomsGame() {
    // The frame lease has already released the ES context when Run returns.
    // Keep GPU deletion in the same explicit scope as streaming uploads.
    auto context=getGraphicsDeviceProperty().GetRenderer().AcquireThreadContextLeaseEXT();
    chunks_.clear();
    spareVertices_.clear();
    entityVertices_.reset();
    entityShadowVertices_.reset();
    materials_.reset();
    effect_.reset();
}

const std::string& BackroomsGame::GetTypeName() const {
    static const std::string name = "Backrooms.BackroomsGame";
    return name;
}

void BackroomsGame::Initialize() {
    Game::Initialize();
}

void BackroomsGame::LoadContent() {
    std::cerr << "Renderer ready: OPENGLES3, MSAA "
              << getGraphicsDeviceProperty().getPresentationParametersProperty()
                    .getMultiSampleCountProperty() << " samples applied\n";
    const auto directory=FindAssetDirectory();
    effect_ = std::make_unique<BasicEffect>(getGraphicsDeviceProperty());
    materials_ = std::make_unique<Materials>(getGraphicsDeviceProperty(),directory);
    effect_->VertexColorEnabled = true;
    effect_->setTextureEnabledProperty(true);
    effect_->setLightingEnabledProperty(false);
    effect_->setFogEnabledProperty(true);
    effect_->setFogStartProperty(LevelInfo(world_).fogStart);
    effect_->setFogEndProperty(LevelInfo(world_).fogEnd);
    setIsMouseVisibleProperty(false);
    Mouse::setIsRelativeMouseModeEXTProperty(true);
    captured_ = true;
    BuildEntityMesh();
    BuildChunk(ChunkAt(x_,z_));
    try {
        humSound_=std::make_unique<Audio::SoundEffect>((directory/"hum.wav").string());
        step_=std::make_unique<Audio::SoundEffect>((directory/"step.wav").string());
        carpetStep_=std::make_unique<Audio::SoundEffect>(
            (directory/"step-carpet.wav").string());
        transition_=std::make_unique<Audio::SoundEffect>((directory/"transition.wav").string());
        hum_=std::make_unique<Audio::SoundEffectInstance>(humSound_->CreateInstance());
        hum_->setVolumeProperty(0.34f);
        hum_->setIsLoopedProperty(true);
        hum_->Play();
        if (hum_->getStateProperty()!=Audio::SoundState::Playing)
            throw std::runtime_error("fluorescent hum did not start playing");
        std::cerr << "Audio ready: hum, carpet/hard-floor steps and transition loaded from "
                  << directory << '\n';
    } catch (const std::exception& error) {
        hum_.reset(); humSound_.reset(); step_.reset(); carpetStep_.reset();
        transition_.reset();
        std::cerr << "Audio unavailable: " << error.what() << '\n';
    }
}

void BackroomsGame::BuildEntityMesh() {
    Meshes meshes;
    const Material body=Material::Upholstery;
    const Color coat(89,95,113),skin(112,110,112);
    struct Ring { float x,y,z,width,depth; };
    const auto shape=[&](std::initializer_list<Ring> rings,Color tint,int facets=8) {
        const auto shade=[&](float angle) {
            return Scale(tint,0.73f+0.27f*std::max(0.0f,
                0.5f*std::cos(angle)+0.866f*std::sin(angle)));
        };
        const auto point=[](const Ring& ring,float angle) {
            return Vector3(ring.x+ring.width*std::cos(angle),ring.y,
                           ring.z+ring.depth*std::sin(angle));
        };
        constexpr float turn=6.28318530718f;
        for (auto lower=rings.begin();lower+1!=rings.end();++lower) {
            const auto& upper=*(lower+1);
            for (int face=0;face<facets;++face) {
                const float a=turn*face/facets,b=turn*(face+1)/facets;
                QuadColors(meshes,body,point(*lower,a),point(*lower,b),
                    point(upper,b),point(upper,a),
                    {shade(a),shade(b),shade(b),shade(a)},
                    {a/turn,lower->y},{b/turn,lower->y},
                    {b/turn,upper.y},{a/turn,upper.y});
            }
        }
        auto& mesh=meshes[static_cast<int>(body)];
        for (int end=0;end<2;++end) {
            const auto& ring=end ? *(rings.end()-1) : *rings.begin();
            const Vector3 center(ring.x,ring.y,ring.z);
            const Color cap=Scale(tint,end ? 0.95f : 0.70f);
            for (int face=0;face<facets;++face) {
                const float a=turn*face/facets,b=turn*(face+1)/facets;
                mesh.emplace_back(center,cap,Vector2(0.5f,0.5f));
                mesh.emplace_back(point(ring,a),cap,
                    Vector2(0.5f+0.5f*std::cos(a),0.5f+0.5f*std::sin(a)));
                mesh.emplace_back(point(ring,b),cap,
                    Vector2(0.5f+0.5f*std::cos(b),0.5f+0.5f*std::sin(b)));
            }
        }
    };
    // The torso and relaxed limbs overlap at joints. One immutable mesh is
    // shared by every distant figure; no animation or physical agent is needed.
    shape({{0,0.76f,0,0.205f,0.12f},{0,1.13f,0,0.18f,0.105f},
           {0,1.55f,0,0.245f,0.135f},{0,1.68f,0,0.085f,0.07f}},coat);
    for (const float side:{-1.0f,1.0f}) {
        shape({{side*0.12f,0.0f,0.025f,0.065f,0.105f},
               {side*0.12f,0.11f,0,0.06f,0.07f},
               {side*0.11f,0.44f,0.015f,0.065f,0.08f},
               {side*0.105f,0.82f,0,0.085f,0.10f}},coat);
        shape({{side*0.30f,0.80f,0.035f,0.042f,0.05f},
               {side*0.32f,1.12f,0.02f,0.055f,0.06f},
               {side*0.275f,1.53f,0,0.072f,0.075f}},coat);
    }
    shape({{0,1.63f,0,0.061f,0.055f},{0,1.76f,0.015f,0.06f,0.055f}},skin);
    shape({{0,1.69f,0.018f,0.025f,0.03f},
           {0,1.74f,0.024f,0.085f,0.088f},
           {0,1.84f,0.02f,0.111f,0.105f},
           {0,1.95f,0.008f,0.083f,0.081f},
           {0,2.005f,0,0.012f,0.012f}},skin,12);
    auto& mesh=meshes[static_cast<int>(body)];
    entityTriangles_=static_cast<int>(mesh.size()/3);
    entityVertices_=std::make_unique<VertexBuffer>(getGraphicsDeviceProperty(),
        VertexPositionColorTexture::getVertexDeclarationStatic(),
        static_cast<int>(mesh.size()),BufferUsage::WriteOnly);
    entityVertices_->SetData(mesh.data(),static_cast<int>(mesh.size()));
    Mesh shadow;
    constexpr float turn=6.28318530718f;
    for (float foot:{-0.12f,0.12f}) for (int side=0;side<8;++side) {
        const float a=turn*side/8,b=turn*(side+1)/8;
        shadow.emplace_back(Vector3(foot,0.007f,0.025f),Color(0,0,0,56),Vector2(0,0));
        shadow.emplace_back(Vector3(foot+0.16f*std::cos(a),0.007f,
                                    0.025f+0.20f*std::sin(a)),Color(0,0,0,0),Vector2(0,0));
        shadow.emplace_back(Vector3(foot+0.16f*std::cos(b),0.007f,
                                    0.025f+0.20f*std::sin(b)),Color(0,0,0,0),Vector2(0,0));
    }
    entityShadowTriangles_=static_cast<int>(shadow.size()/3);
    entityShadowVertices_=std::make_unique<VertexBuffer>(getGraphicsDeviceProperty(),
        VertexPositionColorTexture::getVertexDeclarationStatic(),
        static_cast<int>(shadow.size()),BufferUsage::WriteOnly);
    entityShadowVertices_->SetData(shadow.data(),static_cast<int>(shadow.size()));

}

std::unique_ptr<VertexBuffer> BackroomsGame::AcquireBuffer(int vertexCount) {
    std::size_t best=spareVertices_.size();
    for (std::size_t i=0;i<spareVertices_.size();++i) {
        const int capacity=spareVertices_[i]->getVertexCountProperty();
        if (capacity>=vertexCount &&
            (best==spareVertices_.size() ||
             capacity<spareVertices_[best]->getVertexCountProperty()))
            best=i;
    }
    if (best!=spareVertices_.size()) {
        auto buffer=std::move(spareVertices_[best]);
        spareVertices_.erase(spareVertices_.begin()+static_cast<std::ptrdiff_t>(best));
        ++bufferReuses_;
        return buffer;
    }
    ++bufferCreations_;
    const int capacity=((vertexCount+511)/512)*512;
    return std::make_unique<VertexBuffer>(getGraphicsDeviceProperty(),
        VertexPositionColorTexture::getVertexDeclarationStatic(),
        capacity,BufferUsage::WriteOnly);
}

void BackroomsGame::RetireChunk(Chunk& chunk) {
    // Crossing one chunk boundary retires up to five chunks at once. A full
    // row of material buffers can be reused while their replacements upload.
    constexpr std::size_t kSpareBufferLimit=48;
    for (auto& buffer:chunk.vertices) {
        if (buffer && spareVertices_.size()<kSpareBufferLimit)
            spareVertices_.push_back(std::move(buffer));
    }
}

void BackroomsGame::BuildChunk(ChunkCoord coord) {
    const auto start = Clock::now();
    Meshes meshes;
    BakedLighting lighting(world_);
    Chunk chunk;
    const int ox=coord.x*kChunkCells, oz=coord.z*kChunkCells;
    const int level=world_.level;
    const auto& definition=LevelInfo(world_);
    const float height=definition.ceilingHeight;
    const float doorHeight=definition.doorwayHeight;
    const Material wallMat=level==0 ? Material::Wallpaper :
                           level==1 ? Material::ConcreteWall : Material::TunnelWall;
    const Material floorMat=level==0 ? Material::Carpet :
                            level==1 ? Material::ConcreteFloor : Material::TunnelFloor;
    const Material ceilingMat=level==0 ? Material::CeilingTile :
                              level==1 ? Material::IndustrialCeiling : Material::TunnelCeiling;
    const Color wallA=FromRgb(definition.wall),wallB=FromRgb(definition.pillar);
    const Color trim=FromRgb(definition.trim),floorA=FromRgb(definition.floor);
    const Color ceiling=FromRgb(definition.ceiling),grid=FromRgb(definition.structure);
    const Color lamp=FromRgb(definition.fluorescent);

    for (int lx=0; lx<kChunkCells; ++lx) for (int lz=0; lz<kChunkCells; ++lz) {
        const int gx=ox+lx, gz=oz+lz;
        const float x=lx*5.0f, z=lz*5.0f;
        const auto h=CellHash(world_,gx,gz,41);
        const bool chamber=IsServiceChamber(world_,gx,gz);
        const auto lampInfo=LampAt(world_,gx,gz);
        const Color floorColor=floorA;
        const double wx=gx*kCellSize,wz=gz*kCellSize;
        const std::array<float,4> light{{
            lighting.Sample(wx,wz),lighting.Sample(wx+5,wz),
            lighting.Sample(wx+5,wz+5),lighting.Sample(wx,wz+5)
        }};
        std::array<std::array<float,3>,3> lightGrid{};
        for (int ix=0;ix<3;++ix) for (int iz=0;iz<3;++iz) {
            if (ix==0 && iz==0) lightGrid[ix][iz]=light[0];
            else if (ix==2 && iz==0) lightGrid[ix][iz]=light[1];
            else if (ix==2 && iz==2) lightGrid[ix][iz]=light[2];
            else if (ix==0 && iz==2) lightGrid[ix][iz]=light[3];
            else lightGrid[ix][iz]=lighting.Sample(
                wx+ix*2.5,wz+iz*2.5);
        }
        for (int fx=0;fx<2;++fx) for (int fz=0;fz<2;++fz) {
            const float tileX=x+fx*2.5f,tileZ=z+fz*2.5f;
            FlatShaded(meshes,floorMat,tileX,tileZ,
                tileX+2.5f,tileZ+2.5f,0,
                {Scale(floorColor,lightGrid[fx][fz]),
                 Scale(floorColor,lightGrid[fx+1][fz]),
                 Scale(floorColor,lightGrid[fx+1][fz+1]),
                 Scale(floorColor,lightGrid[fx][fz+1])},
                level==0 ? kCarpetRepeat : 0.5f);
        }
        if (level==0) {
            const auto ceilingLight=[&](float factor) {
                const float bounce=definition.ceilingBounce;
                return Scale(ceiling,bounce+(1-bounce)*factor);
            };
            for (int fx=0;fx<2;++fx) for (int fz=0;fz<2;++fz) {
                const float tileX=x+fx*2.5f,tileZ=z+fz*2.5f;
                FlatShaded(meshes,ceilingMat,tileX,tileZ,
                    tileX+2.5f,tileZ+2.5f,height,
                    {ceilingLight(lightGrid[fx][fz]),
                     ceilingLight(lightGrid[fx+1][fz]),
                     ceilingLight(lightGrid[fx+1][fz+1]),
                    ceilingLight(lightGrid[fx][fz+1])},0.4f);
            }
            if (h%5==0) {
                const int tileX=static_cast<int>((h>>8)&7U);
                const int tileZ=static_cast<int>((h>>12)&7U);
                // Most of the grid stays clean; isolated tinted panels suggest
                // old water damage without laying props across every ceiling.
                if ((tileX<3 || tileX>4) || (tileZ<3 || tileZ>4)) {
                    const float px=x+tileX*0.625f;
                    const float pz=z+tileZ*0.625f;
                    const Color stain=(h%37==0) ?
                        Color(191,183,147) : Color(222,213,181);
                    const float tint=0.52f+0.48f*lighting.Sample(
                        wx+(tileX+0.5)*0.625,
                        wz+(tileZ+0.5)*0.625);
                    Flat(meshes,Material::CeilingTile,px,pz,
                         px+0.625f,pz+0.625f,height-0.008f,
                         Scale(stain,tint),0.4f);
                }
            }
        } else {
            const auto shade=[&](float light) {
                const float bounce=definition.ceilingBounce;
                return Scale(ceiling,bounce+(1-bounce)*light);
            };
            for (int fx=0;fx<2;++fx) for (int fz=0;fz<2;++fz) {
                const float px=x+fx*2.5f,pz=z+fz*2.5f;
                FlatShaded(meshes,Material::ConcreteCeiling,px,pz,px+2.5f,pz+2.5f,height,
                    {shade(lightGrid[fx][fz]),shade(lightGrid[fx+1][fz]),
                     shade(lightGrid[fx+1][fz+1]),shade(lightGrid[fx][fz+1])},0.2f);
            }
        }
        if (level==1) {
            const int rx=FloorDiv(gx,kRegionCells),rz=FloorDiv(gz,kRegionCells);
            const int lx=gx-rx*kRegionCells,lz=gz-rz*kRegionCells;
            const auto layout=CellHash(world_,rx,rz,2901);
            const int phase=static_cast<int>((layout>>4)%3);
            const bool alongX=(layout&1U)!=0;
            const int line=alongX ? lz : lx;
            const auto kind=RegionAt(world_,gx,gz);
            // Supported bays put the framing over their column centers;
            // enclosed utility rooms keep a separate region-wide phase.
            const bool beam=kind==RegionKind::Columns ? line==1 || line==4 :
                            kind==RegionKind::Storage ? line==2 || line==5 :
                            (line+phase)%3==0;
            if (beam) {
                if (alongX)
                    BoxRange(meshes,ceilingMat,x,z+2.40f,x+5,z+2.60f,
                             height-0.32f,height-0.10f,grid);
                else
                    BoxRange(meshes,ceilingMat,x+2.40f,z,x+2.60f,z+5,
                             height-0.32f,height-0.10f,grid);
            }
        } else if (level==2 && !chamber) {
            // Roof ledges meet real solid walls instead of tracing every cell
            // through open space. Their upper edge is attached to the ceiling.
            if (VerticalEdge(world_,gx,gz)==Edge::Solid)
                BoxRange(meshes,Material::TunnelWall,x+0.09f,z,x+0.26f,z+5,
                         height-0.22f,height,grid);
            if (VerticalEdge(world_,gx+1,gz)==Edge::Solid)
                BoxRange(meshes,Material::TunnelWall,x+4.74f,z,x+4.91f,z+5,
                         height-0.22f,height,grid);
            if (HorizontalEdge(world_,gx,gz)==Edge::Solid)
                BoxRange(meshes,Material::TunnelWall,x,z+0.09f,x+5,z+0.26f,
                         height-0.22f,height,grid);
            if (HorizontalEdge(world_,gx,gz+1)==Edge::Solid)
                BoxRange(meshes,Material::TunnelWall,x,z+4.74f,x+5,z+4.91f,
                         height-0.22f,height,grid);
        }
        if (level==0 && lampInfo.fixture) {
            const float halfX=lampInfo.longAxisX ? 0.625f : 0.3125f;
            const float halfZ=lampInfo.longAxisX ? 0.3125f : 0.625f;
            const float cx=x+lampInfo.x,cz=z+lampInfo.z;
            Flat(meshes,Material::IndustrialCeiling,
                 cx-halfX,cz-halfZ,cx+halfX,cz+halfZ,
                 height-0.022f,Color(247,240,223),0.8f);
            const Color panelColor=lampInfo.lit ?
                Color(255,254,244) : Color(137,137,122);
            LightPanel(meshes,cx-halfX+0.035f,cz-halfZ+0.035f,
                       cx+halfX-0.035f,cz+halfZ-0.035f,height-0.035f,
                       panelColor,lampInfo.longAxisX);
        } else if (level!=0 && lampInfo.fixture) {
            const float cx=x+lampInfo.x,cz=z+lampInfo.z;
            const float hx=lampInfo.longAxisX ? 1.20f : 0.23f;
            const float hz=lampInfo.longAxisX ? 0.23f : 1.20f;
            const float panelY=level==1 ? lampInfo.y : height-0.032f;
            if (level==1) {
                // Hanging fixtures clear the structural beam soffits.
                BoxRange(meshes,Material::GalvanizedMetal,cx-hx,cz-hz,cx+hx,cz+hz,
                         panelY+0.008f,panelY+0.08f,grid);
                for (float offset:{-0.85f,0.85f}) {
                    const float mx=cx+(lampInfo.longAxisX ? offset : 0);
                    const float mz=cz+(lampInfo.longAxisX ? 0 : offset);
                    BoxRange(meshes,Material::GalvanizedMetal,
                             mx-0.018f,mz-0.018f,mx+0.018f,mz+0.018f,
                             panelY+0.08f,height,grid);
                }
            } else
                Flat(meshes,ceilingMat,cx-hx,cz-hz,cx+hx,cz+hz,
                     height-0.024f,grid,0.8f);
            LightPanel(meshes,cx-hx+0.10f,cz-hz+0.07f,
                       cx+hx-0.10f,cz+hz-0.07f,panelY,
                       lampInfo.lit ? lamp : Color(106,115,102),lampInfo.longAxisX);
        }
        const Color cellWall=wallA;
        const auto wallLighting=[&](bool vertical) {
            WallLighting result{};
            for (int side=0;side<2;++side) {
                const int normal=side==0 ? -1 : 1;
                for (int sample=0;sample<3;++sample) {
                    const double px=wx+(vertical ? normal*0.14 : sample*2.5);
                    const double pz=wz+(vertical ? sample*2.5 : normal*0.14);
                    if (level==0) {
                        for (int row=0;row<3;++row)
                            result[side][sample][row]=lighting.WallSample(
                                px,pz,vertical ? normal : 0,vertical ? 0 : normal,
                                row*height*0.5);
                    } else result[side][sample].fill(lighting.WallSample(
                                px,pz,vertical ? normal : 0,vertical ? 0 : normal));
                }
            }
            return result;
        };
        const Edge verticalEdge=VerticalEdge(world_,gx,gz);
        const Edge horizontalEdge=HorizontalEdge(world_,gx,gz);
        const auto verticalOpening=OpeningForEdge(world_,verticalEdge,true,gx,gz);
        const auto horizontalOpening=OpeningForEdge(world_,horizontalEdge,false,gx,gz);
        if (verticalEdge!=Edge::Open)
            Partition(meshes,wallMat,verticalEdge,verticalOpening,true,x,z,
                      height,doorHeight,cellWall,trim,wallLighting(true));
        if (horizontalEdge!=Edge::Open)
            Partition(meshes,wallMat,horizontalEdge,horizontalOpening,false,z,x,
                      height,doorHeight,cellWall,trim,wallLighting(false));
        if (level==0) {
            ContactShadow(meshes,lighting,verticalEdge,verticalOpening,true,x,z,
                          ox*kCellSize,oz*kCellSize,floorColor);
            ContactShadow(meshes,lighting,horizontalEdge,horizontalOpening,false,z,x,
                          ox*kCellSize,oz*kCellSize,floorColor);
        }
        if (level==2) {
            const Color cabinet(133,139,126);
            const int regionX=FloorDiv(gx,kRegionCells);
            const int regionZ=FloorDiv(gz,kRegionCells);
            const auto pipeLayout=CellHash(world_,regionX,regionZ,3311);
            const int pipeStyle=static_cast<int>(pipeLayout%4);
            const auto serviceWall=[&](bool vertical, float boundary,
                                       bool positiveSide, std::uint32_t hash) {
                const float side=positiveSide ? 1.0f : -1.0f;
                const float start=vertical ? z : x;
                const float cross=boundary+side*0.21f;
                const bool hasCabinet=hash%13==0;
                const int edgeX=gx+(vertical && !positiveSide ? 1 : 0);
                const int edgeZ=gz+(!vertical && !positiveSide ? 1 : 0);
                const auto adjacent=[&](int offset) {
                    const int cellX=gx+(vertical ? 0 : offset);
                    const int cellZ=gz+(vertical ? offset : 0);
                    const int bx=edgeX+(vertical ? 0 : offset);
                    const int bz=edgeZ+(vertical ? offset : 0);
                    const auto nextLayout=CellHash(world_,FloorDiv(cellX,kRegionCells),
                                                 FloorDiv(cellZ,kRegionCells),3311);
                    const Edge nextEdge=vertical ? VerticalEdge(world_,bx,bz) :
                                                   HorizontalEdge(world_,bx,bz);
                    return std::pair{nextEdge==Edge::Solid &&
                                     nextLayout%4==static_cast<unsigned>(pipeStyle),nextLayout};
                };
                const auto previous=adjacent(-1),next=adjacent(1);
                const auto pipe=[&](float pipeHeight,float radius,Color color) {
                    const float illumination=lighting.WallSample(
                        wx+(vertical ? cross-x : 2.5f),
                        wz+(vertical ? 2.5f : cross-z),
                        vertical ? static_cast<int>(side) : 0,
                        vertical ? 0 : static_cast<int>(side));
                    color=Scale(color,illumination);
                    const auto run=[&](float from,float to,float r,Color tint) {
                        PipeRun(meshes,vertical,cross,from,to,pipeHeight,r,tint,
                                Material::GalvanizedMetal);
                    };
                    const auto continues=[&](const auto& neighbor) {
                        if (!neighbor.first) return false;
                        const bool optional=pipeStyle==3 || (pipeStyle==0 && pipeHeight<1.0f);
                        return !optional || neighbor.second%3!=0;
                    };
                    const bool before=continues(previous),after=continues(next);
                    const float from=start+(before ? 0.0f : 0.38f);
                    const float to=start+5.0f-(after ? 0.0f : 0.38f);
                    if (hasCabinet && pipeHeight+radius>1.03f &&
                        pipeHeight-radius<1.66f) {
                        run(from,start+1.90f,radius,color);
                        run(start+2.68f,to,radius,color);
                    } else run(from,to,radius,color);
                    // Neighbor queries use global edges, so a straight run is
                    // continuous across chunks. Only real terminations bend
                    // through the wall, without jutting into the opening.
                    if (!before) PipeWallReturn(meshes,vertical,cross,from,side,-1,
                                                pipeHeight,radius,color);
                    if (!after) PipeWallReturn(meshes,vertical,cross,to,side,1,
                                               pipeHeight,radius,color);
                    for (const float along:{start+0.40f,start+4.30f}) {
                        run(along-0.025f,along+0.025f,radius*1.18f,Scale(color,0.84f));
                        const float wall=boundary+side*0.105f;
                        const float a=std::min(wall,cross),b=std::max(wall,cross);
                        if (vertical)
                            BoxRange(meshes,Material::GalvanizedMetal,a,along-0.028f,b,along+0.028f,
                                     pipeHeight-radius-0.045f,pipeHeight-radius+0.015f,
                                     Scale(color,0.75f));
                        else
                            BoxRange(meshes,Material::GalvanizedMetal,along-0.028f,a,along+0.028f,b,
                                     pipeHeight-radius-0.045f,pipeHeight-radius+0.015f,
                                     Scale(color,0.75f));
                    }
                };
                if (pipeStyle==0) {
                    pipe(1.95f,0.075f,Color(225,218,193));
                    if (pipeLayout%3!=0) pipe(0.89f,0.055f,Color(179,186,164));
                } else if (pipeStyle==1) {
                    pipe(1.28f,0.13f,Color(201,212,193));
                    pipe(2.03f,0.05f,Color(223,211,185));
                } else if (pipeStyle==2) {
                    pipe(2.04f,0.065f,Color(211,165,122));
                    pipe(1.77f,0.055f,Color(192,158,122));
                } else if (pipeLayout%3!=0) {
                    pipe(1.70f,0.085f,Color(167,192,176));
                    pipe(0.55f,0.060f,Color(194,199,172));
                }
                if (hasCabinet) {
                    const float along=(vertical ? z : x)+2.0f;
                    const float depth=boundary+side*0.16f;
                    if (vertical)
                        BoxRange(meshes,Material::IndustrialCeiling,
                                 depth-0.075f,along,depth+0.075f,along+0.58f,
                                 1.03f,1.66f,cabinet);
                    else
                        BoxRange(meshes,Material::IndustrialCeiling,
                                 along,depth-0.075f,along+0.58f,depth+0.075f,
                                 1.03f,1.66f,cabinet);
                    const float front=depth+side*0.084f;
                    WallFace(meshes,Material::IndustrialCeiling,
                             vertical,front,along+0.055f,along+0.525f,
                             1.08f,1.61f,Color(91,103,97));
                    WallFace(meshes,Material::IndustrialCeiling,
                             vertical,front+side*0.006f,
                             along+0.425f,along+0.485f,
                             1.29f,1.37f,Color(197,186,151));
                }
            };
            if (verticalEdge==Edge::Solid)
                serviceWall(true,x,true,CellHash(world_,gx,gz,2701));
            if (VerticalEdge(world_,gx+1,gz)==Edge::Solid)
                serviceWall(true,x+5,false,CellHash(world_,gx+1,gz,2701));
            if (horizontalEdge==Edge::Solid)
                serviceWall(false,z,true,CellHash(world_,gx,gz,2707));
            if (HorizontalEdge(world_,gx,gz+1)==Edge::Solid)
                serviceWall(false,z+5,false,CellHash(world_,gx,gz+1,2707));
            if (h%11==0) {
                const Color duct(107,111,104);
                BoxRange(meshes,Material::IndustrialCeiling,
                         x+0.42f,z+0.73f,x+4.58f,z+1.12f,
                         height-0.45f,height-0.17f,duct);
                for (const float support:{x+1.0f,x+4.0f})
                    BoxRange(meshes,Material::GalvanizedMetal,
                             support-0.025f,z+0.89f,support+0.025f,z+0.96f,
                             height-0.17f,height,Scale(duct,1.2f));
                Flat(meshes,Material::IndustrialCeiling,
                     x+0.42f,z+0.73f,x+4.58f,z+1.12f,
                     height-0.45f,Scale(duct,0.75f),0.5f);
            }
        }
        const auto prop=PropAt(world_,gx,gz);
        const auto obstacles=CellObstacles(world_,gx,gz);
        const int structuralCount=obstacles.count-
            static_cast<int>(prop.kind!=PropKind::None);
        const int fullHeightCount=FullHeightObstaclesAt(world_,gx,gz).count;
        for (int obstacleId=0;obstacleId<structuralCount;++obstacleId) {
            const auto& obstacle=obstacles.walls[obstacleId];
            const float bx0=static_cast<float>(obstacle.minX-ox*kCellSize);
            const float bz0=static_cast<float>(obstacle.minZ-oz*kCellSize);
            const float bx1=static_cast<float>(obstacle.maxX-ox*kCellSize);
            const float bz1=static_cast<float>(obstacle.maxZ-oz*kCellSize);
            if (level==0) {
                OfficeObstacle(meshes,lighting,bx0,bz0,bx1,bz1,
                               ox*kCellSize,oz*kCellSize,height,
                               wallB,trim,floorColor);
            } else if (level==1) {
                if (obstacleId<fullHeightCount)
                    StructuralColumn(meshes,lighting,bx0,bz0,bx1,bz1,
                                     ox*kCellSize,oz*kCellSize,height,
                                     Scale(wallB,1.20f),trim,floorColor,floorMat);
                else StorageRack(meshes,bx0,bz0,bx1,bz1,
                                 CellHash(world_,gx,gz,3413+obstacleId));
            } else if (obstacleId<fullHeightCount) {
                StructuralColumn(meshes,lighting,bx0,bz0,bx1,bz1,
                                 ox*kCellSize,oz*kCellSize,height,
                                 wallB,trim,floorColor,floorMat);
            } else {
                const bool pressureRack=CellHash(world_,FloorDiv(gx,kRegionCells),
                                                 FloorDiv(gz,kRegionCells),3931)%2==0;
                UtilityEquipment(meshes,lighting,bx0,bz0,bx1,bz1,
                                 x+2.5f,z+2.5f,ox*kCellSize,oz*kCellSize,
                                 height-0.23f,floorColor,pressureRack,UtilityAlongZAt(world_,gx,gz));
            }
        }
        if (prop.kind!=PropKind::None)
            Furniture(meshes,prop,ox*kCellSize,oz*kCellSize,level,
                      0.30f+0.70f*lighting.Sample(prop.x,prop.z),h);
        if (level==0 && (prop.kind==PropKind::Chair ||
                        prop.kind==PropKind::EmbeddedChair ||
                        prop.kind==PropKind::Table)) {
            const auto bounds=PropBounds(prop);
            const float sx0=static_cast<float>(bounds.minX-ox*kCellSize);
            const float sz0=static_cast<float>(bounds.minZ-oz*kCellSize);
            const float sx1=static_cast<float>(bounds.maxX-ox*kCellSize);
            const float sz1=static_cast<float>(bounds.maxZ-oz*kCellSize);
            FloorContactShadow(meshes,lighting,sx0,sz0,sx1,sz1,
                                ox*kCellSize,oz*kCellSize,floorColor,0.86f);
        }
        const auto alcove=OfficeAlcoveAt(world_,gx,gz);
        if (alcove)
            OfficeAlcoveVisual(meshes,lighting,*alcove,ox*kCellSize,oz*kCellSize,
                               height,wallA,ceiling);
        const auto doorHash=CellHash(world_,gx,gz,1501);
        if (!alcove && doorHash%170==0) {
            if ((doorHash&1U)==0 && VerticalEdge(world_,gx,gz)==Edge::Solid)
                FalseDoor(meshes,true,x,z,level,level<=1 ?
                    lighting.WallSample(wx+0.14,wz+2.5,1,0) : 1.0f,1,height);
            else if (HorizontalEdge(world_,gx,gz)==Edge::Solid)
                FalseDoor(meshes,false,z,x,level,level<=1 ?
                    lighting.WallSample(wx+2.5,wz+0.14,0,1) : 1.0f,1,height);
        }
        if (const auto portal=PortalAt(world_,gx,gz))
            PortalVisual(meshes,lighting,world_,*portal,ox*kCellSize,oz*kCellSize,
                         height,wallA,trim,floorColor);
        if (const auto entity=EntityAt(world_,gx,gz))
            chunk.entities.push_back(*entity);
    }
    bool firstVertex=true;
    for (const auto& mesh:meshes) for (const auto& vertex:mesh) {
        const auto& point=vertex.Position;
        if (firstVertex) {
            chunk.bounds=BoundingBox(point,point);
            firstVertex=false;
        } else {
            // Avoid copying six native math objects for every generated vertex.
            auto& low=chunk.bounds.Min;
            auto& high=chunk.bounds.Max;
            low.X=std::min(low.X,point.X);high.X=std::max(high.X,point.X);
            low.Y=std::min(low.Y,point.Y);high.Y=std::max(high.Y,point.Y);
            low.Z=std::min(low.Z,point.Z);high.Z=std::max(high.Z,point.Z);
        }
    }
    const auto geometryEnd=Clock::now();
    getGraphicsDeviceProperty().SetVertexBuffer(nullptr);
    for (int id=0;id<kMaterialCount;++id) {
        auto& mesh=meshes[id];
        if (mesh.empty()) continue;
        chunk.materialTriangles[id]=static_cast<int>(mesh.size()/3);
        chunk.triangles+=chunk.materialTriangles[id];
        chunk.vertices[id]=AcquireBuffer(static_cast<int>(mesh.size()));
        chunk.vertices[id]->SetData(mesh.data(),static_cast<int>(mesh.size()));
    }
    chunk.buildMs = std::chrono::duration<double,std::milli>(Clock::now()-start).count();
    if (chunk.buildMs>16.67) {
        const double cpuMs=std::chrono::duration<double,std::milli>(geometryEnd-start).count();
        std::cerr << "Chunk build spike " << coord.x << ',' << coord.z
                  << " level " << world_.level << ": " << chunk.buildMs
                  << " ms (geometry " << cpuMs << ", upload "
                  << chunk.buildMs-cpuMs << ")\n";
    }
    lastBuildMs_ = chunk.buildMs;
    peakBuildMs_ = std::max(peakBuildMs_,chunk.buildMs);
    chunks_.emplace(coord,std::move(chunk));
}

void BackroomsGame::Stream() {
    // CNA EasyGL currently does not acquire a context for SetData on existing
    // vertex buffers during Update. This also covers retiring GPU resources.
    // See bugs.md; no sibling engine changes are required.
    auto context=getGraphicsDeviceProperty().GetRenderer().AcquireThreadContextLeaseEXT();
    const auto center=ChunkAt(x_,z_);
    for (auto it=chunks_.begin(); it!=chunks_.end();) {
        if (std::abs(it->first.x-center.x)>2 || std::abs(it->first.z-center.z)>2) {
            RetireChunk(it->second);
            it=chunks_.erase(it);
        } else ++it;
    }
    // One upload per update bounds a frame's streaming work. Nearest missing chunk first.
    int best=100;
    ChunkCoord next{};
    bool found=false;
    for (int dx=-2; dx<=2; ++dx) for (int dz=-2; dz<=2; ++dz) {
        ChunkCoord candidate{center.x+dx,center.z+dz};
        const int distance=dx*dx+dz*dz;
        if (!chunks_.contains(candidate) && distance<best) {
            next=candidate; best=distance; found=true;
        }
    }
    if (found) BuildChunk(next);
    if (chunks_.size()>25) throw std::runtime_error("streaming exceeded chunk limit");
}

void BackroomsGame::Transition(int level) {
    if (transition_ && !transition_->Play(0.45f,0,0))
        std::cerr << "Audio: transition cue could not acquire a voice\n";
    auto context=getGraphicsDeviceProperty().GetRenderer().AcquireThreadContextLeaseEXT();
    world_.level=level;
    chunks_.clear();
    spareVertices_.clear();
    x_=2.5; z_=2.5;
    yaw_=level==2 ? 0.0f : 1.5707963f;
    pitch_=0;
    stepDistance_=0;
    BuildChunk({0,0});
}

void BackroomsGame::UpdateTitle(double elapsed) {
    statsTime_+=elapsed;
    ++updateCount_;
    if (statsTime_<1.0) return;
    auto sorted=drawIntervals_;
    std::sort(sorted.begin(),sorted.begin()+drawIntervalCount_);
    double total=0;
    for (std::size_t i=0;i<drawIntervalCount_;++i) total+=sorted[i];
    const double fps=total>0 ? 1000.0*drawIntervalCount_/total : 0;
    const double p95=drawIntervalCount_ ?
        sorted[(drawIntervalCount_*95+99)/100-1] : 0;
    const double maximum=drawIntervalCount_ ? sorted[drawIntervalCount_-1] : 0;
    int triangles=0,entities=0;
    std::size_t vertexCapacity=0;
    for (const auto& [coord,chunk]:chunks_) {
        (void)coord;
        triangles+=chunk.triangles;
        entities+=static_cast<int>(chunk.entities.size());
        for (const auto& buffer:chunk.vertices)
            if (buffer) vertexCapacity+=buffer->getVertexCountProperty();
    }
    for (const auto& buffer:spareVertices_)
        vertexCapacity+=buffer->getVertexCountProperty();
    if (entityVertices_) vertexCapacity+=entityVertices_->getVertexCountProperty();
    if (entityShadowVertices_) vertexCapacity+=entityShadowVertices_->getVertexCountProperty();
    const double bufferMiB=vertexCapacity*
        VertexPositionColorTexture::getVertexDeclarationStatic().getVertexStrideProperty()/1048576.0;
    const auto here=ChunkAt(x_,z_);
    std::ostringstream title;
    title << "cna-backrooms | Level " << world_.level << " (" << LevelInfo(world_).name
          << ") | seed " << world_.seed
          << " | pos " << std::fixed << std::setprecision(1) << x_ << ',' << z_
          << " | view " << std::remainder(yaw_*57.2957795f,360.0f)
          << ',' << pitch_*57.2957795f
          << " | chunk " << here.x << ',' << here.z
          << " | loaded " << chunks_.size() << "/25 | tris " << triangles
          << " | draw " << drawnChunks_ << "/25 " << drawnTriangles_ << " tris"
          << " | VBO " << bufferCreations_ << '/' << bufferReuses_
          << " pool " << spareVertices_.size() << " gpu " << bufferMiB << " MiB"
          << " | rooms " << roomLayouts_.Size()
          << " | entities " << entities
          << " | nearest " << std::setprecision(2) << nearestEntityDistance_
          << '/' << nearestEntityOpacity_
          << " | build " << std::setprecision(2) << lastBuildMs_ << " ms"
          << " | peak " << peakBuildMs_ << " ms"
          << " | " << (running_ ? "run" : "walk")
          << " | audio " << (hum_ ? "on" : "off")
          << " | " << static_cast<int>(fps+0.5) << " FPS"
          << " | draw p95/max " << p95 << '/' << maximum << " ms"
          << " | submit " << drawWorkMs_ << " ms"
          << " | " << static_cast<int>(updateCount_/statsTime_) << " UPS";
    getWindowProperty().setTitleProperty(title.str());
    if (streamTest_) std::cout << title.str() << '\n';
    statsTime_=0;
    updateCount_=0;
}

void BackroomsGame::Update(GameTime& time) {
    const double dt=std::min(0.1,time.getElapsedGameTimeProperty().getTotalSecondsProperty());
    if (streamTest_) {
        // Streaming diagnostic: north, south past origin, and back through the
        // live renderer. Collision is bypassed so walls cannot stop the sweep.
        streamTestTime_+=dt;
        const double distance=streamTestTime_*45.0;
        const double leg=streamTestMetres_*0.25;
        z_=2.5+(distance<leg ? distance :
                 distance<3.0*leg ? 2.0*leg-distance :
                 distance-4.0*leg);
        Stream();
        UpdateTitle(dt);
        if (distance>=streamTestMetres_) Exit();
        Game::Update(time);
        return;
    }
    const auto keys=Keyboard::GetState();
    const bool shiftDown=keys.IsKeyDown(Keys::LeftShift) ||
                         keys.IsKeyDown(Keys::RightShift);
    const bool wasShiftDown=previousKeys_.IsKeyDown(Keys::LeftShift) ||
                            previousKeys_.IsKeyDown(Keys::RightShift);
    if (shiftDown && !wasShiftDown)
        running_=!running_;
    if (keys.IsKeyDown(Keys::Escape) && previousKeys_.IsKeyUp(Keys::Escape)) {
        if (captured_) {
            captured_=false;
            Mouse::setIsRelativeMouseModeEXTProperty(false);
            setIsMouseVisibleProperty(true);
        } else Exit();
    }
    auto mouse=Mouse::GetState();
    if (!captured_ && mouse.getLeftButtonProperty()==ButtonState::Pressed) {
        captured_=true;
        Mouse::setIsRelativeMouseModeEXTProperty(true);
        setIsMouseVisibleProperty(false);
    }
    if (captured_ && getIsActiveProperty()) {
        // CNA/SDL reports positive relative X for physical movement to the right.
        // With this camera's +Z forward convention, decreasing yaw looks right.
        yaw_ -= mouse.getXProperty()*0.0022f;
        pitch_=std::clamp(pitch_-mouse.getYProperty()*0.0022f,-1.43f,1.43f);
        double forward=(keys.IsKeyDown(Keys::W)?1.0:0.0)-(keys.IsKeyDown(Keys::S)?1.0:0.0);
        double strafe=(keys.IsKeyDown(Keys::D)?1.0:0.0)-(keys.IsKeyDown(Keys::A)?1.0:0.0);
        const double length=std::hypot(forward,strafe);
        if (length>0) { forward/=length; strafe/=length; }
        const double speed=(running_?runSpeed_:walkSpeed_)*dt;
        const double dx=(std::sin(yaw_)*forward-std::cos(yaw_)*strafe)*speed;
        const double dz=(std::cos(yaw_)*forward+std::sin(yaw_)*strafe)*speed;
        const double oldX=x_, oldZ=z_;
        MoveWithCollision(world_,x_,z_,dx,dz,0.31);
        stepDistance_ += std::hypot(x_-oldX,z_-oldZ);
        const double stepLength=running_?1.5:1.05;
        if (stepDistance_>stepLength) {
            stepDistance_-=stepLength;
            auto* sound=world_.level==0 ? carpetStep_.get() : step_.get();
            constexpr std::array<float,4> pitches{{-0.025f,0.016f,-0.009f,0.030f}};
            const float pitch=pitches[stepCount_++%pitches.size()];
            const float volume=world_.level==0 ? 0.62f : 0.74f;
            if (sound && !sound->Play(volume,pitch,0) && !stepWarningShown_) {
                std::cerr << "Audio: footstep could not acquire a voice\n";
                stepWarningShown_=true;
            }
        }
    }
    if (keys.IsKeyDown(Keys::R) && previousKeys_.IsKeyUp(Keys::R)) {
        x_=2.5; z_=2.5;
        yaw_=world_.level==2 ? 0.0f : 1.5707963f;
        pitch_=0;
        stepDistance_=0;
    }
    const auto portal=PortalTarget(world_,x_,z_);
    if (portal && !insidePortal_) Transition(*portal);
    insidePortal_=portal.has_value();
    Stream();
    UpdateTitle(dt);
    previousKeys_=keys;
    Game::Update(time);
}

void BackroomsGame::RecordDrawInterval() {
    const auto now=Clock::now();
    if (lastDraw_!=Clock::time_point{}) {
        drawIntervals_[nextDrawInterval_]=
            std::chrono::duration<double,std::milli>(now-lastDraw_).count();
        nextDrawInterval_=(nextDrawInterval_+1)%drawIntervals_.size();
        drawIntervalCount_=std::min(drawIntervalCount_+1,drawIntervals_.size());
    }
    lastDraw_=now;
}

void BackroomsGame::Draw(const GameTime& time) {
    // Start-to-start intervals include streaming, frame submission and present.
    // A fixed update rate alone can hide render stalls and skipped draws.
    RecordDrawInterval();
    const auto drawStart=Clock::now();
    auto& device=getGraphicsDeviceProperty();
    const Color fog=FromRgb(LevelInfo(world_).fog);
    device.Clear(fog);
    device.setDepthStencilStateProperty(DepthStencilState::Default);
    device.setBlendStateProperty(BlendState::Opaque);
    device.setRasterizerStateProperty(RasterizerState::CullNone);
    device.getSamplerStatesProperty()[0]=SamplerState::AnisotropicWrap;
    effect_->setFogColorProperty(fog.ToVector3());
    effect_->setFogStartProperty(LevelInfo(world_).fogStart);
    effect_->setFogEndProperty(LevelInfo(world_).fogEnd);
    const Vector3 eye(0,1.68f,0);
    const Vector3 direction(std::sin(yaw_)*std::cos(pitch_),std::sin(pitch_),
                            std::cos(yaw_)*std::cos(pitch_));
    effect_->setViewProperty(Matrix::CreateLookAt(eye,eye+direction,Vector3::Up));
    effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
        verticalFovDegrees_*0.01745329252f,
        device.getViewportProperty().getAspectRatioProperty(),0.08f,105.0f));
    const BoundingFrustum frustum(effect_->getViewProperty()*effect_->getProjectionProperty());
    drawnChunks_=drawnTriangles_=0;
    for (const auto& [coord,chunk]:chunks_) {
        const Vector3 offset(static_cast<float>(coord.x*kChunkSize-x_),0,
                             static_cast<float>(coord.z*kChunkSize-z_));
        if (!frustum.Intersects(BoundingBox(chunk.bounds.Min+offset,
                                           chunk.bounds.Max+offset))) continue;
        ++drawnChunks_;
        drawnTriangles_+=chunk.triangles;
        effect_->setWorldProperty(Matrix::CreateTranslation(offset));
        for (int id=0;id<kMaterialCount;++id) {
            if (!chunk.vertices[id]) continue;
            effect_->setTextureProperty(materials_->Get(static_cast<Material>(id)));
            device.SetVertexBuffer(chunk.vertices[id].get());
            auto& passes=effect_->getCurrentTechniqueProperty()->getPassesProperty();
            for (int i=0; i<passes.getCountProperty(); ++i) {
                passes[i]->Apply();
                device.DrawPrimitives(PrimitiveType::TriangleList,0,
                                      chunk.materialTriangles[id]);
            }
        }
    }
    effect_->setTextureProperty(materials_->Get(Material::Upholstery));
    device.SetVertexBuffer(entityVertices_.get());
    const float seconds=static_cast<float>(
        time.getTotalGameTimeProperty().getTotalSecondsProperty());
    nearestEntityDistance_=0;
    nearestEntityOpacity_=0;
    bool haveNearestEntity=false;
    for (const auto& [coord,chunk]:chunks_) {
        (void)coord;
        for (const auto& entity:chunk.entities) {
            const double ex=entity.x+0.55*std::sin(seconds*0.28f+entity.phase);
            const double ez=entity.z+0.55*std::cos(seconds*0.21f+entity.phase);
            const double distance=std::hypot(ex-x_,ez-z_);
            // Retreat gradually as the player approaches. No mesh can surround
            // the camera, and there is no interaction or physical obstacle.
            const float proximity=static_cast<float>(std::clamp((distance-1.5)/3.5,0.0,1.0));
            const float opacity=proximity*proximity*(3-2*proximity);
            if (!haveNearestEntity || distance<nearestEntityDistance_) {
                haveNearestEntity=true;
                nearestEntityDistance_=distance;
                nearestEntityOpacity_=opacity;
            }
            if (opacity<=0 || distance>82.0) continue;
            const float facing=static_cast<float>(std::atan2(x_-ex,z_-ez));
            effect_->setWorldProperty(Matrix::CreateRotationY(facing)*
                Matrix::CreateTranslation(static_cast<float>(ex-x_),0,
                                          static_cast<float>(ez-z_)));
            if (distance<20) {
                // Two soft contact spots follow the same pose as the feet.
                // Native blending reads depth without writing into the floor.
                effect_->setTextureEnabledProperty(false);
                effect_->setFogEnabledProperty(false);
                effect_->setAlphaProperty(opacity*static_cast<float>(1-distance/20));
                device.setBlendStateProperty(BlendState::AlphaBlend);
                device.setDepthStencilStateProperty(DepthStencilState::DepthRead);
                device.SetVertexBuffer(entityShadowVertices_.get());
                auto& passes=effect_->getCurrentTechniqueProperty()->getPassesProperty();
                for (int i=0;i<passes.getCountProperty();++i) {
                    passes[i]->Apply();
                    device.DrawPrimitives(PrimitiveType::TriangleList,0,entityShadowTriangles_);
                }
                effect_->setTextureEnabledProperty(true);
                effect_->setFogEnabledProperty(true);
                effect_->setAlphaProperty(1);
                device.setBlendStateProperty(BlendState::Opaque);
                device.setDepthStencilStateProperty(DepthStencilState::Default);
            }
            device.SetVertexBuffer(entityVertices_.get());
            auto& passes=effect_->getCurrentTechniqueProperty()->getPassesProperty();
            if (opacity<1) {
                // Resolve the nearest cloth surface before blending. This
                // avoids seeing overlapping back faces through the body and
                // needs only CNA's native color-write mask and depth states.
                device.setBlendStateProperty(entityDepthOnlyBlend_);
                for (int i=0;i<passes.getCountProperty();++i) {
                    passes[i]->Apply();
                    device.DrawPrimitives(PrimitiveType::TriangleList,0,entityTriangles_);
                }
                device.setBlendStateProperty(BlendState::AlphaBlend);
                device.setDepthStencilStateProperty(DepthStencilState::DepthRead);
                effect_->setAlphaProperty(opacity);
            }
            for (int i=0;i<passes.getCountProperty();++i) {
                passes[i]->Apply();
                device.DrawPrimitives(PrimitiveType::TriangleList,0,entityTriangles_);
            }
            effect_->setAlphaProperty(1);
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
        }
    }
    Game::Draw(time);
    drawWorkMs_=std::chrono::duration<double,std::milli>(Clock::now()-drawStart).count();
}

} // namespace Backrooms
