// SPDX-License-Identifier: GPL-2.0-or-later
#include "dock.hpp"
#include "pitch-shared.hpp"

#include <obs.h>
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/config-file.h>

#include <QAbstractItemView>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <string>
#include <vector>

#define T_(key) obs_module_text(key)

namespace {

constexpr const char *DOCK_ID = "cb_pitch_shift_dock";

constexpr const char *CFG_SECTION = "cbPitchDock";
constexpr const char *CFG_TARGET_UUID = "targetOwnerUuid";
constexpr const char *CFG_TARGET_NAME = "targetOwnerName";

constexpr int POLL_MS = 250;

struct TargetEntry {
	std::string ownerUuid;
	std::string ownerName;
	std::string filterName;

	bool audioActive = false;
	obs_weak_source_t *weak = nullptr;
};

class CbPitchDock : public QWidget {
	Q_OBJECT

public:
	CbPitchDock();
	~CbPitchDock() override;

	void rescan();

private:
	QComboBox *combo_ = nullptr;
	QPushButton *down_ = nullptr;
	QPushButton *up_ = nullptr;
	QPushButton *reset_ = nullptr;
	QLabel *readout_ = nullptr;
	QLabel *warn_ = nullptr;
	QTimer *timer_ = nullptr;

	std::vector<TargetEntry> entries_;
	std::string lastSig_;

	void releaseEntries();
	int selectedIndex() const;

	obs_source_t *acquireTarget() const;
	void applyDelta(int delta);
	void pollTick();
	void refreshReadout();
	void updateWarning();
	void saveSelection();
	void setControlsEnabled(bool on);
	void setReadout(int semitones, bool known);
};

CbPitchDock::CbPitchDock()
{
	setObjectName(DOCK_ID);

	combo_ = new QComboBox(this);
	combo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

	down_ = new QPushButton(QString::fromUtf8("\xE2\x88\x92"), this);
	up_ = new QPushButton(QStringLiteral("+"), this);
	reset_ = new QPushButton(T_("DockReset"), this);
	down_->setToolTip(T_("DockLower"));
	up_->setToolTip(T_("DockRaise"));

	readout_ = new QLabel(this);
	readout_->setAlignment(Qt::AlignCenter);
	readout_->setMinimumWidth(48);

	QFont f = readout_->font();
	f.setPointSizeF(f.pointSizeF() * 1.6);
	f.setBold(true);
	readout_->setFont(f);

	auto *root = new QVBoxLayout(this);
	root->setContentsMargins(8, 8, 8, 8);
	root->setSpacing(6);

	auto *targetRow = new QHBoxLayout();
	targetRow->setSpacing(6);
	targetRow->addWidget(new QLabel(T_("DockTarget"), this));
	targetRow->addWidget(combo_, 1);
	root->addLayout(targetRow);

	auto *ctrlRow = new QHBoxLayout();
	ctrlRow->setSpacing(6);
	ctrlRow->addWidget(down_);
	ctrlRow->addWidget(readout_, 1);
	ctrlRow->addWidget(up_);
	ctrlRow->addWidget(reset_);
	root->addLayout(ctrlRow);

	warn_ = new QLabel(this);
	warn_->setWordWrap(true);
	warn_->setStyleSheet(QStringLiteral("color:#d9822b;"));
	warn_->setVisible(false);
	root->addWidget(warn_);

	root->addStretch(1);

	connect(down_, &QPushButton::clicked, this, [this] { applyDelta(-1); });
	connect(up_, &QPushButton::clicked, this, [this] { applyDelta(+1); });
	connect(reset_, &QPushButton::clicked, this, [this] { applyDelta(0); });
	connect(combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
		saveSelection();
		refreshReadout();
	});

	timer_ = new QTimer(this);
	timer_->setInterval(POLL_MS);
	connect(timer_, &QTimer::timeout, this, [this] { pollTick(); });
	timer_->start();

	rescan();
}

CbPitchDock::~CbPitchDock()
{
	releaseEntries();
}

void CbPitchDock::releaseEntries()
{
	for (auto &e : entries_) {
		if (e.weak)
			obs_weak_source_release(e.weak);
	}
	entries_.clear();
}

int CbPitchDock::selectedIndex() const
{
	const int idx = combo_->currentIndex();
	if (idx < 0 || idx >= int(entries_.size()))
		return -1;
	return idx;
}

obs_source_t *CbPitchDock::acquireTarget() const
{
	const int idx = selectedIndex();
	if (idx < 0)
		return nullptr;
	return obs_weak_source_get_source(entries_[idx].weak);
}

struct ScanCtx {
	std::vector<TargetEntry> *out;
	bool withWeak;
};

void enumPitchTargets(std::vector<TargetEntry> &out, bool withWeak)
{
	ScanCtx ctx{&out, withWeak};
	obs_enum_sources(
		[](void *param, obs_source_t *source) -> bool {
			auto *c = static_cast<ScanCtx *>(param);
			obs_source_enum_filters(
				source,
				[](obs_source_t *parent, obs_source_t *child, void *p) {
					auto *c = static_cast<ScanCtx *>(p);
					const char *id = obs_source_get_id(child);
					if (!id || strcmp(id, CB_PITCH_ID) != 0)
						return;
					TargetEntry e;
					const char *ou = obs_source_get_uuid(parent);
					const char *on = obs_source_get_name(parent);
					const char *fn = obs_source_get_name(child);
					e.ownerUuid = ou ? ou : "";
					e.ownerName = on ? on : "";
					e.filterName = fn ? fn : "";
					e.audioActive = obs_source_audio_active(parent);
					if (c->withWeak)
						e.weak = obs_source_get_weak_source(child);
					c->out->push_back(std::move(e));
				},
				c);
			return true;
		},
		&ctx);
}

std::string signatureOf(const std::vector<TargetEntry> &v)
{
	std::vector<std::string> parts;
	parts.reserve(v.size());
	for (const auto &e : v)
		parts.push_back(e.ownerUuid + "|" + e.ownerName + "|" + e.filterName + (e.audioActive ? "|1" : "|0"));
	std::sort(parts.begin(), parts.end());
	std::string s;
	for (const auto &p : parts) {
		s += p;
		s += ";";
	}
	return s;
}

void CbPitchDock::rescan()
{
	const int prevIdx = selectedIndex();
	std::string prevUuid = prevIdx >= 0 ? entries_[prevIdx].ownerUuid : std::string();

	releaseEntries();

	enumPitchTargets(entries_, true);
	lastSig_ = signatureOf(entries_);

	QSignalBlocker block(combo_);
	combo_->clear();
	for (size_t i = 0; i < entries_.size(); i++) {
		const auto &e = entries_[i];
		bool dup = false;
		for (size_t j = 0; j < entries_.size(); j++)
			if (j != i && entries_[j].ownerName == e.ownerName) {
				dup = true;
				break;
			}
		QString label = QString::fromUtf8(e.ownerName.c_str());
		if (dup && !e.filterName.empty())
			label += QStringLiteral(" (") + QString::fromUtf8(e.filterName.c_str()) + QStringLiteral(")");

		if (!e.audioActive)
			label += QStringLiteral(" ⚠");
		combo_->addItem(label);
	}

	if (entries_.empty()) {
		combo_->addItem(T_("DockNoTarget"));
		combo_->setCurrentIndex(0);
		setControlsEnabled(false);
		setReadout(0, false);
		return;
	}

	int pick = -1;
	config_t *cfg = obs_frontend_get_user_config();
	std::string wantUuid = !prevUuid.empty() ? prevUuid : std::string();
	std::string wantName;
	if (cfg) {
		if (wantUuid.empty()) {
			const char *u = config_get_string(cfg, CFG_SECTION, CFG_TARGET_UUID);
			if (u)
				wantUuid = u;
		}
		const char *n = config_get_string(cfg, CFG_SECTION, CFG_TARGET_NAME);
		if (n)
			wantName = n;
	}
	if (!wantUuid.empty())
		for (size_t i = 0; i < entries_.size(); i++)
			if (entries_[i].ownerUuid == wantUuid) {
				pick = int(i);
				break;
			}
	if (pick < 0 && !wantName.empty())
		for (size_t i = 0; i < entries_.size(); i++)
			if (entries_[i].ownerName == wantName) {
				pick = int(i);
				break;
			}
	if (pick < 0)
		pick = 0;

	combo_->setCurrentIndex(pick);
	setControlsEnabled(true);
	block.unblock();
	saveSelection();
	refreshReadout();
}

void CbPitchDock::applyDelta(int delta)
{
	obs_source_t *filter = acquireTarget();
	if (!filter) {

		rescan();
		return;
	}

	obs_data_t *settings = obs_source_get_settings(filter);
	const int cur = int(obs_data_get_int(settings, S_SEMITONES));
	const int next = (delta == 0) ? 0 : std::clamp(cur + delta, CB_PITCH_MIN, CB_PITCH_MAX);

	if (next != cur) {
		obs_data_set_int(settings, S_SEMITONES, next);

		obs_source_update(filter, settings);
	}
	setReadout(next, true);

	obs_data_release(settings);
	obs_source_release(filter);
}

void CbPitchDock::pollTick()
{

	std::vector<TargetEntry> live;
	enumPitchTargets(live, false);
	const std::string sig = signatureOf(live);
	const bool popupOpen = combo_->view() && combo_->view()->isVisible();
	if (sig != lastSig_ && !popupOpen) {
		rescan();
		return;
	}
	refreshReadout();
}

void CbPitchDock::updateWarning()
{
	const int idx = selectedIndex();
	const bool warn = idx >= 0 && idx < int(entries_.size()) && !entries_[idx].audioActive;
	if (warn)
		warn_->setText(T_("DockNoAudio"));
	warn_->setVisible(warn);
}

void CbPitchDock::refreshReadout()
{
	updateWarning();
	if (entries_.empty())
		return;
	obs_source_t *filter = acquireTarget();
	if (!filter) {

		rescan();
		return;
	}
	obs_data_t *settings = obs_source_get_settings(filter);
	const int cur = int(obs_data_get_int(settings, S_SEMITONES));
	setReadout(cur, true);
	obs_data_release(settings);
	obs_source_release(filter);
}

void CbPitchDock::saveSelection()
{
	const int idx = selectedIndex();
	if (idx < 0)
		return;
	config_t *cfg = obs_frontend_get_user_config();
	if (!cfg)
		return;
	config_set_string(cfg, CFG_SECTION, CFG_TARGET_UUID, entries_[idx].ownerUuid.c_str());
	config_set_string(cfg, CFG_SECTION, CFG_TARGET_NAME, entries_[idx].ownerName.c_str());
}

void CbPitchDock::setControlsEnabled(bool on)
{
	combo_->setEnabled(on);
	down_->setEnabled(on);
	up_->setEnabled(on);
	reset_->setEnabled(on);
}

void CbPitchDock::setReadout(int semitones, bool known)
{
	if (!known) {
		readout_->setText(QStringLiteral("—"));
		return;
	}
	QString s = QString::number(semitones);
	if (semitones > 0)
		s = QStringLiteral("+") + s;
	readout_->setText(s);
}

CbPitchDock *g_dock = nullptr;

void on_frontend_event(enum obs_frontend_event event, void *)
{
	if (!g_dock)
		return;
	switch (event) {
	case OBS_FRONTEND_EVENT_FINISHED_LOADING:
	case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED:

		g_dock->rescan();
		break;
	default:
		break;
	}
}

} // namespace

void cb_pitch_dock_register()
{
	if (g_dock)
		return;

	g_dock = new CbPitchDock();

	if (!obs_frontend_add_dock_by_id(DOCK_ID, T_("DockTitle"), g_dock)) {

		delete g_dock;
		g_dock = nullptr;
		blog(LOG_WARNING, "[cb-pitch-shift] dock: add_dock_by_id failed");
		return;
	}

	obs_frontend_add_event_callback(on_frontend_event, nullptr);
	blog(LOG_INFO, "[cb-pitch-shift] dock registered");
}

void cb_pitch_dock_unregister()
{

	obs_frontend_remove_event_callback(on_frontend_event, nullptr);
	if (g_dock) {

		obs_frontend_remove_dock(DOCK_ID);
		g_dock = nullptr;
	}
}

#include "dock.moc"
