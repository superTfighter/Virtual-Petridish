# Virtual-Petridish — Project Configuration
# Location: /home/lextomi/Applications/Claude/apps/Virtual-Petridish

## What this project is
A Windows-only C++ desktop application ("Virtual Petridish" / "CellularPotts
Model") simulating cell differentiation and movement using the **Cellular
Potts Model (CPM)**. Rendering is parallelized with **OpenCL**; the UI is
**Dear ImGui** over **Direct3D 11** via Win32. This is a Hungarian
university thesis ("Szakdolgozat") project — `Documents/` holds the thesis
writeup, presentation, and university paperwork (background material, not
a source of technical truth). README.md is in Hungarian; source code
comments/identifiers are in English.

## Tech stack
- C++ / Win32 / Direct3D 11 (rendering + swapchain)
- OpenCL (GPU-parallel simulation rendering — see `CPM/kernel.cl`,
  `CPM/OpenCL.h/.cpp`)
- Dear ImGui (vendored source in `CPM/imgui*.cpp/.h` — debug/parameter UI)
- stb_image.h (vendored, single header)
- SFML is vendored under `CPM/include/SFML` and `CPM/lib/sfml-*.lib` but is
  **not referenced anywhere in source** and not linked in the `.vcxproj`.
  Treat it as present-but-unused; do not assume audio/graphics features
  built on it exist.

No package manager (vcpkg/NuGet/Conan) is used. All third-party code is
vendored directly into `CPM/include/`, `CPM/lib/`, or dropped as loose
source files, and wired up via hard-coded paths in `CPM.vcxproj`
(`$(ProjectDir)include`, `$(ProjectDir)lib`).

## Build / run
- Build system is a Visual Studio 2019 solution: `CPM.sln`
  (`PlatformToolset v142`). There is no CMake/Makefile for this project
  (the `.cmake` files under `CPM/lib/cmake/SFML/` are vendored SFML
  package-config files, unrelated to building this app).
- Build the `CPM` project for **`Release|x64` or `Debug|x64`** — the x64
  configs have `AdditionalIncludeDirectories`/`AdditionalLibraryDirectories`
  /`AdditionalDependencies` wired up; the **Win32 configs do not** and will
  likely fail to link against OpenCL/D3D11.
- Command line (from a VS Developer Command Prompt):
  `msbuild CPM.sln /p:Configuration=Release /p:Platform=x64`
- Run: `x64/Debug/CPM.exe` or `x64/Release/CPM.exe` (both are already built
  and committed). Requires Windows, a GPU/driver with a working OpenCL
  runtime, and Direct3D 11 support. Does not run on Linux without
  Wine/emulation, and cannot be built or launched from this workspace's
  typical Linux Claude Code environment — verify changes by careful code
  review, and note when something needs a Windows build to confirm.

## Testing, linting, CI
- **No test framework, no CI config, no linter/formatter config exist.**
  The only "test" is `Simulation::testFunction()`, a debug stub that just
  prints `" DONE"` — not a real test.
- There is no automated way to verify changes in this repo. Treat manual
  review (and, when possible, an actual Windows build) as the only
  verification available.

## Code organization
- All application source lives flat in one directory, `CPM/`, organized by
  component/class rather than by feature (no `src/`, no per-feature
  subfolders). Class names map ~1:1 to files: `Cell`, `Grid`,
  `CellularPotts`, `Simulation`, `Display`, `Parameters`, etc.
- Entry point: `CPM/main.cpp` — Win32 window + D3D11 device setup + ImGui
  init + a 2-thread pool + main render loop.
- `CellularPotts` is the model engine (grid, cells, Monte Carlo step,
  serial + parallel); `HamiltonianConstraint` is an abstract base for
  pluggable energy terms, with concrete subclasses `AdhesionConstraint`,
  `VolumeConstraint`, `PerimeterConstraint`, `ActivityContraint` (sic),
  `PersistenceConstraint` (incomplete — see the `TODO` in `main.cpp`).
- `Simulation::setupSimulation(int)` hard-codes 7 demo scenarios (cases
  0–6, selected via the ImGui `ExampleChooser` in `Display`) — there is no
  config file or CLI-arg based scenario selection.
- `OpenCL::getRenderImage()` runs the `calculate`/`border` kernels in
  `CPM/kernel.cl` to produce an RGBA buffer, uploaded each frame as a D3D11
  texture in `Display`/`main.cpp` and explicitly `Release()`-d after every
  frame — deliberate manual GPU-memory management; preserve this pattern
  if touching the render path.
- No environment variables or `.env` files are used anywhere; all
  configuration is hard-coded in `Parameters` constructors or adjusted at
  runtime via ImGui.

## Naming quirks to preserve
Several typo-like identifiers are used consistently across many files and
are effectively part of the project's public API. Do not "fix" these
without being asked — a silent rename would be a large, breaking,
repo-wide diff:
- `GridManadger` (class/file name — typo of "Manager")
- `ActivityContraint` (file/class name — drops the "s" in "Constraint";
  contrast with `PersistenceConstraint`, which spells it correctly)
- `LAMDA_DIR` (a `Parameters` field — missing "B" from "LAMBDA")
- `EPHILIA` (a scenario label string, likely meant "Epithelia")

## Git / repo state notes
- Single branch (`master`), tracked to `origin/master` at
  `github.com/superTfighter/Virtual-Petridish`. Informal, single-developer
  commit history (short casual messages, no conventional-commit prefixes).
- `.gitignore` only excludes `.vs/`. **Built binaries are committed to
  git**: `x64/Debug/CPM.exe`, `x64/Release/CPM.exe`, `.pdb`, `.ilk`, plus
  vendored third-party `.lib`/`.pdb` files under `CPM/lib/`. Be aware that
  rebuilding locally can produce large binary diffs in these paths — flag
  this to the user rather than committing rebuilt artifacts by default.
