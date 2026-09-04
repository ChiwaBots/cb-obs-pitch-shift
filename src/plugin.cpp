// SPDX-License-Identifier: GPL-2.0-or-later
#include <obs-module.h>
#include <media-io/audio-io.h>

#include <signalsmith-stretch.h>

#include "pitch-shared.hpp"
#include "dock.hpp"

#include <cstring>
#include <new>
#include <string>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

OBS_DECLARE_MODULE()

OBS_MODULE_USE_DEFAULT_LOCALE("cb-pitch-shift", "en-US")

#define T_(key) obs_module_text(key)
#define T_NAME T_("PitchShift")
#define T_SEMITONES T_("Semitones")
#define T_SPREAD_CPU T_("SpreadCpu")
#define T_NOTE T_("Note")

namespace {

struct PitchFilter {
	obs_source_t *context = nullptr;
	signalsmith::stretch::SignalsmithStretch<float> stretch;

	uint32_t channels = 0;
	uint32_t sample_rate = 0;
	int semitones = 0;
	bool cheaper = false;

	std::vector<float> in_storage;
	std::vector<float> out_storage;
	std::vector<float *> in_ptrs;
	std::vector<float *> out_ptrs;

	void configure()
	{
		if (!channels || !sample_rate)
			return;

		if (cheaper)
			stretch.presetCheaper(int(channels), float(sample_rate));
		else
			stretch.presetDefault(int(channels), float(sample_rate));
		stretch.setTransposeSemitones(float(semitones));
	}

	void prepare_buffers(size_t frames)
	{
		const size_t need = frames * channels;
		if (in_storage.size() < need) {
			in_storage.resize(need);
			out_storage.resize(need);
		}
		in_ptrs.resize(channels);
		out_ptrs.resize(channels);
		for (uint32_t c = 0; c < channels; c++) {
			in_ptrs[c] = in_storage.data() + size_t(c) * frames;
			out_ptrs[c] = out_storage.data() + size_t(c) * frames;
		}
	}
};

const char *filter_get_name(void *)
{

	static const std::string name = std::string(T_NAME) + CB_BRAND_SUFFIX;
	return name.c_str();
}

void filter_update(void *data, obs_data_t *settings)
{
	auto *f = static_cast<PitchFilter *>(data);

	const int semitones = int(obs_data_get_int(settings, S_SEMITONES));
	const bool cheaper = obs_data_get_bool(settings, S_CHEAPER);

	const bool preset_changed = cheaper != f->cheaper;
	f->cheaper = cheaper;
	f->semitones = semitones;

	if (preset_changed)
		f->configure();
	else
		f->stretch.setTransposeSemitones(float(semitones));
}

void *filter_create(obs_data_t *settings, obs_source_t *source)
{
	auto *f = new (std::nothrow) PitchFilter();
	if (!f)
		return nullptr;

	f->context = source;

	struct obs_audio_info oai = {};
	if (obs_get_audio_info(&oai)) {
		f->sample_rate = oai.samples_per_sec;
		f->channels = get_audio_channels(oai.speakers);
	}

	f->cheaper = obs_data_get_bool(settings, S_CHEAPER);
	f->semitones = int(obs_data_get_int(settings, S_SEMITONES));
	f->configure();

	blog(LOG_INFO, "[cb-pitch-shift] created: %u ch @ %u Hz, %d semitones, latency in=%d out=%d samples",
	     f->channels, f->sample_rate, f->semitones, f->stretch.inputLatency(), f->stretch.outputLatency());

	return f;
}

void filter_destroy(void *data)
{
	delete static_cast<PitchFilter *>(data);
}

void filter_defaults(obs_data_t *settings)
{
	obs_data_set_default_int(settings, S_SEMITONES, 0);
	obs_data_set_default_bool(settings, S_CHEAPER, false);
}

obs_properties_t *filter_properties(void *)
{
	obs_properties_t *props = obs_properties_create();

	obs_properties_add_int_slider(props, S_SEMITONES, T_SEMITONES, -12, 12, 1);

	obs_properties_add_bool(props, S_CHEAPER, T_SPREAD_CPU);

	obs_properties_add_text(props, "note", T_NOTE, OBS_TEXT_INFO);

	return props;
}

struct obs_audio_data *filter_audio(void *data, struct obs_audio_data *audio)
{
	auto *f = static_cast<PitchFilter *>(data);

	if (!f->channels || !audio->frames || f->semitones == 0)
		return audio;

	const size_t frames = audio->frames;
	f->prepare_buffers(frames);

	for (uint32_t c = 0; c < f->channels; c++) {
		if (audio->data[c])
			memcpy(f->in_ptrs[c], audio->data[c], frames * sizeof(float));
		else
			memset(f->in_ptrs[c], 0, frames * sizeof(float));
	}

	f->stretch.process(f->in_ptrs.data(), int(frames), f->out_ptrs.data(), int(frames));

	for (uint32_t c = 0; c < f->channels; c++) {
		if (audio->data[c])
			memcpy(audio->data[c], f->out_ptrs[c], frames * sizeof(float));
	}

	return audio;
}

} // namespace

MODULE_EXPORT const char *obs_module_description(void)
{
	return "ChiwaBots pitch shift audio filter (transpose without changing tempo)";
}

MODULE_EXPORT bool obs_module_load(void)
{
	struct obs_source_info info = {};

	info.id = CB_PITCH_ID;
	info.type = OBS_SOURCE_TYPE_FILTER;
	info.output_flags = OBS_SOURCE_AUDIO;
	info.get_name = filter_get_name;
	info.create = filter_create;
	info.destroy = filter_destroy;
	info.update = filter_update;
	info.get_defaults = filter_defaults;
	info.get_properties = filter_properties;
	info.filter_audio = filter_audio;

	obs_register_source(&info);

	blog(LOG_INFO, "[cb-pitch-shift] loaded (libobs %d.%d.%d)", LIBOBS_API_MAJOR_VER, LIBOBS_API_MINOR_VER,
	     LIBOBS_API_PATCH_VER);

#if defined(_WIN32)
	// Windows: obs-frontend-api.dll is delay-loaded, so only touch obs_frontend_*
	// once we know it is present — otherwise the delay-load helper aborts.
	if (!GetModuleHandleW(L"obs-frontend-api.dll")) {
		blog(LOG_INFO, "[cb-pitch-shift] no frontend (headless?) — dock skipped");
		return true;
	}
#endif
	// macOS/Linux: obs-frontend-api is linked directly. A GUI OBS always provides
	// it; a headless host without it just means the module fails to load, the
	// accepted cost of the Qt dock (phase-212 D2).
	cb_pitch_dock_register();

	return true;
}

MODULE_EXPORT void obs_module_unload(void)
{
#if defined(_WIN32)
	if (!GetModuleHandleW(L"obs-frontend-api.dll"))
		return;
#endif
	cb_pitch_dock_unregister();
}
