#include <sys/regdef.h>
#include <sys/asm.h>

LEAF(__atomic_fetch_add_8)
    .set push
    .set mips64r2
    .set noreorder
    dins a2, a3, 32, 32
1:
    sync
    lld v0, 0(a0)
    daddu v1, v0, a2
    scd v1, 0(a0)
    beqz v1, 1b
    dext v1, v0, 32, 32
    sync
    jr ra
    dext v0, v0, 0, 32
    .set pop
END(__atomic_fetch_add_8)

