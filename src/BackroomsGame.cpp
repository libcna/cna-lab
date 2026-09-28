#include "BackroomsGame.hpp"
#include "Assets.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <tuple>
#include <vector>

#include "CNA/Internal/Renderers/Common/IGraphicsRenderer.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPassCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
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

Color Scale(Color color, float factor) {
    const auto scaled=[&](int channel) {
        return static_cast<std::uint8_t>(std::clamp(
            static_cast<int>(std::lround(channel*factor)),0,255));
    };
    return Color(scaled(color.getRProperty()),scaled(color.getGProperty()),
                 scaled(color.getBProperty()));
}

void WallFace(Meshes& meshes, Material material, bool vertical, float boundary,
              float a, float b, float y0, float y1, Color color,
              float lightStart=1.0f, float lightEnd=1.0f) {
    const float repeat=material==Material::Wallpaper ? 0.8f : 0.6f;
    const float u0=a*repeat,u1=b*repeat;
    const float v0=1.0f-y0*repeat,v1=1.0f-y1*repeat;
    const auto shade=[&](float height) {
        if (material==Material::Wallpaper)
            return 0.83f+0.17f*std::clamp(height/3.0f,0.0f,1.0f);
        if (material==Material::ConcreteWall)
            return 0.90f+0.10f*std::clamp(height/4.1f,0.0f,1.0f);
        return 1.0f;
    };
    const Color bottomStart=Scale(color,shade(y0)*lightStart);
    const Color bottomEnd=Scale(color,shade(y0)*lightEnd);
    const Color topStart=Scale(color,shade(y1)*lightStart);
    const Color topEnd=Scale(color,shade(y1)*lightEnd);
    if (vertical)
        QuadColors(meshes,material,{boundary,y0,a},{boundary,y0,b},
                   {boundary,y1,b},{boundary,y1,a},
                   {bottomStart,bottomEnd,topEnd,topStart},
                   {u0,v0},{u1,v0},{u1,v1},{u0,v1});
    else
        QuadColors(meshes,material,{a,y0,boundary},{b,y0,boundary},
                   {b,y1,boundary},{a,y1,boundary},
                   {bottomStart,bottomEnd,topEnd,topStart},
                   {u0,v0},{u1,v0},{u1,v1},{u0,v1});
}

using WallLighting=std::array<std::array<float,3>,2>;

void Partition(Meshes& meshes, Material material, Edge edge,
               OpeningSpan opening, bool vertical,
               float boundary, float along, float height, float doorHeight,
               Color wall, Color trim, const WallLighting& lights) {
    if (edge==Edge::Open) return;
    const bool thick=material==Material::Wallpaper;
    const auto lightAt=[&](float coordinate,int side) {
        const float t=std::clamp((coordinate-along)/5.0f,0.0f,1.0f);
        const auto& l=lights[side];
        return t<=0.5f ? l[0]+(l[1]-l[0])*2.0f*t :
                         l[1]+(l[2]-l[1])*(2.0f*t-1.0f);
    };
    const auto section=[&](float a,float b,float y0,float y1,Color tint,bool painted=false) {
        for (int side=0;side<(thick ? 2 : 1);++side) {
            const float face=boundary+(thick ? (side==0 ? -kWallHalfThickness : kWallHalfThickness) : 0);
            const auto draw=[&](float from,float to) {
                WallFace(meshes,painted ? Material::PaintedTrim : material,
                         vertical,face,from,to,y0,y1,tint,lightAt(from,side),lightAt(to,side));
            };
            const float middle=along+2.5f;
            if (thick && a<middle && b>middle) { draw(a,middle);draw(middle,b); }
            else draw(a,b);
        }
    };
    const auto full=[&](float a,float b) {
        const float base=thick ? 0.12f : 0.19f;
        section(a,b,0,base,trim,thick);
        section(a,b,base,thick ? height : height-0.10f,wall);
        if (!thick) section(a,b,height-0.10f,height,trim);
    };
    if (edge==Edge::Solid) full(along,along+5);
    else {
        const float first=static_cast<float>(opening.start);
        const float last=static_cast<float>(opening.end);
        full(along,along+first);full(along+last,along+5);
        const auto cap=[&](float end) {
            const float light=(lightAt(end,0)+lightAt(end,1))*0.5f;
            const float base=thick ? 0.12f : 0.19f;
            WallFace(meshes,material,!vertical,end,
                     boundary-kWallHalfThickness,boundary+kWallHalfThickness,base,
                     thick ? height : height-0.10f,Scale(wall,0.82f),light,light);
            WallFace(meshes,thick ? Material::PaintedTrim : material,!vertical,end,
                     boundary-kWallHalfThickness,boundary+kWallHalfThickness,0,base,trim,light,light);
            if (!thick)
                WallFace(meshes,material,!vertical,end,
                         boundary-kWallHalfThickness,boundary+kWallHalfThickness,height-0.10f,height,
                         trim,light,light);
        };
        cap(along+first);cap(along+last);
        if (edge==Edge::Door) {
            section(along+first,along+last,doorHeight,height,wall);
            if (vertical)
                Flat(meshes,material,boundary-kWallHalfThickness,along+first,
                     boundary+kWallHalfThickness,along+last,doorHeight,Scale(wall,0.83f),0.6f);
            else
                Flat(meshes,material,along+first,boundary-kWallHalfThickness,
                     along+last,boundary+kWallHalfThickness,doorHeight,Scale(wall,0.83f),0.6f);
        }
    }
}

void ContactShadow(Meshes& meshes, Edge edge, OpeningSpan opening,
                   bool vertical, float boundary, float along,
                   Color floorColor) {
    if (edge==Edge::Open) return;
    const Color outer=floorColor;
    const Color inner=Scale(floorColor,0.84f);
    const auto section=[&](float start,float end) {
        if (end<=start) return;
        const float a=along+start,b=along+end;
        constexpr float inset=0.105f,outset=0.34f,y=0.004f;
        if (vertical) {
            FlatShaded(meshes,Material::Carpet,boundary-outset,a,
                       boundary-inset,b,y,
                       {outer,inner,inner,outer},0.25f);
            FlatShaded(meshes,Material::Carpet,boundary+inset,a,
                       boundary+outset,b,y,
                       {inner,outer,outer,inner},0.25f);
        } else {
            FlatShaded(meshes,Material::Carpet,a,boundary-outset,
                       b,boundary-inset,y,
                       {outer,outer,inner,inner},0.25f);
            FlatShaded(meshes,Material::Carpet,a,boundary+inset,
                       b,boundary+outset,y,
                       {inner,inner,outer,outer},0.25f);
        }
    };
    if (edge==Edge::Solid) section(0,5);
    else {
        section(0,static_cast<float>(opening.start));
        section(static_cast<float>(opening.end),5);
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
             float end, float height, float radius, Color color) {
    constexpr int facets=6;
    constexpr float turn=6.28318530718f;
    for (int facet=0;facet<facets;++facet) {
        const float a=turn*facet/facets,b=turn*(facet+1)/facets;
        const float ha=std::cos(a)*radius,hb=std::cos(b)*radius;
        const float ya=height+std::sin(a)*radius;
        const float yb=height+std::sin(b)*radius;
        const float shade=0.77f+0.23f*std::max(0.0f,
                           std::sin((a+b)*0.5f));
        const Color face=Scale(color,shade);
        const Vector3 p0=alongZ ? Vector3(cross+ha,ya,start) :
                                  Vector3(start,ya,cross+ha);
        const Vector3 p1=alongZ ? Vector3(cross+ha,ya,end) :
                                  Vector3(end,ya,cross+ha);
        const Vector3 p2=alongZ ? Vector3(cross+hb,yb,end) :
                                  Vector3(end,yb,cross+hb);
        const Vector3 p3=alongZ ? Vector3(cross+hb,yb,start) :
                                  Vector3(start,yb,cross+hb);
        Quad(meshes,Material::IndustrialCeiling,p0,p1,p2,p3,face,
             {start*0.3f,0.0f},{end*0.3f,0.0f},
             {end*0.3f,1.0f},{start*0.3f,1.0f});
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
               int level, float illumination) {
    const float height=level==1 ? 2.58f : level==2 ? 2.15f : 2.18f;
    const Material material=level==0 ? Material::Wood : Material::ConcreteWall;
    const Color panel=Scale(level==0 ? Color(190,185,163) : Color(85,96,94),illumination);
    const Color frame=Scale(level==0 ? Color(227,217,188) : Color(126,136,130),illumination);
    const float left=along+2.04f,right=along+2.96f;
    const float face=boundary+(level==0 ? kWallHalfThickness+0.005f : 0.045f);
    WallFace(meshes,material,vertical,face,left,right,0.02f,height,panel);
    WallFace(meshes,material,vertical,face+0.004f,left-0.075f,left,
             0,height+0.08f,frame);
    WallFace(meshes,material,vertical,face+0.004f,right,right+0.075f,
             0,height+0.08f,frame);
    WallFace(meshes,material,vertical,face+0.004f,left,right,
             height,height+0.08f,frame);
    WallFace(meshes,Material::IndustrialCeiling,vertical,face+0.008f,
             right-0.22f,right-0.15f,0.99f,1.06f,
             Scale(Color(215,209,184),illumination));
}

Color Vary(Color first, Color second, std::uint32_t hash) {
    return hash%5 == 0 ? second : first;
}


int FloorDiv(int value, int size) {
    return value>=0 ? value/size : (value-size+1)/size;
}

bool LightBlocked(const std::vector<Wall>& walls, double x, double z,
                  double lightX, double lightZ) {
    const double dx=lightX-x,dz=lightZ-z;
    for (const auto& wall:walls) {
        double enter=0,leave=1;
        const auto clip=[&](double start,double direction,double low,double high) {
            if (std::abs(direction)<1e-8) return start>=low && start<=high;
            double a=(low-start)/direction,b=(high-start)/direction;
            if (a>b) std::swap(a,b);
            enter=std::max(enter,a);leave=std::min(leave,b);
            return enter<=leave;
        };
        if (clip(x,dx,wall.minX,wall.maxX) &&
            clip(z,dz,wall.minZ,wall.maxZ) && enter>1e-5 && enter<0.98)
            return true;
    }
    return false;
}

class BakedLighting {
public:
    explicit BakedLighting(const WorldConfig& world):world_(world) {}

    float Sample(double wx,double wz,int normalX=0,int normalZ=0) {
        const auto key=std::tuple{wx,wz,normalX,normalZ};
        if (const auto it=samples_.find(key);it!=samples_.end()) return it->second;
        const float base=world_.level==0 ? 0.54f : world_.level==1 ? 0.60f : 0.55f;
        const float strength=world_.level==0 ? 0.54f : world_.level==1 ? 0.38f : 0.41f;
        float value=base;
        const int cx=CellOf(wx),cz=CellOf(wz);
        const std::vector<Wall>* walls=nullptr;
        if (world_.level==0) {
            auto [it,inserted]=walls_.try_emplace({cx,cz});
            if (inserted) it->second=NearbyFullHeightWalls(world_,wx,wz);
            walls=&it->second;
        }
        for (int dx=-1;dx<=1;++dx) for (int dz=-1;dz<=1;++dz) {
            const int gx=cx+dx,gz=cz+dz;
            auto [it,inserted]=lamps_.try_emplace({gx,gz});
            if (inserted) it->second=LampAt(world_,gx,gz);
            const auto& lamp=it->second;
            if (!lamp.lit) continue;
            const double lightX=gx*kCellSize+lamp.x;
            const double lightZ=gz*kCellSize+lamp.z;
            const double lx=lightX-wx,lz=lightZ-wz;
            const float attenuation=std::max(0.0f,1.0f-
                static_cast<float>(std::hypot(lx,lz))/6.5f);
            if (attenuation<=0) continue;
            float incidence=1;
            if (world_.level==0 && (normalX || normalZ))
                incidence=static_cast<float>(std::max(0.0,
                    (lx*normalX+lz*normalZ)/std::sqrt(lx*lx+lz*lz+2.25)));
            // Keep ambient/bounced illumination in enclosed spaces. These
            // values become vertex colors; there are no runtime shadow maps.
            const float visibility=walls && LightBlocked(*walls,wx,wz,lightX,lightZ) ? 0.12f : 1.0f;
            value+=strength*attenuation*visibility*incidence;
        }
        value=std::min(1.0f,value);
        samples_.emplace(key,value);
        return value;
    }

    float WallSample(double wx,double wz,int normalX,int normalZ) {
        const double span=kRegionCells*kCellSize;
        const int rx=static_cast<int>(std::floor(wx/span));
        const int rz=static_cast<int>(std::floor(wz/span));
        const float u=static_cast<float>(wx/span-rx);
        const float v=static_cast<float>(wz/span-rz);
        const auto finish=[&](int x,int z) {
            return 0.98f+static_cast<float>(CellHash(world_,x,z,4001)%401)*0.0001f;
        };
        const float a=finish(rx,rz)*(1-u)+finish(rx+1,rz)*u;
        const float b=finish(rx,rz+1)*(1-u)+finish(rx+1,rz+1)*u;
        return (0.32f+0.68f*Sample(wx,wz,normalX,normalZ))*(a*(1-v)+b*v);
    }

    float FloorSample(double wx,double wz) {
        // Match the existing floor triangles rather than evaluating a brighter
        // lamp sample at the prop center. Contact shading must only darken it.
        const double x=std::floor(wx/2.5)*2.5,z=std::floor(wz/2.5)*2.5;
        const float u=static_cast<float>((wx-x)/2.5);
        const float v=static_cast<float>((wz-z)/2.5);
        const float a=Sample(x,z),b=Sample(x+2.5,z);
        const float c=Sample(x+2.5,z+2.5),d=Sample(x,z+2.5);
        return u>=v ? a*(1-u)+b*(u-v)+c*v :
                      a*(1-v)+c*u+d*(v-u);
    }

private:
    const WorldConfig& world_;
    std::map<std::pair<int,int>,std::vector<Wall>> walls_;
    std::map<std::pair<int,int>,LampInfo> lamps_;
    std::map<std::tuple<double,double,int,int>,float> samples_;
};

void OfficeObstacle(Meshes& meshes, BakedLighting& lighting,
                    float x0, float z0, float x1, float z1,
                    double chunkX, double chunkZ, float height,
                    Color wall, Color trim, Color floor) {
    const auto face=[&](bool vertical,float boundary,float a,float b,int normal) {
        const auto light=[&](float along) {
            const double wx=chunkX+(vertical ? boundary+normal*0.04f : along);
            const double wz=chunkZ+(vertical ? along : boundary+normal*0.04f);
            return lighting.WallSample(wx,wz,vertical ? normal : 0,
                                             vertical ? 0 : normal);
        };
        const float start=light(a),end=light(b);
        WallFace(meshes,Material::PaintedTrim,vertical,boundary,a,b,
                 0,0.12f,trim,start,end);
        WallFace(meshes,Material::Wallpaper,vertical,boundary,a,b,
                 0.12f,height,wall,start,end);
    };
    face(true,x0,z0,z1,-1);face(true,x1,z0,z1,1);
    face(false,z0,x0,x1,-1);face(false,z1,x0,x1,1);
    const Color outer=Scale(floor,lighting.Sample(chunkX+(x0+x1)*0.5,
                                            chunkZ+(z0+z1)*0.5));
    const Color inner=Scale(outer,0.82f);
    constexpr float y=0.004f;
    FlatShaded(meshes,Material::Carpet,x0-0.33f,z0,x0-0.10f,z1,y,
               {outer,inner,inner,outer},0.25f);
    FlatShaded(meshes,Material::Carpet,x1+0.10f,z0,x1+0.33f,z1,y,
               {inner,outer,outer,inner},0.25f);
    FlatShaded(meshes,Material::Carpet,x0,z0-0.33f,x1,z0-0.10f,y,
               {outer,outer,inner,inner},0.25f);
    FlatShaded(meshes,Material::Carpet,x0,z1+0.10f,x1,z1+0.33f,y,
               {inner,inner,outer,outer},0.25f);
}

std::optional<int> PortalTarget(const WorldConfig& world, double x, double z) {
    const auto portal=PortalAt(world,CellOf(x),CellOf(z));
    if (!portal) return std::nullopt;
    const double px=(portal->cellX+0.5)*kCellSize;
    const double pz=(portal->cellZ+0.5)*kCellSize;
    const double depth=portal->alongX ? x-px : z-pz;
    const double side=portal->alongX ? z-pz : x-px;
    if (depth>0.92 && depth<2.05 && std::abs(side)<1.30)
        return portal->target;
    return std::nullopt;
}

void PortalVisual(Meshes& meshes, int level, bool alongX, float cx,
                  float cz, float ceiling) {
    const Material frameMat=level==0 ? Material::Wallpaper : Material::TunnelWall;
    const Color frame=level==0 ? Color(112,101,68) : Color(129,111,85);
    const Color dark(49,45,40);
    const float near=0.43f, far=1.92f, half=1.1f;
    if (alongX) {
        BoxRange(meshes,frameMat,cx+near,cz-half-0.18f,
                 cx+far,cz-half,0,ceiling,frame);
        BoxRange(meshes,frameMat,cx+near,cz+half,
                 cx+far,cz+half+0.18f,0,ceiling,frame);
        BoxRange(meshes,frameMat,cx+near,cz-half,
                 cx+far,cz+half,2.25f,ceiling,frame);
        WallFace(meshes,Material::TunnelWall,true,cx+far,cz-half,cz+half,
                 0,2.25f,dark);
        Flat(meshes,Material::TunnelFloor,cx+near,cz-half,
             cx+far,cz+half,0.012f,Color(115,105,82),0.5f);
        Flat(meshes,Material::TunnelCeiling,cx+near,cz-half,
             cx+far,cz+half,2.245f,Color(107,99,79),0.6f);
        Flat(meshes,Material::Fluorescent,cx+1.08f,cz-0.25f,
             cx+1.48f,cz+0.25f,2.235f,Color(154,136,99),1.0f);
    } else {
        BoxRange(meshes,frameMat,cx-half-0.18f,cz+near,
                 cx-half,cz+far,0,ceiling,frame);
        BoxRange(meshes,frameMat,cx+half,cz+near,
                 cx+half+0.18f,cz+far,0,ceiling,frame);
        BoxRange(meshes,frameMat,cx-half,cz+near,
                 cx+half,cz+far,2.25f,ceiling,frame);
        WallFace(meshes,Material::TunnelWall,false,cz+far,cx-half,cx+half,
                 0,2.25f,dark);
        Flat(meshes,Material::TunnelFloor,cx-half,cz+near,
             cx+half,cz+far,0.012f,Color(115,105,82),0.5f);
        Flat(meshes,Material::TunnelCeiling,cx-half,cz+near,
             cx+half,cz+far,2.245f,Color(107,99,79),0.6f);
        Flat(meshes,Material::Fluorescent,cx-0.25f,cz+1.08f,
             cx+0.25f,cz+1.48f,2.235f,Color(154,136,99),1.0f);
    }
}
}

BackroomsGame::BackroomsGame(std::uint64_t seed, bool streamTest,
                             int startLevel, double startX, double startZ,
                             double walkSpeed, double runSpeed,
                             double streamTestMetres,float verticalFovDegrees)
    : graphics_(this), streamTest_(streamTest),
      streamTestMetres_(streamTestMetres) {
    if (startLevel<0 || startLevel>2 || !std::isfinite(startX) ||
        !std::isfinite(startZ) || std::abs(startX)>1.0e8 ||
        std::abs(startZ)>1.0e8 || !std::isfinite(walkSpeed) ||
        !std::isfinite(runSpeed) || walkSpeed<=0 || runSpeed<=walkSpeed ||
        runSpeed>20.0 || !std::isfinite(streamTestMetres) ||
        streamTestMetres<400.0 || streamTestMetres>1.0e7 ||
        !std::isfinite(verticalFovDegrees) || verticalFovDegrees<45 ||
        verticalFovDegrees>90)
        throw std::invalid_argument("invalid start, movement speed, field of view, or streaming distance");
    world_.seed = seed;
    world_.level=startLevel;
    if (Collides(world_,startX,startZ,0.31))
        throw std::invalid_argument("start position intersects generated geometry");
    x_=startX;
    z_=startZ;
    walkSpeed_=walkSpeed;
    runSpeed_=runSpeed;
    verticalFovDegrees_=verticalFovDegrees;
    yaw_=startLevel==2 ? 0.0f : 1.5707963f;
    graphics_.setPreferredBackBufferWidthProperty(1280);
    graphics_.setPreferredBackBufferHeightProperty(720);
    graphics_.setSynchronizeWithVerticalRetraceProperty(true);
    getWindowProperty().setTitleProperty("cna-backrooms");
}

BackroomsGame::~BackroomsGame() {
    // The frame lease has already released the ES context when Run returns.
    // Keep GPU deletion in the same explicit scope as streaming uploads.
    auto context=getGraphicsDeviceProperty().GetRenderer().AcquireThreadContextLeaseEXT();
    chunks_.clear();
    spareVertices_.clear();
    entityVertices_.reset();
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
    const auto directory=FindAssetDirectory();
    effect_ = std::make_unique<BasicEffect>(getGraphicsDeviceProperty());
    materials_ = std::make_unique<Materials>(getGraphicsDeviceProperty(),directory);
    effect_->VertexColorEnabled = true;
    effect_->setTextureEnabledProperty(true);
    effect_->setLightingEnabledProperty(false);
    effect_->setFogEnabledProperty(true);
    effect_->setFogStartProperty(LevelInfo(world_.level).fogStart);
    effect_->setFogEndProperty(LevelInfo(world_.level).fogEnd);
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
    const Material body=Material::TunnelWall;
    const Color coat(72,72,70),head(85,78,69);
    BoxRange(meshes,body,-0.27f,-0.17f,0.27f,0.18f,
             0.72f,1.68f,coat);
    BoxRange(meshes,body,-0.22f,-0.18f,0.22f,0.20f,
             1.71f,2.07f,head);
    BoxRange(meshes,body,-0.22f,-0.15f,-0.04f,0.12f,
             0,0.76f,coat);
    BoxRange(meshes,body,0.04f,-0.15f,0.22f,0.12f,
             0,0.76f,coat);
    BoxRange(meshes,body,-0.39f,-0.12f,-0.26f,0.12f,
             0.82f,1.57f,coat);
    BoxRange(meshes,body,0.26f,-0.12f,0.39f,0.12f,
             0.82f,1.57f,coat);
    auto& mesh=meshes[static_cast<int>(body)];
    entityTriangles_=static_cast<int>(mesh.size()/3);
    entityVertices_=std::make_unique<VertexBuffer>(getGraphicsDeviceProperty(),
        VertexPositionColorTexture::getVertexDeclarationStatic(),
        static_cast<int>(mesh.size()),BufferUsage::WriteOnly);
    entityVertices_->SetData(mesh.data(),static_cast<int>(mesh.size()));
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
    const float height=LevelInfo(level).ceilingHeight;
    const float doorHeight=level==0 ? 2.62f : level==1 ? 2.8f : 2.08f;
    const Material wallMat=level==0 ? Material::Wallpaper :
                           level==1 ? Material::ConcreteWall : Material::TunnelWall;
    const Material floorMat=level==0 ? Material::Carpet :
                            level==1 ? Material::ConcreteFloor : Material::TunnelFloor;
    const Material ceilingMat=level==0 ? Material::CeilingTile :
                              level==1 ? Material::IndustrialCeiling : Material::TunnelCeiling;
    const Color wallA=level==0 ? Color(255,250,239) :
                      level==1 ? Color(223,229,227) : Color(187,175,149);
    const Color wallB=level==0 ? Color(231,224,204) :
                      level==1 ? Color(188,202,200) : Color(151,139,118);
    const Color trim=level==0 ? Color(255,251,229) :
                     level==1 ? Color(110,130,128) : Color(99,83,67);
    const Color floorA=level==0 ? Color(246,240,222) :
                       level==1 ? Color(218,222,217) : Color(215,204,178);
    const Color ceiling=level==0 ? Color(255,252,235) :
                        level==1 ? Color(205,220,219) : Color(150,143,124);
    const Color grid=level==0 ? Color(143,139,115) :
                     level==1 ? Color(94,114,112) : Color(93,79,65);
    const Color lamp=level==0 ? Color(255,251,228) :
                     level==1 ? Color(204,230,230) : Color(255,198,137);

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
                level==0 ? 0.25f : 0.5f);
        }
        if (level==0) {
            const auto ceilingLight=[&](float factor) {
                return Scale(ceiling,0.52f+0.48f*factor);
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
        } else
            Flat(meshes,chamber ? Material::IndustrialCeiling : ceilingMat,
                 x,z,x+5,z+5,height,
                 Scale(ceiling,lampInfo.lit?1.0f:0.91f),0.8f);
        if (level==1) {
            const int rx=FloorDiv(gx,kRegionCells),rz=FloorDiv(gz,kRegionCells);
            const int lx=gx-rx*kRegionCells,lz=gz-rz*kRegionCells;
            const auto layout=CellHash(world_,rx,rz,2901);
            const int phase=static_cast<int>((layout>>4)%3);
            if (layout&1U) {
                if ((lz+phase)%3==0)
                    BoxRange(meshes,ceilingMat,x,z+0.22f,x+5,z+0.42f,
                             height-0.32f,height-0.10f,grid);
            } else if ((lx+phase)%3==0)
                BoxRange(meshes,ceilingMat,x+0.22f,z,x+0.42f,z+5,
                         height-0.32f,height-0.10f,grid);
        } else if (level==2 && !chamber) {
            BoxRange(meshes,Material::TunnelWall,x+0.35f,z,x+0.50f,z+5,
                     height-0.33f,height-0.17f,grid);
            BoxRange(meshes,Material::TunnelWall,x+4.50f,z,x+4.65f,z+5,
                     height-0.33f,height-0.17f,grid);
        }
        if (level==0 && lampInfo.fixture) {
            const float halfX=lampInfo.longAxisX ? 0.625f : 0.3125f;
            const float halfZ=lampInfo.longAxisX ? 0.3125f : 0.625f;
            const float cx=x+lampInfo.x,cz=z+lampInfo.z;
            Flat(meshes,Material::IndustrialCeiling,
                 cx-halfX,cz-halfZ,cx+halfX,cz+halfZ,
                 height-0.022f,Color(247,240,223),0.8f);
            const Material panelMat=Material::Fluorescent;
            const Color panelColor=lampInfo.lit ?
                Color(255,254,244) : Color(137,137,122);
            Quad(meshes,panelMat,
                 {cx-halfX+0.035f,height-0.035f,cz-halfZ+0.035f},
                 {cx+halfX-0.035f,height-0.035f,cz-halfZ+0.035f},
                 {cx+halfX-0.035f,height-0.035f,cz+halfZ-0.035f},
                 {cx-halfX+0.035f,height-0.035f,cz+halfZ-0.035f},panelColor,
                 {0,0},{1,0},{1,1},{0,1});
            if (lampInfo.lit) {
                const Color tube(255,255,246);
                if (lampInfo.longAxisX) {
                    Flat(meshes,Material::Fluorescent,
                         cx-0.50f,cz-0.20f,cx+0.50f,cz-0.11f,
                         height-0.039f,tube,0.6f);
                    Flat(meshes,Material::Fluorescent,
                         cx-0.50f,cz+0.11f,cx+0.50f,cz+0.20f,
                         height-0.039f,tube,0.6f);
                } else {
                    Flat(meshes,Material::Fluorescent,
                         cx-0.20f,cz-0.50f,cx-0.11f,cz+0.50f,
                         height-0.039f,tube,0.6f);
                    Flat(meshes,Material::Fluorescent,
                         cx+0.11f,cz-0.50f,cx+0.20f,cz+0.50f,
                         height-0.039f,tube,0.6f);
                }
            }
        } else if (level!=0 && lampInfo.lit) {
            Flat(meshes,ceilingMat,x+1.30f,z+2.27f,x+3.70f,z+2.73f,
                 height-0.024f,grid,0.8f);
            Flat(meshes,Material::Fluorescent,x+1.40f,z+2.34f,
                 x+3.60f,z+2.66f,height-0.032f,lamp,1.0f);
        }
        const Color cellWall=Scale(level==0 ? wallA : Vary(wallA,wallB,h),
            level==0 ? 1.0f : lampInfo.lit ? 1.0f : 0.80f);
        const auto wallLighting=[&](bool vertical) {
            WallLighting result{{{{1,1,1}},{{1,1,1}}}};
            if (level!=0) return result;
            for (int side=0;side<2;++side) {
                const int normal=side==0 ? -1 : 1;
                for (int sample=0;sample<3;++sample) {
                    const double px=wx+(vertical ? normal*0.14 : sample*2.5);
                    const double pz=wz+(vertical ? sample*2.5 : normal*0.14);
                    result[side][sample]=lighting.WallSample(
                        px,pz,vertical ? normal : 0,vertical ? 0 : normal);
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
            const float ambient=(light[0]+light[1]+light[2]+light[3])*0.25f;
            const Color shadowFloor=Scale(floorColor,ambient);
            ContactShadow(meshes,verticalEdge,verticalOpening,true,x,z,
                          shadowFloor);
            ContactShadow(meshes,horizontalEdge,horizontalOpening,false,z,x,
                          shadowFloor);
        }
        if (level==2) {
            const Color cabinet(133,139,126);
            const int regionX=FloorDiv(gx,kRegionCells);
            const int regionZ=FloorDiv(gz,kRegionCells);
            const int pipeStyle=static_cast<int>(
                CellHash(world_,regionX,regionZ,3311)%4);
            const auto serviceWall=[&](bool vertical, float boundary,
                                       bool positiveSide, std::uint32_t hash) {
                const float side=positiveSide ? 1.0f : -1.0f;
                const float start=vertical ? z : x;
                const float cross=boundary+side*0.19f;
                const bool hasCabinet=hash%13==0;
                const auto pipe=[&](float height,float radius,Color color) {
                    if (hasCabinet && height+radius>1.03f &&
                        height-radius<1.66f) {
                        PipeRun(meshes,vertical,cross,start,start+1.90f,
                                height,radius,color);
                        PipeRun(meshes,vertical,cross,start+2.68f,start+5.0f,
                                height,radius,color);
                    } else
                        PipeRun(meshes,vertical,cross,start,start+5.0f,
                                height,radius,color);
                };
                if (pipeStyle==0) {
                    pipe(1.95f,0.055f,Color(207,190,157));
                    if (hash%3!=0)
                        pipe(0.89f,0.048f,Color(151,147,127));
                } else if (pipeStyle==1) {
                    pipe(1.28f,0.080f,Color(174,190,180));
                } else if (pipeStyle==2) {
                    pipe(2.04f,0.054f,Color(209,157,111));
                    pipe(1.79f,0.046f,Color(171,137,105));
                } else if (hash%3!=0) {
                    pipe(1.70f,0.061f,Color(132,160,154));
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
                Flat(meshes,Material::IndustrialCeiling,
                     x+0.42f,z+0.73f,x+4.58f,z+1.12f,
                     height-0.45f,Scale(duct,0.75f),0.5f);
            }
        }
        const auto prop=PropAt(world_,gx,gz);
        const auto obstacles=CellObstacles(world_,gx,gz);
        const int structuralCount=obstacles.count-
            static_cast<int>(prop.kind!=PropKind::None);
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
                StorageRack(meshes,bx0,bz0,bx1,bz1,
                            CellHash(world_,gx,gz,3413+obstacleId));
            } else {
                BoxRange(meshes,chamber ? Material::ConcreteWall :
                         Material::TunnelWall,bx0,bz0,bx1,bz1,
                         0,chamber ? height : height-0.23f,
                         chamber ? Color(195,190,166) : Color(136,122,101));
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
            constexpr float feather=0.28f,dim=0.86f;
            const auto shadow=[&](float x0,float z0,float x1,float z1,
                                   const std::array<float,4>& strength) {
                const auto color=[&](float px,float pz,float scale) {
                    return Scale(floorColor,scale*lighting.FloorSample(
                        ox*kCellSize+px,oz*kCellSize+pz));
                };
                FlatShaded(meshes,Material::Carpet,x0,z0,x1,z1,0.005f,
                    {color(x0,z0,strength[0]),color(x1,z0,strength[1]),
                     color(x1,z1,strength[2]),color(x0,z1,strength[3])},0.25f);
            };
            shadow(sx0,sz0,sx1,sz1,{dim,dim,dim,dim});
            shadow(sx0-feather,sz0,sx0,sz1,{1,dim,dim,1});
            shadow(sx1,sz0,sx1+feather,sz1,{dim,1,1,dim});
            shadow(sx0,sz0-feather,sx1,sz0,{1,1,dim,dim});
            shadow(sx0,sz1,sx1,sz1+feather,{dim,dim,1,1});
            shadow(sx0-feather,sz0-feather,sx0,sz0,{1,1,dim,1});
            shadow(sx1,sz0-feather,sx1+feather,sz0,{1,1,1,dim});
            shadow(sx1,sz1,sx1+feather,sz1+feather,{dim,1,1,1});
            shadow(sx0-feather,sz1,sx0,sz1+feather,{1,dim,1,1});
        }
        const auto doorHash=CellHash(world_,gx,gz,1501);
        if (doorHash%170==0) {
            if ((doorHash&1U)==0 && VerticalEdge(world_,gx,gz)==Edge::Solid)
                FalseDoor(meshes,true,x,z,level,level==0 ?
                    lighting.WallSample(wx+0.14,wz+2.5,1,0) : 1.0f);
            else if (HorizontalEdge(world_,gx,gz)==Edge::Solid)
                FalseDoor(meshes,false,z,x,level,level==0 ?
                    lighting.WallSample(wx+2.5,wz+0.14,0,1) : 1.0f);
        }
        if (const auto portal=PortalAt(world_,gx,gz))
            PortalVisual(meshes,level,portal->alongX,x+2.5f,z+2.5f,height);
        const auto entityHash=CellHash(world_,gx,gz,919);
        const unsigned rarity=level==0 ? 850U : level==1 ? 650U : 750U;
        if (entityHash%rarity==0 && obstacles.count==0 &&
            (std::abs(gx)>4 || std::abs(gz)>4)) {
            chunk.entities.push_back({(gx+0.5)*kCellSize,
                                      (gz+0.5)*kCellSize,
                                      static_cast<float>((entityHash>>8)%628)/100.0f});
        }
    }
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
    ++frameCount_;
    if (statsTime_<1.0) return;
    int triangles=0,entities=0;
    for (const auto& [coord,chunk]:chunks_) {
        (void)coord;
        triangles+=chunk.triangles;
        entities+=static_cast<int>(chunk.entities.size());
    }
    const auto here=ChunkAt(x_,z_);
    std::ostringstream title;
    title << "cna-backrooms | Level " << world_.level << " | seed " << world_.seed
          << " | pos " << std::fixed << std::setprecision(1) << x_ << ',' << z_
          << " | chunk " << here.x << ',' << here.z
          << " | loaded " << chunks_.size() << "/25 | tris " << triangles
          << " | VBO " << bufferCreations_ << '/' << bufferReuses_
          << " pool " << spareVertices_.size()
          << " | entities " << entities
          << " | build " << std::setprecision(2) << lastBuildMs_ << " ms"
          << " | peak " << peakBuildMs_ << " ms"
          << " | " << (running_ ? "run" : "walk")
          << " | audio " << (hum_ ? "on" : "off")
          << " | " << static_cast<int>(frameCount_/statsTime_) << " FPS";
    getWindowProperty().setTitleProperty(title.str());
    if (streamTest_) std::cout << title.str() << '\n';
    statsTime_=0;
    frameCount_=0;
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

void BackroomsGame::Draw(const GameTime& time) {
    auto& device=getGraphicsDeviceProperty();
    const Color fog=world_.level==0 ? Color(108,101,78) :
                    world_.level==1 ? Color(56,67,67) : Color(41,35,29);
    device.Clear(fog);
    device.setDepthStencilStateProperty(DepthStencilState::Default);
    device.setRasterizerStateProperty(RasterizerState::CullNone);
    effect_->setFogColorProperty(fog.ToVector3());
    effect_->setFogStartProperty(LevelInfo(world_.level).fogStart);
    effect_->setFogEndProperty(LevelInfo(world_.level).fogEnd);
    const Vector3 eye(0,1.68f,0);
    const Vector3 direction(std::sin(yaw_)*std::cos(pitch_),std::sin(pitch_),
                            std::cos(yaw_)*std::cos(pitch_));
    effect_->setViewProperty(Matrix::CreateLookAt(eye,eye+direction,Vector3::Up));
    effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
        verticalFovDegrees_*0.01745329252f,
        device.getViewportProperty().getAspectRatioProperty(),0.08f,105.0f));
    for (const auto& [coord,chunk]:chunks_) {
        effect_->setWorldProperty(Matrix::CreateTranslation(
            static_cast<float>(coord.x*kChunkSize-x_),0,
            static_cast<float>(coord.z*kChunkSize-z_)));
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
    effect_->setTextureProperty(materials_->Get(Material::TunnelWall));
    device.SetVertexBuffer(entityVertices_.get());
    const float seconds=static_cast<float>(
        time.getTotalGameTimeProperty().getTotalSecondsProperty());
    for (const auto& [coord,chunk]:chunks_) {
        (void)coord;
        for (const auto& entity:chunk.entities) {
            const double ex=entity.x+0.55*std::sin(seconds*0.28f+entity.phase);
            const double ez=entity.z+0.55*std::cos(seconds*0.21f+entity.phase);
            const double distance=std::hypot(ex-x_,ez-z_);
            // These silhouettes are atmosphere, not physical obstacles. Let
            // them disappear before their mesh can surround the camera.
            if (distance<3.5 || distance>82.0) continue;
            const float facing=static_cast<float>(std::atan2(x_-ex,z_-ez));
            effect_->setWorldProperty(Matrix::CreateRotationY(facing)*
                Matrix::CreateTranslation(static_cast<float>(ex-x_),0,
                                          static_cast<float>(ez-z_)));
            auto& passes=effect_->getCurrentTechniqueProperty()->getPassesProperty();
            for (int i=0;i<passes.getCountProperty();++i) {
                passes[i]->Apply();
                device.DrawPrimitives(PrimitiveType::TriangleList,0,entityTriangles_);
            }
        }
    }
    Game::Draw(time);
}

} // namespace Backrooms
