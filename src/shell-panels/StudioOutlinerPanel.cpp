// SPDX-License-Identifier: MS-PL
/**
 * @file StudioOutlinerPanel.cpp
 * @brief The World Outliner.
 */

#include "CNA/Studio/ShellPanels/StudioOutlinerPanel.hpp"

#include "CNA/Studio/ShellPanels/StudioContentBrowser.hpp"

#include "CNA/Studio/Scene/BuiltinComponents.hpp"
#include "CNA/Studio/Scene/SceneDocument.hpp"
#include "CNA/Studio/Scene/SceneCommands.hpp"
#include "CNA/Studio/StudioContext.hpp"
#include "CNA/Studio/Core/StudioCommand.hpp"

#include <memory>

#include <algorithm>
#include <limits>
#include <unordered_map>

namespace CNA::Studio
{
    namespace
    {
        /**
         * @brief What an entity *is*, from what it carries.
         *
         * `STUDIO-35030`. The first thing anybody looks for in an outliner is which row is the
         * camera, and reading that off a detail column of component names is reading rather than
         * scanning. Decided from the components rather than from a field on the entity, because a
         * scene has no such field and inventing one would put a *presentation* concern into the
         * document format -- where it would then have to be migrated, validated and exported.
         *
         * Ordered by how much the answer tells a user, not by how common the component is. An
         * entity with a camera and a light is a camera with a light attached, because the camera is
         * the thing somebody is looking for.
         */
        StudioIcon iconFor(const StudioEntity& entity)
        {
            const auto has = [&entity](const char* typeId) {
                for (const StudioComponent& component : entity.getComponents())
                {
                    if (component.getTypeId() == typeId) { return true; }
                }
                return false;
            };

            if (has(BuiltinComponentIds::kCamera)) { return StudioIcon::Camera; }
            if (has(BuiltinComponentIds::kLight)) { return StudioIcon::Light; }
            if (has(BuiltinComponentIds::kModelRenderer)) { return StudioIcon::Mesh; }
            if (has(BuiltinComponentIds::kSpriteRenderer)
                || has(BuiltinComponentIds::kSpriteAnimation)
                || has(BuiltinComponentIds::kTilemap)) { return StudioIcon::Sprite; }
            if (has(BuiltinComponentIds::kAudioSource)
                || has(BuiltinComponentIds::kAudioListener)) { return StudioIcon::Audio; }

            // A transform and nothing else is still an entity and still gets an icon: an empty
            // used as a pivot or a group is a real thing in the scene, and a blank where every
            // other row has a picture reads as a row that failed to load.
            return StudioIcon::Entity;
        }

        /** @brief The empty child list a leaf gets, so the walk needs no null check. */
        const std::vector<Uuid>& noChildren()
        {
            static const std::vector<Uuid> empty;
            return empty;
        }

        /**
         * @brief Visits @p id and, when it is open, its children, in display order.
         *
         * Depth-limited rather than cycle-detecting. A scene document is kept acyclic by every
         * operation that can reparent, so a cycle here would be a bug elsewhere -- but it would
         * present as a hang, and a hang is the one failure a user cannot diagnose or report.
         *
         * `STUDIO-30013`: @p hierarchy is derived once by the caller rather than asked for per
         * node. `SceneDocument::getChildren` scans every entity, so calling it here made the walk
         * O(n²) -- 9 ms a frame at 250 entities and 309 ms at 2 000, measured by `--ui-benchmark`.
         * Rows are virtualised, so the *drawing* was already flat and nothing in a capture or a
         * draw-call assertion could have shown it.
         *
         * `STUDIO-13011`: and one function counts *and* builds, rather than two that agree by
         * inspection. The panel has to know how many rows there are before it can know which ones
         * are worth building, and a count that walked the tree by slightly different rules than the
         * build would put the scrollbar and the rows quietly out of step -- the sort of defect that
         * shows up as the last entity in a large scene being unreachable.
         *
         * @param index Running position in the whole list; advanced once per visited row.
         * @param first First position worth building. Everything before it is counted only.
         * @param last One past the last worth building; `npos` for "to the end".
         * @param out Where rows land, or null to count without building any.
         */
        void walk(const SceneDocument& scene,
                  const std::unordered_map<Uuid, std::vector<Uuid>>& hierarchy,
                  const Uuid& id, int depth,
                  const std::vector<Uuid>& selection, const StudioTreeState& state,
                  const StudioOutlinerFilter& filter, std::size_t& index, std::size_t first,
                  std::size_t last, std::vector<StudioTreeRow>* out,
                  std::vector<Uuid>* outIds = nullptr)
        {
            constexpr int kMaxDepth = 64;

            const StudioEntity* entity = scene.findEntity(id);
            if (entity == nullptr || depth > kMaxDepth) { return; }

            // Not kept by the search: neither this row nor anything under it, because the kept set
            // already holds every ancestor of every match -- so a node outside it has no match
            // anywhere below and the whole subtree can be skipped without descending
            // (`plan.md` STUDIO-13002).
            if (filter.active && filter.kept.find(id) == filter.kept.end()) { return; }

            const auto found = hierarchy.find(id);
            const std::vector<Uuid>& children =
                found == hierarchy.end() ? noChildren() : found->second;

            const std::size_t position = index++;
            const bool wanted = out != nullptr && position >= first && position < last;

            // Counted without being built. This is the whole of STUDIO-13011: a scene of fifty
            // thousand entities costs fifty thousand `findEntity` lookups and one increment each,
            // rather than fifty thousand rows carrying three strings apiece -- twice a frame, to
            // show forty.
            if (!wanted)
            {
                if (out != nullptr && position >= last) { return; }

                // `collapsedCount()` first, so a tree with nothing collapsed -- which is every
                // tree until the user closes something -- never formats the id at all. Counting
                // twenty thousand entities was building forty thousand thirty-six-character
                // strings a frame to ask a set that was empty.
                // While a search is in force every kept row is open, whatever the user collapsed:
                // a match hidden inside a closed branch is a match the search did not find, as far
                // as anybody looking at the screen can tell.
                if (filter.active || state.collapsedCount() == 0 || state.isExpanded(id.toString()))
                {
                    for (const Uuid& child : children)
                    {
                        walk(scene, hierarchy, child, depth + 1, selection, state, filter, index,
                             first, last, out, outIds);
                        if (out != nullptr && index >= last) { return; }
                    }
                }
                return;
            }

            StudioTreeRow row;
            row.id = id.toString();
            row.label = entity->getName().empty() ? std::string{"(unnamed)"} : entity->getName();
            row.depth = depth;
            row.hasChildren = !children.empty();
            row.selected = std::find(selection.begin(), selection.end(), id) != selection.end();
            row.enabled = entity->isEnabled();

            // Kept but not matched: this row is the *way* to a match rather than one itself.
            // Dimmed and still fully clickable, which is exactly what `muted` is for -- a path
            // the user cannot click is a path they have to close the search to walk.
            row.muted = filter.active && filter.matched.find(id) == filter.matched.end();
            row.icon = iconFor(*entity);

            // `STUDIO-35060`. An outliner where hiding an entity means selecting it, finding the
            // Details panel and unticking a box is one where nobody hides anything -- and hiding
            // things is how a large scene is worked on at all.
            //
            // The entity's `enabled` flag rather than a second "visible" one. A scene has no such
            // field, and inventing one would put a presentation concern into the document format,
            // where it would then have to be migrated, validated and exported -- and it would be a
            // second thing that hides an entity, which is one too many.
            row.toggleIcon = StudioIcon::Visible;
            row.toggleOffIcon = StudioIcon::Hidden;
            row.toggleOn = entity->isEnabled();
            row.toggleTooltip = entity->isEnabled() ? "Hide this entity" : "Show this entity";

            // `STUDIO-07058`. Every row is both a drag source and a drop target, because
            // rearranging a hierarchy is dragging one entity onto another and both roles belong to
            // every row -- the prototype's outliner has said so since it was written.
            //
            // The value is the entity's id rather than its name: two entities may share a name,
            // and a reparent that picked whichever one the walk found first would be a rearrangement
            // the user did not ask for and cannot undo into the one they wanted.
            row.dragType = std::string{kStudioEntityDragType};
            row.dragValue = id.toString();
            row.dropTypes = {std::string{kStudioEntityDragType},
                             std::string{kStudioAssetDragType}};

            // The component list is what tells a camera from a sprite at a glance, and it is the
            // first thing anybody looks for in an outliner. One name reads; five is a wall.
            if (entity->getComponents().size() == 1)
            {
                row.detail = entity->getComponents().front().getTypeId();
            }
            else if (entity->getComponents().size() > 1)
            {
                row.detail = std::to_string(entity->getComponents().size()) + " components";
            }

            out->push_back(std::move(row));

            // The same traversal answers both questions (`plan.md` STUDIO-13003). A second walk
            // with its own copy of the filter-and-expansion rules is a second walk free to drift,
            // and the day it did a Shift-range would select rows that are not on the screen.
            if (outIds != nullptr) { outIds->push_back(id); }

            // The row's own id, already formatted just above, rather than a second `toString()`.
            if (!filter.active && state.collapsedCount() != 0 && !state.isExpanded(out->back().id))
            {
                return;
            }
            for (const Uuid& child : children)
            {
                walk(scene, hierarchy, child, depth + 1, selection, state, filter, index, first,
                     last, out, outIds);
                if (index >= last) { return; }
            }
        }

        /** @brief ASCII lower-case, which is what a name search needs and all the atlas carries. */
        char lowerAscii(char character)
        {
            return (character >= 'A' && character <= 'Z')
                       ? static_cast<char>(character - 'A' + 'a')
                       : character;
        }

        /** @brief Walks every root of @p scene, and answers how many rows there were. */
        std::size_t walkRoots(const SceneDocument& scene,
                              const std::unordered_map<Uuid, std::vector<Uuid>>& hierarchy,
                              const std::vector<Uuid>& selection, const StudioTreeState& state,
                              const StudioOutlinerFilter& filter, std::size_t first,
                              std::size_t last, std::vector<StudioTreeRow>* out,
                              std::vector<Uuid>* outIds = nullptr)
        {
            // The nil Uuid's entry is the roots, so this also replaces `getRootEntities()` --
            // which is the same scan under another name.
            const auto roots = hierarchy.find(Uuid{});
            if (roots == hierarchy.end()) { return 0; }

            std::size_t index = 0;
            for (const Uuid& root : roots->second)
            {
                walk(scene, hierarchy, root, 0, selection, state, filter, index, first, last, out,
                     outIds);
                if (out != nullptr && index >= last) { break; }
            }
            return index;
        }
    }

    StudioOutlinerFilter studioOutlinerFilter(const SceneDocument& scene, std::string_view text)
    {
        StudioOutlinerFilter filter;
        if (text.empty()) { return filter; }

        filter.active = true;

        std::string needle;
        needle.reserve(text.size());
        for (const char character : text) { needle.push_back(lowerAscii(character)); }

        // Every match first, then the paths to them. Two passes rather than one recursive descent,
        // because whether to keep a node depends on what is *underneath* it and a walk down cannot
        // know that until it has come back up -- while walking *up* from each match is one step per
        // ancestor and visits nothing that is not on a path.
        for (const StudioEntity& entity : scene.getEntities())
        {
            std::string name;
            name.reserve(entity.getName().size());
            for (const char character : entity.getName()) { name.push_back(lowerAscii(character)); }

            if (name.find(needle) == std::string::npos) { continue; }

            filter.matched.insert(entity.getId());

            for (Uuid at = entity.getId(); at.isValid();)
            {
                // `insert` answers whether it was new, so a path already walked stops here: with a
                // hundred matches under one root, the root is reached once rather than a hundred
                // times.
                if (!filter.kept.insert(at).second) { break; }

                const StudioEntity* node = scene.findEntity(at);
                if (node == nullptr) { break; }
                at = node->getParentId();
            }
        }

        return filter;
    }

    std::size_t studioOutlinerRowCount(const SceneDocument& scene, const StudioTreeState& state,
                                       const StudioOutlinerFilter& filter)
    {
        // Once for the whole walk (STUDIO-30013): `getChildrenByParent` is a pass over the scene,
        // and asking for it per node is what made this O(n^2) in the first place.
        return walkRoots(scene, scene.getChildrenByParent(), {}, state, filter, 0,
                         std::numeric_limits<std::size_t>::max(), nullptr);
    }

    std::vector<StudioTreeRow> studioOutlinerRowWindow(const SceneDocument& scene,
                                                       const std::vector<Uuid>& selection,
                                                       const StudioTreeState& state,
                                                       std::size_t first, std::size_t count,
                                                       const StudioOutlinerFilter& filter)
    {
        if (count == 0) { return {}; }

        std::vector<StudioTreeRow> rows;
        rows.reserve(std::min(count, scene.getEntityCount()));

        const std::size_t last = count == std::numeric_limits<std::size_t>::max()
            ? count
            : first + count;
        (void)walkRoots(scene, scene.getChildrenByParent(), selection, state, filter, first, last,
                        &rows);
        return rows;
    }

    std::vector<StudioTreeRow> studioOutlinerRows(const SceneDocument& scene,
                                                  const std::vector<Uuid>& selection,
                                                  const StudioTreeState& state,
                                                  const StudioOutlinerFilter& filter)
    {
        return studioOutlinerRowWindow(scene, selection, state, 0,
                                       std::numeric_limits<std::size_t>::max(), filter);
    }

    std::vector<Uuid> studioOutlinerRange(const SceneDocument& scene, const StudioTreeState& state,
                                          const StudioOutlinerFilter& filter, const Uuid& from,
                                          const Uuid& to)
    {
        if (!from.isValid() || !to.isValid()) { return {}; }
        if (from == to) { return scene.findEntity(from) != nullptr ? std::vector<Uuid>{from}
                                                                   : std::vector<Uuid>{}; }

        // The rows are built and thrown away: the ids come out of the same traversal, and adding a
        // build-nothing mode would be a second set of rules to keep in step with the first for the
        // sake of a gesture a user makes a few times a minute.
        std::vector<StudioTreeRow> rows;
        std::vector<Uuid> shown;
        (void)walkRoots(scene, scene.getChildrenByParent(), {}, state, filter, 0,
                        std::numeric_limits<std::size_t>::max(), &rows, &shown);

        const auto first = std::find(shown.begin(), shown.end(), from);
        const auto second = std::find(shown.begin(), shown.end(), to);

        // Either end off the screen means no range: a user cannot have meant "everything between
        // here and a row that is not shown", and guessing which rows they did mean would be worse
        // than doing nothing.
        if (first == shown.end() || second == shown.end()) { return {}; }

        const auto low = first <= second ? first : second;
        const auto high = first <= second ? second : first;
        return std::vector<Uuid>{low, high + 1};
    }

    std::vector<Uuid> studioOutlinerDragSet(const SceneDocument& scene,
                                            const std::vector<Uuid>& selection, const Uuid& dragged)
    {
        if (!dragged.isValid() || scene.findEntity(dragged) == nullptr) { return {}; }

        // A drag that starts on a row outside the selection moves that row alone. It does not
        // silently take the selection with it: the user is pointing at something they have not
        // selected, and moving forty entities they cannot see highlighted would be the worst kind
        // of surprise -- one whose result is off the screen.
        if (std::find(selection.begin(), selection.end(), dragged) == selection.end())
        {
            return {dragged};
        }

        std::vector<Uuid> moving;
        moving.reserve(selection.size());
        for (const Uuid& id : selection)
        {
            if (!id.isValid() || scene.findEntity(id) == nullptr) { continue; }

            // Already coming with its parent. Reparenting it as well would pull it out of the
            // thing it is travelling with and leave it a sibling, which is the opposite of what
            // dragging a parent and its child together looks like it should do.
            const bool underAnotherMember =
                std::any_of(selection.begin(), selection.end(), [&](const Uuid& other) {
                    return other != id && scene.isAncestorOf(other, id);
                });
            if (underAnotherMember) { continue; }

            moving.push_back(id);
        }

        return moving;
    }

    StudioReparentPlan studioOutlinerReparentPlan(const SceneDocument& scene,
                                                  const std::vector<Uuid>& moving,
                                                  const Uuid& parent)
    {
        StudioReparentPlan plan;

        // The nil parent is the root, which is a real destination rather than a missing one: it is
        // what a detach means. Anything else has to exist, or the drop landed on a row the scene no
        // longer has and doing nothing is the only honest answer.
        if (parent.isValid() && scene.findEntity(parent) == nullptr) { return plan; }

        for (const Uuid& id : moving)
        {
            if (!id.isValid() || scene.findEntity(id) == nullptr) { continue; }

            // Checked here rather than left to the document. `reparentEntity` rejects a cycle and
            // leaves the scene untouched, so pushing the command would be *harmless* -- and would
            // put an undo entry on the stack that undoes nothing.
            if (id == parent || (parent.isValid() && scene.isAncestorOf(id, parent)))
            {
                plan.refused = true;
                plan.entities.clear();
                return plan;
            }

            if (scene.findEntity(id)->getParentId() == parent) { continue; }

            plan.entities.push_back(id);
        }

        return plan;
    }

    bool studioBeginOutlinerRename(const SceneDocument& scene, const Uuid& entityId,
                                   StudioTreeState& state)
    {
        const StudioEntity* entity = scene.findEntity(entityId);
        if (entity == nullptr) { return false; }

        state.beginRename(entityId.toString(), entity->getName());
        return true;
    }

    StudioOutlinerResult studioOutlinerPanel(StudioFrame& frame, const UiRect& bounds,
                                             StudioContext& context, StudioTreeState& state,
                                             std::string* search)
    {
        StudioOutlinerResult result;

        const StudioTheme& theme = frame.theme();
        const SceneDocument& scene = context.getScene();

        // Two empties, said apart. "No scene is open" and "this scene is empty" call for different
        // next actions, and a panel that gave the same words for both would send half its readers
        // looking in the wrong place.
        const std::string_view empty = context.hasProject()
            ? std::string_view{"This scene has no entities yet."}
            : std::string_view{"No project is open."};

        // A third empty, and it needs saying apart from the other two: a search that matches
        // nothing looks exactly like an empty scene, and a user who cannot tell them apart starts
        // wondering where their level went rather than clearing the box.
        const std::string_view noMatches{"Nothing here matches that."};

        if (bounds.width <= 0.0f || bounds.height <= 0.0f) { return result; }

        // The search field, and the tree below whatever is left (`plan.md` STUDIO-13002). A caller
        // that passes nothing gets the panel exactly as it was, which is what keeps every existing
        // test and the headless paths meaning what they meant.
        UiRect treeBounds = bounds;
        if (search != nullptr)
        {
            const float pad = static_cast<float>(theme.metric(StudioMetric::SpacingSmall));
            const float height = static_cast<float>(theme.metric(StudioMetric::ControlHeight));
            const UiRect field{bounds.x + pad, bounds.y + pad, bounds.width - pad * 2.0f, height};

            if (field.width > 0.0f && field.height + pad * 2.0f < bounds.height)
            {
                StudioTextFieldOptions options;
                options.placeholder = "Search entities";
                options.font = StudioFontRole::BodySmall;
                (void)studioTextField(frame, frame.ids().make("outliner.search"), field, *search,
                                      options);

                treeBounds = UiRect{bounds.x, field.bottom() + pad, bounds.width,
                                    bounds.height - (field.bottom() + pad - bounds.y)};
            }
        }

        const StudioOutlinerFilter filter =
            search != nullptr ? studioOutlinerFilter(scene, *search) : StudioOutlinerFilter{};

        // Derived once and shared by the count and the window (STUDIO-13011), and since
        // STUDIO-30011 the document keeps it between frames -- so a scene nobody has changed is
        // walked without being rebuilt at all. A reference rather than a copy: at twenty thousand
        // entities the copy was twenty thousand vectors, which is most of what the cache saves.
        const std::unordered_map<Uuid, std::vector<Uuid>>& hierarchy = scene.getChildrenByParent();

        // The count first, then the window. Counting walks the tree and builds nothing, which is
        // the difference between a scene of fifty thousand entities costing fifty thousand
        // increments and costing fifty thousand rows of three strings each, twice a frame.
        const std::size_t total = walkRoots(scene, hierarchy, {}, state, filter, 0,
                                            std::numeric_limits<std::size_t>::max(), nullptr);
        result.rowsTotal = total;

        if (total == 0)
        {
            if (frame.isDrawPass())
            {
                studioDrawText(
                    frame,
                    treeBounds.inset(UiEdges{static_cast<float>(theme.metric(
                        StudioMetric::SpacingMedium))}),
                    filter.active ? noMatches : empty, StudioFontRole::Body,
                    theme.color(StudioColorRole::TextSecondary));
            }
            return result;
        }

        StudioScrollOptions scroll;
        scroll.contentHeight = static_cast<float>(total) * studioTreeRowHeight(theme);
        scroll.wheelStep = studioTreeRowHeight(theme) * 3.0f;

        const StudioScrollResult view =
            studioBeginScroll(frame, frame.ids().make("treescroll"), treeBounds, scroll);

        const StudioTreeWindow window = studioTreeWindow(view, total, theme);

        std::vector<StudioTreeRow> rows;
        rows.reserve(window.rowCount);
        (void)walkRoots(scene, hierarchy, context.getSelection(), state, filter, window.firstRow,
                        window.firstRow + window.rowCount, &rows);
        result.rowsBuilt = rows.size();

        const StudioTreeResult tree = studioTreeRows(frame, view, rows, state, window);
        studioEndScroll(frame);
        result.rowsDrawn = tree.rowsDrawn;

        if (tree.renamed.has_value())
        {
            const Uuid id = Uuid::parse(rows[*tree.renamed].id);
            if (id.isValid())
            {
                // Through the history, like every other edit. A rename that could not be undone
                // would be the one change in the editor that is not a change.
                context.execute(std::make_unique<RenameEntityCommand>(context.getScene(), id,
                                                                      tree.renamedTo));
                result.renamed = true;
            }
        }

        if (tree.toggledRowAction.has_value())
        {
            const Uuid id = Uuid::parse(rows[*tree.toggledRowAction].id);
            const StudioEntity* entity = context.getScene().findEntity(id);
            if (entity != nullptr)
            {
                // Through the history, like every other edit, and *before* the click below is
                // considered: a press on the toggle is not a press on the row, and handling both
                // would hide an entity and select it in one gesture.
                context.execute(std::make_unique<SetEntityEnabledCommand>(
                    context.getScene(), id, !entity->isEnabled()));
                result.visibilityChanged = true;
            }
            return result;
        }

        // Before the click, and returning rather than falling through: a drop lands on the row it
        // was released over, and treating that as a press as well would reparent an entity and
        // select the thing it was dropped onto in one gesture.
        if (tree.dropped.has_value() && *tree.dropped < rows.size())
        {
            const Uuid parent = Uuid::parse(rows[*tree.dropped].id);

            // An asset dropped on a row means "put one of these in the scene, under that"
            // (STUDIO-09008) -- a different operation from the reparent an entity drop means, and
            // told apart by the payload's type rather than by guessing from the id.
            if (tree.droppedType == kStudioAssetDragType)
            {
                result.assetDropped = Uuid::parse(tree.droppedValue);
                result.assetDropParent = parent;
                return result;
            }

            const Uuid child = Uuid::parse(tree.droppedValue);

            // The whole selection when the drag started on part of it (`plan.md` STUDIO-13004).
            // A user who has just shift-selected forty entities and drags one of them means all
            // forty; an outliner that moved the one row under the pointer would make them repeat
            // the gesture thirty-nine times.
            const std::vector<Uuid> moving =
                studioOutlinerDragSet(context.getScene(), context.getSelection(), child);
            const StudioReparentPlan plan =
                studioOutlinerReparentPlan(context.getScene(), moving, parent);

            if (plan.refused) { result.reparentRefused = true; }
            else if (!plan.entities.empty())
            {
                // Through the history, like every other edit, and one entry for the whole drop:
                // a gesture the user made once is a gesture one Ctrl+Z puts back. The children of
                // each entity come with it because they are found *through* it, so there is
                // nothing else to record.
                auto batch = std::make_unique<CompositeCommand>(
                    plan.entities.size() == 1
                        ? std::string{"Reparent entity"}
                        : "Reparent " + std::to_string(plan.entities.size()) + " entities");
                for (const Uuid& id : plan.entities)
                {
                    // Each command reads the entity's world transform as it is built, which is
                    // before any of them runs. Safe here because nothing in the set moves anything
                    // else in it: descendants of a moving entity were left out of the set, and a
                    // target underneath one of them is the cycle the plan already refused.
                    batch->add(std::make_unique<ReparentEntityCommand>(context.getScene(), id,
                                                                      parent));
                }
                context.execute(std::move(batch));

                result.reparented = true;
                result.reparentedCount = plan.entities.size();
            }
            return result;
        }

        if (tree.clicked.has_value())
        {
            const StudioTreeRow& row = rows[*tree.clicked];
            // The row id *is* the entity's UUID, printed by the flattener above -- so a parse that
            // came back nil would mean the rows and the scene had gone out of step, not that the
            // user clicked something odd. Checked anyway, because selecting the nil entity would
            // clear the inspector and look like a bug in the inspector.
            const Uuid id = Uuid::parse(row.id);
            if (id.isValid())
            {
                // Through the context, which is what the viewport, the inspector and the gizmos
                // all read. A panel with its own idea of what is selected disagrees with the rest
                // of the editor the moment anything else changes it.
                const Uuid anchor = Uuid::parse(state.getSelectionAnchor());
                const std::vector<Uuid> range =
                    tree.rangeSelect ? studioOutlinerRange(scene, state, filter, anchor, id)
                                     : std::vector<Uuid>{};

                if (!range.empty())
                {
                    // Control *and* Shift adds the run to what is already selected, which is how a
                    // selection is built out of several of them. Shift alone replaces, exactly as
                    // a plain click does.
                    std::vector<Uuid> merged = tree.additive ? context.getSelection()
                                                             : std::vector<Uuid>{};
                    for (const Uuid& member : range)
                    {
                        if (std::find(merged.begin(), merged.end(), member) == merged.end())
                        {
                            merged.push_back(member);
                        }
                    }
                    context.setSelection(std::move(merged));

                    // The anchor stays where it was: a user extending a run by shift-clicking
                    // further down means "from the same place, further", and moving it would make
                    // the second shift-click measure from the first one's far end.
                }
                else if (tree.additive)
                {
                    context.toggleSelection(id);
                    state.setSelectionAnchor(row.id);
                }
                else
                {
                    context.select(id);
                    state.setSelectionAnchor(row.id);
                }

                result.selectionChanged = true;
            }
        }

        return result;
    }
}
