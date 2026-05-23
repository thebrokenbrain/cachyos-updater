#!/usr/bin/env bash
# Instalador de CachyOS Updater
# Uso (como usuario normal): curl -fsSL https://raw.githubusercontent.com/thebrokenbrain/cachyos-updater/main/install.sh | bash
# El script descarga el último .pkg.tar.zst de GitHub Releases y lo instala vía pacman.

set -euo pipefail

REPO="thebrokenbrain/cachyos-updater"
TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

have()  { command -v "$1" >/dev/null 2>&1; }
say()   { printf '\033[1;36m►\033[0m %s\n' "$*"; }
warn()  { printf '\033[1;33m!!\033[0m %s\n' "$*" >&2; }
die()   { printf '\033[1;31m✖\033[0m %s\n' "$*" >&2; exit 1; }

[[ $EUID -eq 0 ]] && die "No ejecutes este instalador como root. Te pedirá la contraseña cuando haga falta."

for tool in curl pacman sudo; do
    have "$tool" || die "Falta '$tool'. Este instalador requiere CachyOS o un Arch Linux."
done

if ! grep -q '^ID=cachyos\|^ID_LIKE=.*arch' /etc/os-release 2>/dev/null; then
    warn "Este programa está pensado para CachyOS (o Arch). Sigues bajo tu propio riesgo."
fi

say "Buscando la última versión de CachyOS Updater…"
api_url="https://api.github.com/repos/${REPO}/releases/latest"
asset_url=$(curl -fsSL "$api_url" \
    | grep -E '"browser_download_url":.*\.pkg\.tar\.zst"' \
    | head -n1 \
    | sed -E 's/.*"browser_download_url": *"([^"]+)".*/\1/')

[[ -n "${asset_url:-}" ]] || die "No se encontró un paquete .pkg.tar.zst en la última release."

pkg_name=$(basename "$asset_url")
say "Descargando ${pkg_name}…"
curl -fL --progress-bar "$asset_url" -o "${TMPDIR}/${pkg_name}"

# Dependencias en tiempo de ejecucion. Las instalamos/actualizamos explicitamente
# para no depender del estado de la BD local: si el sistema lleva meses sin
# tocarse, 'pacman -U' puede fallar resolviendo qt6-base. Sin --needed para
# forzar upgrade si la version local es mas antigua que la que el binario espera.
RUNTIME_DEPS=(qt6-base qt6-svg polkit)

say "Refrescando la base de datos de paquetes…"
sudo pacman -Sy --noconfirm

say "Instalando/actualizando dependencias gráficas (${RUNTIME_DEPS[*]})…"
sudo pacman -S --noconfirm "${RUNTIME_DEPS[@]}"

say "Instalando ${pkg_name}…"
sudo pacman -U --noconfirm "${TMPDIR}/${pkg_name}"

say "Listo. Lanza 'CachyOS Updater' desde el menú de aplicaciones o ejecuta 'cachyos-updater'."
