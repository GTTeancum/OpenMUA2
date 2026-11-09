// SPDX-License-Identifier: GPL-3.0-or-later
//
// OpenMUA2 original-Xbox toolchain smoke test (P0.3 in
// docs/ORIGINAL-XBOX-PORT-PLAN.md). Contains no game code or data.
// Proves the nxdk build path and prints the physical memory budget that a
// real port would have to live inside, so it can be compared with the
// MEM1/MEM2 census from the Windows runtime.

#include <hal/debug.h>
#include <hal/video.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#define PAGE_KIB 4u

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    MM_STATISTICS stats;
    stats.Length = sizeof(stats);
    NTSTATUS status = MmQueryStatistics(&stats);

    while (1) {
        debugClearScreen();
        debugPrint("OpenMUA2 original-xbox smoke test\n\n");
        if (NT_SUCCESS(status)) {
            debugPrint("Total physical: %lu KiB\n", stats.TotalPhysicalPages * PAGE_KIB);
            debugPrint("Available:      %lu KiB\n", stats.AvailablePages * PAGE_KIB);
            debugPrint("Image:          %lu KiB\n", stats.ImagePagesCommitted * PAGE_KIB);
            debugPrint("Pool:           %lu KiB\n", stats.PoolPagesCommitted * PAGE_KIB);
            debugPrint("Stacks:         %lu KiB\n", stats.StackPagesCommitted * PAGE_KIB);
        } else {
            debugPrint("MmQueryStatistics failed: 0x%08lx\n", (unsigned long)status);
        }
        debugPrint("\nWii guest RAM for comparison: 90112 KiB (MEM1+MEM2)\n");
        Sleep(2000);
    }

    return 0;
}
