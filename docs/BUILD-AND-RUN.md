# Build and run commands

Run commands from the repository root. Windows wrappers resolve their own root, so the tree can move; the user's intended location is `D:\Programming\GitHub\OpenMUA2`. Do not install packages or update upstream branches as an implicit build step.

## Entry points

`OpenMUA2.cmd setup` verifies the baseline and initializes Git once. `OpenMUA2.cmd test` runs the workspace tests. `OpenMUA2.cmd doctor` lists required host tools and source entry points without building. `Build.cmd` activates installed MSVC and runs the entire diagnostic build pipeline. Direct build subcommands should be run from an x64 developer prompt:

```bat
OpenMUA2.cmd build-tools --jobs 2
OpenMUA2.cmd extract --wit "D:\Tools\Wiimms\bin\wit.exe"
OpenMUA2.cmd generate --jobs 2
OpenMUA2.cmd build --jobs 2
OpenMUA2.cmd run
```

`build-tools` builds and tests **DolRecomp only**, not every runtime tool. `extract` and `generate` validate/rebuild DolRecomp as needed. A successful previous extraction is reused only with matching WBFS input receipts and valid game hashes. Do not mix a changed input image into an existing `.local\game` directory.

Input paths with spaces are passed as separate arguments, not shell-concatenated commands. A split image's `.wbf1`, `.wbf2`, etc. must be beside its `.wbfs`; companions are hashed and must be contiguous. The original image and companions are opened read-only by the workspace script. Extraction writes a new staging directory, validates the result, rechecks input hashes, then publishes `.local\game`. Failed staging output is retained for diagnosis and is not treated as a completed extraction.

## Options

`--jobs` defaults to 2 to avoid exhausting memory on large generated translation units. Raise it deliberately for your host. `--config` is `Release` by default; `Debug` and `RelWithDebInfo` are available. `--cc`, `--cxx` and `--module-cc` select existing compilers. Do not switch compiler/generator in an existing CMake cache; use a separate working copy or deliberately move that build directory first. The wrapper never silently deletes a build.

Windows defaults use `cl` for the native source and module. The supplied native module CMake has explicit MSVC strict floating-point options. These settings are a source-based build plan, not an executed Windows validation. Do not use `--module-opt 1/2/3` as a workaround for a correctness failure; O0/IPO-off is the recovery baseline. Windows dependency DLL availability and complete build compatibility remain to be tested on the target host.

`--wit` selects an already present WIT executable; `WIT` in the environment is also recognized. `--image` selects an exact filename in the repository root. `--sdk` is Linux-only and selects an already extracted private frontend SDK. `openmua2.json` is descriptive project metadata, **not** an override configuration parsed by the command runner.

## Linux recovery path

On an already provisioned Linux x86-64 machine:

```sh
python3 tools/workspace.py setup
python3 tools/workspace.py build-tools --jobs 2
python3 tools/workspace.py build --jobs 2 --wit /path/to/wit
python3 tools/workspace.py run --graphics OGL --audio 'Null'
```

The source's Linux system audio/graphics/input development dependencies still need to be present, or supply the original private SDK using `--sdk`. This package vendors source, not operating-system development packages. No WSL requirement is imposed by the Windows workflow, and no compiler installer is included.

## Outputs and failure handling

Build products live under `.local/build/windows-x64/` or `linux-x64/`: `dolrecomp`, `runtime`, `module-mg01` and `native-audit`. Generated game C goes to `.local/generated-mg01/generated`, outside tracked source. Runtime user files go to `.local/user`. Logs and exit-code receipts go to `.local/logs` and `.local/receipts`.

`Run.cmd` requires the successful local build receipt and matching runner/module hashes. Editing a generated module or replacing a binary invalidates the old receipt; rebuild and validate rather than bypassing this check. The module verifier checks DOL text tiling, every chunk hash, ABI, entry point and safe rejection of an uncovered address. It does not execute gameplay.

Controller settings are not automatically replaced. Inspect the configuration the runtime creates under `.local/user/Config`. The pinned runtime's default emulated Wii profile is not evidence of validated MUA2 Nunchuk bindings; configure/test those deliberately. Local input, game sound and saving remain unverified.

Never turn on Wii shadow/lockstep diagnostics from the historical MR01 instructions until the missing MEM2 restoration and comparison-boundary repairs have been reconstructed. A passing DOL hash audit is not permission to disable fallback, code-hash checks, instruction semantics, or game timing.
