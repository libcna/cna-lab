// SPDX-License-Identifier: MS-PL
/**
 * @file StudioMaterialEditorTests.cpp
 * @brief The material asset editor in the native Details panel.
 *
 * `plan.md` STUDIO-07046, the last of the five Inspector sections `STUDIO-07041` found with no
 * native answer — and the one `docs/MIGRATION-INVENTORY.md` had recorded as not existing at all.
 * It said "there is no `.cnamaterial` editor to port". There is one: `InspectorPanel::
 * drawMaterialAsset`, a *section* of the Inspector rather than a panel, which is exactly why an
 * inventory of panels could not see it.
 *
 * A material is a **file**, which makes this the one editor in Studio whose document is not the
 * scene or the asset database. Every case here is about that: an edit rewrites the file, undo
 * replays the bytes that were there, and a file this build cannot read is refused rather than
 * shown as defaults — because an editable form over a file that was not parsed is an offer to
 * overwrite it with less than it holds.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/Assets/AssetDatabase.hpp"
#include "CNA/Studio/Assets/AssetCommands.hpp"
#include "CNA/Studio/Assets/MaterialCapabilities.hpp"
#include "CNA/Studio/Assets/MaterialPreview.hpp"
#include "CNA/Studio/Assets/MaterialDocument.hpp"
#include "CNA/Studio/Core/Json.hpp"
#include "CNA/Studio/ShellPanels/StudioContentBrowser.hpp"
#include "CNA/Studio/ShellPanels/StudioDetailsPanel.hpp"
#include "CNA/Studio/StudioContext.hpp"
#include "CNA/Studio/UiCore/StudioFontAtlas.hpp"
#include "CNA/Studio/UiCore/StudioFrame.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

using namespace CNA::Studio;

namespace
{
    float metricOf(const StudioTheme& theme, StudioMetric metric)
    {
        return static_cast<float>(theme.metric(metric));
    }

    UiInputState at(float x, float y, bool leftDown = false)
    {
        UiInputState input;
        input.displayWidth = 1280.0f;
        input.displayHeight = 900.0f;
        input.mouseX = x;
        input.mouseY = y;
        input.mouseInWindow = true;
        input.setMouseDown(UiMouseButton::Left, leftDown);
        return input;
    }

    /** @brief A project root with a real `.cnamaterial` in it, removed on the way out. */
    class ScopedProject
    {
    public:
        explicit ScopedProject(const std::string& name)
        {
            path_ = std::filesystem::temp_directory_path()
                  / ("cna-studio-material-" + name + "-" + std::to_string(counter()++));
            std::error_code code;
            std::filesystem::remove_all(path_, code);
            std::filesystem::create_directories(path_ / "Assets", code);
        }

        ~ScopedProject()
        {
            std::error_code code;
            std::filesystem::remove_all(path_, code);
        }

        ScopedProject(const ScopedProject&) = delete;
        ScopedProject& operator=(const ScopedProject&) = delete;

        [[nodiscard]] std::string root() const { return path_.generic_string(); }

        void write(const std::string& relative, const std::string& text) const
        {
            const std::filesystem::path file = path_ / relative;
            std::error_code code;
            std::filesystem::create_directories(file.parent_path(), code);
            std::ofstream stream{file, std::ios::binary};
            stream << text;
        }

        [[nodiscard]] std::string read(const std::string& relative) const
        {
            std::ifstream stream{path_ / relative, std::ios::binary};
            if (!stream) { return {}; }
            return std::string{std::istreambuf_iterator<char>{stream},
                               std::istreambuf_iterator<char>{}};
        }

    private:
        static int& counter() { static int value = 0; return value; }
        std::filesystem::path path_;
    };

    /** @brief The text of a material as a person would have written it. */
    std::string materialText()
    {
        return R"({
 "formatVersion": 1,
 "name": "Painted Red",
 "diffuseColor": [0.85, 0.12, 0.12],
 "emissiveColor": [0, 0, 0],
 "metallic": 0,
 "roughness": 0.6,
 "alpha": 1
}
)";
    }

    /** @brief The Details panel over a selected material asset. */
    struct Fixture
    {
        ScopedProject project;
        StudioContext context;
        StudioFrame frame{StudioTheme::dark()};
        StudioFontAtlas fonts;
        UiRect bounds{0.0f, 0.0f, 520.0f, 880.0f};
        StudioDetailsResult last;
        Uuid material;

        /** @brief Set to have the editor say which effect this build draws through. */
        std::string effect;

        explicit Fixture(const std::string& name, const std::string& text = materialText())
            : project(name)
        {
            frame.setFontAtlas(&fonts);
            project.write("Assets/PaintedRed.cnamaterial", text);
            context.getAssets().setProjectRoot(project.root());

            AssetRecord record;
            record.id = Uuid::generate();
            record.sourcePath = "Assets/PaintedRed.cnamaterial";
            record.type = AssetType::Material;
            material = record.id;
            (void)context.getAssets().add(std::move(record));
            context.selectAsset(material);
        }

        void run(const UiInputState& input)
        {
            runStudioFrame(frame, input, [&](StudioFrame& f) {
                StudioDetailsServices services;
                if (!effect.empty())
                {
                    services.modelEffectName = [this] { return effect; };
                }
                const StudioDetailsResult result = studioDetailsPanel(f, bounds, context, services);
                if (f.isDrawPass()) { last = result; }
            });
        }

        void settle() { run(at(-1.0f, -1.0f)); }

        void click(float x, float y)
        {
            run(at(x, y));
            run(at(x, y, /*leftDown=*/true));
            run(at(x, y));
        }

        void type(const std::vector<char16_t>& characters, float x, float y)
        {
            UiInputState input = at(x, y);
            input.characters = characters;
            run(input);
        }

        void press(UiKey key, float x, float y)
        {
            UiInputState input = at(x, y);
            input.setKeyDown(key, true);
            run(input);
            run(at(x, y));
        }

        /**
         * @brief Focuses the field at @p x, @p y and replaces everything in it with @p text.
         *
         * Select-all rather than click-and-type, because a click puts the caret where it landed:
         * typing "0.2" into a focused 0.6 produces "0.0.26", which is not a number, and a test
         * that did that would be asserting that an invalid edit is refused while believing it was
         * asserting that a valid one lands.
         */
        void replace(float x, float y, const std::vector<char16_t>& text)
        {
            click(x, y);

            UiInputState selectAll = at(x, y);
            selectAll.modifiers = withControl();
            selectAll.setKeyDown(UiKey::A, true);
            run(selectAll);
            run(at(x, y));

            type(text, x, y);
            press(UiKey::Enter, x, y);
        }

        /** @brief The row rectangle at @p index of the panel's content. */
        [[nodiscard]] UiRect row(std::size_t index) const
        {
            const float height = std::max(metricOf(frame.theme(), StudioMetric::ControlHeight),
                                          metricOf(frame.theme(), StudioMetric::MinimumHitTarget));
            const float spacing = metricOf(frame.theme(), StudioMetric::SpacingXSmall);
            const UiRect area =
                bounds.inset(UiEdges{metricOf(frame.theme(), StudioMetric::SpacingSmall)});
            return UiRect{area.x, area.y + static_cast<float>(index) * (height + spacing),
                          area.width, height};
        }

        [[nodiscard]] float controlLeft() const
        {
            const UiRect area =
                bounds.inset(UiEdges{metricOf(frame.theme(), StudioMetric::SpacingSmall)});
            return area.x + std::round(area.width * 0.38f)
                   + metricOf(frame.theme(), StudioMetric::SpacingSmall);
        }

        /**
         * @brief A point inside the *number* of a ranged scalar row.
         *
         * A property with a declared range draws a slider and the number beside it
         * (`plan.md` STUDIO-19003), so the left of the control column is now the slider's track
         * and a click there sets the value by position rather than focusing the field. Near the
         * right edge rather than at it, because the field ends where the column does.
         */
        [[nodiscard]] float numberFieldX() const
        {
            const UiRect area =
                bounds.inset(UiEdges{metricOf(frame.theme(), StudioMetric::SpacingSmall)});
            return area.right() - 20.0f;
        }

        /** @brief The material as the file currently holds it. */
        [[nodiscard]] MaterialDocument onDisk() const
        {
            MaterialDocument document;
            (void)loadMaterialDocument(context.getAssets(), material, document);
            return document;
        }
    };

    // name, path, type, id, a gap, the Material heading, then the fields.
    constexpr std::size_t kNameRow = 6;
    constexpr std::size_t kBaseColourRow = 7;
    constexpr std::size_t kRoughnessRow = 10;

    // The alpha mode follows the six values (`plan.md` STUDIO-19004), and the texture slots follow
    // that. The Mask-only cutoff row sits between them and is absent in every other mode, which is
    // why these are the Opaque-mode positions.
    constexpr std::size_t kAlphaModeRow = 12;
    constexpr std::size_t kBaseColourMapRow = 13;
    constexpr std::size_t kOcclusionMapRow = 17;
}

CNA_STUDIO_TEST(SelectingAMaterialShowsItsFieldsRatherThanAnImporterApology)
{
    // What this row used to say: "A material is edited by the material editor, which the native
    // shell does not have yet". It has one.
    Fixture fixture{"fields"};
    fixture.settle();

    // Six values, the alpha mode (`plan.md` STUDIO-19004) and five texture slots
    // (`STUDIO-19002`). The slots were the half of a material that could only be filled in by
    // editing the JSON by hand: the document has carried four of the ids since ED-403 and the
    // editor could set none of them. Twelve rather than thirteen because this material is Opaque
    // and the cutoff is drawn only for a Mask one.
    CNA_STUDIO_EXPECT_EQ(fixture.last.materialFields, std::size_t{12});
    CNA_STUDIO_EXPECT(fixture.last.rowsDrawn >= kRoughnessRow);
    CNA_STUDIO_EXPECT_EQ(fixture.frame.phaseViolations(), std::size_t{0});
}

CNA_STUDIO_TEST(EditingAMaterialRewritesItsFileAndUndoPutsTheBytesBack)
{
    Fixture fixture{"edit"};
    fixture.settle();

    const std::string before = fixture.project.read("Assets/PaintedRed.cnamaterial");
    CNA_STUDIO_EXPECT(!before.empty());
    CNA_STUDIO_EXPECT_EQ(fixture.onDisk().roughness, 0.6f);

    // The Roughness *number*, which since STUDIO-19003 shares the control column with a slider:
    // the left of that column is the track, and clicking it sets the value by position.
    const UiRect box = fixture.row(kRoughnessRow);
    const float x = fixture.numberFieldX();
    fixture.replace(x, box.centerY(), {u'0', u'.', u'2'});

    CNA_STUDIO_EXPECT_EQ(fixture.onDisk().roughness, 0.2f);
    CNA_STUDIO_EXPECT(fixture.last.edited || fixture.context.getHistory().canUndo());

    // Through the history, which for a file means the previous bytes are replayed verbatim.
    CNA_STUDIO_EXPECT(fixture.context.getHistory().canUndo());
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT_EQ(fixture.project.read("Assets/PaintedRed.cnamaterial"), before);
}

CNA_STUDIO_TEST(EditingOneChannelOfAColourLeavesTheOthersAlone)
{
    // The classic property-grid defect, asked of a row that is three floats and a swatch rather
    // than of the vector editor the component grid uses.
    Fixture fixture{"channel"};
    fixture.settle();

    const UiRect box = fixture.row(kBaseColourRow);

    // Past the swatch, then the middle of the three channel boxes. `numericComponents` divides
    // what is left of the control column into `count` fields with `SpacingSmall` between them.
    const float swatch = metricOf(fixture.frame.theme(), StudioMetric::ControlHeight);
    const float spacing = metricOf(fixture.frame.theme(), StudioMetric::SpacingSmall);
    const float left = fixture.controlLeft() + swatch + spacing;
    const float width = (fixture.row(0).right() - left - spacing * 2.0f) / 3.0f;
    const float x = left + width + spacing + width * 0.5f;

    fixture.replace(x, box.centerY(), {u'0', u'.', u'5'});

    const MaterialDocument written = fixture.onDisk();
    CNA_STUDIO_EXPECT_EQ(written.diffuseColor.y, 0.5f);
    CNA_STUDIO_EXPECT_EQ(written.diffuseColor.x, 0.85f);
    CNA_STUDIO_EXPECT_EQ(written.diffuseColor.z, 0.12f);
}

CNA_STUDIO_TEST(RenamingAMaterialKeepsEverythingElseItHeld)
{
    Fixture fixture{"rename"};
    fixture.settle();

    const UiRect box = fixture.row(kNameRow);
    const float x = fixture.controlLeft() + 40.0f;
    fixture.replace(x, box.centerY(), {u'B', u'l', u'u', u'e'});

    const MaterialDocument written = fixture.onDisk();
    CNA_STUDIO_EXPECT_EQ(written.name, std::string{"Blue"});
    CNA_STUDIO_EXPECT_EQ(written.roughness, 0.6f);
    CNA_STUDIO_EXPECT_EQ(written.diffuseColor.x, 0.85f);
}

CNA_STUDIO_TEST(AMaterialThisBuildCannotReadIsRefusedRatherThanShownAsDefaults)
{
    // Offering an editable form over a file that did not parse is offering to overwrite it with
    // less than it holds. There are no fields at all, so there is nothing to commit.
    Fixture fixture{"broken", "{ this is not json"};
    fixture.settle();

    CNA_STUDIO_EXPECT_EQ(fixture.last.materialFields, std::size_t{0});

    // And the file is untouched: a panel that refused to show it must not have written to it.
    CNA_STUDIO_EXPECT_EQ(fixture.project.read("Assets/PaintedRed.cnamaterial"),
                         std::string{"{ this is not json"});
}

CNA_STUDIO_TEST(AMaterialFromANewerStudioIsRefusedRatherThanDowngraded)
{
    // The one hard failure `MaterialDocument::loadFromJson` has. A future editor's material has
    // fields this build knows nothing about, and saving it back would drop every one of them.
    Fixture fixture{"newer", R"({"formatVersion": 99, "name": "From The Future"})"};
    fixture.settle();

    CNA_STUDIO_EXPECT_EQ(fixture.last.materialFields, std::size_t{0});
    CNA_STUDIO_EXPECT(fixture.project.read("Assets/PaintedRed.cnamaterial")
                          .find("From The Future") != std::string::npos);
}

CNA_STUDIO_TEST(AMaterialWhoseFileHasGoneSaysSo)
{
    Fixture fixture{"missing"};
    std::error_code code;
    std::filesystem::remove(std::filesystem::path{fixture.project.root()}
                                / "Assets/PaintedRed.cnamaterial", code);

    fixture.settle();
    CNA_STUDIO_EXPECT_EQ(fixture.last.materialFields, std::size_t{0});
}

CNA_STUDIO_TEST(TheEditorSaysWhichEffectThisBuildDrawsThrough)
{
    // Which effect a build got decides whether metallic and roughness reach the screen at all
    // (CNA gap G-05). A build with no renderer to ask leaves the line off rather than guessing.
    Fixture fixture{"effect"};
    fixture.settle();
    const std::size_t withoutEffect = fixture.last.rowsDrawn;

    fixture.effect = "PbrEffect";
    fixture.settle();
    CNA_STUDIO_EXPECT_EQ(fixture.last.rowsDrawn, withoutEffect + 1);
}

CNA_STUDIO_TEST(TheMaterialProviderAndTheEditorReadTheSameFileTheSameWay)
{
    // One reader, shared. Two copies of "open it, parse it, load it, and decide what a failure
    // means" would be two chances to disagree about a material that is half-written -- and the
    // editor's copy is the one a user is looking at while the other decides what to draw.
    Fixture fixture{"shared"};

    MaterialDocument direct;
    CNA_STUDIO_EXPECT(loadMaterialDocument(fixture.context.getAssets(), fixture.material, direct)
                      == MaterialLoadProblem::None);

    const MaterialProvider provider = fixture.context.makeMaterialProvider();
    const std::optional<MeshMaterial> resolved = provider(fixture.material);
    CNA_STUDIO_EXPECT(resolved.has_value());
    if (!resolved.has_value()) { return; }

    const MeshMaterial expected = direct.toMeshMaterial();
    CNA_STUDIO_EXPECT_EQ(resolved->diffuseColor.x, expected.diffuseColor.x);
    CNA_STUDIO_EXPECT_EQ(resolved->specularPower, expected.specularPower);
    CNA_STUDIO_EXPECT_EQ(resolved->alpha, expected.alpha);
}

CNA_STUDIO_TEST(LoadingSaysWhichOfTheThreeFailuresItWas)
{
    // Three different problems with three different answers, and only the last of them means
    // "do not offer to overwrite it".
    Fixture fixture{"problems"};
    MaterialDocument document;

    CNA_STUDIO_EXPECT(loadMaterialDocument(fixture.context.getAssets(), Uuid::generate(), document)
                      == MaterialLoadProblem::NotAMaterial);

    std::error_code code;
    std::filesystem::remove(std::filesystem::path{fixture.project.root()}
                                / "Assets/PaintedRed.cnamaterial", code);
    CNA_STUDIO_EXPECT(loadMaterialDocument(fixture.context.getAssets(), fixture.material, document)
                      == MaterialLoadProblem::Unreadable);

    fixture.project.write("Assets/PaintedRed.cnamaterial", "{\"formatVersion\": 99}");
    CNA_STUDIO_EXPECT(loadMaterialDocument(fixture.context.getAssets(), fixture.material, document)
                      == MaterialLoadProblem::UnreadableFormat);

    // And a refused load leaves the caller's own value alone rather than half-overwriting it.
    MaterialDocument untouched;
    untouched.name = "Mine";
    (void)loadMaterialDocument(fixture.context.getAssets(), fixture.material, untouched);
    CNA_STUDIO_EXPECT_EQ(untouched.name, std::string{"Mine"});
}

// ------------------------------------------------------------------------------------------------
// Texture slots (STUDIO-19002)
// ------------------------------------------------------------------------------------------------

/**
 * @brief The occlusion map the document gained, through the format and out the other side.
 *
 * Additive at `formatVersion` 1: a material written before it existed still loads, and one written
 * with it is still a version-1 file. The pair of assertions is the whole of what "additive" means
 * and it is the pair that a careless bump would break.
 */
CNA_STUDIO_TEST(AMaterialCarriesASeparateOcclusionMapAndOlderOnesStillLoad)
{
    MaterialDocument material;
    material.occlusionTexture = Uuid::generate();

    MaterialDocument reloaded;
    CNA_STUDIO_EXPECT(reloaded.loadFromJson(material.toJson()));
    CNA_STUDIO_EXPECT(reloaded.occlusionTexture == material.occlusionTexture);

    // Still version 1: a bump would make every material this build writes unreadable by the
    // previous one, for one optional texture.
    CNA_STUDIO_EXPECT_EQ(material.toJson()["formatVersion"].asInt(0),
                         MaterialDocument::kFormatVersion);

    // A material written before the field existed reads as having no occlusion map rather than
    // failing, which is what `loadFromJson` keeping its defaults is for.
    JsonValue older = material.toJson();
    older.remove("occlusionTexture");
    MaterialDocument before;
    CNA_STUDIO_EXPECT(before.loadFromJson(older));
    CNA_STUDIO_EXPECT(!before.occlusionTexture.isValid());

    // Unset is absent from the file rather than written as a nil id -- the two mean the same
    // thing and only one of them is noise.
    MaterialDocument plain;
    CNA_STUDIO_EXPECT_EQ(plain.toJson()["occlusionTexture"].asString("absent"),
                         std::string{"absent"});
}

/**
 * @brief The editor can fill a texture slot, which is the half of a material it could not reach.
 *
 * `MaterialDocument` has carried four texture ids since ED-403 and the editor drew six values and
 * no slots: the only way to give a material a map was to write the JSON by hand, which is the same
 * state `STUDIO-10007` found the file itself in.
 *
 * Driven as a drop rather than through the picker, because that is the gesture -- a user with the
 * Content Browser open drags the image onto the slot -- and because it exercises the typed
 * filtering of `STUDIO-19009` at the same time.
 */
CNA_STUDIO_TEST(ATextureDroppedOnAMaterialSlotIsWrittenToTheFile)
{
    Fixture fixture{"textureslot"};

    AssetRecord texture;
    texture.id = Uuid::generate();
    texture.sourcePath = "Assets/Rust.png";
    texture.type = AssetType::Texture2D;
    const Uuid textureId = texture.id;
    CNA_STUDIO_EXPECT(fixture.context.getAssets().add(std::move(texture)));

    fixture.settle();

    CNA_STUDIO_EXPECT(!fixture.onDisk().diffuseTexture.isValid());

    // Five slots, each offering "(none)" and the project's one texture. Unfiltered they would each
    // offer the material itself as well, which is fifteen rows rather than ten.
    CNA_STUDIO_EXPECT_EQ(fixture.last.assetChoicesOffered, std::size_t{10});

    StudioFrame::StudioDragPayload payload;
    payload.type = std::string{kStudioAssetDragType};
    payload.value = textureId.toString();
    payload.label = "Rust.png";

    // Onto the Base Colour Map row exactly, because this fixture knows where its rows are -- no
    // sweep is needed and a sweep would pass over four other slots on the way.
    const UiRect box = fixture.row(kBaseColourMapRow);
    const float x = fixture.controlLeft() + 20.0f;
    const float y = box.centerY();

    fixture.run(at(x, y, /*leftDown=*/true));
    CNA_STUDIO_EXPECT(fixture.frame.beginDrag(fixture.frame.ids().make("source"), payload));
    fixture.run(at(x, y, /*leftDown=*/true));
    fixture.run(at(x, y));

    CNA_STUDIO_EXPECT(fixture.onDisk().diffuseTexture == textureId);

    // Through the history like every other material edit, because this rewrites the file.
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    CNA_STUDIO_EXPECT(!fixture.onDisk().diffuseTexture.isValid());
}

/** @brief The occlusion slot is a slot of its own rather than a second name for the packed map. */
CNA_STUDIO_TEST(TheOcclusionSlotIsSeparateFromTheMetallicRoughnessOne)
{
    Fixture fixture{"occlusionslot"};

    AssetRecord texture;
    texture.id = Uuid::generate();
    texture.sourcePath = "Assets/Occlusion.png";
    texture.type = AssetType::Texture2D;
    const Uuid textureId = texture.id;
    CNA_STUDIO_EXPECT(fixture.context.getAssets().add(std::move(texture)));

    fixture.settle();

    StudioFrame::StudioDragPayload payload;
    payload.type = std::string{kStudioAssetDragType};
    payload.value = textureId.toString();
    payload.label = "Occlusion.png";

    const UiRect box = fixture.row(kOcclusionMapRow);
    const float x = fixture.controlLeft() + 20.0f;
    const float y = box.centerY();

    fixture.run(at(x, y, /*leftDown=*/true));
    CNA_STUDIO_EXPECT(fixture.frame.beginDrag(fixture.frame.ids().make("source"), payload));
    fixture.run(at(x, y, /*leftDown=*/true));
    fixture.run(at(x, y));

    const MaterialDocument written = fixture.onDisk();
    CNA_STUDIO_EXPECT(written.occlusionTexture == textureId);

    // The packed map is untouched: glTF carries occlusion in the R channel of that image *and*
    // permits a separate one, and `PbrEffect` takes both textures.
    CNA_STUDIO_EXPECT(!written.metallicRoughnessTexture.isValid());
}

/**
 * @brief Every texture the document names resolves to a path the renderer can open.
 *
 * The one thing `toMeshMaterial` deliberately cannot do for itself: the document speaks in asset
 * ids and `MeshMaterial` speaks in paths, because that is the currency the glTF importer uses for
 * a texture it found beside a model. Resolving ids to paths is what lets the model pass stay one
 * code path over both kinds of material -- and a slot dropped in that translation is a map the
 * user filled in, saved, and never sees, with nothing anywhere reporting it.
 */
CNA_STUDIO_TEST(EveryMapAMaterialNamesReachesTheRendererAsAPath)
{
    Fixture fixture{"maps"};

    const auto track = [&fixture](const std::string& path) {
        AssetRecord record;
        record.id = Uuid::generate();
        record.sourcePath = path;
        record.type = AssetType::Texture2D;
        const Uuid id = record.id;
        CNA_STUDIO_EXPECT(fixture.context.getAssets().add(std::move(record)));
        return id;
    };

    MaterialDocument material;
    CNA_STUDIO_EXPECT(loadMaterialDocument(fixture.context.getAssets(), fixture.material, material)
                      == MaterialLoadProblem::None);

    material.diffuseTexture = track("Assets/Base.png");
    material.normalTexture = track("Assets/Normal.png");
    material.metallicRoughnessTexture = track("Assets/Packed.png");
    material.emissiveTexture = track("Assets/Glow.png");
    material.occlusionTexture = track("Assets/Occlusion.png");

    fixture.project.write("Assets/PaintedRed.cnamaterial", Json::write(material.toJson(), true));

    const MaterialProvider provider = fixture.context.makeMaterialProvider();
    const std::optional<MeshMaterial> resolved = provider(fixture.material);
    CNA_STUDIO_EXPECT(resolved.has_value());
    if (!resolved.has_value()) { return; }

    CNA_STUDIO_EXPECT_EQ(resolved->diffuseTexturePath, std::string{"Assets/Base.png"});
    CNA_STUDIO_EXPECT_EQ(resolved->normalTexturePath, std::string{"Assets/Normal.png"});
    CNA_STUDIO_EXPECT_EQ(resolved->metallicRoughnessTexturePath, std::string{"Assets/Packed.png"});
    CNA_STUDIO_EXPECT_EQ(resolved->emissiveTexturePath, std::string{"Assets/Glow.png"});
    CNA_STUDIO_EXPECT_EQ(resolved->occlusionTexturePath, std::string{"Assets/Occlusion.png"});

    // A slot pointing at an asset that has gone resolves to no path rather than to a stale one:
    // the renderer draws the material without that map, which is the honest degradation.
    material.emissiveTexture = Uuid::generate();
    fixture.project.write("Assets/PaintedRed.cnamaterial", Json::write(material.toJson(), true));

    // The provider reads through the document cache since `STUDIO-19006`, and this case rewrites
    // the file behind the database's back -- the record's stamp does not move, so the cache is
    // entitled to keep what it has. In the editor a command invalidates it and an external edit
    // moves the stamp; here neither happens, so the test says so itself.
    fixture.context.getDocuments().invalidate();

    const std::optional<MeshMaterial> partial = provider(fixture.material);
    CNA_STUDIO_EXPECT(partial.has_value());
    if (partial.has_value())
    {
        CNA_STUDIO_EXPECT(partial->emissiveTexturePath.empty());
        CNA_STUDIO_EXPECT_EQ(partial->occlusionTexturePath, std::string{"Assets/Occlusion.png"});
    }
}

// ------------------------------------------------------------------------------------------------
// Transparency (STUDIO-19004)
// ------------------------------------------------------------------------------------------------

/**
 * @brief The alpha mode round-trips, defaults to glTF's, and is written only when it is not that.
 *
 * Additive at `formatVersion` 1 like the occlusion map: a material written before this existed has
 * no mode and reads as Opaque, which is what every material drew as.
 */
CNA_STUDIO_TEST(AMaterialsAlphaModeSurvivesTheFileAndDefaultsToOpaque)
{
    MaterialDocument material;
    CNA_STUDIO_EXPECT(material.alphaMode == MeshAlphaMode::Opaque);
    CNA_STUDIO_EXPECT(std::fabs(material.alphaCutoff - 0.5f) < 0.001f);

    // Absent from the file at the default, for the reason an unset texture is: a material carrying
    // every field anybody ever looked at makes its diff noise.
    CNA_STUDIO_EXPECT_EQ(material.toJson()["alphaMode"].asString("absent"), std::string{"absent"});
    CNA_STUDIO_EXPECT_EQ(material.toJson()["alphaCutoff"].asString("absent"),
                         std::string{"absent"});

    material.alphaMode = MeshAlphaMode::Mask;
    material.alphaCutoff = 0.25f;

    MaterialDocument reloaded;
    CNA_STUDIO_EXPECT(reloaded.loadFromJson(material.toJson()));
    CNA_STUDIO_EXPECT(reloaded.alphaMode == MeshAlphaMode::Mask);
    CNA_STUDIO_EXPECT(std::fabs(reloaded.alphaCutoff - 0.25f) < 0.001f);

    // Spelt as glTF spells it, because that is where an imported material's answer comes from and
    // a second spelling would be a second vocabulary for one fact.
    CNA_STUDIO_EXPECT_EQ(material.toJson()["alphaMode"].asString(""), std::string{"MASK"});

    // A mode this build does not know is Opaque -- glTF's own rule for an unrecognised value, so a
    // file from a later editor draws solid rather than not at all.
    JsonValue future = material.toJson();
    future.set("alphaMode", JsonValue{std::string{"DITHER"}});
    MaterialDocument unknown;
    CNA_STUDIO_EXPECT(unknown.loadFromJson(future));
    CNA_STUDIO_EXPECT(unknown.alphaMode == MeshAlphaMode::Opaque);

    // And it reaches the renderer's own form, which is what decides the pass a part is drawn in.
    material.alphaMode = MeshAlphaMode::Blend;
    CNA_STUDIO_EXPECT(material.toMeshMaterial().alphaMode == MeshAlphaMode::Blend);
}

/** @brief The cutoff is drawn only for a Mask material, where it is the only mode it means anything in. */
CNA_STUDIO_TEST(TheAlphaCutoffRowAppearsOnlyForAMaskedMaterial)
{
    MaterialDocument opaque;
    Fixture fixture{"cutoffrow", Json::write(opaque.toJson(), true)};
    fixture.settle();

    const std::size_t withoutCutoff = fixture.last.materialFields;

    MaterialDocument masked;
    masked.alphaMode = MeshAlphaMode::Mask;
    fixture.project.write("Assets/PaintedRed.cnamaterial", Json::write(masked.toJson(), true));

    // The document cache keys on the record's stamp, and this fixture has none, so the panel is
    // asked afresh: the services seam is unset here, which is the path that reads the file.
    fixture.settle();

    CNA_STUDIO_EXPECT_EQ(fixture.last.materialFields, withoutCutoff + 1);

    // A control beside a material it means nothing for is a control that does nothing, which is
    // the state STUDIO-12004 took the gizmo space toggle out of.
    CNA_STUDIO_EXPECT(withoutCutoff > std::size_t{0});
}

// ------------------------------------------------------------------------------------------------
// Renderer capability diagnostics (STUDIO-19008)
// ------------------------------------------------------------------------------------------------

/**
 * @brief The rule on its own: what a material asks for that the effect drawing it cannot give.
 *
 * Studio carries five texture slots and three alpha modes end to end, and the build draws through
 * `BasicEffect`, which samples one texture and has no alpha test. Four of those maps and one of
 * those modes reach the file, the database, the dependency graph and the renderer -- and not the
 * screen. `STUDIO-19002` and `STUDIO-19004` both recorded that in their acceptance entries, which
 * is the right place for a decision and the wrong place for a warning.
 */
CNA_STUDIO_TEST(AMaterialsUnusableFeaturesAreNamedOneByOne)
{
    MaterialDocument plain;

    // A material the effect can draw in full reports nothing, whichever effect it is.
    CNA_STUDIO_EXPECT(studioMaterialCapabilityIssues("BasicEffect", plain).empty());
    CNA_STUDIO_EXPECT(studioMaterialCapabilityIssues("PbrEffect", plain).empty());

    MaterialDocument rich;
    rich.diffuseTexture = Uuid::generate();
    rich.normalTexture = Uuid::generate();
    rich.metallicRoughnessTexture = Uuid::generate();
    rich.emissiveTexture = Uuid::generate();
    rich.occlusionTexture = Uuid::generate();
    rich.alphaMode = MeshAlphaMode::Mask;

    // `PbrEffect` takes all of it, including the occlusion map and glTF's alpha coverage.
    CNA_STUDIO_EXPECT(studioMaterialCapabilityIssues("PbrEffect", rich).empty());

    const std::vector<StudioMaterialCapabilityIssue> issues =
        studioMaterialCapabilityIssues("BasicEffect", rich);

    // Four maps and the mask. The base-colour map is *not* among them: BasicEffect samples one
    // texture and that is the one, which is the difference between "some of this will not draw"
    // and a list a user can act on.
    CNA_STUDIO_EXPECT_EQ(issues.size(), std::size_t{5});

    const auto names = [&issues] {
        std::string joined;
        for (const StudioMaterialCapabilityIssue& issue : issues) { joined += issue.feature + "|"; }
        return joined;
    }();
    CNA_STUDIO_EXPECT(names.find("Normal map") != std::string::npos);
    CNA_STUDIO_EXPECT(names.find("Metallic-roughness map") != std::string::npos);
    CNA_STUDIO_EXPECT(names.find("Emissive map") != std::string::npos);
    CNA_STUDIO_EXPECT(names.find("Occlusion map") != std::string::npos);
    CNA_STUDIO_EXPECT(names.find("Mask alpha") != std::string::npos);
    CNA_STUDIO_EXPECT(names.find("Base Colour") == std::string::npos);

    // Every one says what happens instead, which is the half a user can act on.
    for (const StudioMaterialCapabilityIssue& issue : issues)
    {
        CNA_STUDIO_EXPECT(!issue.detail.empty());
        CNA_STUDIO_EXPECT(issue.detail.size() > issue.feature.size());
    }

    // Blend is not reported: it is the device's blend state rather than the effect's, so it draws
    // correctly on both. A diagnostic that warned about it would be telling the user to change
    // something that works.
    MaterialDocument blended;
    blended.alphaMode = MeshAlphaMode::Blend;
    CNA_STUDIO_EXPECT(studioMaterialCapabilityIssues("BasicEffect", blended).empty());

    // An effect this build does not recognise reports nothing rather than guessing: a headless
    // preview has no device, and a CNA that grows a third effect should make Studio quiet rather
    // than wrong.
    CNA_STUDIO_EXPECT(studioMaterialCapabilityIssues("", rich).empty());
    CNA_STUDIO_EXPECT(studioMaterialCapabilityIssues("none", rich).empty());
    CNA_STUDIO_EXPECT(studioMaterialCapabilityIssues("SomeFutureEffect", rich).empty());
}

/** @brief And the editor says them, which is the whole point of knowing them. */
CNA_STUDIO_TEST(TheMaterialEditorNamesWhatThisBuildCannotDraw)
{
    MaterialDocument rich;
    rich.normalTexture = Uuid::generate();
    rich.occlusionTexture = Uuid::generate();
    rich.alphaMode = MeshAlphaMode::Mask;

    Fixture fixture{"capability", Json::write(rich.toJson(), true)};

    // No effect named: a headless preview has no device, and the panel says nothing rather than
    // warning about a renderer nobody is using.
    fixture.settle();
    CNA_STUDIO_EXPECT_EQ(fixture.last.materialCapabilityIssues, std::size_t{0});

    fixture.effect = "PbrEffect";
    fixture.settle();
    CNA_STUDIO_EXPECT_EQ(fixture.last.materialCapabilityIssues, std::size_t{0});

    fixture.effect = "BasicEffect";
    fixture.settle();
    CNA_STUDIO_EXPECT_EQ(fixture.last.materialCapabilityIssues, std::size_t{3});

    // Each is a row of its own, so the panel is three rows taller than it was.
    const std::size_t withWarnings = fixture.last.rowsDrawn;
    fixture.effect = "PbrEffect";
    fixture.settle();
    CNA_STUDIO_EXPECT_EQ(withWarnings, fixture.last.rowsDrawn + 3);
}

// ------------------------------------------------------------------------------------------------
// Material preview (STUDIO-19007)
// ------------------------------------------------------------------------------------------------

namespace
{
    /** @brief The RGBA of one pixel of a preview. */
    struct PreviewPixel
    {
        int r = 0;
        int g = 0;
        int b = 0;
        int a = 0;
    };

    [[nodiscard]] PreviewPixel pixelAt(const StudioThumbnail& image, std::uint32_t x,
                                       std::uint32_t y)
    {
        const std::size_t offset = (static_cast<std::size_t>(y) * image.width + x) * 4u;
        if (offset + 3 >= image.pixels.size()) { return {}; }
        return PreviewPixel{int{image.pixels[offset]}, int{image.pixels[offset + 1]},
                            int{image.pixels[offset + 2]}, int{image.pixels[offset + 3]}};
    }

    /** @brief The brightest pixel's total, which is where a highlight lands. */
    [[nodiscard]] int brightestTotal(const StudioThumbnail& image)
    {
        int best = 0;
        for (std::size_t offset = 0; offset + 3 < image.pixels.size(); offset += 4)
        {
            if (image.pixels[offset + 3] == 0) { continue; }
            const int total = int{image.pixels[offset]} + int{image.pixels[offset + 1]}
                              + int{image.pixels[offset + 2]};
            best = std::max(best, total);
        }
        return best;
    }
}

/**
 * @brief A material previews as a sphere with transparent corners, not a square of colour.
 *
 * A `.cnamaterial` in the Content Browser was a generic icon, so a folder of forty materials was
 * forty identical rows told apart only by name -- which is the one thing about a material a user
 * did not derive from how it looks.
 */
CNA_STUDIO_TEST(AMaterialPreviewsAsASphereWithNothingInTheCorners)
{
    MaterialDocument material;
    material.diffuseColor = StudioVector3{0.85f, 0.2f, 0.2f};

    const StudioThumbnail preview = studioRenderMaterialPreview(material, 64);
    CNA_STUDIO_EXPECT_EQ(preview.width, std::uint32_t{64});
    CNA_STUDIO_EXPECT_EQ(preview.height, std::uint32_t{64});
    CNA_STUDIO_EXPECT_EQ(preview.pixels.size(), std::size_t{64 * 64 * 4});

    // Transparent rather than a colour: the browser's card is whatever the theme says, and a
    // thumbnail with its own grey corners would be a grey square in the light theme and a
    // different grey square in the dark one.
    CNA_STUDIO_EXPECT_EQ(pixelAt(preview, 0, 0).a, 0);
    CNA_STUDIO_EXPECT_EQ(pixelAt(preview, 63, 0).a, 0);
    CNA_STUDIO_EXPECT_EQ(pixelAt(preview, 0, 63).a, 0);
    CNA_STUDIO_EXPECT_EQ(pixelAt(preview, 63, 63).a, 0);

    // And the middle is the material: opaque, and the colour it was given rather than a grey.
    const PreviewPixel centre = pixelAt(preview, 32, 32);
    CNA_STUDIO_EXPECT_EQ(centre.a, 255);
    CNA_STUDIO_EXPECT(centre.r > centre.g);
    CNA_STUDIO_EXPECT(centre.r > centre.b);

    // The rim is antialiased rather than a staircase: somewhere on the silhouette there is a
    // pixel that is neither fully inside nor fully outside. Asserted because the coverage term is
    // the only thing that produces one -- the early-out that skips pixels beyond the sphere
    // already makes the *corners* transparent, so without this the two are redundant and the
    // antialiasing could be removed with nothing noticing.
    bool partial = false;
    for (std::size_t offset = 3; offset < preview.pixels.size(); offset += 4)
    {
        const int alpha = int{preview.pixels[offset]};
        if (alpha > 0 && alpha < 255) { partial = true; }
    }
    CNA_STUDIO_EXPECT(partial);

    // An empty request is an empty image rather than a crash or a one-pixel one.
    CNA_STUDIO_EXPECT(studioRenderMaterialPreview(material, 0).isEmpty());
}

/**
 * @brief The preview shows the parameters, which is the only reason to render one.
 *
 * Each case is a pair that differs in one parameter, because "the sphere is lit" is a claim a
 * constant-colour disc satisfies. What a thumbnail has to do is let a user tell two materials
 * apart at a glance, and these are the four ways this format can differ.
 */
CNA_STUDIO_TEST(AMaterialPreviewShowsRoughnessMetalnessAndEmission)
{
    MaterialDocument smooth;
    smooth.diffuseColor = StudioVector3{0.5f, 0.5f, 0.5f};
    smooth.roughness = 0.05f;

    MaterialDocument rough = smooth;
    rough.roughness = 1.0f;

    // A smooth surface concentrates its highlight, so its brightest pixel is brighter than a
    // matte one's -- which is what a user reads "shiny" from.
    CNA_STUDIO_EXPECT(brightestTotal(studioRenderMaterialPreview(smooth, 64))
                      > brightestTotal(studioRenderMaterialPreview(rough, 64)));

    // A metal reflects its own colour and a dielectric reflects white, which is the one line of
    // the PBR model that survives the trip to Blinn-Phong. On a red material the difference is
    // visible in the highlight's *hue*, not its brightness.
    MaterialDocument metal;
    metal.diffuseColor = StudioVector3{0.8f, 0.1f, 0.1f};
    metal.metallic = 1.0f;
    metal.roughness = 0.1f;

    MaterialDocument dielectric = metal;
    dielectric.metallic = 0.0f;

    const StudioThumbnail metalImage = studioRenderMaterialPreview(metal, 64);
    const StudioThumbnail plasticImage = studioRenderMaterialPreview(dielectric, 64);

    // The defining difference, asserted as brightness rather than as hue. A metal's reflectance
    // *is* its base colour -- 0.8 here -- while a dielectric's is 0.04, so the metal reflects
    // twenty times as much light and its highlight is far brighter.
    //
    // Two earlier drafts of this case read the blue channel at the brightest pixel, and both were
    // wrong: the first had the sign backwards, because a red metal reflects more blue than a
    // dielectric's near-colourless 0.04, not less; and the core of the highlight saturates on
    // both, so a single pixel says nothing either way.
    CNA_STUDIO_EXPECT(brightestTotal(metalImage) > brightestTotal(plasticImage));

    // Emission lifts the *unlit* side, which is the half of a sphere nothing else reaches.
    MaterialDocument dark;
    dark.diffuseColor = StudioVector3{0.1f, 0.1f, 0.1f};

    MaterialDocument glowing = dark;
    glowing.emissiveColor = StudioVector3{0.0f, 0.6f, 0.0f};

    const StudioThumbnail darkImage = studioRenderMaterialPreview(dark, 64);
    const StudioThumbnail glowImage = studioRenderMaterialPreview(glowing, 64);

    // The far corner of the lit sphere, away from the key light: bottom-right, since the light is
    // up and to the left.
    CNA_STUDIO_EXPECT(pixelAt(glowImage, 42, 42).g > pixelAt(darkImage, 42, 42).g + 40);
}

/** @brief A see-through material previews see-through, which a flat swatch cannot show at all. */
CNA_STUDIO_TEST(ABlendedMaterialPreviewsAsATransparentSphere)
{
    MaterialDocument glass;
    glass.diffuseColor = StudioVector3{0.6f, 0.7f, 0.9f};
    glass.alphaMode = MeshAlphaMode::Blend;
    glass.alpha = 0.3f;

    const StudioThumbnail preview = studioRenderMaterialPreview(glass, 64);
    const PreviewPixel centre = pixelAt(preview, 32, 32);
    CNA_STUDIO_EXPECT(centre.a > 0);
    CNA_STUDIO_EXPECT(centre.a < 128);

    // An opaque material with the same alpha *factor* is solid: the mode is what decides, not the
    // number, which is the same rule the renderer follows (`plan.md` STUDIO-19004).
    MaterialDocument solid = glass;
    solid.alphaMode = MeshAlphaMode::Opaque;
    CNA_STUDIO_EXPECT_EQ(pixelAt(studioRenderMaterialPreview(solid, 64), 32, 32).a, 255);

    // And a masked one too: a cut-out is solid everywhere its map keeps, and the preview has no
    // map to cut against.
    MaterialDocument masked = glass;
    masked.alphaMode = MeshAlphaMode::Mask;
    CNA_STUDIO_EXPECT_EQ(pixelAt(studioRenderMaterialPreview(masked, 64), 32, 32).a, 255);
}

// ------------------------------------------------------------------------------------------------
// Live preview (STUDIO-19006)
// ------------------------------------------------------------------------------------------------

/**
 * @brief The viewport shows a material edit at once, and does not pay a file read per model to.
 *
 * Both halves, because either alone is the wrong thing. `makeMaterialProvider` read the file every
 * call and was called once per model entity per *frame*, so a scene of two hundred models opened
 * and parsed two hundred files sixty times a second: live, and paid for with the frame it was
 * previewing. Caching it without invalidating would be the opposite mistake -- a material a user
 * is editing while looking at it, showing the version before their own edit.
 */
CNA_STUDIO_TEST(AMaterialEditIsVisibleAtOnceAndCostsOneFileReadRatherThanOnePerDraw)
{
    Fixture fixture{"livepreview"};

    const MaterialProvider provider = fixture.context.makeMaterialProvider();

    const std::uint64_t before = fixture.context.getDocuments().getFileReadCount();

    // Ten resolutions, which is what ten model entities naming this material cost in one frame.
    for (int draw = 0; draw < 10; ++draw)
    {
        const std::optional<MeshMaterial> resolved = provider(fixture.material);
        CNA_STUDIO_EXPECT(resolved.has_value());
    }

    // One read, not ten. The number is the whole point: a cache that reloaded per call would
    // return the right material every time and still be the defect.
    CNA_STUDIO_EXPECT_EQ(fixture.context.getDocuments().getFileReadCount(), before + 1);

    const std::optional<MeshMaterial> first = provider(fixture.material);
    CNA_STUDIO_EXPECT(first.has_value());
    if (!first.has_value()) { return; }
    CNA_STUDIO_EXPECT(std::fabs(first->roughness - 0.6f) < 0.001f);

    // Now edit it the way the Inspector does -- through a command -- and the very next resolution
    // is the new material. No rescan, no watcher tick, no frame in between.
    MaterialDocument edited;
    CNA_STUDIO_EXPECT(loadMaterialDocument(fixture.context.getAssets(), fixture.material, edited)
                      == MaterialLoadProblem::None);
    edited.roughness = 0.1f;

    fixture.context.execute(std::make_unique<SetMaterialCommand>(
        fixture.context.getAssets().resolvePath("Assets/PaintedRed.cnamaterial"), edited,
        std::string{"roughness"}));

    const std::optional<MeshMaterial> after = provider(fixture.material);
    CNA_STUDIO_EXPECT(after.has_value());
    if (after.has_value())
    {
        CNA_STUDIO_EXPECT(std::fabs(after->roughness - 0.1f) < 0.001f);

        // And the derived half follows, which is what the viewport actually shades with.
        CNA_STUDIO_EXPECT(after->specularPower > first->specularPower);
    }

    // Undo puts the bytes back and the viewport follows that too, because undo goes through the
    // same history the invalidation hangs off.
    // Announced the way the Undo action announces it, which is the only thing that tells anything
    // outside the history that the document moved. Nothing is invalidated by hand here: a first
    // draft of this case did that, and it hid the defect -- undo bypasses `execute`, so the cache
    // was dropped on an edit and not on its reversal, leaving the viewport drawing the version
    // the user had just taken back.
    CNA_STUDIO_EXPECT(fixture.context.getHistory().undo());
    if (const StudioCommand* entry =
            fixture.context.getHistory().getCommandAt(fixture.context.getHistory().getCursor()))
    {
        fixture.context.announceCommand(*entry);
    }

    const std::optional<MeshMaterial> undone = provider(fixture.material);
    CNA_STUDIO_EXPECT(undone.has_value());
    if (undone.has_value())
    {
        CNA_STUDIO_EXPECT(std::fabs(undone->roughness - 0.6f) < 0.001f);
    }
}
