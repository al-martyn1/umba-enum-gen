@rem Add next option to UMBA_MDPP_EXTRA_OPTIONS variable to regenerate all images on next generation
@rem --clear-generation-cache=true
@call "%~dp0.bat\make-cli-options-docs.bat"

@set BRIEF_SCAN_PATHS="--scan=%~dp0"
@rem "--scan=%~dp0\../src" "--scan=%~dp0\../_src" "--scan=%~dp0\../examples"
@rem set BRIEF_EXTRA_OPTS_TXT=--scan-notes "--notes-output-path=%~dp0\doc\_md"
@set BRIEF_EXTRA_OPTS_MD=--scan-notes "--notes-output-path=%~dp0\doc\_md"

@rem call "%~dp0\.bat\fix_tabs.bat"

@set CLOCMD=_md
@call "%~dp0\.bat\run_clock.bat"

@if exist "%~dp0.bat\gen-doc.bat" @(
    @set UMBA_MDPP_EXTRA_OPTIONS="--batch-generate-page-index" "--batch-page-index-file=doc/Index.md"
    @call "%~dp0.bat\gen-doc.bat"
) else @(
    @echo.
)
