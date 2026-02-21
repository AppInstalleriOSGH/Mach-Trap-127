.align 4

.pool
.set gKernelSlide, 0x4141414141414141
.set copyin_addr, 0x4242424242424242
.set copyout_addr, 0x4343434343434343
.set current_task_addr, 0x4444444444444444
.set get_task_pmap_addr, 0x4545454545454545
.set pmap_find_phys_addr, 0x4646464646464646
.set bcopy_phys_addr, 0x4747474747474747

.global _start, _getKernelSlide, _copyin, _copyout, _current_task, _get_task_pmap, _pmap_find_phys, _bcopy_phys

_start:
    b _c_start

_getKernelSlide:
    ldr x0, =gKernelSlide
    ret

_copyin:
    ldr x8, =copyin_addr
    br x8

_copyout:
    ldr x8, =copyout_addr
    br x8

_current_task:
    ldr x8, =current_task_addr
    br x8
    
_get_task_pmap:
    ldr x8, =get_task_pmap_addr
    br x8

_pmap_find_phys:
    ldr x8, =pmap_find_phys_addr
    br x8

_bcopy_phys:
    ldr x8, =bcopy_phys_addr
    br x8
