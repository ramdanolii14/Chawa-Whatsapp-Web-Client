# Maintainer: Ramdan Oli <developer@nyanpixel.my.id>
pkgname=chawa-whatsapp-web-client
pkgver=1.0
pkgrel=1
pkgdesc="WhatsApp Web client based on GTK4 dan WebKitGTK"
arch=('x86_64')
url="https://github.com/ramdanolii14/Chawa-Whatsapp-Web-Client"
license=('GPL-3.0-or-later')
depends=(
    'gtk4'
    'webkitgtk-6.0'
    'libnotify'
    'libva'
    'mesa'
    'gstreamer'
    'gst-plugins-bad'
)
makedepends=(
    'meson'
    'ninja'
    'pkg-config'
)
source=("$pkgname-$pkgver.tar.gz::$url/archive/v$pkgver.tar.gz")
sha256sums=('SKIP')

build() {
    cd "$pkgname-$pkgver"
    meson setup build \
        --prefix=/usr \
        --buildtype=release \
        --optimization=3 \
        -Dstrip=true
    ninja -C build
}

package() {
    cd "$pkgname-$pkgver"
    DESTDIR="$pkgdir" ninja -C build install
}
