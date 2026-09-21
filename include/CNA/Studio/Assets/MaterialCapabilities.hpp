// SPDX-License-Identifier: MS-PL
#pragma once

/**
 * @file CNA/Studio/Assets/MaterialCapabilities.hpp
 * @brief What a material asks for that the effect drawing it cannot give (`plan.md` STUDIO-19008).
 *
 * **The gap this closes is one the plan kept writing down and the editor never said.** Studio
 * carries five texture slots and three alpha modes end to end; the build draws through
 * `BasicEffect`, which samples one texture and has no alpha test, so four of those maps and one of
 * those modes reach the file, the database, the dependency graph and the renderer — and not the
 * screen. `STUDIO-19002` and `STUDIO-19004` both recorded that in their acceptance entries, which
 * is the right place for a decision and the wrong place for a warning: nobody authoring a material
 * is reading the plan.
 *
 * **Named per feature rather than as one sentence.** "Some of this material will not draw" is a
 * line a user cannot act on. Which slot, and what happens instead, is one they can — either they
 * fill a different slot, or they know why the model looks flat and stop looking for the bug.
 *
 * **An effect this build does not recognise reports nothing.** A headless preview has no device
 * and no effect name, and a CNA that grows a third effect should make Studio quiet rather than
 * wrong: a diagnostic that fires on an effect it has never heard of is a diagnostic people learn
 * to ignore.
 *
 * CNA-free, like every other decision in Studio that could be wrong. The effect arrives as the
 * name the model pass reports, which is the one thing about the device that crosses this seam.
 */

#include <string>
#include <string_view>
#include <vector>

#include "CNA/Studio/Assets/MaterialDocument.hpp"

namespace CNA::Studio
{
    /** @brief One thing a material asks for that the effect drawing it will not do. */
    struct StudioMaterialCapabilityIssue
    {
        /** @brief The material's own word for it, as the editor labels the row. */
        std::string feature;

        /** @brief What happens instead, in a sentence a user can act on. */
        std::string detail;
    };

    /**
     * @brief What @p material asks for that @p effectName cannot draw.
     *
     * @param effectName The model pass's own answer: "PbrEffect", "BasicEffect", "none", or empty
     *        where there is no device at all. Anything unrecognised reports nothing.
     * @return The gaps, in the order the editor lists the slots. Empty when everything draws.
     */
    [[nodiscard]] std::vector<StudioMaterialCapabilityIssue> studioMaterialCapabilityIssues(
        std::string_view effectName, const MaterialDocument& material);

    /**
     * @brief What a light of @p lightKind asks for that @p effectName cannot draw.
     *
     * `plan.md` STUDIO-20003. The same question one component over, and it lives here rather than
     * in a second file because the answer has the same shape and the same seam: a feature the
     * document carries, an effect that may or may not draw it, and a sentence a user can act on.
     *
     * `PbrEffect` implements CNA's punctual-light extension, so it draws a point light with a real
     * position and a spot light with its cone. `BasicEffect` does not, so on a build drawing
     * through it both are flattened into the directional slots: a point light behaves like one
     * between objects and cannot fall off across a large one, and a spot light's cone is not drawn
     * at all.
     *
     * This replaced a *scene validation* rule that said the same thing unconditionally. That rule
     * was wrong twice over -- it was written believing CNA had no punctual light, and even as a
     * statement about `BasicEffect` it fired in a report that has no idea which effect the build
     * uses.
     *
     * @param lightKind `CNA.Light`'s own spelling: "Directional", "Point" or "Spot".
     */
    [[nodiscard]] std::vector<StudioMaterialCapabilityIssue> studioLightCapabilityIssues(
        std::string_view effectName, std::string_view lightKind);
}
