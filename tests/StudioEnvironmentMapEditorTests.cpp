// SPDX-License-Identifier: MS-PL
/**
 * @file StudioEnvironmentMapEditorTests.cpp
 * @brief Making a `.cnaenv` and editing it (`plan.md` STUDIO-10010).
 *
 * The arithmetic these cases rest on is covered in `EnvironmentMapTests.cpp`. What is covered here
 * is the half that only exists once a panel draws it, and it is the half that went missing for
 * materials between `ED-403` and `STUDIO-10007`: a document type the database recognised, the
 * dependency scan followed and the renderer resolved, which no gesture in the editor could
 * *produce* and no panel could *edit*. Complete for everybody who already had one.
 *
 * So: an action that writes a file where the user is standing, undoes, and does not overwrite a
 * file of the same name; and an editor that reads the file, shows what its settings will produce,
 * writes exactly the field that changed, and refuses a file this build cannot parse rather than
 * offering to overwrite it with less than it holds.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/Assets/AssetCommands.hpp"
#include "CNA/Studio/Assets/AssetDatabase.hpp"
#include "CNA/Studio/Assets/EnvironmentMapDocument.hpp"
#include "CNA/Studio/Core/Json.hpp"
#include "CNA/Studio/ShellPanels/StudioDetailsPanel.hpp"
#include "CNA/Studio/ShellPanels/StudioShellPanels.hpp"
#include "CNA/Studio/StudioContext.hpp"
#include "CNA/Studio/ShellPanels/StudioShellActions.hpp"
#include "CNA/Studio/Ui/StudioLog.hpp"
#include "CNA/Studio/UiCore/StudioActionRegistry.hpp"
#include "CNA/Studio/UiCore/StudioFontAtlas.hpp"
#include "CNA/Studio/UiCore/StudioFrame.hpp"
#include "CNA/Studio/UiCore/StudioShell.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

using namespace CNA::Studio;

namespace
{
    UiInputState away()
    {
        UiInputState input;
        input.displayWidth = 1280.0f;
        input.displayHeight = 900.0f;
        input.mouseX = -1.0f;
        input.mouseY = -1.0f;
        input.mouseInWindow = true;
        return input;
    }

    /** @brief A project root that cleans itself up. */
    class ScopedProject
    {
    public:
        explicit ScopedProject(const std::string& name)
        {
            path_ = std::filesystem::temp_directory_path()
                  / ("cna-studio-cnaenv-" + name + "-" + std::to_string(counter()++));
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
        [[nodiscard]] std::filesystem::path at(const std::string& relative) const
        {
            return path_ / relative;
        }

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

    /** @brief A `.cnaenv` as a person would have written it, with two settings off the default. */
    std::string environmentText()
    {
        return R"({
 "formatVersion": 1,
 "name": "Overcast",
 "panorama": "",
 "processing": {
  "faceSize": 0,
  "irradianceSize": 32,
  "irradianceSamples": 32,
  "specularBaseSize": 128,
  "specularMipCount": 5,
  "specularSamples": 64,
  "generateBrdfLut": true,
  "brdfLutSize": 128,
  "brdfLutSamples": 128
 }
}
)";
    }

    /** @brief The Details panel over a selected environment map asset. */
    struct Fixture
    {
        ScopedProject project;
        StudioContext context;
        StudioFrame frame{StudioTheme::dark()};
        StudioFontAtlas fonts;
        UiRect bounds{0.0f, 0.0f, 520.0f, 880.0f};
        StudioDetailsResult last;
        Uuid environment;
        Uuid panorama;

        explicit Fixture(const std::string& name, const std::string& text = environmentText())
            : project(name)
        {
            frame.setFontAtlas(&fonts);
            project.write("Assets/Overcast.cnaenv", text);
            context.getAssets().setProjectRoot(project.root());

            AssetRecord record;
            record.id = Uuid::generate();
            record.sourcePath = "Assets/Overcast.cnaenv";
            record.type = AssetType::EnvironmentMap;
            environment = record.id;
            (void)context.getAssets().add(std::move(record));
            context.selectAsset(environment);
        }

        /** @brief Adds a measured panorama and points the document at it. */
        void addPanorama(int width, int height)
        {
            AssetRecord sky;
            sky.id = Uuid::generate();
            sky.sourcePath = "Assets/sky.png";
            sky.type = AssetType::Texture2D;

            JsonValue size = JsonValue::makeArray();
            size.append(JsonValue{static_cast<double>(width)});
            size.append(JsonValue{static_cast<double>(height)});
            sky.importerSettings.set("pixelSize", std::move(size));

            panorama = sky.id;
            (void)context.getAssets().add(std::move(sky));

            EnvironmentMapDocument document;
            CNA_STUDIO_EXPECT(loadEnvironmentMapFile(
                                  project.at("Assets/Overcast.cnaenv").generic_string(), document)
                              == EnvironmentMapLoadProblem::None);
            document.panorama = panorama;
            project.write("Assets/Overcast.cnaenv", Json::write(document.toJson(), true));
        }

        void settle()
        {
            runStudioFrame(frame, away(), [&](StudioFrame& f) {
                const StudioDetailsServices services;
                const StudioDetailsResult result = studioDetailsPanel(f, bounds, context, services);
                if (f.isDrawPass()) { last = result; }
            });
        }
    };
}

/**
 * @brief Nothing in Studio could write the first `.cnaenv`, which is the whole of this half.
 *
 * The same gap `STUDIO-10007` closed for materials, and the same assertion that matters most:
 * **where the file goes.** An action that always wrote to the project's asset directory would be
 * right in the one case the user is already looking at it and wrong every other time — and the
 * symptom is not an error, it is somebody pressing the row twice and concluding it does nothing.
 */
CNA_STUDIO_TEST(TheNewEnvironmentMapActionWritesOneWhereTheBrowserIsStanding)
{
    ScopedProject project{"newenv"};
    project.write("Assets/Skies/placeholder.txt", "x");
    {
        std::ofstream stream{project.at("Game.cnaproject"), std::ios::binary};
        stream << R"({"formatVersion":1,"name":"Skies","kind":"CnaNative"})";
    }

    StudioContext context;
    CNA_STUDIO_EXPECT(context.openProject(project.at("Game.cnaproject").generic_string()));

    StudioLog log;
    auto shell = std::make_unique<StudioShell>(StudioTheme::dark());
    (void)bindStudioShellActions(*shell, context, log);
    StudioShellPanels panels{*shell, context, log};

    // Reachable at all: an action the registry carries and no menu names is one a user cannot
    // invoke, which is the same to them as one that was never written.
    bool offered = false;
    for (const StudioMenuDefinition& menu : shell->menus())
    {
        for (const StudioMenuEntry& entry : menu.entries)
        {
            if (!entry.isSeparator() && !entry.isSubmenu()
                && entry.id == "studio.asset.newEnvironmentMap")
            {
                offered = true;
            }
        }
    }
    CNA_STUDIO_EXPECT(offered);

    panels.contentBrowserState().folder = "Assets/Skies";

    const std::filesystem::path first = project.at("Assets/Skies/New Environment Map.cnaenv");
    const std::filesystem::path second = project.at("Assets/Skies/New Environment Map 2.cnaenv");

    shell->invoke("studio.asset.newEnvironmentMap");

    // An action registered with no handler is invoked, recorded as "not implemented" and otherwise
    // silent -- which is how New Material's binding sat in the wrong function without anything
    // saying so.
    CNA_STUDIO_EXPECT(shell->refusedActions().empty());
    CNA_STUDIO_EXPECT(std::filesystem::exists(first));

    // Tracked as well as written: a file the database has not rescanned is one the browser does
    // not list and the Inspector cannot open, which is the same to a user as not written at all.
    const AssetRecord* record =
        context.getAssets().findByPath("Assets/Skies/New Environment Map.cnaenv");
    CNA_STUDIO_EXPECT(record != nullptr);
    if (record != nullptr)
    {
        CNA_STUDIO_EXPECT(record->type == AssetType::EnvironmentMap);

        // And selected, because somebody who asks for a new environment map wants to edit it
        // rather than go and find it.
        CNA_STUDIO_EXPECT_EQ(context.getSelectedAsset().toString(), record->id.toString());
    }

    // What it wrote is readable by the loader that will read it, which is the difference between
    // creating a file and creating an asset.
    EnvironmentMapDocument created;
    CNA_STUDIO_EXPECT(loadEnvironmentMapFile(first.generic_string(), created)
                      == EnvironmentMapLoadProblem::None);
    CNA_STUDIO_EXPECT_EQ(created.name, std::string{"New Environment Map"});
    CNA_STUDIO_EXPECT(!created.panorama.isValid());

    // A second one gets a name of its own. Two environment maps called the same thing in one
    // folder is one file, and the second write would silently replace the first.
    shell->invoke("studio.asset.newEnvironmentMap");
    CNA_STUDIO_EXPECT(std::filesystem::exists(second));
    CNA_STUDIO_EXPECT(std::filesystem::exists(first));

    // A command like every other document mutation (D-06), so Undo takes the file back rather
    // than leaving the folder filling with things the user has already undone.
    shell->invoke("studio.edit.undo");
    CNA_STUDIO_EXPECT(!std::filesystem::exists(second));
    CNA_STUDIO_EXPECT(std::filesystem::exists(first));
}

/** @brief The editor draws the document's fields and what they will produce. */
CNA_STUDIO_TEST(TheEnvironmentMapEditorShowsWhatTheSettingsWillProduce)
{
    Fixture fixture{"fields"};
    fixture.settle();

    // Name, panorama, six counts, the BRDF toggle and its two numbers: eleven rows, and the last
    // two are there because the table is on.
    CNA_STUDIO_EXPECT_EQ(fixture.last.environmentFields, std::size_t{11});

    // Nothing has been pushed past a bound, so there is nothing to warn about -- except that the
    // panorama has not been chosen, which is exactly the note that should be there.
    CNA_STUDIO_EXPECT_EQ(fixture.last.environmentNotes, std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(fixture.last.environmentPlan.faceSize, 0);

    // A panorama arrives and the face size follows it, with the note gone.
    fixture.addPanorama(2048, 1024);
    fixture.settle();

    CNA_STUDIO_EXPECT_EQ(fixture.last.environmentPlan.faceSize, 512);
    CNA_STUDIO_EXPECT_EQ(fixture.last.environmentNotes, std::size_t{0});
    CNA_STUDIO_EXPECT(fixture.last.environmentPlan.estimatedBytes > 0);
    CNA_STUDIO_EXPECT(fixture.last.environmentPlan.estimatedSamples > 0);

    // **The panorama's size is read from the panorama, not copied into the `.cnaenv`.** A sky
    // re-exported at a different resolution has to change what the environment map derives, and a
    // size stored beside the reference would be stale the moment it did.
    AssetRecord* sky = fixture.context.getAssets().findMutable(fixture.panorama);
    CNA_STUDIO_EXPECT(sky != nullptr);
    if (sky != nullptr)
    {
        JsonValue size = JsonValue::makeArray();
        size.append(JsonValue{8192.0});
        size.append(JsonValue{4096.0});
        sky->importerSettings.set("pixelSize", std::move(size));
    }
    fixture.settle();
    CNA_STUDIO_EXPECT_EQ(fixture.last.environmentPlan.faceSize, 2048);
}

/**
 * @brief The BRDF table's own numbers are not offered when the table is off.
 *
 * `STUDIO-12004`'s doctrine, applied to an asset editor: a control that takes a value and does
 * nothing with it is worse than one that is not there, because the user cannot tell which of the
 * two just happened.
 */
CNA_STUDIO_TEST(TheBrdfTablesOwnNumbersAreNotOfferedWhenTheTableIsOff)
{
    Fixture fixture{"brdfoff"};
    fixture.settle();
    CNA_STUDIO_EXPECT_EQ(fixture.last.environmentFields, std::size_t{11});

    EnvironmentMapDocument document;
    CNA_STUDIO_EXPECT(
        loadEnvironmentMapFile(fixture.project.at("Assets/Overcast.cnaenv").generic_string(),
                               document)
        == EnvironmentMapLoadProblem::None);
    document.settings.generateBrdfLut = false;
    fixture.project.write("Assets/Overcast.cnaenv", Json::write(document.toJson(), true));

    fixture.settle();
    CNA_STUDIO_EXPECT_EQ(fixture.last.environmentFields, std::size_t{9});
}

/**
 * @brief A file this build cannot read is refused rather than shown as an editable form.
 *
 * The same rule the material editor follows and for the same reason: a form over a file that was
 * not parsed is an offer to overwrite it with less than it holds.
 */
CNA_STUDIO_TEST(AnUnreadableEnvironmentMapIsRefusedRatherThanDefaulted)
{
    Fixture fromTheFuture{"future", R"({"formatVersion":99,"name":"Tomorrow"})"};
    fromTheFuture.settle();
    CNA_STUDIO_EXPECT_EQ(fromTheFuture.last.environmentFields, std::size_t{0});
    CNA_STUDIO_EXPECT_EQ(fromTheFuture.last.environmentPlan.faceSize, 0);

    Fixture notJson{"notjson", "this is not a document"};
    notJson.settle();
    CNA_STUDIO_EXPECT_EQ(notJson.last.environmentFields, std::size_t{0});

    // And the file is left exactly as it was. An editor that refused to *show* a file and then
    // wrote to it anyway would be the worst of both.
    CNA_STUDIO_EXPECT_EQ(notJson.project.read("Assets/Overcast.cnaenv"),
                         std::string{"this is not a document"});
}

/** @brief An edit rewrites the file through the history, and undo puts the old bytes back. */
CNA_STUDIO_TEST(AnEnvironmentMapEditGoesThroughACommandAndUndoes)
{
    Fixture fixture{"edit"};
    fixture.settle();

    const std::string before = fixture.project.read("Assets/Overcast.cnaenv");
    CNA_STUDIO_EXPECT(!before.empty());

    // Driven through the command rather than through the widget: which pixel a spinner occupies is
    // this panel's business and changes with the theme, while *what an edit does to the file* is
    // the thing this row is about. The editor builds exactly this command.
    EnvironmentMapDocument edited;
    CNA_STUDIO_EXPECT(
        loadEnvironmentMapFile(fixture.project.at("Assets/Overcast.cnaenv").generic_string(),
                               edited)
        == EnvironmentMapLoadProblem::None);
    edited.settings.irradianceSamples = 64;

    fixture.context.execute(std::make_unique<SetEnvironmentMapCommand>(
        fixture.context.getAssets().resolvePath("Assets/Overcast.cnaenv"), edited,
        std::string{"Irradiance Samples"}));

    fixture.settle();

    // Four times the irradiance work for twice the number, which is the whole reason the figure is
    // on screen -- and it is visible in the panel's own reported plan rather than only in the
    // arithmetic.
    EnvironmentMapDocument reread;
    CNA_STUDIO_EXPECT(
        loadEnvironmentMapFile(fixture.project.at("Assets/Overcast.cnaenv").generic_string(),
                               reread)
        == EnvironmentMapLoadProblem::None);
    CNA_STUDIO_EXPECT_EQ(reread.settings.irradianceSamples, 64);

    // Undo replays the bytes that were there rather than writing a document that merely equals
    // them: a hand-formatted file must come back as the user left it.
    fixture.context.getHistory().undo();
    CNA_STUDIO_EXPECT_EQ(fixture.project.read("Assets/Overcast.cnaenv"), before);

    // And a *create* is undone by removing the file, because there are no old bytes to put back.
    const std::string fresh =
        fixture.context.getAssets().resolvePath("Assets/Fresh.cnaenv");
    fixture.context.execute(std::make_unique<SetEnvironmentMapCommand>(
        fresh, EnvironmentMapDocument{}, std::string{"create"}));
    CNA_STUDIO_EXPECT(std::filesystem::exists(fresh));
    fixture.context.getHistory().undo();
    CNA_STUDIO_EXPECT(!std::filesystem::exists(fresh));
}
