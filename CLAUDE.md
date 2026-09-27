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

## Project backlog
`feature_list.txt` (repo root) is the user's running backlog — a flat
`[ ]`/`[x]` checklist they keep extending by hand, with a one-line legend
at the top. When asked to "continue the feature set" or similar, pick up
the next unchecked item there. Feel free to reorder/split items into
smaller subtasks yourself when that makes implementation easier (already
established with the user) — don't just track status passively. Mark an
item `[x]` with a short `-- done: ...` note summarizing what was actually
built and how it was verified, not just that it compiles.

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
Two parallel build systems are kept in sync (same source files, same
`.cpp`/`.h` set — any added/removed/renamed file needs updating in both):

- **Windows**: Visual Studio 2019 solution `CPM.sln` (`PlatformToolset v142`).
  Build the `CPM` project for **`Release|x64` or `Debug|x64`** — the x64
  configs have `AdditionalIncludeDirectories`/`AdditionalLibraryDirectories`
  /`AdditionalDependencies` wired up; the **Win32 configs do not** and will
  likely fail to link against OpenCL/D3D11. Command line (from a VS
  Developer Command Prompt): `msbuild CPM.sln /p:Configuration=Release /p:Platform=x64`.
  Run: `x64/Debug/CPM.exe` or `x64/Release/CPM.exe`.
- **Linux**: a root-level `CMakeLists.txt` builds the same sources against
  GLFW + OpenGL3 instead of Win32 + D3D11 (selected via `#ifdef _WIN32` in
  `main.cpp`/`Display.h`/`Display.cpp`/`CellularPotts.h` for the handful of
  genuinely platform-specific pieces — window/context setup, the ImGui
  backend, texture upload). Build: `cmake -S . -B build && cmake --build build -j`
  (or `-DCMAKE_BUILD_TYPE=Release`; VS Code's CMake Tools extension also
  works via its variant picker). Run: `./build/CPM`. Requires
  `libglfw3-dev`, OpenGL dev headers, and a working OpenCL runtime+dev
  package (`ocl-icd-opencl-dev` + a vendor ICD like `intel-opencl-icd`) —
  this workspace's Linux environment has all of that installed and the app
  builds/runs here natively; it is **not** Windows-only or Wine-dependent
  anymore.

## Testing, linting, CI
- **No CI config, no linter/formatter config exist**, but there IS a real,
  checked-in test suite now: `tests/` (CMake + CTest), added as
  `feature_list.txt` item 26. The Linux CMake build splits the simulation
  engine (`CellularPotts`/`Grid`/constraints/`Parameters`/`OpenCL`) into a
  `cpm_engine` STATIC library that both the `CPM` executable and every test
  target link, so tests run headlessly with no window/GL context (only a
  working OpenCL runtime, for the two tests that touch rendering). Build +
  run: `cmake -S . -B build && cmake --build build -j && ctest --test-dir build --output-on-failure`.
  5 executables, each its own CTest entry: `test_docopy` (Metropolis
  acceptance criterion, via a `CPM_ENABLE_TEST_HOOKS`-gated friend accessor
  to the private `docopy()`), `test_adhesion` (`AdhesionConstraint`
  kind-vs-ID-invariance), `test_scenarios` (every demo scenario runs Monte
  Carlo steps without throwing), `test_render` (`getImageData()` returns a
  correctly-sized, actually-populated buffer), `test_gradient_constraints`
  (`ResourceSeekingConstraint`/`ChemotaxisConstraint` bias direction, via
  direct `deltaH()` calls rather than a stochastic simulation loop — see
  the file's header comment for why: an earlier version drove
  `monteCarloStep()` and measured centroid drift, which was flaky because
  `CellularPotts`'s `rand()` usage is never re-seeded between models, so
  the RNG's own centroid random-walk could swamp the constraint's actual
  bias). `tests/test_framework.h` is a minimal custom `TEST_CASE`/`CHECK`/
  `TEST_MAIN` framework with no external dependency, matching this
  project's existing zero-dependency vendoring philosophy.
- The Windows `.vcxproj` build is **not** wired to the test suite — it
  still compiles one flat `CPM` executable with every source file, same as
  before item 26. The test suite is Linux/CMake-only so far.
- Beyond the checked-in suite: build both configurations when touching
  shared logic, run the app interactively, and for anything not yet
  covered by `tests/`, compile a small throwaway `main()` against the
  relevant `.cpp` files (outside the repo, e.g. in a scratch dir) the same
  way the checked-in tests do. This ad hoc approach — before item 26 — is
  what caught several real bugs (a self-deadlock, a crash-on-cell-death,
  incorrect physics) that a build-only check would have missed; prefer
  adding a real `tests/` case over a throwaway script when the thing being
  verified is worth protecting against regressing again.

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
- `Simulation::setupSimulation(int)` hard-codes demo scenarios (an
  if/else-if chain on an integer, selected via the ImGui `ExampleChooser`
  in `Display`, indices must match `Display.cpp`'s `items[]` list 1:1) —
  there is no config file or CLI-arg based scenario selection. The list has
  grown past the original 7 (bug fixes/new scenarios/a Sandbox mode were
  added later) — check `Simulation.cpp` and `Display.cpp`'s `items[]` for
  the current exact count and mapping rather than assuming a fixed number.
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
