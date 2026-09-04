# cb-pitch-shift — OBS 變調（升降 key）音訊濾鏡

[English](README.md) · **繁體中文** · [日本語](README.ja.md)

一個 [OBS Studio](https://obsproject.com/) 音訊濾鏡，能把來源的聲音**在不改變速度的前提下**
上下移調（整數半音），並附一個**停駐視窗（dock）**，讓你直播中一鍵升降 key。

典型用途是翻唱／卡拉OK 直播：把伴奏放在瀏覽器來源或媒體來源上、加上這個濾鏡，就能依歌手音域
升降 key——放在另一個來源的人聲麥克風完全不受影響。

**目錄：** [功能](#功能) · [安裝](#安裝) · [解除安裝](#解除安裝) · [使用教學](#使用教學) · [從原始碼建置](#從原始碼建置) · [授權](#授權)

## 功能

- **移調 −12 … +12 半音**，速度不變（相位聲碼器式的變調）。
- **0 半音＝直接放行** —— 不處理、不增加延遲。
- 啟用時**固定約 60 ms 音訊延遲**（見濾鏡內說明）。畫面不會延遲，所以若要精確對齊，可在同一個
  來源再加一個「畫面延遲（非同步）」濾鏡。
- **停駐視窗（在 OBS 裡是「升降 key (ChiwaBots.com)」）**：選目標來源、用 `−` / `+` 微調、歸零，
  並隨時看到目前是第幾個半音。當目標來源的音訊沒有走 OBS 時，面板會標示提醒（見下）。
- **六語介面**：English、繁體中文、简体中文、日本語、한국어、Español。
- **Windows、macOS、Linux**，OBS **31.1** 以上。

## 安裝

到 *Releases* 頁下載你平台對應的檔案，然後：

- **Windows** —— 兩種方式：
  - **安裝程式**（`…-windows-x64.exe`）：雙擊照精靈走。它**未經程式碼簽章**，所以 Windows
    SmartScreen 會跳一次「不明發行者」提示 —— 點 *更多資訊 → 仍要執行*。之後要移除：*設定 → 應用程式*
    （或*控制台 → 程式和功能*）→ **升降 key (ChiwaBots.com)** → 解除安裝。安裝精靈會**跟隨你的
    Windows 顯示語言**（英／繁中／简中／日／韓／西）。
  - **ZIP**（`…-windows-x64.zip`）：把 `cb-pitch-shift` 資料夾複製到
    `%ProgramData%\obs-studio\plugins\`。沒有 SmartScreen 提示，但安裝與移除都要自己來。

  兩種裝完都要**重開 OBS**。

  > **綠色版（可攜式）OBS** 掃的是相對於它自己資料夾的位置、不是 `%ProgramData%`，所以安裝程式
  > 不適用它 —— 請改用 ZIP。從 ZIP 把 `cb-pitch-shift\bin\64bit\cb-pitch-shift.dll` 複製到
  > `<你的OBS>\obs-plugins\64bit\`、把 `cb-pitch-shift\data\` 內的東西複製到
  > `<你的OBS>\data\obs-plugins\cb-pitch-shift\`。

- **macOS** —— 打開 `.pkg` 照安裝程式走，然後重開 OBS。這個 `.pkg` **未公證**（免費路線），所以
  macOS Gatekeeper 第一次會擋 —— 見下方。

- **Linux** —— 兩種方式：
  - **`.deb`**（`sudo apt install ./cb-pitch-shift-*.deb`）：裝到 `/usr`，對得上**官方 OBS PPA**
    （`ppa:obsproject/obs-studio`）或你發行版 apt 套件的 OBS。若 OBS 沒抓到，多半是你的 OBS 裝在
    不同前綴（例如某個下載的 build 裝在 `/usr/local`，或 Flatpak/Snap 沙盒）—— 見下方。
  - **使用者目錄 drop-in**（適用**任何**非沙盒 OBS、免 `sudo`、OBS 升級也不會被洗掉）：把 tarball
    解壓（或把 `.deb` 的檔案複製）到 `~/.config/obs-studio/plugins/cb-pitch-shift/`，版面如下：

    ```
    ~/.config/obs-studio/plugins/cb-pitch-shift/bin/64bit/cb-pitch-shift.so
    ~/.config/obs-studio/plugins/cb-pitch-shift/data/locale/*.ini
    ```

  裝完都要重開 OBS。

  > **「裝了但 OBS 沒看到濾鏡」** 幾乎都是路徑對不上：OBS 掃的是**它自己**安裝前綴底下的插件目錄。
  > 用 `dpkg -L obs-studio | grep obs-plugins` 看 OBS 自己的插件放哪，`.so` 就得放在同一棵樹底下。
  > Flatpak/Snap 的 OBS 是沙盒、完全看不到系統安裝的插件 —— 請改用使用者目錄 drop-in，或改裝官方 PPA 的 OBS。

### macOS：Gatekeeper 會擋 `.pkg`

因為這個 build 未簽章／只有 ad-hoc 簽章（沒有付費 Apple 開發者帳號），雙擊下載的 `.pkg` 會出現
*「Apple 無法驗證……是否為惡意軟體」*，而且**只有「完成」和「丟到垃圾桶」**、沒有「打開」按鈕。
這是預期行為。請按**完成**（不要按丟到垃圾桶），然後擇一放行一次：

- **終端機** —— 清掉下載隔離標記，再正常打開 `.pkg`：

  ```bash
  xattr -dr com.apple.quarantine ~/Downloads/cb-pitch-shift-*-macos-universal.pkg
  ```

- **或** *系統設定 → 隱私權與安全性 → 往下捲 →* 在被封鎖的 `.pkg` 旁點**「仍要打開」**，輸入密碼後
  再開一次 `.pkg`。

安裝程式會把 `cb-pitch-shift.plugin` 放到 `~/Library/Application Support/obs-studio/plugins/`。
Apple Silicon 至少要有 ad-hoc 簽章才載得起來，而 CI build 已經簽好了，所以只要 `.pkg` 跑完，
插件就會正常載入 OBS。

> 要完全免去這個提示（下載即用），`.pkg` 需要付費的 Apple Developer ID 簽章＋公證 —— 這在免費
> 路線上刻意延後。

## 解除安裝

- **Windows（安裝程式）：** *設定 → 應用程式*（或*控制台 → 程式和功能*）→
  **升降 key (ChiwaBots.com)** → 解除安裝。
- **Windows（ZIP）：** 刪掉 `%ProgramData%\obs-studio\plugins\` 底下的 `cb-pitch-shift` 資料夾。
- **macOS：** 刪掉 `~/Library/Application Support/obs-studio/plugins/cb-pitch-shift.plugin`。
  macOS 的 `.pkg` 沒有內建反安裝器，所以這是手動步驟 —— 把那一個 bundle 拖到垃圾桶就是全部。
- **Linux（.deb）：** `sudo apt remove cb-pitch-shift`（或 `sudo dpkg -r cb-pitch-shift`）。
- **Linux（使用者目錄 drop-in）：** 刪掉 `~/.config/obs-studio/plugins/cb-pitch-shift/`。

之後重開 OBS。

## 使用教學

設定一次，之後直播中就能從停駐視窗即時升降 key。

### 1. 把伴奏放在自己的來源上

把伴奏加成**瀏覽器來源**（YouTube／卡拉OK 連結）或**媒體來源**（本機檔案）。歌手的麥克風放在
*另一個*來源 —— 這個濾鏡只影響你加它的那個來源，所以人聲不受影響。

### 2. 讓伴奏的音訊走 OBS —— 這步別跳過

這是**音訊濾鏡**，只會影響「走 OBS」的聲音。瀏覽器來源預設把聲音直接送到你的喇叭，所以濾鏡收不到
任何取樣、**完全沒作用**。請打開伴奏來源的**屬性**，勾選**「使用 OBS 控制音訊」**。開了之後，來源就會
出現在**音效混音器**裡 —— 那就代表它的音訊有走 OBS。

![在來源屬性裡勾選「使用 OBS 控制音訊」](docs/img/zh-TW/usage-control-audio.png)

### 3. 加上升降 key 濾鏡

右鍵點伴奏來源 → **濾鏡**。在**音訊濾鏡**底下點 **＋**，選 **升降 key (ChiwaBots.com)**。

![加入「升降 key (ChiwaBots.com)」音訊濾鏡](docs/img/zh-TW/usage-add-filter.png)

### 4. 設定 key

拖動 **升降 key (半音)** 來移調 −12 … +12 半音。**0＝直接放行**（不處理、不增加延遲）；數值越高、
key 越高。

![升降 key 濾鏡面板與滑桿](docs/img/zh-TW/usage-filter-panel.png)

### 5. 從停駐視窗即時升降 key

從 OBS 選單列打開 **停駐視窗 → 升降 key (ChiwaBots.com)**。在下拉選單選**目標來源**，再按 **−** / **+**
微調、或按 **歸零** 回到 0。目前值顯示在兩顆按鈕中間，濾鏡的滑桿也會跟著動 —— 一首歌唱到一半也只要
點一下。

![「升降 key (ChiwaBots.com)」停駐視窗](docs/img/zh-TW/usage-dock.png)

> 若目標來源的音訊沒走 OBS（第 2 步），停駐視窗會用 **⚠** 標示並顯示提示 —— 在你打開
> *使用 OBS 控制音訊* 之前，變調不會有任何作用。

### 小提醒與疑難排解

- **啟用時約 60 ms 延遲。** 只有聲音被延遲、畫面沒有，所以畫面會略早一點。要對齊的話，在同一個
  來源加一個 **畫面延遲（非同步）** 濾鏡、設成約 60 ms。
- **聲音沒變化？** 來源的音訊沒走 OBS —— 重做第 2 步（停駐視窗上的 **⚠** 就是線索）。
- **裝完看不到濾鏡或停駐視窗？** 重開 OBS，並確認插件有裝到你這套 OBS 對的位置（見 [安裝](#安裝)）。

## 從原始碼建置

建置走的是標準的 [obs-plugintemplate](https://github.com/obsproject/obs-plugintemplate) 工具鏈：
CMake preset 會自動下載 pin 好的 OBS、obs-deps、Qt6（見 `buildspec.json`）與 DSP 標頭，所以你不需要
本機先建 OBS。

需求：CMake 3.28+、Git，以及各平台工具鏈 —— Windows 用 Visual Studio 2022（C++ 桌面工作負載）、
macOS 用 Xcode、Linux 用 GCC/Clang + Ninja。

```bash
# Windows
cmake --preset windows-x64
cmake --build --preset windows-x64

# macOS
cmake --preset macos
cmake --build --preset macos

# Linux
cmake --preset ubuntu-x86_64
cmake --build --preset ubuntu-x86_64
```

三平台的 Release 產物由 GitHub Actions（見 `.github/workflows/`）在每次 push 時產出。

## 第三方元件

- [Signalsmith Stretch](https://github.com/Signalsmith-Audio/signalsmith-stretch) 與
  [signalsmith-linear](https://github.com/Signalsmith-Audio/linear) —— MIT、header-only
  （變調 DSP），在 configure 時抓取。
- Qt 6 —— 用於停駐視窗，連結 OBS 自帶的那份 Qt（LGPL/GPL）。
- libobs / obs-frontend-api（OBS Studio）—— GPL-2.0-or-later。

## 授權

**GPL-2.0-or-later** —— 見 [LICENSE](LICENSE)。本插件連結 libobs，而 libobs 以 GNU GPL 散布，所以
本插件以相容授權釋出。
