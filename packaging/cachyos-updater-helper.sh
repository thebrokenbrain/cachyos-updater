#!/usr/bin/env bash
# cachyos-updater-helper
# Ejecutado como root vía pkexec. Optimiza mirrors y actualiza el sistema.
# Toda la salida va a stdout para que la GUI la muestre en directo.

set -uo pipefail

export LC_ALL="${LC_ALL:-C.UTF-8}"
export TERM=dumb
export NO_COLOR=1

phase() { printf '==> phase: %s\n' "$*"; }
say()   { printf '► %s\n' "$*"; }
warn()  { printf '!! %s\n' "$*" >&2; }
have()  { command -v "$1" >/dev/null 2>&1; }

if [[ "${EUID:-$(id -u)}" -ne 0 ]]; then
    warn "Este ayudante debe ejecutarse como root (usa pkexec)."
    exit 126
fi

trap 'phase "Interrumpido por el usuario."; exit 130' INT TERM

phase "Comprobando entorno…"
# shellcheck source=/dev/null
say "Distribución: $(. /etc/os-release && printf '%s' "${PRETTY_NAME:-desconocida}")"
say "Fecha: $(date '+%Y-%m-%d %H:%M:%S')"

# 1) Sincronizar reloj si está disponible (los errores de mirrors suelen venir de hora desfasada)
if have timedatectl; then
    timedatectl set-ntp true >/dev/null 2>&1 || true
fi

# 2) Asegurar que pacman tiene un keyring sano. Es la causa #1 de errores tras meses sin actualizar.
phase "Verificando llaves de firma…"
if have pacman-key; then
    pacman-key --init        2>&1 || warn "pacman-key --init falló"
    pacman-key --populate    2>&1 || warn "pacman-key --populate falló"
fi

# 3) Refrescar mirrorlist específico de CachyOS (paquete oficial: cachyos-rate-mirrors)
phase "Optimizando mirrors de CachyOS…"
if have cachyos-rate-mirrors; then
    if cachyos-rate-mirrors 2>&1; then
        say "Mirrors de CachyOS optimizados."
    else
        warn "cachyos-rate-mirrors falló, se continúa con la lista actual."
    fi
else
    warn "cachyos-rate-mirrors no está instalado (paquete: cachyos-rate-mirrors)."
fi

# 4) Refrescar mirrorlist de Arch usando reflector si está
phase "Optimizando mirrors de Arch…"
if have reflector; then
    if reflector \
            --protocol https \
            --age 24 \
            --latest 20 \
            --sort rate \
            --save /etc/pacman.d/mirrorlist 2>&1; then
        say "Mirrorlist de Arch actualizado."
    else
        warn "reflector falló, se continúa con la lista actual."
    fi
else
    warn "reflector no está instalado (paquete: reflector)."
fi

# 5) Actualizar el sistema. --noconfirm porque la GUI no es interactiva.
#    Hacemos un intento normal; si falla por mirror caído o BD obsoleta, reintentamos con -yy.
phase "Sincronizando base de datos de paquetes…"
if ! pacman -Sy --noconfirm 2>&1; then
    warn "Fallo al sincronizar; reintentando con refresco forzado."
    pacman -Syy --noconfirm 2>&1 || {
        phase "Sincronización imposible."
        exit 2
    }
fi

phase "Actualizando paquetes (puede tardar varios minutos)…"
if ! pacman -Su --noconfirm --needed 2>&1; then
    warn "Fallo en la actualización; reintentando tras re-rate de mirrors."
    if have cachyos-rate-mirrors; then
        cachyos-rate-mirrors 2>&1 || true
    fi
    pacman -Syu --noconfirm --needed 2>&1 || {
        phase "La actualización ha fallado."
        exit 3
    }
fi

# 6) Limpieza ligera de caché (mantenemos las últimas 3 versiones por seguridad)
if have paccache; then
    phase "Limpiando caché de paquetes antiguos…"
    paccache -rk3 2>&1 || true
fi

phase "Actualización completada."
say "Si se ha actualizado el kernel o un driver, reinicia para aplicarlo."
exit 0
