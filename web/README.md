# `web/` — the CNA Studio presentation site

Four static pages and one stylesheet. No build step, no bundler, no dependency beyond a web font
that falls back to the system stack if it cannot be fetched.

```sh
xdg-open web/index.html            # or just open the file
python3 -m http.server -d web      # if you would rather serve it
```

| Page | What is on it |
|------|---------------|
| [`index.html`](index.html) | What Studio is and deliberately is not, the screenshots, the shortest possible start |
| [`tutorial.html`](tutorial.html) | **The detailed tutorial.** Fourteen parts from a clone to a running game, with the commands, what you should see, checkpoints and a troubleshooting table |
| [`workflow.html`](workflow.html) | The eleven Core steps in detail, and the list of absences |
| [`reference.html`](reference.html) | Requirements, every default shortcut, headless command lines, build options, the renderer model |
| [`styles.css`](styles.css) | Shared. Tokens first, both themes defined, no per-page CSS |

**Where the words come from.** These pages present
[`docs/USER-GUIDE.md`](../docs/USER-GUIDE.md), [`docs/GETTING-STARTED.md`](../docs/GETTING-STARTED.md),
[`docs/RENDERERS-AND-PLATFORMS.md`](../docs/RENDERERS-AND-PLATFORMS.md) and the root
[`README.md`](../README.md). Those are the sources; when they and these pages disagree, they are
right and these pages are stale. The keyboard table is generated from the real chords in
`src/ui-core/StudioActionRegistry.cpp` rather than copied from prose, because that is the one table
that goes quietly wrong.

**The screenshots** in [`images/`](images) are copies of the ones in
[`docs/images/`](../docs/images), kept here so the directory can be deployed on its own. Refresh both
together — the commands that produce them are on [`reference.html`](reference.html#headless).
