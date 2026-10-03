# CNA Lab - Experimental Repositories

This repository contains experimental repositories related to CNA, a C++ reimplementation of
XNA 4.0. Projects are organized here for collaborative development and testing and are integrated
using Git subtrees.

## Structure

Each project directory is integrated from a dedicated branch whose root is the project root. This
approach allows:

- **History preservation**: The source history of every currently included project is maintained
- **Centralized access**: Related projects are organized in a single location
- **Experimental development**: Organized workspace for testing and collaboration

## Working with Subtrees

To update a specific subtree from its upstream repository, use:

```bash
git subtree pull --prefix=<subtree-path> <remote-url> <branch>
```

To view the history of a specific subtree:

```bash
git log <subtree-path>
```

## Repository Management

For more information about managing Git subtrees, refer to the [Git documentation on subtrees](https://git-scm.com/book/en/v2/Git-Tools-Subtrees).

---

**Last updated**: 2026-10-03
