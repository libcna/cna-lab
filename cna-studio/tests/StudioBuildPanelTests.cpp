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

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <thread>
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
        explicit ScopedProjectFile(const std::string& name, bool withBuildFile = false)
        {
            path_ = std::filesystem::temp_directory_path()
                  / ("cna-studio-build-" + name + "-" + std::to_string(counter()++));
            std::error_code code;
            std::filesystem::remove_all(path_, code);
            std::filesystem::create_directories(path_, code);

            std::ofstream stream{path_ / "Game.cnaproject", std::ios::binary};
            stream << R"({"formatVersion":1,"name":"Built","kind":"CnaNative"})";

            // Optional, because most cases here are about the *panel* and a project that cannot
            // be built is a perfectly good fixture for them. The cases about the commands need
            // one: `describeBuildProblem` refuses a directory with no build file, and a plan of
            // no steps would make every assertion about those commands hold vacuously.
            if (!withBuildFile) { return; }
            std::ofstream build{path_ / "CMakeLists.txt", std::ios::binary};
            build << "cmake_minimum_required(VERSION 3.20)\nproject(Built)\n";
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

        explicit Harness(const std::string& name, bool withBuildFile = false)
            : project(name, withBuildFile)
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

        /**
         * @brief Clicks the middle of every control the panel routed input to, one at a time.
         *
         * `plan.md` CORE-01's gestures are buttons and rows, and a test that pressed them by
         * hard-coded coordinate would be a test of this panel's current layout: it would pass a
         * refactor that moved Clean Build under Cancel, and fail one that added a row above the
         * commands. So the frame is asked what is interactive -- which is exactly what it knows --
         * and every one of those rectangles is pressed.
         *
         * A case then asserts *how many* controls did the thing, which is the claim worth making:
         * "one control in this panel asks for a clean build" is true of a correct panel and false
         * of one where the button is missing, disabled, or drawn where nothing can press it.
         *
         * @return One result per control pressed, in the order the panel described them.
         */
        /** @brief Every rectangle the panel routed input to on the last settled frame. */
        std::vector<UiRect> controlRectangles()
        {
            settle();

            std::vector<UiRect> controls;
            for (const StudioRecordedInteraction& widget : frame.recordedInteractions())
            {
                if (widget.bounds.width <= 0.0f || widget.bounds.height <= 0.0f) { continue; }
                controls.push_back(widget.bounds);
            }
            return controls;
        }

        std::vector<StudioBuildPanelResult> clickEveryControl()
        {
            const std::vector<UiRect> controls = controlRectangles();

            std::vector<StudioBuildPanelResult> results;
            for (const UiRect& control : controls)
            {
                // A fresh frame between presses, so a button left in its pressed state does not
                // answer for the next one.
                settle();
                click(control.centerX(), control.centerY());
                results.push_back(last);
                last = StudioBuildPanelResult{};
            }
            return results;
        }

        /**
         * @brief Runs a build whose only step prints @p output, and waits for it.
         *
         * A real `BuildProcess` over a real child, because the thing under test is what the panel
         * makes of a *log the build wrote* -- and a test-only setter that handed it a path would
         * be testing a code path no build takes. The child is `/bin/sh`, which is as much of a
         * compiler as this needs: `BuildProcess` redirects its output to the log, so whatever it
         * prints is what the panel will read.
         *
         * @return False where there is no shell to run, which is the caller's cue to stop.
         */
        bool runBuildPrinting(const std::string& output)
        {
            if (!std::filesystem::exists("/bin/sh")) { return false; }

            const std::filesystem::path directory =
                std::filesystem::temp_directory_path()
                / ("cna-studio-buildlog-" + std::to_string(counter()++));
            std::error_code code;
            std::filesystem::create_directories(directory, code);
            logDirectory = directory;

            // The text goes in a file the step reads, rather than in an argument. `BuildProcess`
            // writes each step's *command line* into the log before running it, so an argument
            // carrying compiler output would appear in the log twice -- once echoed and once
            // printed -- and the panel would honestly report every diagnostic in it twice over.
            const std::filesystem::path source = directory / "compiler-output.txt";
            {
                std::ofstream stream{source, std::ios::binary};
                stream << output;
            }

            BuildStep step;
            step.description = "Compile";
            step.executable = "/bin/sh";
            step.arguments = {"-c", "cat \"$0\"", source.generic_string()};

            StudioBuildJob job;
            job.buildDirectory = directory.generic_string();
            job.description = "fixture";
            job.steps = {step};

            std::string problem;
            if (!build.start(job, &problem)) { return false; }

            for (int attempt = 0; attempt < 2000 && build.getState() == BuildState::Running;
                 ++attempt)
            {
                build.poll();
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
            return build.getState() != BuildState::Running;
        }

        ~Harness()
        {
            if (logDirectory.empty()) { return; }
            std::error_code code;
            std::filesystem::remove_all(logDirectory, code);
        }

        Harness(const Harness&) = delete;
        Harness& operator=(const Harness&) = delete;

        std::filesystem::path logDirectory;

    private:
        static int& counter() { static int value = 0; return value; }
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

// ------------------------------------------------------------------------------------------------
// `plan.md` CORE-01 — build the project, and understand the failure
// ------------------------------------------------------------------------------------------------

CNA_STUDIO_TEST(CleanAndIncrementalAreSeparateGesturesAndEachDoesWhatItsNameSays)
{
    // Two buttons rather than one and a modifier. A modifier somebody forgets is set turns every
    // build into a clean one, and the two are pressed at different moments: Build every few
    // minutes, Clean Build when the build tree itself is the suspect.
    Harness harness{"cleanbuild", /*withBuildFile=*/true};

    std::size_t incremental = 0;
    std::size_t clean = 0;
    for (const StudioBuildPanelResult& result : harness.clickEveryControl())
    {
        if (!result.buildRequested) { continue; }
        if (result.buildKind == StudioBuildKind::Clean) { ++clean; } else { ++incremental; }
    }

    CNA_STUDIO_EXPECT_EQ(incremental, std::size_t{1});
    CNA_STUDIO_EXPECT_EQ(clean, std::size_t{1});

    // And they plan different commands. The clean job runs the project's own `clean` target
    // *between* the configure and the build -- the target does not exist until the tree has been
    // generated, so cleaning first would fail on a first-ever build, which is exactly when a user
    // reaches for it after a failed one.
    const StudioBuildJob ordinary = harness.panel.planBuild(StudioBuildKind::Incremental);
    const StudioBuildJob cleaned = harness.panel.planBuild(StudioBuildKind::Clean);

    CNA_STUDIO_EXPECT_EQ(ordinary.steps.size(), std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(cleaned.steps.size(), std::size_t{3});
    if (cleaned.steps.size() < 3) { return; }

    CNA_STUDIO_EXPECT_EQ(cleaned.steps[0].description, ordinary.steps[0].description);
    CNA_STUDIO_EXPECT_EQ(cleaned.steps[1].description, std::string{"Clean"});
    CNA_STUDIO_EXPECT(cleaned.steps[1].toCommandLine().find("--target clean") != std::string::npos);
    CNA_STUDIO_EXPECT_EQ(cleaned.steps[2].toCommandLine(), ordinary.steps[1].toCommandLine());

    // Not a deletion of the build directory. That is a destructive operation on a path a user can
    // type, one typo away from a source tree; `--target clean` is the build system removing what
    // it made.
    for (const BuildStep& step : cleaned.steps)
    {
        CNA_STUDIO_EXPECT(step.toCommandLine().find("rm ") == std::string::npos);
        CNA_STUDIO_EXPECT(step.toCommandLine().find("-E remove") == std::string::npos);
    }
}

CNA_STUDIO_TEST(TheCommandsCanBeTakenAwayAndPastedIntoAShell)
{
    // "Displayed, selectable, and produces the same result when pasted into a shell." The rows in
    // the panel are truncated to its width, so reading a long configure line off the screen is not
    // something a user can do -- and a command line they cannot take away is one they have to
    // reconstruct by hand, which is the moment they stop using the panel.
    Harness harness{"copycommands", /*withBuildFile=*/true};
    harness.settle();

    CNA_STUDIO_EXPECT(!harness.panel.planBuild().steps.empty());

    // Compared against the plan *as it stood when the button was pressed*. Pressing every control
    // in turn also presses the renderer and feature controls, which change the commands -- so a
    // plan taken before the sweep would be the wrong thing to compare against, and a test that
    // did would fail for a reason that has nothing to do with copying.
    std::size_t copies = 0;
    harness.frame.setClipboardText("");

    for (const UiRect& control : harness.controlRectangles())
    {
        harness.settle();

        std::string expected;
        for (const BuildStep& step : harness.panel.planBuild().steps)
        {
            expected += step.toCommandLine() + "\n";
        }

        harness.frame.setClipboardText("");
        harness.click(control.centerX(), control.centerY());

        const std::string clipboard = harness.frame.clipboardText();
        if (clipboard.empty()) { continue; }

        ++copies;
        // Every step, in order, untruncated and shell-quoted -- which is what makes "the same
        // result when pasted" a claim rather than a hope.
        CNA_STUDIO_EXPECT_EQ(clipboard, expected);
    }

    CNA_STUDIO_EXPECT_EQ(copies, std::size_t{1});

    // And the quoting is real: a project in a directory with a space in its name has to survive
    // the round trip, which is the case that makes a naive join wrong.
    BuildStep spaced;
    spaced.executable = "/usr/bin/cmake";
    spaced.arguments = {"-S", "/home/a b/Game", "-B", "/home/a b/Game/build"};
    CNA_STUDIO_EXPECT(spaced.toCommandLine().find("\"/home/a b/Game\"") != std::string::npos);
}

CNA_STUDIO_TEST(ACompilerErrorBecomesARowThatOpensTheFileAtTheLine)
{
    // Core step 9, and the half of it the panel could not do. A build that failed with nothing but
    // "Build failed" takes the operation a developer performs most and makes it less informative
    // than the terminal they would otherwise have used.
    Harness harness{"errors"};

    // A build that really ran and really wrote this into its log. The output is a compiler's
    // rather than a compiler run, which is the only part faked: what the panel has to get right is
    // what it makes of the log, not that GCC produces one.
    if (!harness.runBuildPrinting(
            "[1/2] Building CXX object CMakeFiles/Game.dir/Source/Main.cpp.o\n"
            "Source/Main.cpp:12:5: error: 'undefined_function' was not declared in this scope\n"
            "Source/Main.cpp:18:9: warning: unused variable 'speed' [-Wunused-variable]\n"
            "ninja: build stopped: subcommand failed.\n"))
    {
        return;
    }
    harness.settle();

    CNA_STUDIO_EXPECT_EQ(harness.last.errorsShown, std::size_t{2});

    // One of the rows, when activated, asks for that file at that line. Which row is which is the
    // panel's business; that exactly one of them asks for line 12 of Main.cpp is the claim.
    std::size_t opened = 0;
    StudioSourceLocation asked;
    for (const StudioBuildPanelResult& result : harness.clickEveryControl())
    {
        if (!result.openLocation.isValid()) { continue; }
        ++opened;
        if (result.openLocation.line == 12) { asked = result.openLocation; }
    }

    CNA_STUDIO_EXPECT_EQ(opened, std::size_t{2});
    CNA_STUDIO_EXPECT_EQ(asked.file, std::string{"Source/Main.cpp"});
    CNA_STUDIO_EXPECT_EQ(asked.line, 12);
}

CNA_STUDIO_TEST(TheWholeUnparsedLogIsReachableWhateverTheParserMadeOfIt)
{
    // The other half of the same requirement. The parser above is allowed to recognise nothing --
    // a linker error, a toolchain Studio has never seen, a build system that writes its own
    // format -- and when it does, the log is the only record there is. Showing its *path* is not
    // reaching it; a path is something to retype.
    Harness harness{"wholelog"};

    if (!harness.runBuildPrinting("some output this build's parser has never seen\n"))
    {
        return;
    }
    harness.settle();

    // Nothing was recognised, which is the point of this case.
    CNA_STUDIO_EXPECT_EQ(harness.last.errorsShown, std::size_t{0});

    std::size_t offered = 0;
    for (const StudioBuildPanelResult& result : harness.clickEveryControl())
    {
        if (result.openLogRequested) { ++offered; }
    }
    CNA_STUDIO_EXPECT_EQ(offered, std::size_t{1});
}
