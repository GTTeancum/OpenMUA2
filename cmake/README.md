# CMake entry points

Use `tools/workspace.py` / `Build.cmd` for the concrete configure/build/test pipeline. DolRecomp, ModernGekko and RecompCore's native module template have separate CMake roots; the upstream top-level template is not a replacement for the recovered DOL-only pipeline. Keep new project-specific CMake helpers here rather than modifying generated CMake files under `.local`.
