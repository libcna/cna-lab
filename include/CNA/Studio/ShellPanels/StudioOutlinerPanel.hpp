// SPDX-License-Identifier: MS-PL
/**
 * @file CNA/Studio/ShellPanels/StudioOutlinerPanel.hpp
 * @brief The World Outliner on the Studio UI.
 *
 * `plan.md` STUDIO-07006.
 *
 * ### Why this lives in its own module
 *
 * The Output Log could go in `cna-studio-ui-core` because its model, `StudioLog`, is part of
 * `cna-studio-ui`, which ui-core already depends on. This one reads a `SceneDocument` and writes a
 * selection, and ui-core depends on neither — deliberately, because that is what keeps the widget
 * layer reusable and testable without a document model.
 *
 * So the ported panels get a module of their own, above both: the widgets know nothing about
 * scenes, the scene knows nothing about widgets, and this is the seam where the two are put
 * together. Every panel ported after this one belongs here.
 *
 * ### Selection is the context's, not the panel's
 *
 * Clicking a row calls through to `StudioContext`, which is what the ImGui outliner does and what
 * the viewport, the inspector and the gizmos all read. A panel that kept its own idea of what is
 * selected would be a panel that disagrees with the rest of the editor the moment anything else
 * changes it.
 */

#pragma once

#include "CNA/Studio/Core/Uuid.hpp"
#include "CNA/Studio/UiCore/StudioFrame.hpp"
#include "CNA/Studio/UiCore/StudioTreeView.hpp"
#include "CNA/Studio/UiCore/UiRect.hpp"

#include <cstddef>
#include <string>
#include <unordered_set>
#include <string_view>
#include <vector>

namespace CNA::Studio
{
    class SceneDocument;
    class StudioContext;

    /** @brief What the user asked the outliner to do. */
    struct StudioOutlinerResult
    {
        /** @brief How many rows were drawn. */
        std::size_t rowsDrawn = 0;

        /** @brief How many rows the scene and the current expansion produced. */
        std::size_t rowsTotal = 0;

        /**
         * @brief How many rows the panel actually *built* this pass (`plan.md` STUDIO-13011).
         *
         * Not the same as @ref rowsDrawn, and the difference is the whole of the task. The tree
         * has culled its drawing since `STUDIO-03034`, so a panel that flattened fifty thousand
         * entities and drew forty of them already had a bounded @ref rowsDrawn — and was spending
         * the frame building rows nobody would see. This is the number that says otherwise.
         */
        std::size_t rowsBuilt = 0;

        /** @brief Whether the selection changed this frame. Input pass only. */
        bool selectionChanged = false;

        /** @brief Whether an entity was renamed in place this frame. Input pass only. */
        bool renamed = false;

        /** @brief Whether an entity was shown or hidden from its row this frame. Input pass only. */
        bool visibilityChanged = false;

        /**
         * @brief Whether an entity was locked or unlocked from its row this frame. Input pass only.
         *
         * `plan.md` STUDIO-13005. Reported apart from @ref visibilityChanged because the two rows
         * of buttons do different things and the shell says different things about them -- and
         * because a lock is the change a user is most likely to make by accident and then wonder
         * about, the viewport having gone quiet under their pointer.
         */
        bool lockChanged = false;

        /** @brief Which entity it was, and what it became. Meaningless unless @ref lockChanged. */
        Uuid lockedEntity;

        /** @brief The state the entity was put into. @see lockedEntity */
        bool lockedNow = false;

        /**
         * @brief Whether an entity was reparented by a drag this frame. Input pass only.
         *
         * `STUDIO-07058`.
         */
        bool reparented = false;

        /**
         * @brief How many entities the drop moved (`plan.md` STUDIO-13004).
         *
         * One for the ordinary drag, and the size of the selection for a drag that started on part
         * of it. Reported so the shell can say which it was: "Reparented 12 entities" is the only
         * confirmation a user gets that the drop took the selection and not just the row they were
         * pointing at.
         */
        std::size_t reparentedCount = 0;

        /**
         * @brief An asset was dropped on a row, and is to be put in the scene. Input pass only.
         *
         * `plan.md` STUDIO-09008. Reported rather than acted on here, because what an asset
         * *becomes* is one decision shared with the viewport (`Scene/AssetDrop.hpp`) and the panel
         * that owns the tree is not where a shared decision belongs.
         */
        Uuid assetDropped;

        /** @brief The row it was dropped on, which becomes the new entity's parent. */
        Uuid assetDropParent;

        /**
         * @brief Whether a drop was refused because it would have made a cycle. Input pass only.
         *
         * Reported rather than swallowed, and reported *separately* from a reparent that happened:
         * a refusal that looked like success would leave the user watching a tree that did not
         * change and wondering which of the two they were looking at.
         */
        bool reparentRefused = false;
    };

    /**
     * @brief The payload type an entity is dragged as, within the outliner.
     *
     * One constant rather than a literal at each end: a source and a target that disagree about
     * the spelling produce a drag that silently does nothing, which is the hardest failure to see.
     * Distinct from the asset type, so a texture dragged from the Content Browser onto a row does
     * not read as a reparent.
     */
    inline constexpr std::string_view kStudioEntityDragType = "entity";

    /**
     * @brief Which entities a search keeps, worked out once for a whole frame.
     *
     * `plan.md` STUDIO-13002. Filtering a *tree* is not filtering a list: a row that matches is
     * useless without the path that leads to it, so the kept set is the matches **and every
     * ancestor of a match**. The two are kept apart so a row can be drawn as what it is -- the
     * thing the user searched for, or the way to it.
     *
     * Computed once and handed to both the count and the window, for the reason the hierarchy is:
     * a scene of fifty thousand entities walked twice a frame is a scene filtered twice a frame.
     */
    struct StudioOutlinerFilter
    {
        /** @brief False when no search is in force, in which case the sets are empty and unused. */
        bool active = false;

        /** @brief The entities to show: the matches, and the ancestors that lead to one. */
        std::unordered_set<Uuid> kept;

        /** @brief Just the matches, so the path to one can be dimmed rather than read as a hit. */
        std::unordered_set<Uuid> matched;
    };

    /**
     * @brief Returns the filter @p text describes over @p scene.
     *
     * Case-insensitive substring over the entity's name, which is what a name search means and what
     * the Content Browser's own search already does. Empty text returns an inactive filter rather
     * than one that matches everything -- "no filter" and "a filter nothing failed" are the same
     * picture and different costs, and only one of them should walk the scene.
     *
     * An entity whose *name* does not match is kept when any descendant's does. That is the whole
     * of the difference from filtering a list, and it is why this cannot be a predicate the walk
     * applies row by row: whether to keep a node depends on what is underneath it.
     */
    [[nodiscard]] StudioOutlinerFilter studioOutlinerFilter(const SceneDocument& scene,
                                                            std::string_view text);

    /**
     * @brief Returns the entities from @p from to @p to inclusive, in the order they are shown.
     *
     * `plan.md` STUDIO-13003. What a Shift-click means: everything between where the user last
     * clicked and where they just clicked, as they see it -- which is display order, not document
     * order and not hierarchy order. A range taken in document order would select entities that are
     * not between the two on the screen, which is the one thing the gesture promises.
     *
     * Either end may be the earlier one; the pair is normalised here rather than at the call site.
     * Rows hidden by a collapsed branch or by a search are not in the range, because they are not
     * between the two as far as anybody looking can tell -- and that is guaranteed rather than
     * arranged, because this walks the tree with the same function that builds the rows.
     *
     * Empty when either end is not currently shown: a range with no visible end is not a range the
     * user can have meant.
     */
    [[nodiscard]] std::vector<Uuid> studioOutlinerRange(const SceneDocument& scene,
                                                        const StudioTreeState& state,
                                                        const StudioOutlinerFilter& filter,
                                                        const Uuid& from, const Uuid& to);

    /**
     * @brief Which of a row's toggles is which (`plan.md` STUDIO-13005).
     *
     * The tree reports the index it drew, and the panel has to turn that back into a meaning. Named
     * constants rather than a bare 0 and 1 at the comparison, because the two are in a fixed order
     * on every row and a reader of the click path should not have to count the builder's pushes.
     */
    inline constexpr std::size_t studioOutlinerVisibilityToggle = 0;

    /** @brief The lock button's index in a row's toggles. @see studioOutlinerVisibilityToggle */
    inline constexpr std::size_t studioOutlinerLockToggle = 1;

    /**
     * @brief What a drag starting on @p dragged actually moves.
     *
     * `plan.md` STUDIO-13004. A drag picks up the whole selection when it starts on part of it, and
     * only the row it started on otherwise -- which is what every list does, and what a user who
     * has just selected forty entities means when they drag one of them.
     *
     * **Descendants of a moving entity are left out**, because they are already coming: an entity
     * travels with its parent, and reparenting a child as well would tear it out of the parent it
     * is moving with and leave it a sibling. A selection of a parent and its child is therefore a
     * drag of the parent, not of two things.
     *
     * @param scene The scene, for the ancestry.
     * @param selection What is currently selected.
     * @param dragged The entity the drag started on.
     * @return The entities to move, in selection order; empty when @p dragged is not in the scene.
     */
    [[nodiscard]] std::vector<Uuid> studioOutlinerDragSet(const SceneDocument& scene,
                                                          const std::vector<Uuid>& selection,
                                                          const Uuid& dragged);

    /** @brief What dropping a set of entities onto one parent would do. */
    struct StudioReparentPlan
    {
        /**
         * @brief The entities that would actually move.
         *
         * Entities already directly under the target are left out: they are a no-op, not a
         * conflict, and an undo entry that undoes nothing is the kind of history a user stops
         * trusting.
         */
        std::vector<Uuid> entities;

        /**
         * @brief Whether the drop is refused outright.
         *
         * True when *any* member of the set cannot go under the target -- it is the target, or it
         * is one of the target's ancestors and the move would make the tree a ring. The whole
         * gesture is refused rather than the offending members dropped, because a drag that moved
         * four of five entities would leave a hierarchy the user did not ask for and cannot see
         * the shape of, and one Ctrl+Z would not be an obvious way back.
         */
        bool refused = false;
    };

    /**
     * @brief Works out what dropping @p moving onto @p parent would do.
     *
     * `plan.md` STUDIO-13004. A CNA-free function so the rule can be asserted without a frame,
     * a shell or a drag: what a drop *means* is a question about the scene, and only carrying it
     * out is a question about the editor.
     *
     * @param scene The scene.
     * @param moving The entities being dragged, from @ref studioOutlinerDragSet.
     * @param parent The entity they were dropped on. Nil makes them roots.
     * @return The plan. Both empty and not refused means there is nothing to do.
     */
    [[nodiscard]] StudioReparentPlan studioOutlinerReparentPlan(const SceneDocument& scene,
                                                                const std::vector<Uuid>& moving,
                                                                const Uuid& parent);

    /**
     * @brief Starts renaming @p entityId in the outliner, if it is in the scene.
     *
     * Here rather than on `StudioTreeState` because the state knows nothing about entities: it
     * takes the row id and the label it starts with, and turning an entity into those two is the
     * outliner's job.
     *
     * @param scene The scene holding the entity.
     * @param entityId Entity to rename.
     * @param state Tree state to put into renaming mode.
     * @return True when the entity exists and the rename has begun.
     */
    bool studioBeginOutlinerRename(const SceneDocument& scene, const Uuid& entityId,
                                   StudioTreeState& state);

    /**
     * @brief Flattens a scene into tree rows, honouring @p state and the current selection.
     *
     * Separate from drawing so a test can assert on the shape of the tree without a frame, and so
     * the panel's two jobs — deciding what the tree *is* and drawing it — can fail independently.
     *
     * @param scene The scene.
     * @param selection Ids currently selected.
     * @param state Which rows are open.
     * @return Rows in display order, parents before their children.
     */
    /**
     * @brief How many rows the outliner would show for @p scene under @p state.
     *
     * `plan.md` STUDIO-13011. Asked before any row is built, because the scroll region needs the
     * count to size itself and the caller needs the size to know which rows are worth building.
     * Walks the tree and constructs nothing.
     *
     * @param scene The scene.
     * @param state Which rows are open.
     * @return The number of visible rows.
     */
    [[nodiscard]] std::size_t studioOutlinerRowCount(const SceneDocument& scene,
                                                     const StudioTreeState& state,
                                                     const StudioOutlinerFilter& filter = {});

    /**
     * @brief The rows at `[first, first + count)` of the flattened scene.
     *
     * The same rows @ref studioOutlinerRows produces, built for a window rather than in full.
     * Reaching position @p first still costs @p first steps of the walk — a hierarchy has no index
     * to seek into — but a step is a lookup and an increment, where a row is three strings.
     *
     * @param scene The scene.
     * @param selection Ids currently selected.
     * @param state Which rows are open.
     * @param first Index of the first row wanted.
     * @param count How many. `SIZE_MAX` means "to the end".
     * @return The rows of that window, in display order.
     */
    [[nodiscard]] std::vector<StudioTreeRow> studioOutlinerRowWindow(
        const SceneDocument& scene, const std::vector<Uuid>& selection,
        const StudioTreeState& state, std::size_t first, std::size_t count,
        const StudioOutlinerFilter& filter = {});

    [[nodiscard]] std::vector<StudioTreeRow> studioOutlinerRows(
        const SceneDocument& scene, const std::vector<Uuid>& selection,
        const StudioTreeState& state, const StudioOutlinerFilter& filter = {});

    /**
     * @brief Draws the World Outliner and applies what the user clicked.
     *
     * @param frame The frame.
     * @param bounds The panel's content rectangle.
     * @param context The editor. Its scene is read; its selection is written.
     * @param state Expansion state, owned by the caller so it survives the frame.
     * @return What happened.
     */
    StudioOutlinerResult studioOutlinerPanel(StudioFrame& frame, const UiRect& bounds,
                                             StudioContext& context, StudioTreeState& state,
                                             std::string* search = nullptr);
}
