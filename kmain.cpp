#include "multiboot2.h"
#include "earlyprint.h"

#include "memallocator.h"
#include "pageallocator.h"
#include "pagemapper.h"
#include "kutils.h"

#include "acpi.h"

const char* multiboot_cmdline;
const char* multiboot_loader;

struct BootModule {
    char* cmdline;
    uint32 start;
    uint32 end;
};

// Yes, a fixed array. Our heap space is precious at initial boot time.
BootModule boot_modules[16];
uint8 num_boot_modules;

uint64 acpi_rsdt = 0;
uint64 acpi_xsdt = 0;
char acpi_oem[7] = {0,0,0,0,0,0,0};
uint8 acpi_revision;

bool parse_multiboot_tag(const char** list) {
    const char* addr = *list;
    const multiboot_tag* tag = (multiboot_tag*)addr;
    switch(tag->type) {
        case MULTIBOOT_TAG_TYPE_END:
            return false;
        case MULTIBOOT_TAG_TYPE_CMDLINE:
            multiboot_cmdline = strdup(addr+8);
            break;
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
            multiboot_loader = strdup(addr+8);
            break;
        case MULTIBOOT_TAG_TYPE_MODULE: {
                if(num_boot_modules == 16) {
                    earlyprint("Too many modules specified at boot time.");
                    khalt();
                }
                multiboot_tag_module* module = (multiboot_tag_module*)tag;
                boot_modules[num_boot_modules].cmdline = strdup(module->cmdline);
                boot_modules[num_boot_modules].start = module->mod_start;
                boot_modules[num_boot_modules].end = module->mod_end;
                num_boot_modules++;
            }
            break;
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
            break;
        case MULTIBOOT_TAG_TYPE_BOOTDEV:
            break;
        case MULTIBOOT_TAG_TYPE_MMAP: {
                multiboot_tag_mmap* mmap = (multiboot_tag_mmap*)tag;
                unsigned int num_entries = (mmap->size-16)/mmap->entry_size;
                const char* entries = addr+16;
                for(unsigned int i = 0; i < num_entries; i++, entries+=mmap->entry_size) {
                    const multiboot_mmap_entry* entry = (multiboot_mmap_entry*)entries;
                    uint32 type = entry->type;
                    
                    if(type == MULTIBOOT_MEMORY_AVAILABLE)
                        PageAllocator::addPageRange(entry->addr, entry->addr+entry->len);
                    // TODO: Tag ACPI reclaimable ranges to be reclaimed once ACPI has been parsed
                }
            }
            break;
        case MULTIBOOT_TAG_TYPE_VBE:
            break;
        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
            break;
        case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
            break;
        case MULTIBOOT_TAG_TYPE_APM:
            break;
        case MULTIBOOT_TAG_TYPE_EFI32:
            break;
        case MULTIBOOT_TAG_TYPE_EFI64:
            break;
        case MULTIBOOT_TAG_TYPE_SMBIOS:
            break;
        case MULTIBOOT_TAG_TYPE_ACPI_OLD:
        case MULTIBOOT_TAG_TYPE_ACPI_NEW: {
            break;
            multiboot_tag_old_acpi* acpi = (multiboot_tag_old_acpi*)tag;
            ACPI::RSDP* rsdp = (ACPI::RSDP*)acpi->rsdp;
            switch(rsdp->revision) {
                case 1:
                    acpi_xsdt = rsdp->xsdt_addr;
                    break;
                default:
                    acpi_rsdt = rsdp->rsdt_addr;
                    break;
            }
            //memcpy(acpi_oem, rsdp->oem_id, 6);
            acpi_revision = rsdp->revision;
            break;
        }
        case MULTIBOOT_TAG_TYPE_NETWORK:
            break;
        default:
            break;
    }
    *list = addr + tag->size;
    if (tag->size % 8 != 0) {
        *list += 8 - (tag->size % 8);
    }
    return true;
}

extern "C" uint64 start_ctors;
extern "C" uint64 end_ctors;
extern "C" uint64 kernel_start;
extern "C" uint64 kernel_end;

typedef void(*ctor)();

extern "C" void kinit( unsigned long magic, void* mbd ) {
    // Clear the hardware console before doing anything else
    earlycls();

    if ( magic != MULTIBOOT2_BOOTLOADER_MAGIC )
    {
        earlyprint("Not loaded by a multiboot2 compliant loader. Aborting bootup and halting");
        return;
    }

    MemAllocator::init();
    PageAllocator::init();

    num_boot_modules = 0;

    // parse the multiboot structures
    const char* tag = (const char*)mbd;
    const char* multiboot_end = (*(uint32*)mbd) + (const char*)tag;
    tag+=8; // skip the total size info
    while(tag < multiboot_end && parse_multiboot_tag(&tag)) {};

    PageAllocator::reservePageRange((uint64)&kernel_start, (uint64)&kernel_end);

    for(uint8 i = 0; i < num_boot_modules; i++) {
        uint32 start = boot_modules[i].start & 0xfffff000;
        uint32 end = boot_modules[i].end;
        if(end & 0xfff) {
            end += 0x1000;
            end = end & 0xfffff000;
        }
        PageAllocator::reservePageRange(start, end);
    }
    PageMapper::buildDefaultMapping();
}

extern "C" void kmain() {
    //PageMapper::removeBootPages();

    // The command lines for the modules were allocated to the old heap. They need to be
    // moved before we can do anything else.
    for(uint8 i = 0; i < num_boot_modules; i++) {
        boot_modules[i].cmdline = (char*)PageMapper::fixupHeap(boot_modules[i].cmdline);
    }
    multiboot_cmdline = (char*)PageMapper::fixupHeap(multiboot_cmdline);
    multiboot_loader = (char*)PageMapper::fixupHeap(multiboot_loader);

    // print a nice hello message
    // TODO: put this in the console instead of to the earlyprint buffer
    earlyprint("This is 64BOSS");
    if(multiboot_loader && multiboot_loader[0]) {
        earlyprint(" loaded via ");
        earlyprint(multiboot_loader);
    }
    earlyprint("\n");
    if(multiboot_cmdline && multiboot_cmdline[0]) {
        earlyprint("Arguments: ");
        earlyprint(multiboot_cmdline);
        earlyprint("\n");
    }
    
    // At this point, memory allocation is set up, and we have a sane kernel page mapping
    // Remaining core kernel services need to be initialized, then the boot modules can be loaded.

    // static kernel modules are initialized through static constructors
    uint64* ctor_list = &start_ctors;
    while(ctor_list != &end_ctors) {
        ((ctor)*ctor_list)();
        ctor_list++;
    }
    
    // TODO: Parse the kernel command line
    // TODO: Initialize remaining services
    // TODO: load boot-time modules

    // Now that everything's been initialized, it's time to execute the init process.
    // This is either /sbin/init, or whatever executable was specified on the command line
    // TODO: launch init
}
