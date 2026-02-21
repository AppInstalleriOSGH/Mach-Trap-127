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

## Usage

Mach-Trap-127 exposes a single custom syscall via **Mach trap 127**.  
All interaction happens through `issue_command`, which routes requests to the kernel payload.

### `issue_command` trap stub

```asm
_issue_command:
    mov x16, -127
    svc 0x80
    ret
```

```c
uint64_t issue_command(uint64_t command, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6, uint64_t arg7, uint64_t arg8);
```

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
