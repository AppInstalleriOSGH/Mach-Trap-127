# Mach-Trap-127

Mach-Trap-127 patches **Mach trap 127** and installs a small custom payload that exposes a clean interface for kernel interaction.

It provides an automated, offset-free alternative to `tfp0` for kernel research and development.

---

## Features

- Kernel slide retrieval
- Kernel read/write (`kreadbuf` / `kwritebuf`)
- Physical memory read/write (`physreadbuf` / `physwritebuf`)
- `kcall8` (call arbitrary kernel functions)
- `current_task`
- Task → pmap
- `vtophys` via `pmap_find_phys`

---

## Credits

- Built on **PongoOS** by the [checkra1n team](https://github.com/checkra1n/pongoOS)  
- This project is independent and **not affiliated** with checkra1n

---

## Warning

⚠️ **Use at your own risk.**

This code:

- Patches the kernel
- Grants full kernel privileges
- Can panic or hang your device

Intended for research and educational purposes only.

---
