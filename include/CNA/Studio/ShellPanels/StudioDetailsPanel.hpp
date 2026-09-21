// SPDX-License-Identifier: MS-PL
/**
 * @file CNA/Studio/ShellPanels/StudioDetailsPanel.hpp
 * @brief The Details panel — the selected entity's components and their properties.
 *
 * `plan.md` STUDIO-07007.
 *
 * The third panel ported, and the first that *writes* to the document. That is the whole difference
 * between it and the outliner: showing a scene wrong is a bad afternoon, and editing one wrong is a
 * lost afternoon's work, so every edit here goes through the command history rather than touching
 * an entity directly.
 *
 * ### Committed, not continuous
 *
 * A field writes on Enter or on losing focus, never on each keystroke. A property bound to a field
 * that wrote per character would put one undo entry per letter, and would parse a number while it
 * is half-typed — `1e` on the way to `1e-3` is not a number, and rejecting it mid-word is how an
 * inspector becomes impossible to type into.
 *
 * ### What is editable, and what is honestly not yet
 *
 * Booleans, integers, floats, strings, enumerations and the two- and three-component vectors are
 * editable. Colours, quaternions, rectangles, references, lists and structures are *shown* with
 * what they hold and labelled as not editable yet — because a property nobody can see is worse than
 * one nobody can change, and a control that looked editable and silently did nothing would be worse
 * than both. `STUDIO-07018` is the rest, and it wants pickers rather than more text fields: a
 * colour typed as four numbers and a rotation typed as four is how an inspector gets a reputation.
 *
 * ### The audio preview belongs to the component, not to the entity
 *
 * `STUDIO-07044`. The prototype draws one preview per *entity*, found with `findComponent` — the
 * first audio source on it. `CNA.AudioSource` is declared `unique = false`, so an entity may carry
 * several, and the prototype's preview can only ever hear one of them. Here each source draws its
 * own, under its own properties, playing its own clip at its own volume, pan and pitch.
 */

#pragma once

#include "CNA/Studio/Assets/AssetDependencies.hpp"
#include "CNA/Studio/Assets/AssetDocumentCache.hpp"
#include "CNA/Studio/Assets/TextureImport.hpp"
#include "CNA/Studio/Core/PropertyValue.hpp"
#include "CNA/Studio/Core/Uuid.hpp"
#include "CNA/Studio/Scene/SceneValidation.hpp"
#include "CNA/Studio/Scene/SpriteAnimation.hpp"
#include "CNA/Studio/Ui/UiDrawData.hpp"
#include "CNA/Studio/UiCore/StudioFrame.hpp"
#include "CNA/Studio/UiCore/UiRect.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include <string>

namespace CNA::Studio
{
    class StudioAudio;
    class StudioContext;

    /**
     * @brief What an audio preview control did this frame.
     *
     * `plan.md` STUDIO-07044. Reported rather than logged here, like every other result this panel
     * returns: a panel that wrote to the Output Log would be one that has to be given a log, and
     * the binder that already has one is where the other panels' messages come from.
     */
    struct StudioAudioPreviewResult
    {
        /** @brief How many preview controls were drawn -- one per audio source, plus the asset's. */
        std::size_t controls = 0;

        /** @brief Play was pressed this frame. Input pass only. */
        bool played = false;

        /** @brief Stop was pressed this frame. Input pass only. */
        bool stopped = false;

        /**
         * @brief Whether the device actually took the clip.
         *
         * Separate from @ref played because the two differ and the difference is the user's to
         * know: a clip that will not load and a clip of silence sound identical, and only one of
         * them is something to fix.
         */
        bool started = false;

        /** @brief The clip a Play was asked for, by source path, for the message. */
        std::string clip;
    };

    /**
     * @brief What the Details panel may reach beyond the document.
     *
     * `plan.md` STUDIO-07044, STUDIO-07043. The same shape as the viewport's services and for the
     * same reason: playing a sound and sampling a texture both need CNA, exactly one module may
     * link CNA, and the panel has to keep working in a headless run that has neither.
     */
    struct StudioDetailsServices
    {
        /**
         * @brief Plays one clip at a time. Unset means this build cannot play anything.
         *
         * A null seam draws the control *disabled and says why*, rather than hiding it or
         * offering a button that does nothing. A preview that is simply absent on a build without
         * audio is a feature the user cannot tell from one that was never written.
         */
        StudioAudio* audio = nullptr;

        /**
         * @brief Resolves an image asset to a UI texture, or `kUiTextureNone`.
         *
         * `plan.md` STUDIO-07043. Unset means this build cannot show a picture of anything: the
         * sprite preview then draws the frame's box and says which texels it names, which is
         * everything the animation is except the pixels — and is what a build with no device
         * honestly knows.
         */
        std::function<UiTextureId(const Uuid&)> thumbnail;

        /**
         * @brief Names the effect this build's model pass actually draws through.
         *
         * `plan.md` STUDIO-07046. Which effect a build got decides whether metallic and roughness
         * reach the screen at all (CNA gap G-05), so the material editor says which one it is
         * rather than leaving a user to wonder why a slider does nothing. Unset leaves the line
         * off, which is honest for a build with no renderer to ask.
         */
        std::function<std::string()> modelEffectName;

        /**
         * @brief The project's asset reference graph, for the asset inspector's dependency section.
         *
         * `plan.md` STUDIO-09012. A seam rather than something the panel builds, because building
         * it reads every scene, prefab and material in the project — which is a decision about
         * *when*, and the panel that draws a section is the wrong place to make it.
         *
         * Unset draws the section saying the index has not been built, rather than omitting it: a
         * section that is simply absent is one the user cannot tell from an asset nothing
         * references, and those are opposite answers to the question they asked.
         */
        const AssetDependencyIndex* dependencies = nullptr;

        /**
         * @brief Materials and prefabs, read when they change rather than when they are drawn.
         *
         * `plan.md` STUDIO-30016. A seam rather than something the panel owns, for the reason the
         * dependency index is: deciding *when* a file is re-read is a policy, and the function that
         * draws a section is the wrong place for one.
         *
         * Unset reads the file on every frame, which is what this panel did before and is still
         * correct — a test that constructs the panel with no services gets the same picture, more
         * slowly.
         */
        StudioAssetDocumentCache* documents = nullptr;
    };

    /**
     * @brief A byte count as something a person reads: `"1.4 MB"`.
     *
     * `plan.md` STUDIO-09014. Binary units, because that is what a file manager on every platform
     * shows, and an asset browser disagreeing with the file manager beside it is a browser people
     * stop trusting for the numbers they *can* check.
     *
     * Exposed rather than left inside the panel because rounding is the kind of thing that is wrong
     * without looking wrong, and a test that had to build a frame to check it would not be written.
     *
     * @param bytes The size.
     * @return The size with a unit, one decimal below ten and none above.
     */
    [[nodiscard]] std::string studioDescribeByteSize(std::uint64_t bytes);

    /**
     * @brief A scan's modification stamp as a local date and time.
     *
     * `plan.md` STUDIO-09014. The stamp is seconds on the *filesystem* clock, whose epoch is not
     * the system clock's on every platform — so it is converted rather than handed to a Unix-seconds
     * formatter, which would produce a date decades out wherever the two differ.
     *
     * @param fileClockSeconds `AssetRecord::sourceModifiedTime`. Zero means unknown.
     * @return The date and time, or `"unknown"`.
     */
    [[nodiscard]] std::string studioDescribeFileTime(std::int64_t fileClockSeconds);

    /**
     * @brief What the prefab section reported and did this frame.
     *
     * `plan.md` STUDIO-07042. Returned rather than logged, like everything else this panel
     * produces: the binder that already owns the Output Log is where the message belongs.
     */
    struct StudioPrefabSectionResult
    {
        /** @brief The selected entity is part of a prefab instance, so the section was drawn. */
        bool present = false;

        /** @brief How many ways the instance differs from its prefab. */
        std::size_t overrides = 0;

        /** @brief Revert was pressed and the command ran. Input pass only. */
        bool reverted = false;

        /** @brief Apply was pressed and the command ran. Input pass only. */
        bool applied = false;

        /**
         * @brief What to say about it: a summary on success, the reason on a failure.
         *
         * Apply *writes the prefab file*, which is the one action in this panel that changes an
         * asset every other instance of that prefab is about to be compared against — so "it did
         * not work" has to reach the user rather than being a button that did nothing.
         */
        std::string message;

        /** @brief True when @ref message is a failure rather than a summary. */
        bool failed = false;
    };

    /** @brief What the Details panel did this frame. */
    struct StudioDetailsResult
    {
        /**
         * @brief A component section was opened or closed this frame (`plan.md` STUDIO-14001).
         *
         * Input pass only. Reported because folding is the one thing the panel does that changes
         * nothing in the document -- so a shell that redrew only on a document change would leave
         * the click looking like it had missed.
         */
        bool sectionFolded = false;

        /**
         * @brief How many rows the panel *measured* before drawing any (`plan.md` STUDIO-14001).
         *
         * Not the same number as @ref rowsDrawn and not meant to be: the measure reserves the
         * optional sections at their maximum, because it runs before the comparisons that decide
         * whether they appear, and a scroll view a row too tall is invisible where one a row too
         * short clips the last control.
         *
         * Reported so the two can be held in step where it matters. Folding a section has to change
         * both by the same amount -- the scroll view is sized from the measure, and a measure that
         * kept counting rows the draw had stopped drawing would let the panel scroll past its own
         * last control with no way to see it from outside.
         */
        std::size_t rowsMeasured = 0;

        /**
         * @brief How many rows were laid out and then skipped, being off screen (STUDIO-14018).
         *
         * @ref rowsDrawn counts every row the cursor advanced past, which is what the scroll
         * region is sized against and what tells a caller whether a section folded. It is *not* a
         * measure of work: a row off the top of the viewport still advances the cursor. The
         * difference between the two is what the panel actually described, and that is the number
         * that has to stay bounded however many properties a component declares.
         */
        std::size_t rowsCulled = 0;

        /**
         * @brief How many properties were reset to their default this frame (`plan.md` STUDIO-14012).
         *
         * Input pass only. Reported apart from @ref edited because a reset is the one edit a user
         * makes to *undo* their own work, and a shell that could not tell the two apart would say
         * edited about a click that put something back the way it was.
         */
        std::size_t propertiesReset = 0;

        /** @brief How many property values were copied to the clipboard (`plan.md` STUDIO-14014). */
        std::size_t propertiesCopied = 0;

        /** @brief Rows offered across every asset picker drawn, `(none)` included (STUDIO-19009). */
        std::size_t assetChoicesOffered = 0;

        /**
         * @brief Features this material asks for that the build's effect cannot draw.
         *
         * `plan.md` STUDIO-19008. Counted rather than only drawn, because "the editor warns about
         * this" is exactly the claim that passes a test of the rule while the panel prints
         * nothing.
         */
        std::size_t materialCapabilityIssues = 0;

        /**
         * @brief Assets of the wrong kind dropped on a slot and not taken (STUDIO-19009).
         *
         * Reported rather than swallowed. A drop that changes nothing and says nothing is
         * indistinguishable from a drop the editor missed, and the user's next move is to try it
         * again harder.
         */
        std::size_t dropsRefused = 0;

        /** @brief How many were pasted from it. @see propertiesCopied */
        std::size_t propertiesPasted = 0;

        /**
         * @brief How many entities this panel is editing (`plan.md` STUDIO-14017).
         *
         * One for the ordinary case and the size of the selection otherwise. Reported so the shell
         * can say so: an edit that silently reached five entities is as surprising as one that
         * silently reached one of five.
         */
        std::size_t entitiesEdited = 0;


        /** @brief How many property rows were drawn. */
        std::size_t rowsDrawn = 0;

        /** @brief How many components the selected entity has. */
        std::size_t componentCount = 0;

        /**
         * @brief How many properties were shown as a summary because no editor handles their kind.
         *
         * Reported rather than left to be noticed. A kind that falls through to "(not editable
         * yet)" looks deliberate and reads as a decision, which is how one stays unimplemented
         * long after the widget it needed arrived — so a test can assert on this number instead of
         * on a screenshot nobody will look at twice.
         */
        std::size_t readOnlyProperties = 0;

        /** @brief A property was committed to the document this frame. Input pass only. */
        bool edited = false;

        /** @brief What was edited, for the log: `Transform.position`, say. */
        std::string editedProperty;

        /** @brief What the audio preview controls did. */
        StudioAudioPreviewResult audio;

        /**
         * @brief Which sprite frame the preview is showing, for the viewport to draw the same one.
         *
         * `plan.md` STUDIO-07043. A *snapshot* travels, never the playback: a scene that recorded
         * the frame an artist happened to be paused on would carry it into every save and every
         * diff (`ANALYSIS.md` decision D-07). Inactive when nothing is being previewed.
         */
        AnimationPreview animation;

        /** @brief How many frames the previewed clip has. Zero when there is no preview. */
        std::size_t animationFrames = 0;

        /** @brief What the prefab section reported. */
        StudioPrefabSectionResult prefab;

        /**
         * @brief How many editable material fields were drawn, or zero for anything else.
         *
         * `plan.md` STUDIO-07046. Reported so a test can assert the editor appeared without
         * reading pixels -- and so "the material editor is a heading with nothing under it" is a
         * failure a test can name.
         */
        std::size_t materialFields = 0;

        /**
         * @brief How many dependency rows the asset inspector drew, both directions together.
         *
         * `plan.md` STUDIO-09012. Reported so a test can assert the section found the references
         * rather than reading pixels -- and so "the dependency section is two headings with
         * nothing under them" is a failure a test can name.
         */
        std::size_t dependencyRows = 0;

        /**
         * @brief The asset a dependency row was clicked to go to, on the frame it was.
         *
         * The section is a way *through* the graph, not a read-only report: the answer to "what
         * uses this" is usually followed by "and what does that use", and a list nobody can click
         * makes the user find the file in the Content Browser themselves.
         */
        Uuid navigatedToAsset;

        /**
         * @brief How many relink suggestions the inspector offered for a missing file.
         *
         * `plan.md` STUDIO-09013. Zero when the asset's file is present, which is the ordinary
         * case, and also when nothing in the project looks like it — the two are distinguished by
         * what the panel says rather than by this number, which exists so a test can assert the
         * suggestions were found without reading pixels.
         */
        std::size_t relinkCandidates = 0;

        /** @brief A relink was applied this frame. Input pass only. */
        bool relinked = false;

        /**
         * @brief An import setting was reset to its importer default this frame. Input pass only.
         *
         * `plan.md` STUDIO-09014. Distinguished from @ref edited, which it also sets: resetting
         * *removes* the setting from the sidecar rather than writing the default into it, and a
         * caller that logged both the same way would tell the user the opposite of what happened.
         */
        bool resetProperty = false;

        /**
         * @brief What the shown texture's import settings resolve to, or a default plan otherwise.
         *
         * `plan.md` STUDIO-10003. Recomputed each frame and stored nowhere else: it is a
         * *derivation* of the facts and the settings, and a sidecar holding one would be a
         * `mipLevels` that disagrees with the "Generate Mipmaps" above it the moment somebody
         * unticks the box. Reported so a test can assert the inspector says what a setting
         * actually produces, without reading pixels.
         */
        StudioTextureImportPlan texturePlan;
    };

    /**
     * @brief Draws the Details panel and applies what the user committed.
     *
     * @param frame The frame.
     * @param bounds The panel's content rectangle.
     * @param context The editor. Its selection decides what is shown; its history receives edits.
     * @param services What the panel cannot reach itself -- the audio seam the preview plays
     *        through. Defaulted, so a build with no audio draws the same panel with the preview
     *        disabled rather than a different one.
     * @return What happened.
     */
    /**
     * @brief What a property editor may reach beyond the value it is editing.
     *
     * `plan.md` STUDIO-07045. Only the reference pickers need anything: an asset reference offers
     * every asset in the project and an entity reference every entity in the scene, so the editor
     * has to be able to see them. A null context offers neither and falls back to showing the id,
     * which is what an editor over a value with no document behind it can honestly do.
     */
    struct StudioPropertyEditContext
    {
        /** @brief The document, for the reference pickers. Null offers no picker. */
        const StudioContext* context = nullptr;

        /** @brief An entity that must not appear in an entity-reference picker -- itself. */
        Uuid excludeEntity;

        /**
         * @brief Which kind of asset an asset-reference slot takes, from its descriptor.
         *
         * `plan.md` STUDIO-19009. Empty accepts anything, which is what an editor over a bare
         * value with no descriptor behind it can honestly offer. Carried here rather than read
         * from a descriptor inside the editor, because the editor is given a `PropertyValue` and
         * the descriptor belongs to whoever laid the row out.
         */
        std::string assetType;

        /**
         * @brief The inclusive range a numeric property declares, from its descriptor.
         *
         * `plan.md` STUDIO-19003. When `minimum < maximum` the editor draws a slider beside the
         * field instead of a field alone, and clamps what the field commits. Equal ends mean no
         * range was declared, which is what every property without one has always been.
         *
         * Carried here rather than read from a descriptor inside the editor, for the reason
         * `assetType` is: the editor is given a `PropertyValue` and the descriptor belongs to
         * whoever laid the row out.
         */
        double minimum = 0.0;
        double maximum = 0.0;
    };

    /**
     * @brief One structural change to a list property.
     *
     * `STUDIO-07054`. Separated from the drawing because this is where the off-by-ones live: moving
     * the first element up and removing the last one are the two operations that get written wrong,
     * and neither is visible in a screenshot — a list that silently refused to move its first
     * element looks exactly like a list whose first element is already where it should be.
     */
    enum class StudioListEdit : std::uint8_t
    {
        /** @brief Nothing was asked for. */
        None,
        /** @brief Append an element after the last one. */
        Add,
        /** @brief Remove the element at the index. */
        Remove,
        /** @brief Swap the element with the one before it. */
        MoveUp,
        /** @brief Swap the element with the one after it. */
        MoveDown
    };

    /**
     * @brief Applies @p edit to @p list at @p index.
     *
     * Refuses rather than clamps. An out-of-range index, a move off either end, or a remove from an
     * empty list all leave the list alone and return false — because the caller pushes an undo
     * entry on true, and an entry that changes nothing is one the user presses Ctrl+Z on and
     * watches do nothing.
     *
     * @param list The list, modified in place on success.
     * @param edit What to do.
     * @param index Which element, ignored for @ref StudioListEdit::Add.
     * @param prototype The value a new element starts as. Copied, so an element is added with the
     *                  kind the list already holds rather than as an empty one nothing can read.
     * @return Whether the list changed.
     */
    [[nodiscard]] bool studioApplyListEdit(PropertyValue::ListValue& list, StudioListEdit edit,
                                           std::size_t index, const PropertyValue& prototype);

    /** @brief What one property row's editor produced. */
    struct StudioPropertyEditResult
    {
        /** @brief The new value, when the user committed one. */
        std::optional<PropertyValue> edited;

        /**
         * @brief Whether this kind has no editor and was shown as text.
         *
         * Reported rather than silently drawn, because "this property cannot be edited here" is
         * the answer a caller counting editable rows needs and is invisible in a capture.
         */
        bool readOnlyKind = false;

        /**
         * @brief A numeric scrub is in flight this frame.
         *
         * `STUDIO-07055`. What tells the caller to push its change as
         * `MergePolicy::MergeWithPrevious`, so a drag across forty pixels is one undo entry rather
         * than forty — which is the difference between an undo stack a user can navigate and one
         * they give up on.
         */
        bool dragging = false;

        /**
         * @brief An asset of the wrong kind was dropped on this slot and was not taken.
         *
         * `plan.md` STUDIO-19009. Reported rather than swallowed: a drop that changes nothing and
         * says nothing is indistinguishable from a drop the editor missed, and the user's next
         * move is to try it again harder.
         */
        bool refusedDrop = false;

        /**
         * @brief How many rows an asset picker offered, `(none)` included. Zero for other kinds.
         *
         * `plan.md` STUDIO-19009. The list is built inside the editor and handed to a dropdown,
         * so it leaves no other trace a test can read -- and "the picker filters" is exactly the
         * claim that would pass a test of the filtering rule while the picker ignored it.
         */
        std::size_t assetChoices = 0;
    };

    /**
     * @brief Draws the control for one property value and reports an edit.
     *
     * Extracted from `studioDetailsPanel` by `STUDIO-07045`, which needed the same editors over an
     * importer's settings rather than over a component's properties. It is the same code rather
     * than a second copy on purpose: a property grid that edited a float one way for a component
     * and another way for an asset would drift, and the drift would be invisible until somebody
     * compared two panels side by side.
     *
     * It draws the *control* only. The label, the row and the command the edit goes through belong
     * to the caller, because those are what differ: a component's edit is a `SetPropertyCommand`
     * against the scene and an importer setting's is a `SetImporterSettingCommand` against the
     * asset database.
     *
     * @param frame The frame.
     * @param control The control's rectangle, to the right of the label.
     * @param value The value to edit; its type chooses the control.
     * @param enumOptions The closed set for an enumeration, or empty to fall back to typing.
     * @param editing What the reference pickers may look at.
     * @return The edit, if the user made one.
     */
    StudioPropertyEditResult studioPropertyEditor(StudioFrame& frame, UiRect control,
                                                  const PropertyValue& value,
                                                  const std::vector<std::string>& enumOptions,
                                                  const StudioPropertyEditContext& editing);

    /**
     * @brief One entity's validation issues, arranged the way the Inspector draws them.
     *
     * `plan.md` STUDIO-14015. The Details panel showed no validation at all: the only way to learn
     * an entity was broken was to open the Problems panel and find it in a list, which is a panel
     * away from the one where the fix is made.
     *
     * Prepared by the caller from `validateScene`, for the reason the Outliner's set is
     * (`STUDIO-13010`): validating walks the whole document, and doing it per frame would make
     * every panel that shows an entity pay for every entity in the scene.
     */
    struct StudioInspectorIssues
    {
        /** @brief The issues naming this entity, in the validator's own order. */
        std::vector<SceneIssue> forEntity;

        /**
         * @brief Those naming @p componentTypeId.
         *
         * An issue that names no component belongs to the entity rather than to any one section,
         * and is not returned here -- see @ref forEntityItself.
         */
        [[nodiscard]] std::vector<const SceneIssue*> forComponent(
            const std::string& componentTypeId) const;

        /** @brief Those naming the entity but no component. */
        [[nodiscard]] std::vector<const SceneIssue*> forEntityItself() const;
    };

    /**
     * @brief Picks @p entityId's issues out of @p issues.
     *
     * Scene-wide issues -- the ones naming no entity -- are left out: they belong to the Problems
     * panel, and hanging "two primary cameras" on whichever camera happens to be selected would
     * name a culprit the rule does not have.
     */
    [[nodiscard]] StudioInspectorIssues studioInspectorIssues(const std::vector<SceneIssue>& issues,
                                                              const Uuid& entityId);

    /**
     * @brief The component types every entity in @p selection carries, in the last one's order.
     *
     * `plan.md` STUDIO-14017. The Inspector shows one entity's components and, with several
     * selected, showed the *last* one's and edited only it -- so a user who selected five crates
     * and set their scale changed one and was told nothing.
     *
     * The intersection rather than the union, because a component only some of them have is
     * exactly the case the task's own title excludes: there is no unambiguous answer to what
     * editing it should do, and picking one silently is how a user loses work they did not know
     * they were doing.
     *
     * In the *last* selected entity's order, because that is the entity the panel is built around
     * and a list that reordered itself as the selection grew would be one a user cannot learn.
     *
     * A selection of one is that entity's own components, which is what makes this the only path
     * rather than a second one for the multi case.
     */
    [[nodiscard]] std::vector<std::string> studioSharedComponents(const SceneDocument& scene,
                                                                  const std::vector<Uuid>& selection);

    /**
     * @brief The value every selected entity gives @p propertyName, when they agree.
     *
     * `plan.md` STUDIO-14017. Nothing when they differ, which is the case the editor must not show
     * a single number for: a field reading 3 over five entities of which four are 7 is a field that
     * lies, and the user finds out by overwriting the four.
     *
     * @param scene The scene.
     * @param selection The entities being edited.
     * @param componentTypeId Which component.
     * @param propertyName Which property of it.
     * @param descriptor The component's descriptor, for defaults; may be null.
     */
    [[nodiscard]] std::optional<PropertyValue> studioSharedPropertyValue(
        const SceneDocument& scene, const std::vector<Uuid>& selection,
        const std::string& componentTypeId, const std::string& propertyName,
        const ComponentDescriptor* descriptor);

    /**
     * @brief The clipboard text for @p value.
     *
     * `plan.md` STUDIO-14014. JSON, and the same JSON a scene file holds, because that is already
     * the one definition of what a `PropertyValue` looks like written down -- a second encoding
     * invented for the clipboard would be a second thing to keep in step with the first, and the
     * first is the one that has to survive a release.
     *
     * Plain text on purpose: a user who copies a position and pastes it into a bug report, a chat
     * window or a script should get something readable, and one who pastes a readable thing back
     * should be understood.
     */
    [[nodiscard]] std::string studioCopyPropertyText(const PropertyValue& value);

    /**
     * @brief Reads @p text back as a property of type @p expected, or nothing.
     *
     * `plan.md` STUDIO-14014. Refuses text that is not JSON, and text that parses to a value of a
     * different kind: pasting a colour into a number is a mistake, and coercing it would produce a
     * value the user did not ask for in a field they were not looking at. Refusing says so at the
     * moment they can still do something about it.
     *
     * @param text What the clipboard holds.
     * @param expected The type the target property declares.
     * @param elementType For a list, what its elements are; ignored otherwise.
     */
    [[nodiscard]] std::optional<PropertyValue> studioPastedProperty(
        std::string_view text, PropertyType expected,
        PropertyType elementType = PropertyType::None);

    /** @brief One entry of the Inspector's Add Component list. */
    struct StudioComponentChoice
    {
        /** @brief What the list shows: `"Category / Display Name"`, or the name when uncategorised. */
        std::string label;

        /** @brief What gets added. The list is remembered by *this*, never by index. */
        std::string typeId;
    };

    /**
     * @brief What the Add Component list should offer for @p entity.
     *
     * `plan.md` STUDIO-14002. The decision is a CNA-free function of the registry and the entity,
     * so what is offered can be asserted without a frame, a dropdown or a popup -- the widget then
     * shows this and the button adds what it names, which is wiring.
     *
     * A unique component the entity already carries is left out: listing it would be listing an
     * entry that does nothing, `AddComponentCommand` refuses it anyway, and a control that refuses
     * is indistinguishable from one that is broken.
     *
     * Empty means every type is already on the entity, which the panel says in those words rather
     * than showing an empty list.
     */
    [[nodiscard]] std::vector<StudioComponentChoice> studioAddComponentChoices(
        const ComponentRegistry& registry, const StudioEntity& entity);

    /**
     * @brief Whether an asset slot declaring @p assetType accepts an asset of @p candidate.
     *
     * `plan.md` STUDIO-19009. `PropertyDescriptor::assetType` has said which kind a reference
     * takes since the descriptor existed, and the inspector read it nowhere: a material slot
     * listed every sound, font and texture in the project, and accepted a drop of any of them.
     * The field's own doc comment claimed otherwise, which is the worst state for a rule to be in
     * -- written down, believed, and not there.
     *
     * **An empty declaration accepts anything**, which is what an undeclared slot means and what
     * every list-of-everything slot in the editor still is.
     *
     * **A kind this build does not know accepts anything too**, and that is deliberate rather
     * than a fallback. The field is a string so that a plugin can name an asset kind the editor
     * was never compiled against (`ComponentDescriptor.hpp`); filtering on a name that parses to
     * nothing would leave that plugin's slot offering an empty list and refusing every drop,
     * which is worse than the unfiltered behaviour it replaced.
     *
     * CNA-free and free of the frame, so what a slot offers can be asserted without a dropdown, a
     * drag or a pointer.
     */
    [[nodiscard]] bool studioAssetSlotAccepts(std::string_view assetType, AssetType candidate);

    /**
     * @brief Which component sections the Inspector has closed.
     *
     * `plan.md` STUDIO-14001. Caller-owned, like `StudioTreeState`, and for the same reason: the
     * panel is redrawn from scratch every frame, so anything it must remember between frames
     * belongs to whoever outlives one.
     *
     * **Holds the *collapsed* set rather than the expanded one**, so the default is open. A panel
     * that started every section closed would show a user a column of headings and make them work
     * to discover their entity has anything on it -- the same bargain the World Outliner's tree
     * strikes.
     *
     * **Keyed by component *type*, not by entity.** A user who closes Transform means "I am not
     * working on transforms", not "not on this one's" -- so it stays closed as they click through a
     * scene, which is the only way the gesture saves them anything. The consequence is that two
     * components of the same type on one entity close together; that is rare, visible, and a better
     * trade than a fold that springs open on every selection change.
     */
    class StudioDetailsState
    {
    public:
        /** @brief Whether @p componentTypeId's section is open. Unknown types are open. */
        [[nodiscard]] bool isExpanded(std::string_view componentTypeId) const
        {
            return collapsed_.find(std::string{componentTypeId}) == collapsed_.end();
        }

        /** @brief Opens or closes @p componentTypeId's section. */
        void setExpanded(std::string_view componentTypeId, bool expanded)
        {
            if (expanded) { collapsed_.erase(std::string{componentTypeId}); }
            else { collapsed_.insert(std::string{componentTypeId}); }
        }

        /** @brief How many sections are closed. Zero is the state a fresh panel is in. */
        [[nodiscard]] std::size_t collapsedCount() const { return collapsed_.size(); }

    private:
        std::set<std::string> collapsed_;
    };

    /**
     * @brief Draws the Details panel for the current selection.
     *
     * @param frame The frame.
     * @param bounds The panel's content rectangle.
     * @param context The editor. Its scene is read; edits go through its history.
     * @param services Optional collaborators -- audio preview, asset picking.
     * @param state Which component sections are closed (`plan.md` STUDIO-14001). Optional, and a
     *        null one means every section is open: the headless paths and the many cases that care
     *        about a property rather than about folding should not have to carry one.
     */
    StudioDetailsResult studioDetailsPanel(StudioFrame& frame, const UiRect& bounds,
                                           StudioContext& context,
                                           const StudioDetailsServices& services = {},
                                           StudioDetailsState* state = nullptr,
                                           const StudioInspectorIssues& issues = {});

    /**
     * @brief Draws the Material panel: the selected material asset, or what to do to get one.
     *
     * `plan.md` STUDIO-07046. The native shell has registered a `material` panel since the shell
     * existed and never drawn anything in it — a tab a user can raise onto a blank rectangle,
     * which reads as a broken editor rather than as an unfinished one. It shows the same editor
     * the Details panel does, over the same file, because a second implementation of a property
     * grid over one document is how the two come to disagree.
     *
     * Phase 19 is what grows this into a material *editor* — a preview, texture slots, the shader
     * the material compiles to. This is the panel it grows in.
     *
     * @param frame The frame.
     * @param bounds The panel's content rectangle.
     * @param context The editor. Its selected asset decides what is shown.
     * @param services The effect-name seam.
     * @return What happened.
     */
    StudioDetailsResult studioMaterialPanel(StudioFrame& frame, const UiRect& bounds,
                                            StudioContext& context,
                                            const StudioDetailsServices& services = {});
}
