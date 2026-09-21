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

    // The texture slots follow the six values (`plan.md` STUDIO-19002).
    constexpr std::size_t kBaseColourMapRow = 12;
    constexpr std::size_t kOcclusionMapRow = 16;
}

CNA_STUDIO_TEST(SelectingAMaterialShowsItsFieldsRatherThanAnImporterApology)
{
    // What this row used to say: "A material is edited by the material editor, which the native
    // shell does not have yet". It has one.
    Fixture fixture{"fields"};
    fixture.settle();

    // Six values and five texture slots (`plan.md` STUDIO-19002). The slots were the half of a
    // material that could only be filled in by editing the JSON by hand: the document has carried
    // four of the ids since ED-403 and the editor could set none of them.
    CNA_STUDIO_EXPECT_EQ(fixture.last.materialFields, std::size_t{11});
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

    // The Roughness field: one box across the whole control column.
    const UiRect box = fixture.row(kRoughnessRow);
    const float x = fixture.controlLeft() + 20.0f;
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
    // offer the material itself as well, which is six.
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

    const std::optional<MeshMaterial> partial = provider(fixture.material);
    CNA_STUDIO_EXPECT(partial.has_value());
    if (partial.has_value())
    {
        CNA_STUDIO_EXPECT(partial->emissiveTexturePath.empty());
        CNA_STUDIO_EXPECT_EQ(partial->occlusionTexturePath, std::string{"Assets/Occlusion.png"});
    }
}
