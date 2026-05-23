# Maintainer: thebrokenbrain <jossemi@gmail.com>
pkgname=cachyos-updater
pkgver=0.1.0
pkgrel=1
pkgdesc="Actualizador grafico simple para CachyOS"
arch=('x86_64')
url="https://github.com/thebrokenbrain/cachyos-updater"
license=('MIT')
depends=(
    'qt6-base'
    'qt6-svg'
    'polkit'
    'pacman'
)
optdepends=(
    'cachyos-rate-mirrors: optimizacion de mirrors de CachyOS'
    'reflector: optimizacion de mirrors de Arch'
    'pacman-contrib: limpieza automatica de cache (paccache)'
)
makedepends=(
    'cmake'
    'ninja'
    'qt6-base'
    'qt6-tools'
)
source=()
sha256sums=()

prepare() {
    rm -rf "${srcdir}/build"
}

build() {
    cd "${startdir}"
    cmake -B "${srcdir}/build" -G Ninja \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build "${srcdir}/build"
}

package() {
    DESTDIR="${pkgdir}" cmake --install "${srcdir}/build"
    install -Dm644 "${startdir}/LICENSE" \
        "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
}
