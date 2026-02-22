#include <pongo.h>
#include <xnu/xnu.h>
#include <mach-o/nlist.h>
#include "payload/payload.h"

extern struct mach_header_64* xnu_header();
extern uint64_t xnu_slide_value(struct mach_header_64* header);
extern uint64_t xnu_rebase_va(uint64_t va);
extern void* xnu_va_to_ptr(uint64_t va);
extern uint64_t xnu_ptr_to_va(void* ptr);
extern void* memmem(const void* big, unsigned long blength, const void* little, unsigned long llength);

uint64_t gKernelBase = 0;
uint64_t gKernelSlide = 0;
uint64_t gKernelTextExec = 0;
size_t gKernelTextExecSize = 0;
uint64_t gKernelDataConstConst = 0;
size_t gKernelDataConstConstSize = 0;
char* gKernelSymbolStringTable = 0;
struct nlist_64* gKernelSymbolTable = 0;
struct symtab_command* gKernelSymbolTabCommand = 0;

void krw_init(void) {
    gKernelSlide = xnu_slide_value(xnu_header());
    gKernelBase = 0xfffffff007004000 + gKernelSlide;
}

void kreadbuf(uint64_t addr, void* data, size_t size) {
    void* ptr = xnu_va_to_ptr(xnu_rebase_va(addr));
    memcpy(data, ptr, size);
}

void kwritebuf(uint64_t addr, void* data, size_t size) {
    void* ptr = xnu_va_to_ptr(xnu_rebase_va(addr));
    memcpy(ptr, data, size);
}

uint64_t kread64(uint64_t addr) {
    uint64_t value = 0;
    kreadbuf(addr, &value, sizeof(value));
    return value;
}

uint32_t kread32(uint64_t addr) {
    uint32_t value = 0;
    kreadbuf(addr, &value, sizeof(value));
    return value;
}

void kwrite64(uint64_t addr, uint64_t value) {
    kwritebuf(addr, &value, sizeof(value));
}

void kwrite32(uint64_t addr, uint32_t value) {
    kwritebuf(addr, &value, sizeof(value));
}

void kwrite8(uint64_t addr, uint8_t value) {
    kwritebuf(addr, &value, sizeof(value));
}

void kernel_info_init(void) {
    struct mach_header_64* header = xnu_header();
    struct load_command* command = (void*)header + sizeof(struct mach_header_64);
    struct segment_command_64* linkedit = 0;
    for (int i = 0; i < header->ncmds; i++) {
        if (command->cmd == LC_SEGMENT_64) {
            struct segment_command_64* segment = (void*)command;
            if (strcmp(segment->segname, "__LINKEDIT") == 0) {
                linkedit = (void*)command;
            } else if (strcmp(segment->segname, "__TEXT_EXEC") == 0) {
                gKernelTextExec = segment->vmaddr;
                gKernelTextExecSize = segment->vmsize;
            } else if (strcmp(segment->segname, "__DATA_CONST") == 0) {
                struct section_64* section = (void*)segment + sizeof(struct segment_command_64);
                for (int i = 0; i < segment->nsects; i++) {
                    if (strcmp(section->sectname, "__const") == 0) {
                        gKernelDataConstConst = section->addr;
                        gKernelDataConstConstSize = section->size;
                    }
                    section = (void*)section + sizeof(struct section_64);
                }
            }
        } else if (command->cmd == LC_SYMTAB) {
            gKernelSymbolTabCommand = (void*)command;
        }
        command = (void*)command + command->cmdsize;
    }
    uint64_t sym_str_table_va = linkedit->vmaddr - linkedit->fileoff + gKernelSymbolTabCommand->stroff;
    uint64_t sym_table_va = linkedit->vmaddr - linkedit->fileoff + gKernelSymbolTabCommand->symoff;
    gKernelSymbolStringTable = xnu_va_to_ptr(xnu_rebase_va(sym_str_table_va));
    gKernelSymbolTable = xnu_va_to_ptr(xnu_rebase_va(sym_table_va));
    return;
}

uint64_t kernel_find_symbol(const char* name) {
    for (int i = 0; i < gKernelSymbolTabCommand->nsyms; i++) if (strcmp(&gKernelSymbolStringTable[gKernelSymbolTable[i].n_un.n_strx], name) == 0) return gKernelSymbolTable[i].n_value;
    return 0;
}

int is_mach_traps(void* data, size_t* trap_size) {
    uint64_t kern_invalid = *(uint64_t*)(data + 8);
    if (kern_invalid < gKernelTextExec || kern_invalid >= gKernelTextExec + gKernelTextExecSize) return 0;
    int valid = 1;
    size_t size = 32;
    for (int i = 0; i < 10; i++) {
        uint64_t trap_entry = (uint64_t)data + (i * 32);
        if (*(uint64_t*)(trap_entry + 8) != kern_invalid || *(uint64_t*)trap_entry != 0 || *(uint64_t*)(trap_entry + 16) != 0 || *(uint64_t*)(trap_entry + 24) != 0) {
            valid = 0;
            break;
        }
    }
    if (valid == 0) {
        valid = 1;
        size = 24;
        for (int i = 0; i < 10; i++) {
            uint64_t trap_entry = (uint64_t)data + (i * 24);
            if (*(uint64_t*)(trap_entry + 8) != kern_invalid || *(uint64_t*)trap_entry != 0 || *(uint64_t*)(trap_entry + 16) != 0) {
                valid = 0;
                break;
            }
        }
    }
    if (valid == 0) return 0;
    *trap_size = size;
    return 1;
}

uint64_t find_mach_traps(size_t* trap_size) {
    void* start = xnu_va_to_ptr(xnu_rebase_va(gKernelDataConstConst));
    void* end = start + gKernelDataConstConstSize;
    void* data = start;
    while (data < end) {
        size_t size = 0;
        if (is_mach_traps(data, &size)) {
            *trap_size = size;
            return gKernelDataConstConst + data - start;
        }
        data += 8;
    }
    return 0;
}

void trap_patch() {
    krw_init();
    printf("kernel base: 0x%llx\n", gKernelBase);
    printf("kernel slide: 0x%llx\n", gKernelSlide);
    printf("magic: 0x%x\n", kread32(gKernelBase));
        
    // get kernel info
    kernel_info_init();
    size_t trap_size = 0;
    uint64_t mach_traps = find_mach_traps(&trap_size);
    uint64_t copyin = kernel_find_symbol("_copyin");
    uint64_t copyout = kernel_find_symbol("_copyout");
    uint64_t current_task = kernel_find_symbol("_current_task");
    uint64_t get_task_pmap = kernel_find_symbol("_get_task_pmap");
    uint64_t pmap_find_phys = kernel_find_symbol("_pmap_find_phys");
    uint64_t bcopy_phys = kernel_find_symbol("_bcopy_phys");
    printf("mach_traps: 0x%llx\n", mach_traps);
    printf("text_exec_addr: 0x%llx\n", gKernelTextExec);
    printf("copyin: 0x%llx\n", copyin);
    printf("copyout: 0x%llx\n", copyout);
    printf("current_task: 0x%llx\n", current_task);
    printf("get_task_pmap: 0x%llx\n", get_task_pmap);
    printf("pmap_find_phys: 0x%llx\n", pmap_find_phys);
    printf("bcopy_phys: 0x%llx\n", bcopy_phys);
    
    // write payload
    void* payload_symbols = memmem(payload, sizeof(payload), (uint64_t[]){0x4141414141414141}, 8);
    if (payload_symbols == 0) return;
    uint64_t payload_symbols_offset = payload_symbols - (void*)payload;
    uint64_t rx = gKernelTextExec + 0x1000;
    kwritebuf(rx, payload, sizeof(payload));
    uint64_t symbols[] = {
        gKernelSlide,
        copyin,
        copyout,
        current_task,
        get_task_pmap,
        pmap_find_phys,
        bcopy_phys
    };
    for (int i = 0; i < sizeof(symbols) / 8; i++) {
        kwrite64(rx + payload_symbols_offset + (i * 8), symbols[i]);
    }

    // write trap
    uint64_t trap_entry = mach_traps + (127 * trap_size);
    kwrite64(trap_entry + 8, rx);
    kwrite64(trap_entry + 16, 0);
    if (trap_size == 32) {
        kwrite64(trap_entry, 7);
        kwrite64(trap_entry + 24, 0);
    } else if (trap_size == 24) {
        kwrite8(trap_entry, 7);
        kwrite8(trap_entry + 1, 0);
    } else {
        return;
    }
    queue_rx_string("bootx\n");
}

void module_entry() {
    command_register("trap_patch", "patches mach trap 127 for kernel r/w + kcall", trap_patch);
}

char* module_name = "trap_patcher";

struct pongo_exports exported_symbols[] = {
    {.name = 0, .value = 0}
};
