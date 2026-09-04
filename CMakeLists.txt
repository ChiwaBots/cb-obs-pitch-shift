# SPDX-License-Identifier: GPL-2.0-or-later
#
# cb-pitch-shift — OBS pitch-shift audio filter (+ dock).
#
# Built on obsproject/obs-plugintemplate: the CMake helpers under cmake/, the
# obs-deps download (buildspec.json) and the GitHub Actions CI produce Windows,
# Linux and macOS artifacts from this one file. See phase-212.
#
# This plugin is C++ (Signalsmith DSP + a Qt dock), so it always needs both the
# frontend API and Qt — unlike the stock template those two options default ON.

cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version} LANGUAGES C CXX)

# The dock (frontend + Qt) is not optional for this plugin.
option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" ON)
option(ENABLE_QT "Use Qt functionality" ON)

include(compilerconfig)
include(defaults)
include(helpers)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
  )
endif()

# --- Signalsmith Stretch DSP (MIT, header-only) ------------------------------
# Fetched, not vendored: the linear headers reference each other across the repo
# root and include/ (stft.h does #include "../../stft.h"), so a whole-repo
# checkout is the honest way to capture them. Pinned to the exact revisions the
# Windows verification build (phase-170) measured against, so the DSP behaviour
# does not drift. SOURCE_SUBDIR points at a directory with no CMakeLists.txt so
# FetchContent only populates the headers and never runs the upstream CMake
# (which would otherwise pull a different linear version of its own).
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(
  signalsmith_linear
  GIT_REPOSITORY https://github.com/Signalsmith-Audio/linear.git
  GIT_TAG 0.6.0
  GIT_SHALLOW ON
  SOURCE_SUBDIR do-not-configure
)
FetchContent_Declare(
  signalsmith_stretch
  GIT_REPOSITORY https://github.com/Signalsmith-Audio/signalsmith-stretch.git
  GIT_TAG 57b93f4e9206a089a45387eaa39bdc9f310d3308
  SOURCE_SUBDIR do-not-configure
)
FetchContent_MakeAvailable(signalsmith_linear signalsmith_stretch)

# SYSTEM so the third-party headers' warnings (Signalsmith's fft.h has a pile of
# double->float narrowings) cannot drown out our own. Mirrors the layout the
# verified Windows build used: stretch.h at the repo root, linear under include/.
target_include_directories(
  ${CMAKE_PROJECT_NAME}
  SYSTEM
  PRIVATE "${signalsmith_stretch_SOURCE_DIR}" "${signalsmith_linear_SOURCE_DIR}/include"
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/plugin.cpp src/dock.cpp src/dock.hpp src/pitch-shared.hpp)

if(OS_WINDOWS)
  # /utf-8: our sources carry UTF-8 comments; without it MSVC reads them as the
  # system code page and emits C4819.
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /utf-8)
  target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)

  # Delay-load only obs-frontend-api.dll (all function imports): if the module
  # ever loads in a "Qt present but no frontend" host it still loads, and the
  # GetModuleHandle guard in obs_module_load skips the dock. Qt itself CANNOT be
  # delay-loaded — the dock imports Qt DATA symbols (QWidget::staticMetaObject)
  # and /DELAYLOAD only supports function imports (LNK1194) — so Qt stays a hard
  # dependency. Harmless on Windows, where OBS always ships Qt in bin/64bit.
  target_link_options(${CMAKE_PROJECT_NAME} PRIVATE /DELAYLOAD:obs-frontend-api.dll)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE delayimp)
endif()

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
