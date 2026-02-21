# Mach-Trap-127

Mach Trap 127 patcher module patches **Mach trap 127** and installs a small custom payload that exposes a clean interface for kernel interaction.

It provides an automated, offset-free alternative to `tfp0` for kernel research and development.

Tested devices:
- iPhone 6s Plus on iOS 14.3
- iPod Touch 7th Generation on iOS 15.7.6

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

This patcher exposes a single custom syscall via **Mach trap 127**.  
All interaction happens through `issue_command`, which routes requests to the kernel payload.

### `issue_command` trap stub

```asm
_issue_command:
    mov x16, -127
    svc 0x80
    ret
```

```c
uint64_t issue_command(uint64_t command, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
```

Examples:
```c
issue_command(0x41, 0, 0, 0, 0, 0, 0, 0, 0) == 0x41424344; // check if patched
```
```c
issue_command(0, 0, 0, 0, 0, 0, 0, 0, 0); // get kernel slide
```
```c
kern_return_t kreadbuf(uint64_t addr, void* data, size_t size) {
    return (kern_return_t)issue_command(1, addr, (uint64_t)data, size, 0, 0, 0);
}

kern_return_t kwritebuf(uint64_t addr, void* data, size_t size) {
    return (kern_return_t)issue_command(2, addr, (uint64_t)data, size, 0, 0, 0);
}

int physreadbuf(uint64_t pa, void* data, size_t size) {
    memset(data, 0, size); // fault in page
    return (int)issue_command(3, pa, (uint64_t)data, size, 0, 0, 0);
}

int physwritebuf(uint64_t pa, void* data, size_t size) {
    return (int)issue_command(4, pa, (uint64_t)data, size, 0, 0, 0);
}

uint64_t kcall(uint64_t addr, uint64_t x0, uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4, uint64_t x5, uint64_t x6, uint64_t x7) {
    uint64_t args[8] = {x0, x1, x2, x3, x4, x5, x6, x7};
    uint64_t ret = 0;
    issue_command(5, addr, (uint64_t)&ret, (uint64_t)args, 0, 0, 0);
    return ret;
}

uint64_t get_current_task(void) {
    uint64_t value = 0;
    issue_command(6, (uint64_t)&value, 0, 0, 0, 0, 0);
    return value;
}

uint64_t get_task_pmap(uint64_t task) {
    uint64_t value = 0;
    issue_command(7, task, (uint64_t)&value, 0, 0, 0, 0);
    return value;
}

uint64_t vtophys(uint64_t pmap, uint64_t va) {
    uint64_t value = 0;
    issue_command(8, pmap, va, (uint64_t)&value, 0, 0, 0);
    return value;
}
```

## Credits

- Built on [**PongoOS**](https://github.com/checkra1n/pongoOS) by the checkra1n team 
- This project is independent and **not affiliated** with checkra1n

---

## Warning

⚠️ **Use at your own risk.**

This code:

- Patches the kernel
- Grants full kernel privileges
- Can panic or hang your device
- Is a **work in progress**

Intended for research and educational purposes only.

---
