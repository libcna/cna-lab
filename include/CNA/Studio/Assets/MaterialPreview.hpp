// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Assets/MaterialPreview.hpp
 * @brief A material drawn as a shaded sphere, in software (`plan.md` STUDIO-19007).
 *
 * **Why a sphere at all.** A `.cnamaterial` in the Content Browser is a generic icon, so a folder
 * of forty materials is forty identical rows distinguishable only by name — and a material's name
 * is the one thing about it a user did not derive from how it looks. A sphere shows base colour,
 * roughness, metalness and emission at a glance, which is every parameter this format has except
 * the maps.
 *
 * **On the CPU, and that is a decision rather than a shortcut.** A GPU preview needs a device, a
 * render target and a pass; the thumbnail cache runs its work on a *worker thread*, where there is
 * no device and must not be one. Studio already renders its whole UI through a software rasteriser
 * in the headless build, so a lit sphere at 128 pixels is well inside what this project already
 * does on a worker, and it makes the preview identical in every configuration — including the
 * headless one, where a GPU preview would simply be absent.
 *
 * **The maps are not sampled, and the plan says so rather than implying otherwise.** A material
 * whose appearance is mostly its base-colour texture previews as a plain sphere of its factor
 * colour. Sampling would mean decoding another image on the worker and resolving its path through
 * a database this function deliberately cannot see; what it would buy is a better thumbnail for
 * textured materials and nothing at all for the untextured ones, which are the ones a user is
 * most likely to have several of and least able to tell apart. It is the obvious next step and it
 * is not this row.
 *
 * Deterministic and free of any device, so the picture can be asserted on rather than looked at.
 */

#include <cstdint>

#include "CNA/Studio/Assets/MaterialDocument.hpp"
#include "CNA/Studio/Assets/ThumbnailCache.hpp"

namespace CNA::Studio
{
    /**
     * @brief Renders @p material as a lit sphere, @p edge pixels square.
     *
     * The background is fully transparent rather than a colour: the browser's card is whatever the
     * theme says it is, and a thumbnail with its own grey corners would be a grey square in the
     * light theme and a different grey square in the dark one.
     *
     * A transparent material is drawn *over that transparency*, so a `Blend` material with alpha
     * 0.3 previews as a faint sphere — which is what it looks like in the scene, and is the one
     * property of a material a flat swatch cannot show at all.
     *
     * @param edge The width and height in pixels. Zero returns an empty thumbnail.
     * @return The image, RGBA, top row first, with `key` left for the caller to fill in.
     */
    [[nodiscard]] StudioThumbnail studioRenderMaterialPreview(const MaterialDocument& material,
                                                              std::uint32_t edge);
}
