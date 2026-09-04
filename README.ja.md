# cb-pitch-shift — OBS ピッチシフト（キー変更）音声フィルタ

[English](README.md) · [繁體中文](README.zh-TW.md) · **日本語**

ソースの音声を**テンポを変えずに**半音単位で上下させる [OBS Studio](https://obsproject.com/)
音声フィルタです。配信中にワンクリックでキーを変えられる**ドック**も付いています。

主な用途は歌ってみた／カラオケ配信です。伴奏をブラウザソースやメディアソースに置いてこのフィルタを
追加すれば、歌い手の音域に合わせてキーを上げ下げできます —— 別のソースに載せたマイクの歌声はそのままです。

**目次：** [機能](#機能) · [インストール](#インストール) · [アンインストール](#アンインストール) · [使い方](#使い方) · [ソースからのビルド](#ソースからのビルド) · [ライセンス](#ライセンス)

## 機能

- **−12 … +12 半音の移調**、テンポは保持（位相ボコーダー方式のピッチシフト）。
- **0 半音＝バイパス** —— 処理なし、遅延の増加なし。
- 有効時は**約 60 ms の一定した音声遅延**（フィルタ内の説明を参照）。映像は遅延しないので、正確に
  合わせたい場合は同じソースに「映像の遅延（非同期）」フィルタを追加してください。
- **ドック（OBS 上は「キー変更 (ChiwaBots.com)」）**：対象ソースを選び、`−` / `+` で微調整、リセット、
  現在の半音数がひと目で分かります。対象の音声が OBS を通っていないときはドックが警告します（下記）。
- **6 言語対応**：English、繁體中文、简体中文、日本語、한국어、Español。
- **Windows・macOS・Linux**、OBS **31.1** 以降。

## インストール

*Releases* ページからお使いのプラットフォーム向けのファイルをダウンロードし、次のようにします：

- **Windows** —— 2 通り：
  - **インストーラー**（`…-windows-x64.exe`）：ダブルクリックしてウィザードに従います。**コード署名
    されていない**ため、Windows SmartScreen が一度だけ「不明な発行元」の警告を出します —— *詳細情報 →
    実行* をクリックしてください。後で削除するには *設定 → アプリ*（または*コントロールパネル →
    プログラムと機能*）→ **キー変更 (ChiwaBots.com)** → アンインストール。インストーラーは
    **Windows の表示言語**（英／繁中／简中／日／韓／西）に従います。
  - **ZIP**（`…-windows-x64.zip`）：`cb-pitch-shift` フォルダを
    `%ProgramData%\obs-studio\plugins\` にコピーします。SmartScreen の警告は出ませんが、導入も削除も
    手動になります。

  どちらの場合も **OBS を再起動**してください。

  > **ポータブル版の OBS** は `%ProgramData%` ではなく自身のフォルダを基準にスキャンするため、
  > インストーラーは使えません —— ZIP を使ってください。ZIP から
  > `cb-pitch-shift\bin\64bit\cb-pitch-shift.dll` を `<あなたの OBS>\obs-plugins\64bit\` へ、
  > `cb-pitch-shift\data\` の中身を `<あなたの OBS>\data\obs-plugins\cb-pitch-shift\` へコピーします。

- **macOS** —— `.pkg` を開いてインストーラーに従い、OBS を再起動します。この `.pkg` は**公証されていない**
  （無料ルート）ため、macOS の Gatekeeper が初回にブロックします —— 下記参照。

- **Linux** —— 2 通り：
  - **`.deb`**（`sudo apt install ./cb-pitch-shift-*.deb`）：`/usr` にインストールされ、**公式 OBS PPA**
    （`ppa:obsproject/obs-studio`）やディストリの apt パッケージの OBS と一致します。OBS が認識しない場合、
    あなたの OBS が別のプレフィックス（例：`/usr/local` に置かれたビルド、または Flatpak/Snap サンドボックス）に
    入っている可能性が高いです —— 下記参照。
  - **ユーザーディレクトリへの配置**（**任意の**非サンドボックス OBS で動作、`sudo` 不要、OBS 更新でも
    消えません）：tarball を展開（または `.deb` のファイルをコピー）して
    `~/.config/obs-studio/plugins/cb-pitch-shift/` に次の構成で置きます：

    ```
    ~/.config/obs-studio/plugins/cb-pitch-shift/bin/64bit/cb-pitch-shift.so
    ~/.config/obs-studio/plugins/cb-pitch-shift/data/locale/*.ini
    ```

  どちらの場合も OBS を再起動してください。

  > **「インストールしたのに OBS にフィルタが出ない」** はほぼパスの不一致です：OBS は**自身の**
  > インストールプレフィックス配下のプラグインディレクトリをスキャンします。`dpkg -L obs-studio | grep obs-plugins`
  > で OBS 本体のプラグインの場所を確認し、`.so` を同じツリーに置いてください。Flatpak/Snap の OBS は
  > サンドボックスで、システムに入れたプラグインは一切見えません —— ユーザーディレクトリへの配置を使うか、
  > 公式 PPA の OBS を入れてください。

### macOS：Gatekeeper が `.pkg` をブロックします

ビルドが未署名／ad-hoc 署名（有料の Apple Developer アカウントなし）のため、ダウンロードした `.pkg` を
ダブルクリックすると *「Apple はこのファイルにマルウェアが含まれていないことを確認できません」* と表示され、
**「完了」と「ゴミ箱に入れる」しかなく**、「開く」ボタンがありません。これは想定どおりです。**「完了」**
（ゴミ箱に入れるではなく）を押し、次のどちらかを一度だけ行ってください：

- **ターミナル** —— ダウンロード隔離属性を外してから `.pkg` を普通に開きます：

  ```bash
  xattr -dr com.apple.quarantine ~/Downloads/cb-pitch-shift-*-macos-universal.pkg
  ```

- **または** *システム設定 → プライバシーとセキュリティ → 下にスクロール →* ブロックされた `.pkg` の横の
  **「このまま開く」** をクリックし、認証してから `.pkg` をもう一度開きます。

インストーラーは `cb-pitch-shift.plugin` を `~/Library/Application Support/obs-studio/plugins/` に配置します。
Apple Silicon では読み込みに最低でも ad-hoc 署名が必要ですが、CI ビルドで付与済みなので、`.pkg` を実行すれば
プラグインは OBS で正常に読み込まれます。

> この警告を完全になくす（ダウンロードしてすぐ実行）には、`.pkg` に有料の Apple Developer ID 署名＋公証が
> 必要です —— 無料ルートでは意図的に見送っています。

## アンインストール

- **Windows（インストーラー）：** *設定 → アプリ*（または*コントロールパネル → プログラムと機能*）→
  **キー変更 (ChiwaBots.com)** → アンインストール。
- **Windows（ZIP）：** `%ProgramData%\obs-studio\plugins\` の `cb-pitch-shift` フォルダを削除します。
- **macOS：** `~/Library/Application Support/obs-studio/plugins/cb-pitch-shift.plugin` を削除します。
  macOS の `.pkg` にはアンインストーラーがないため手動です —— その 1 つの bundle をゴミ箱に入れれば完了です。
- **Linux（.deb）：** `sudo apt remove cb-pitch-shift`（または `sudo dpkg -r cb-pitch-shift`）。
- **Linux（ユーザーディレクトリ）：** `~/.config/obs-studio/plugins/cb-pitch-shift/` を削除します。

その後 OBS を再起動してください。

## 使い方

一度セットアップすれば、あとは配信中にドックからキーを変えられます。

### 1. 伴奏を専用のソースに置く

伴奏を**ブラウザソース**（YouTube／カラオケのリンク）または**メディアソース**（ローカルファイル）として
追加します。歌い手のマイクは*別の*ソースに置いてください —— このフィルタは追加したソースだけに効くので、
歌声はそのままです。

### 2. 伴奏の音声を OBS に通す —— ここは飛ばさない

これは**音声フィルタ**なので、OBS を通る音声にしか効きません。ブラウザソースは既定で音声をスピーカーへ
直接流すため、フィルタには何も届かず**何も起きません**。伴奏ソースの**プロパティ**を開き、
**「OBSで音声を制御する」**にチェックを入れてください。有効にすると、そのソースが**音声ミキサー**に表示され、
音声が OBS を通っている合図になります。

![ソースのプロパティで「OBSで音声を制御する」を有効にする](docs/img/ja-JP/usage-control-audio.png)

### 3. キー変更フィルタを追加する

伴奏ソースを右クリック → **フィルタ**。**音声フィルタ**の下で **＋** をクリックし、
**キー変更 (ChiwaBots.com)** を選びます。

![「キー変更 (ChiwaBots.com)」音声フィルタを追加](docs/img/ja-JP/usage-add-filter.png)

### 4. キーを設定する

**キー (半音)** をドラッグして −12 … +12 半音移調します。**0＝バイパス**（処理なし・遅延の増加なし）、
値が大きいほどキーが上がります。

![キー変更フィルタのパネルとスライダー](docs/img/ja-JP/usage-filter-panel.png)

### 5. ドックから配信中にキーを変える

OBS のメニューバーから **ドック → キー変更 (ChiwaBots.com)** を開きます。ドロップダウンで**対象ソース**を選び、
**−** / **+** で微調整、または **リセット** で 0 に戻します。現在値は 2 つのボタンの間に表示され、フィルタの
スライダーも連動します —— 曲の途中でもワンクリックで済みます。

![「キー変更 (ChiwaBots.com)」ドック](docs/img/ja-JP/usage-dock.png)

> 対象ソースの音声が OBS を通っていない場合（手順 2）、ドックは **⚠** を付けてヒントを表示します ——
> *OBSで音声を制御する* を有効にするまで、キー変更は何も起きません。

### メモ・トラブルシューティング

- **有効時は約 60 ms の遅延。** 遅れるのは音声だけで映像は遅れないため、映像がわずかに先行します。合わせるには、
  同じソースに **映像の遅延（非同期）** フィルタを追加し、約 60 ms に設定します。
- **音が変わらない？** ソースの音声が OBS を通っていません —— 手順 2 をやり直してください（ドックの **⚠** が目印）。
- **インストール後にフィルタやドックが見当たらない？** OBS を再起動し、お使いの OBS に対して正しい場所へ
  プラグインが入ったか確認してください（[インストール](#インストール)参照）。

## ソースからのビルド

ビルドには標準の [obs-plugintemplate](https://github.com/obsproject/obs-plugintemplate) ツールチェーンを
使います。CMake プリセットが、ピン留めした OBS・obs-deps・Qt6（`buildspec.json` を参照）と DSP ヘッダを
自動で取得するので、ローカルで OBS をビルドする必要はありません。

必要なもの：CMake 3.28+、Git、および各プラットフォームのツールチェーン —— Windows は Visual Studio 2022
（C++ デスクトップ ワークロード）、macOS は Xcode、Linux は GCC/Clang + Ninja。

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

3 プラットフォームの Release 成果物は、push のたびに GitHub Actions（`.github/workflows/` を参照）が生成します。

## サードパーティ コンポーネント

- [Signalsmith Stretch](https://github.com/Signalsmith-Audio/signalsmith-stretch) と
  [signalsmith-linear](https://github.com/Signalsmith-Audio/linear) —— MIT、ヘッダオンリー
  （ピッチシフトの DSP）、configure 時に取得。
- Qt 6 —— ドックに使用。OBS 同梱の Qt にリンクします（LGPL/GPL）。
- libobs / obs-frontend-api（OBS Studio）—— GPL-2.0-or-later。

## ライセンス

**GPL-2.0-or-later** —— [LICENSE](LICENSE) を参照。本プラグインは libobs にリンクし、libobs は GNU GPL で
配布されているため、本プラグインも互換ライセンスで公開されています。
