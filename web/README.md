# `web/` — the CNA Studio presentation site

One self-contained page: [`index.html`](index.html). Open it in a browser, or serve this directory
with any static file server. There is no build step, no bundler and no dependency beyond a web font
that degrades to the system stack if it cannot be fetched.

```sh
xdg-open web/index.html            # or just open the file
python3 -m http.server -d web      # if you would rather serve it
```

**What is in it.** What Studio is and deliberately is not, the screenshots, getting from a clone to
a running game, all eleven steps of the Core workflow in detail, the list of absences, the keyboard,
the headless command lines, the build options and the renderer/platform model.

**Where the words come from.** The page is a presentation of
[`docs/USER-GUIDE.md`](../docs/USER-GUIDE.md), [`docs/GETTING-STARTED.md`](../docs/GETTING-STARTED.md),
[`docs/RENDERERS-AND-PLATFORMS.md`](../docs/RENDERERS-AND-PLATFORMS.md) and the root
[`README.md`](../README.md). Those are the sources; when they and this page disagree, they are right
and this page is stale.

**The screenshots** in [`images/`](images) are copies of the ones in
[`docs/images/`](../docs/images), kept here so the directory can be deployed on its own. Refresh
both together — the commands that produce them are in the root README, under the figures.
