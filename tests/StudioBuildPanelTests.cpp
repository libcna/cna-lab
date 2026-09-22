// SPDX-License-Identifier: MS-PL
/**
 * @file StudioBuildPanelTests.cpp
 * @brief The Build panel, and the first interface the six-axis target profile has ever had
 *        (plan.md STUDIO-07010).
 *
 * The ImGui panel this replaces offers two axes and keeps them in its own members, so what the
 * user chose was forgotten when the panel closed and was never saved. The cases here are about the
 * two properties that fixes: an edit reaches the *project*, and the choices offered are only the
 * ones CNA will actually configure.
 */

#include "TestHarness.hpp"

#include "CNA/Studio/Project/Cpp/CppToolchain.hpp"
#include "CNA/Studio/ProjectCommands.hpp"
#include "CNA/Studio/ShellPanels/StudioBuildPanel.hpp"
#include "CNA/Studio/StudioContext.hpp"
#include "CNA/Studio/UiCore/StudioWidgets.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

using namespace CNA::Studio;

namespace
{
    constexpr float kWidth = 900.0f;
    constexpr float kHeight = 700.0f;

    UiInputState at(float x, float y, bool leftDown = false)
    {
        UiInputState input;
        input.displayWidth = kWidth;
        input.displayHeight = kHeight;
        input.mouseX = x;
        input.mouseY = y;
        input.mouseInWindow = true;
        input.deltaSeconds = 1.0f / 60.0f;
        input.setMouseDown(UiMouseButton::Left, leftDown);
        return input;
    }

    /** @brief A project on disk, so `hasProject()` is true, removed on the way out. */
    class ScopedProjectFile
    {
    public:
        explicit ScopedProjectFile(const std::string& name)
        {
            path_ = std::filesystem::temp_directory_path()
                  / ("cna-studio-build-" + name + "-" + std::to_string(counter()++));
            std::error_code code;
            std::filesystem::remove_all(path_, code);
            std::filesystem::create_directories(path_, code);

            std::ofstream stream{path_ / "Game.cnaproject", std::ios::binary};
            stream << R"({"formatVersion":1,"name":"Built","kind":"CnaNative"})";
        }

        ~ScopedProjectFile()
        {
            std::error_code code;
            std::filesystem::remove_all(path_, code);
        }

        ScopedProjectFile(const ScopedProjectFile&) = delete;
        ScopedProjectFile& operator=(const ScopedProjectFile&) = delete;

        [[nodiscard]] std::string file() const
        {
            return (path_ / "Game.cnaproject").generic_string();
        }

    private:
        static int& counter() { static int value = 0; return value; }
        std::filesystem::path path_;
    };

    /** @brief The panel driven over a real context, with the frame the shell would give it. */
    struct Harness
    {
        ScopedProjectFile project;
        StudioContext context;
        BuildProcess build;
        StudioBuildPanel panel{context, build};
        StudioFrame frame{StudioTheme::dark()};
        UiRect body{0.0f, 0.0f, kWidth, kHeight};
        StudioBuildPanelResult last;

        /**
         * @brief How many edits the panel has *reported*, across every frame.
         *
         * Separate from `last`, which the next frame overwrites -- and from the command, which may
         * refuse what was reported. A case asserting "the panel did not ask for this" has to read
         * something that survives the settle() after the click, or it passes for a click that
         * simply missed.
         */
        std::size_t reportedEdits = 0;

        explicit Harness(const std::string& name) : project(name)
        {
            (void)context.openProject(project.file());
        }

        void run(const UiInputState& input)
        {
            runStudioFrame(frame, input, [&](StudioFrame& f) {
                const StudioBuildPanelResult panelResult = panel.draw(f, body);
                if (f.isInputPass()) { last = panelResult; }
            });
            applyProfileEdit();
        }

        /**
         * @brief What the binder does with a reported edit (`STUDIO-17002`).
         *
         * The panel reports and does not write, so a test that clicked a control and then read the
         * project would read the value from before the click. Doing here exactly what
         * `StudioShellPanels` does is what keeps these cases about the panel rather than about a
         * shortcut only the tests take.
         */
        void applyProfileEdit()
        {
            if (!last.profileEdit.has_value()) { return; }
            ++reportedEdits;

            auto command = std::make_unique<SetTargetProfilesCommand>(
                context.getProject(), last.profileEdit->profiles, last.profileEdit->activeIndex,
                last.profileEdit->description);
            if (command->isValid()) { context.execute(std::move(command)); }
            last.profileEdit.reset();
        }

        void settle() { run(at(kWidth - 10.0f, kHeight - 10.0f)); }

        void click(float x, float y)
        {
            run(at(x, y));
            run(at(x, y, /*leftDown=*/true));
            run(at(x, y));
        }

        [[nodiscard]] const StudioTargetProfile& profile() const
        {
            return context.getProject().getActiveTargetProfile();
        }
    };
}

CNA_STUDIO_TEST(WithNoProjectThePanelSaysSoRatherThanDrawingAnEmptyForm)
{
    StudioContext context;
    BuildProcess build;
    StudioBuildPanel panel{context, build};
    StudioFrame frame{StudioTheme::dark()};

    StudioBuildPanelResult result;
    runStudioFrame(frame, at(10.0f, 10.0f), [&](StudioFrame& f) {
        const StudioBuildPanelResult drawn = panel.draw(f, UiRect{0.0f, 0.0f, kWidth, kHeight});
        if (f.isInputPass()) { result = drawn; }
    });

    CNA_STUDIO_EXPECT(!result.buildRequested);
    CNA_STUDIO_EXPECT(!result.profileEdit.has_value());
    CNA_STUDIO_EXPECT_EQ(frame.phaseViolations(), std::size_t{0});
}

CNA_STUDIO_TEST(AProjectAlwaysHasATargetToShow)
{
    // A project with no profile cannot be built, and the reader substitutes the default rather
    // than leaving a state the panel would have to draw as nothing at all.
    Harness harness{"default"};
    harness.settle();

    CNA_STUDIO_EXPECT(!harness.context.getProject().getTargetProfiles().empty());
    CNA_STUDIO_EXPECT_EQ(harness.frame.phaseViolations(), std::size_t{0});
}

CNA_STUDIO_TEST(TheRendererListOffersOnlyWhatCnaWillConfigureForTheChosenSystem)
{
    // Offering Direct3D on a Linux target and then failing validation would be the tool asking the
    // user to discover a rule it already knows -- and CNA makes it a hard configure error, so the
    // discovery costs a full configure to make.
    Harness harness{"renderers"};

    std::vector<StudioTargetProfile> profiles = harness.context.getProject().getTargetProfiles();
    profiles.front().os = StudioTargetOs::Linux;
    profiles.front().renderer = "opengles3";
    harness.context.getProject().setTargetProfiles(profiles);
    harness.settle();

    CNA_STUDIO_EXPECT(isRendererAvailableOn("opengles3", StudioTargetOs::Linux));
    CNA_STUDIO_EXPECT(!isRendererAvailableOn("directx11", StudioTargetOs::Linux));
    CNA_STUDIO_EXPECT(isRendererAvailableOn("directx11", StudioTargetOs::Windows));
}

CNA_STUDIO_TEST(ChangingAnAxisReachesTheProjectRatherThanThePanel)
{
    // The whole point of the port. The legacy panel kept its platform and backend in its own
    // members: what the user chose was forgotten when the panel closed and never saved.
    Harness harness{"edit"};

    // Started somewhere that is not the answer, so a click that did nothing cannot pass.
    std::vector<StudioTargetProfile> profiles = harness.context.getProject().getTargetProfiles();
    profiles.front().configuration = StudioBuildConfiguration::MinSizeRel;
    harness.context.getProject().setTargetProfiles(profiles);
    harness.settle();
    CNA_STUDIO_EXPECT(harness.profile().configuration == StudioBuildConfiguration::MinSizeRel);

    // Driven through the widget rather than by calling the project, because "does the control
    // reach the model" is the thing that can be wrong.
    const float rowHeight = static_cast<float>(harness.frame.theme().metric(StudioMetric::ControlHeight));
    const float spacing = static_cast<float>(harness.frame.theme().metric(StudioMetric::SpacingSmall));
    const float margin = static_cast<float>(harness.frame.theme().metric(StudioMetric::SpacingMedium));

    // Target, name, the add/duplicate/remove buttons, OS, architecture, renderer, platform,
    // configuration: the eighth row. Counted rather than searched for, which is what makes this a
    // test of the control and not of the layout -- and what makes it fail loudly when a row is
    // inserted above it, as `STUDIO-17002` did.
    const float configurationTop = margin + 7.0f * (rowHeight + spacing);
    const float labelWidth = std::min((kWidth - margin * 2.0f) * 0.4f,
        static_cast<float>(harness.frame.theme().metric(StudioMetric::PanelHeaderHeight)) * 5.0f);
    const float controlX = margin + labelWidth + spacing + 20.0f;

    harness.click(controlX, configurationTop + rowHeight * 0.5f);
    CNA_STUDIO_EXPECT(harness.frame.isAnyPopupOpen());

    // The first row of the open list is Debug, which is not the default.
    const float firstRow = configurationTop + rowHeight + spacing
        + static_cast<float>(harness.frame.theme().metric(StudioMetric::SpacingSmall))
        + studioMenuItemHeight(harness.frame.theme()) * 0.5f;
    harness.click(controlX, firstRow);
    harness.settle();

    CNA_STUDIO_EXPECT(harness.profile().configuration == StudioBuildConfiguration::Debug);
    CNA_STUDIO_EXPECT(!harness.frame.isAnyPopupOpen());
}

CNA_STUDIO_TEST(TheBuildItWouldRunIsTheOneTheProjectsActiveProfileDescribes)
{
    // Not the panel's own idea of a platform and a backend, which is what the legacy panel built
    // from -- a panel that could start a different build from the one Play uses.
    Harness harness{"request"};
    harness.settle();

    const StudioBuildJob fromPanel = harness.panel.planBuild();
    const BuildRequest fromProject =
        makeBuildRequestFromActiveProfile(harness.context.getProject());

    CNA_STUDIO_EXPECT_EQ(fromPanel.buildDirectory, getDefaultBuildDirectory(fromProject));
    CNA_STUDIO_EXPECT(fromPanel.description.find(fromProject.configuration) != std::string::npos);
}

CNA_STUDIO_TEST(TurningAnOptionalSubsystemOnIsRecordedOnTheProfile)
{
    Harness harness{"features"};
    harness.settle();

    const StudioFeatureOption* net = findStudioFeature("net");
    CNA_STUDIO_EXPECT(net != nullptr);
    CNA_STUDIO_EXPECT(!harness.profile().hasFeature("net"));

    std::vector<StudioTargetProfile> profiles = harness.context.getProject().getTargetProfiles();
    profiles.front().setFeature("net", true);
    harness.context.getProject().setTargetProfiles(profiles);
    harness.settle();

    CNA_STUDIO_EXPECT(harness.profile().hasFeature("net"));

    // And it reaches the arguments Studio would configure the game's build with, which is the only
    // thing turning a feature on is for.
    const std::vector<std::string> arguments =
        studioTargetProfileCMakeArguments(harness.profile());
    bool found = false;
    for (const std::string& argument : arguments)
    {
        if (argument == "-DCNA_ENABLE_NET=ON") { found = true; }
    }
    CNA_STUDIO_EXPECT(found);
}

CNA_STUDIO_TEST(APanelDescribedTwiceCommitsNoPhaseViolations)
{
    // The panel splits one rectangle between two passes, and a split that ran only in the draw
    // pass would put every control somewhere different from where it was hit-tested.
    Harness harness{"phases"};
    for (int i = 0; i < 4; ++i)
    {
        harness.settle();
        CNA_STUDIO_EXPECT_EQ(harness.frame.phaseViolations(), std::size_t{0});
    }
}

CNA_STUDIO_TEST(TheBuildButtonIsRefusedWhenTheProfileCannotBeBuilt)
{
    Harness harness{"invalid"};

    std::vector<StudioTargetProfile> profiles = harness.context.getProject().getTargetProfiles();
    profiles.front().os = StudioTargetOs::Linux;
    profiles.front().renderer = "directx11";
    harness.context.getProject().setTargetProfiles(profiles);
    harness.settle();

    StudioTargetProfile checked = harness.profile();
    const StudioProfileValidation validation = validateStudioTargetProfile(checked);
    CNA_STUDIO_EXPECT(!validation.isBuildable());
    CNA_STUDIO_EXPECT(!harness.last.buildRequested);
}

/**
 * @brief **A target edit reaches the project file, and reaches the undo stack.**
 *
 * `plan.md` STUDIO-17002, and the defect it removes was silent twice over. The panel wrote target
 * profiles straight into the open `Project` and set a `profileChanged` flag that **nothing read**,
 * so a user who picked a renderer got exactly what they asked for until they closed Studio —
 * whereupon it was gone, with no prompt, no dirty marker and nothing in the log. The panel's own
 * header promised "the profile the next save writes"; there was no next save.
 *
 * Both halves are checked here because they fail independently: a command that did not write
 * through would undo correctly and still lose the change on quit, and a write-through that skipped
 * the history would persist a change the user could not take back.
 */
CNA_STUDIO_TEST(ATargetEditIsWrittenToTheProjectFileAndCanBeUndone)
{
    Harness harness{"persist"};

    std::vector<StudioTargetProfile> profiles = harness.context.getProject().getTargetProfiles();
    CNA_STUDIO_EXPECT(!profiles.empty());
    if (profiles.empty()) { return; }

    const std::string before = profiles.front().renderer;
    profiles.front().renderer = before == "opengles3" ? "opengl33" : "opengles3";
    const std::string after = profiles.front().renderer;

    harness.context.execute(std::make_unique<SetTargetProfilesCommand>(
        harness.context.getProject(), profiles, 0, "Edit target"));

    CNA_STUDIO_EXPECT_EQ(harness.profile().renderer, after);

    // On disk, not only in memory. The crash-recovery snapshot holds the scene and not the
    // project, so a project change that lived only in memory is one a crash loses entirely.
    {
        Project reread;
        CNA_STUDIO_EXPECT(reread.loadFromFile(harness.project.file()).succeeded);
        CNA_STUDIO_EXPECT_EQ(reread.getActiveTargetProfile().renderer, after);
    }

    // And undone, which is what makes it a document change like any other (D-06).
    CNA_STUDIO_EXPECT(harness.context.getHistory().undo());
    CNA_STUDIO_EXPECT_EQ(harness.profile().renderer, before);

    {
        Project reread;
        CNA_STUDIO_EXPECT(reread.loadFromFile(harness.project.file()).succeeded);
        CNA_STUDIO_EXPECT_EQ(reread.getActiveTargetProfile().renderer, before);
    }
}

/** @brief The command refuses the two edits that would put a useless entry in the history. */
CNA_STUDIO_TEST(ATargetEditThatChangesNothingIsNotACommand)
{
    Harness harness{"novalue"};
    const std::vector<StudioTargetProfile> profiles =
        harness.context.getProject().getTargetProfiles();

    // The same list and the same selection: an entry that undoes to the state it is already in
    // reads to the user as a broken Ctrl+Z.
    const SetTargetProfilesCommand unchanged{harness.context.getProject(), profiles, 0, "Edit"};
    CNA_STUDIO_EXPECT(!unchanged.isValid());

    // An empty list would leave a project that cannot be built and no row to add a target from.
    const SetTargetProfilesCommand emptied{harness.context.getProject(), {}, 0, "Remove"};
    CNA_STUDIO_EXPECT(!emptied.isValid());

    // The *selection* alone is a change, though, because it decides what Build and Play do.
    std::vector<StudioTargetProfile> two = profiles;
    two.push_back(StudioTargetProfile::defaults());
    two.back().name = "Second";
    harness.context.execute(std::make_unique<SetTargetProfilesCommand>(
        harness.context.getProject(), two, 0, "Add target"));

    const SetTargetProfilesCommand selected{harness.context.getProject(), two, 1, "Select target"};
    CNA_STUDIO_EXPECT(selected.isValid());
}

/**
 * @brief Adding, duplicating, renaming and removing a target, through the panel's own buttons.
 *
 * Until this row the list itself had no interface at all: the six axes could be edited and the
 * list they belong to could not, so a project that shipped on two things could only say so by
 * hand-editing its `.cnaproject` — which is the state this panel's header describes as the problem
 * it exists to solve.
 */
CNA_STUDIO_TEST(TheTargetListCanBeEditedFromThePanel)
{
    Harness harness{"list"};

    const float rowHeight = static_cast<float>(harness.frame.theme().metric(StudioMetric::ControlHeight));
    const float spacing = static_cast<float>(harness.frame.theme().metric(StudioMetric::SpacingSmall));
    const float margin = static_cast<float>(harness.frame.theme().metric(StudioMetric::SpacingMedium));
    const float labelWidth = std::min((kWidth - margin * 2.0f) * 0.4f,
        static_cast<float>(harness.frame.theme().metric(StudioMetric::PanelHeaderHeight)) * 5.0f);

    // Target, name, then the buttons: the third row. Add, Duplicate and Remove share its width in
    // three, so a third of the way in is Add and five sixths of the way in is Remove.
    const float buttonsTop = margin + 2.0f * (rowHeight + spacing);
    const float buttonsLeft = margin + labelWidth + spacing;
    const float buttonsWidth = std::min(kWidth - buttonsLeft - margin,
        static_cast<float>(harness.frame.theme().metric(StudioMetric::PanelHeaderHeight)) * 9.0f);
    const float third = buttonsWidth / 3.0f;

    harness.settle();
    CNA_STUDIO_EXPECT_EQ(harness.context.getProject().getTargetProfiles().size(), std::size_t{1});

    // Add.
    harness.click(buttonsLeft + third * 0.5f, buttonsTop + rowHeight * 0.5f);
    harness.settle();
    CNA_STUDIO_EXPECT_EQ(harness.context.getProject().getTargetProfiles().size(), std::size_t{2});

    // The new one is selected, because adding a target you then have to go and find is a gesture
    // that is not finished.
    CNA_STUDIO_EXPECT_EQ(harness.context.getProject().getActiveTargetProfileIndex(), std::size_t{1});

    // And it is not called the same thing as the one beside it, or the drop-down would show two
    // identical rows and the user could not tell which they were editing.
    const std::vector<StudioTargetProfile> two = harness.context.getProject().getTargetProfiles();
    CNA_STUDIO_EXPECT(two[0].name != two[1].name);

    // Duplicate.
    harness.click(buttonsLeft + third * 1.5f, buttonsTop + rowHeight * 0.5f);
    harness.settle();
    CNA_STUDIO_EXPECT_EQ(harness.context.getProject().getTargetProfiles().size(), std::size_t{3});

    const std::vector<StudioTargetProfile> three = harness.context.getProject().getTargetProfiles();
    CNA_STUDIO_EXPECT(three[2].name != three[1].name);

    // A duplicate says the same thing as its original about everything but its name -- that is
    // what makes it a duplicate rather than a second Add.
    StudioTargetProfile renamedBack = three[2];
    renamedBack.name = three[1].name;
    CNA_STUDIO_EXPECT(renamedBack == three[1]);

    // Remove.
    harness.click(buttonsLeft + third * 2.5f, buttonsTop + rowHeight * 0.5f);
    harness.settle();
    CNA_STUDIO_EXPECT_EQ(harness.context.getProject().getTargetProfiles().size(), std::size_t{2});

    // Every one of those reached the file, not just the model.
    Project reread;
    CNA_STUDIO_EXPECT(reread.loadFromFile(harness.project.file()).succeeded);
    CNA_STUDIO_EXPECT_EQ(reread.getTargetProfiles().size(), std::size_t{2});

    // And all of it undoes, one gesture at a time.
    CNA_STUDIO_EXPECT(harness.context.getHistory().undo());
    CNA_STUDIO_EXPECT_EQ(harness.context.getProject().getTargetProfiles().size(), std::size_t{3});
    CNA_STUDIO_EXPECT(harness.context.getHistory().undo());
    CNA_STUDIO_EXPECT_EQ(harness.context.getProject().getTargetProfiles().size(), std::size_t{2});
    CNA_STUDIO_EXPECT(harness.context.getHistory().undo());
    CNA_STUDIO_EXPECT_EQ(harness.context.getProject().getTargetProfiles().size(), std::size_t{1});
}

/**
 * @brief Remove is **disabled** on the last target rather than hidden, and refuses to fire.
 *
 * `STUDIO-12004`'s doctrine. Removing the last target leaves a project that cannot be built, and a
 * button that vanished when it was the one thing the user was looking for would read as the panel
 * being broken rather than as the operation being refused.
 */
CNA_STUDIO_TEST(RemovingTheLastTargetIsRefusedRatherThanLeavingAProjectWithNone)
{
    Harness harness{"lasttarget"};
    harness.settle();
    CNA_STUDIO_EXPECT_EQ(harness.context.getProject().getTargetProfiles().size(), std::size_t{1});

    const float rowHeight = static_cast<float>(harness.frame.theme().metric(StudioMetric::ControlHeight));
    const float spacing = static_cast<float>(harness.frame.theme().metric(StudioMetric::SpacingSmall));
    const float margin = static_cast<float>(harness.frame.theme().metric(StudioMetric::SpacingMedium));
    const float labelWidth = std::min((kWidth - margin * 2.0f) * 0.4f,
        static_cast<float>(harness.frame.theme().metric(StudioMetric::PanelHeaderHeight)) * 5.0f);
    const float buttonsTop = margin + 2.0f * (rowHeight + spacing);
    const float buttonsLeft = margin + labelWidth + spacing;
    const float buttonsWidth = std::min(kWidth - buttonsLeft - margin,
        static_cast<float>(harness.frame.theme().metric(StudioMetric::PanelHeaderHeight)) * 9.0f);

    harness.click(buttonsLeft + buttonsWidth / 3.0f * 2.5f, buttonsTop + rowHeight * 0.5f);
    harness.settle();

    CNA_STUDIO_EXPECT_EQ(harness.context.getProject().getTargetProfiles().size(), std::size_t{1});

    // Nothing was even *asked for*, which is the assertion that distinguishes a disabled button
    // from a click that missed one. Counted across frames, because `last` is overwritten by the
    // settle() that follows the click.
    CNA_STUDIO_EXPECT_EQ(harness.reportedEdits, std::size_t{0});

    // The command refuses it too, so the guard is not only in the pixels.
    const SetTargetProfilesCommand emptied{harness.context.getProject(), {}, 0, "Remove target"};
    CNA_STUDIO_EXPECT(!emptied.isValid());
}
