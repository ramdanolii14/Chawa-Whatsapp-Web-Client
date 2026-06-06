# Chawa — WhatsApp Web Client

> WhatsApp Web client ringan berbasis GTK4 dan WebKitGTK untuk Linux.

![Platform](https://img.shields.io/badge/platform-Linux-blue)
![License](https://img.shields.io/badge/license-GPL--3.0-green)
![Language](https://img.shields.io/badge/language-C-orange)
![Build](https://img.shields.io/badge/build-Meson-informational)

---

## Fitur

- **Single-instance** — buka dua kali? window yang sudah ada langsung difokuskan
- **Notifikasi native** — pesan masuk muncul sebagai notifikasi desktop lengkap dengan foto profil pengirim
- **Hardware video decode** — VA-API via Mesa/radeonsi untuk efisiensi CPU
- **Link eksternal aman** — link yang diklik di dalam chat terbuka di browser sistem, bukan di dalam app
- **Auto-recovery** — web process crash langsung reload otomatis
- **Izin otomatis** — kamera, mikrofon, dan notifikasi tidak perlu konfirmasi manual

---

## Screenshot

> *(tambahkan screenshot di sini)*

---

## Instalasi

### Arch Linux (AUR) — Direkomendasikan

```bash
yay -S chawa
```

### Manual (dari source)

#### 1. Install dependencies

```bash
sudo pacman -S --needed \
    gtk4 \
    webkitgtk-6.0 \
    libnotify \
    libva \
    libva-mesa-driver \
    mesa \
    gstreamer \
    gst-plugins-bad \
    gst-plugin-va \
    meson \
    ninja \
    base-devel
```

#### 2. Clone dan build

```bash
git clone https://github.com/ramdanolii/Chawa-Whatsapp-Web-Client.git
cd Chawa-Whatsapp-Web-Client
meson setup build --buildtype=release --optimization=3 -Dstrip=true
ninja -C build
```

#### 3. Install

```bash
sudo ninja -C build install
sudo update-desktop-database /usr/local/share/applications/
sudo gtk-update-icon-cache -f /usr/local/share/icons/hicolor/
```

#### 4. Jalankan

```bash
chawa
```

Atau cari **WhatsApp** di app launcher.

---

## Uninstall

```bash
sudo ninja -C build uninstall
```

---

## Dependencies — Penjelasan Lengkap

### Runtime

| Package | Versi | Fungsi dalam Chawa |
|---|---|---|
| `gtk4` | ≥ 4.0 | Framework UI utama — window, event loop, GApplication single-instance |
| `webkitgtk-6.0` | ≥ 2.44 | WebView untuk render WhatsApp Web; menyediakan WebKitWebView, WebKitSettings, WebKitUserContentManager, JSCValue |
| `libnotify` | ≥ 0.7 | Notifikasi desktop native — dipakai `notify_notification_new()` dan `notify_notification_show()` |
| `libva` | ≥ 2.0 | VA-API interface untuk hardware video decode |
| `libva-mesa-driver` | — | Driver VA-API backend Mesa — diperlukan agar `LIBVA_DRIVER_NAME=radeonsi` bekerja |
| `mesa` | — | Driver GPU open-source; dipakai via `MESA_LOADER_DRIVER_OVERRIDE=radeonsi` untuk GPU AMD |
| `gstreamer` | ≥ 1.20 | Pipeline multimedia — dipakai WebKitGTK untuk audio/video di chat |
| `gst-plugins-bad` | — | Plugin GStreamer tambahan termasuk VA-API video decoder (`vaapidecodebin`) |
| `gst-plugin-va` | — | Plugin GStreamer VA-API generasi baru (`vah264dec`, `vah265dec`, dsb.) — diaktifkan via `GST_VAAPI_ALL_DRIVERS=1` |

> **Catatan GPU:** Chawa secara default dikonfigurasi untuk GPU AMD (radeonsi). Untuk GPU Intel atau NVIDIA, kamu bisa override environment variable sebelum menjalankan app:
> ```bash
> LIBVA_DRIVER_NAME=i965 MESA_LOADER_DRIVER_OVERRIDE=i965 chawa   # Intel lama
> LIBVA_DRIVER_NAME=iHD  MESA_LOADER_DRIVER_OVERRIDE=iris  chawa   # Intel baru
> ```

### Build-time

| Package | Fungsi |
|---|---|
| `meson` | Build system — membaca `meson.build`, generate ninja files |
| `ninja` | Compile runner |
| `base-devel` | GCC, binutils, pkg-config, dan toolchain dasar lainnya |

### Implicit (pulled otomatis sebagai dep)

| Package | Ditarik oleh | Fungsi |
|---|---|---|
| `glib2` | `gtk4` | GObject, GApplication, GIO, GMemoryInputStream, g_base64_decode — dipakai intensif di seluruh kode |
| `gdk-pixbuf2` | `gtk4` | `GdkPixbuf` — dipakai di `notify.c` untuk decode dan scale foto profil dari data URL base64 |
| `cairo` | `gtk4` | Rendering 2D backend |
| `pango` | `gtk4` | Layout teks |

---

## Struktur Project

```
Chawa-Whatsapp-Web-Client/
├── src/
│   ├── main.c          # Entry point, setup VA-API env, GApplication
│   ├── window.c        # ChawaWindow — WebView, policy, JS bridge
│   ├── window.h
│   ├── notify.c        # Notifikasi native via libnotify + GdkPixbuf
│   └── notify.h
├── data/
│   ├── chawa.desktop   # Desktop entry (app launcher)
│   └── chawa.svg       # Icon aplikasi
├── meson.build
├── install.sh          # Script install manual (Arch Linux)
└── LICENSE             # GPL-3.0
```

---

## Cara Kerja

```
WhatsApp Web (JS)
      │
      │  window.Notification() di-intercept oleh JS hook
      │
      ▼
WebKit MessageHandler (chawaNotify)
      │
      │  postMessage → title\x01body\x01icon_data_url
      │
      ▼
on_notify_received() [C]
      │
      ├── decode base64 icon → GdkPixbuf → scale 48×48
      └── notify_notification_show() → notifikasi desktop native
```

Link eksternal yang diklik user diarahkan ke browser sistem via `g_app_info_launch_default_for_uri()`. Navigasi internal WebKit (redirect, reload) dibiarkan berjalan di dalam app.

---

## Lisensi

[GPL-3.0](LICENSE) © 2024 Ramdan Oli