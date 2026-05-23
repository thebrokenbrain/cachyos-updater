<div align="center">

<img src="packaging/cachyos-updater.svg" alt="CachyOS Updater" width="160" height="160" />

# CachyOS Updater

**Actualiza tu CachyOS con un solo clic.** Sin terminal, sin comandos, sin sorpresas.

[![build](https://github.com/thebrokenbrain/cachyos-updater/actions/workflows/build.yml/badge.svg)](https://github.com/thebrokenbrain/cachyos-updater/actions/workflows/build.yml)
[![release](https://img.shields.io/github/v/release/thebrokenbrain/cachyos-updater?include_prereleases&label=release)](https://github.com/thebrokenbrain/cachyos-updater/releases/latest)
[![license](https://img.shields.io/github/license/thebrokenbrain/cachyos-updater)](LICENSE)
![platform](https://img.shields.io/badge/platform-CachyOS%20%7C%20Arch%20Linux-7c3aed)

</div>

---

## ¿Por qué?

CachyOS es estupendo, pero mantenerlo al día implica abrir una terminal y escribir comandos. Para quien no quiere lidiar con eso, esta app es como el "Windows Update" de toda la vida: **abre, pulsa un botón, escribe la contraseña, espera**. Eso es todo.

Por debajo:

1. Pide permisos con `pkexec` (diálogo gráfico nativo de KDE/GNOME).
2. Optimiza los mirrors de CachyOS con `cachyos-rate-mirrors` y los de Arch con `reflector`, para que las descargas vayan rápido **y no fallen por mirrors caídos**.
3. Refresca el keyring (causa #1 de errores tras meses sin actualizar).
4. Ejecuta `pacman -Syu` y muestra todo el progreso en una ventana en directo.
5. Limpia caché de paquetes vieja (`paccache -rk3`).

---

## Instalación rápida (una sola línea)

Abre **Konsole** (o cualquier terminal) y pega:

```bash
curl -fsSL https://raw.githubusercontent.com/thebrokenbrain/cachyos-updater/main/install.sh | bash
```

Tras eso aparecerá **CachyOS Updater** en el menú de aplicaciones. Pinchar y olvidarse.

> Si prefieres no usar el script, descarga el `.pkg.tar.zst` de la [última release](https://github.com/thebrokenbrain/cachyos-updater/releases/latest) e instálalo con:
>
> ```bash
> sudo pacman -U cachyos-updater-*.pkg.tar.zst
> ```

---

## Cómo se usa

<div align="center">

```
┌──────────────────────────────────────────────────┐
│  🐱  CachyOS Updater                             │
│      Mantén tu sistema al día con un solo clic.  │
│  ────────────────────────────────────────────    │
│                                                  │
│   ┌──────────────────────────────────────────┐   │
│   │       Actualizar mi sistema              │   │
│   └──────────────────────────────────────────┘   │
│                                                  │
│   Listo para actualizar.                         │
│   ┌────────────────────────────────────────────┐ │
│   │ resolving dependencies...                  │ │
│   │ downloading linux-cachyos (6.15...)        │ │
│   │ ...                                        │ │
│   └────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────┘
```

</div>

1. Abrir **CachyOS Updater** desde el menú.
2. Pulsar **Actualizar mi sistema**.
3. Escribir la contraseña en el diálogo que aparece.
4. Esperar.

---

## Compilar desde el código

```bash
# Dependencias
sudo pacman -S --needed base-devel cmake ninja qt6-base qt6-tools

# Build
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# O directamente, generar paquete pacman
makepkg -f
```

El binario queda en `build/cachyos-updater`. El paquete `.pkg.tar.zst` se genera al lado del `PKGBUILD`.

---

## Estructura del proyecto

```
├── src/                       # Código C++/Qt6 (GUI)
│   ├── main.cpp
│   ├── mainwindow.{h,cpp}     # Ventana, botón, panel de log
│   └── updater.{h,cpp}        # Lanza el helper vía pkexec
├── packaging/
│   ├── cachyos-updater-helper.sh   # El script que corre como root
│   ├── cachyos-updater.desktop     # Entrada del menú
│   ├── cachyos-updater.svg         # Icono / logo
│   └── org.cachyos.updater.policy  # Política polkit
├── resources/resources.qrc    # Recursos embebidos en el binario
├── CMakeLists.txt
├── PKGBUILD                   # Receta makepkg
├── install.sh                 # Instalador one-liner
└── .github/workflows/         # CI: build + release en Arch
```

---

## Mirrors y conectividad

El helper (`packaging/cachyos-updater-helper.sh`) siempre intenta:

| Paso | Herramienta | Si falla |
|------|-------------|----------|
| Refresh de mirrors **CachyOS** | `cachyos-rate-mirrors` | Avisa y continúa |
| Refresh de mirrors **Arch** | `reflector` | Avisa y continúa |
| Sincronizar BD | `pacman -Sy` | Reintenta con `-Syy` |
| Actualizar paquetes | `pacman -Su` | Reintenta tras re-rate de mirrors |

Así, aunque algún mirror esté caído, **la actualización se recupera sola** sin que el usuario tenga que hacer nada.

---

## Licencia

[MIT](LICENSE). Hazlo tuyo.
