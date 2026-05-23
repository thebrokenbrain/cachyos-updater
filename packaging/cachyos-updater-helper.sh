#!/usr/bin/env bash
# cachyos-updater-helper
# Ejecutado como root vía pkexec. Optimiza mirrors y actualiza el sistema.
# Toda la salida va a stdout para que la GUI la muestre en directo.

set -uo pipefail

export LC_ALL="${LC_ALL:-C.UTF-8}"
# Damos a pacman un TERM "tonto" pero válido. Mejor que 'dumb' para evitar
# que oculte salida; con 'script' simulamos TTY más abajo.
export TERM=xterm
export NO_COLOR=1

phase() { printf '==> phase: %s\n' "$*"; }
say()   { printf '► %s\n' "$*"; }
warn()  { printf '!! %s\n' "$*" >&2; }
have()  { command -v "$1" >/dev/null 2>&1; }

# Ejecuta un comando dentro de un pseudo-terminal para que su salida (pacman,
# en particular) no quede bufferizada y aparezca línea a línea en la GUI.
# Si 'script' no está disponible (raro: forma parte de util-linux), cae a stdbuf.
run_live() {
    if have script; then
        # -q: sin "Script started", -f: flush tras cada línea, -e: exit code real
        script -qfec "$*" /dev/null
    elif have stdbuf; then
        stdbuf -oL -eL bash -c "$*"
    else
        bash -c "$*"
    fi
}

if [[ "${EUID:-$(id -u)}" -ne 0 ]]; then
    warn "Este ayudante debe ejecutarse como root (usa pkexec)."
    exit 126
fi

trap 'phase "Interrumpido por el usuario."; exit 130' INT TERM

phase "Comprobando entorno…"
# shellcheck source=/dev/null
say "Distribución: $(. /etc/os-release && printf '%s' "${PRETTY_NAME:-desconocida}")"
say "Fecha: $(date '+%Y-%m-%d %H:%M:%S')"

# 1) Sincronizar reloj (los errores de mirrors suelen venir de hora desfasada)
if have timedatectl; then
    timedatectl set-ntp true >/dev/null 2>&1 || true
fi

# 2) Llaves de firma sanas. Es la causa #1 de errores tras meses sin actualizar.
phase "Verificando llaves de firma…"
if have pacman-key; then
    pacman-key --init        2>&1 || warn "pacman-key --init falló"
    pacman-key --populate    2>&1 || warn "pacman-key --populate falló"
fi

# 3) Refrescar mirrorlist específico de CachyOS
phase "Optimizando mirrors de CachyOS…"
if have cachyos-rate-mirrors; then
    if run_live "cachyos-rate-mirrors"; then
        say "Mirrors de CachyOS optimizados."
    else
        warn "cachyos-rate-mirrors falló, se continúa con la lista actual."
    fi
else
    warn "cachyos-rate-mirrors no está instalado (paquete: cachyos-rate-mirrors)."
fi

# 4) Refrescar mirrorlist de Arch usando reflector
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

# 5) Sincronizar BD
phase "Sincronizando base de datos de paquetes…"
if ! run_live "pacman -Sy --noconfirm --noprogressbar"; then
    warn "Fallo al sincronizar; reintentando con refresco forzado."
    run_live "pacman -Syy --noconfirm --noprogressbar" || {
        phase "Sincronización imposible."
        exit 2
    }
fi

# 6) Contar lo que viene para mostrar feedback antes del bloque "Retrieving packages"
pending_count=$(pacman -Qu 2>/dev/null | wc -l | tr -d ' ')
if [[ "${pending_count:-0}" -gt 0 ]]; then
    say "Paquetes con actualización pendiente: ${pending_count}"
    # Tamaño de descarga aproximado (en MB). pacman -Sup --print escribe URLs.
    download_count=$(pacman -Sup --print-format '%n' 2>/dev/null | grep -cv '^::' || true)
    if [[ "${download_count:-0}" -gt 0 ]]; then
        say "Se descargarán ${download_count} archivos. Verás cada uno aquí abajo."
    fi
else
    say "No hay paquetes nuevos: el sistema ya estaba al día."
fi

# 7) Actualizar el sistema. Con --noprogressbar + pty (run_live), pacman emite
#    una línea por paquete según los va bajando/instalando, en directo.
phase "Descargando e instalando (no cierres la ventana)…"
if ! run_live "pacman -Su --noconfirm --needed --noprogressbar --disable-download-timeout"; then
    warn "Fallo en la actualización; reintentando tras re-rate de mirrors."
    if have cachyos-rate-mirrors; then
        run_live "cachyos-rate-mirrors" || true
    fi
    run_live "pacman -Syu --noconfirm --needed --noprogressbar --disable-download-timeout" || {
        phase "La actualización ha fallado."
        exit 3
    }
fi

# 8) Limpieza ligera de caché (últimas 3 versiones por seguridad)
if have paccache; then
    phase "Limpiando caché de paquetes antiguos…"
    paccache -rk3 2>&1 || true
fi

phase "Actualización completada."
say "Si se ha actualizado el kernel o un driver, reinicia para aplicarlo."
exit 0
