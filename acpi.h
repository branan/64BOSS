#ifndef ACPI_H
#define ACPI_H

#include "ktypes.h"

namespace ACPI {
    struct RSDP {
        char signature[8];
        uint8 checksum;
        char oem_id[6];
        uint8 revision;
        uint32 rsdt_addr;

        uint32 length;
        uint64 xsdt_addr;
        uint8 checksum_extended;
        uint8 reserved[3];
    };

    struct HEADER {
        char signature[4];
        uint32 length;
        uint8 revision;
        uint8 checksum;
        char oem_id[6];
        char oem_table_id[6];
        uint32 oem_revision;
        uint32 creator_id;
        uint32 creator_revision;
    };

    struct RSDT {
        HEADER header;
        uint32 pointers[0];
    };

    struct XSDT {
        HEADER header;
        uint64 pointers[0];
    };
}

#endif // ACPI_H
