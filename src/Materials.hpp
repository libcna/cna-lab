#pragma once

#include <array>
#include <filesystem>
#include <memory>

#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

namespace Backrooms {

enum class Material : int {
    Wallpaper, Carpet, CeilingTile,
    ConcreteWall, ConcreteFloor, IndustrialCeiling,
    TunnelWall, TunnelFloor, TunnelCeiling,
    Wood, Cardboard, Fluorescent, Upholstery, PaintedTrim, Count
};

constexpr int kMaterialCount = static_cast<int>(Material::Count);

class Materials {
public:
    explicit Materials(Microsoft::Xna::Framework::Graphics::GraphicsDevice& device,
                       const std::filesystem::path& assetDirectory);
    Microsoft::Xna::Framework::Graphics::Texture2D* Get(Material material) const {
        return textures_[static_cast<int>(material)].get();
    }

private:
    std::array<std::unique_ptr<Microsoft::Xna::Framework::Graphics::Texture2D>,
               kMaterialCount> textures_;
};

} // namespace Backrooms
