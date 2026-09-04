// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#define CB_PITCH_ID "cb_pitch_shift"

#define S_SEMITONES "semitones"
#define S_CHEAPER "cheaper"

#define CB_PITCH_MIN (-12)
#define CB_PITCH_MAX (12)

// Brand suffix appended to user-visible names (filter name + dock title). Kept out
// of the locale files on purpose — it is not translatable and would drift across the
// six .ini copies. Shared here so plugin.cpp and dock.cpp stay in sync.
#define CB_BRAND_SUFFIX " (ChiwaBots.com)"
