#include "memallocator.h"
#include "pagemapper.h"

#define MALLOC_TAG 0x00CAFEF00DBEEF00

struct MemAllocator::FreeEntry {
    uint64 size;
    FreeEntry* next;
};

struct MallocHeader {
    uint64 size;
    uint64 tag;
};

extern MemAllocator::FreeEntry bootheap_start;

MemAllocator::MemAllocator()
{
    bootheap_start.size = 0x4000;
    bootheap_start.next = 0;
    free_list = &bootheap_start;
}

void* MemAllocator::alloc(uint64 size) {
    FreeEntry *entry = free_list;
    FreeEntry *end_entry = 0;
    FreeEntry *prev_entry = 0;
    FreeEntry *end_prev_entry = 0;

    if(size % 16) { // All alloc'd kernel memory is 16-byte aligned (for great justice)
        size += 16 - size%16;
    }

    size += sizeof(MallocHeader);

    while(entry && entry->size < (size+sizeof(MallocHeader))) {
        prev_entry = entry;
        if(((uint64)entry) + entry->size == heap_end) {
            end_prev_entry = prev_entry;
            end_entry = entry;
        }
        entry = entry->next;
    }

    if(!entry) {
        uint64 required_size = size;
        if(end_entry) {
            required_size -= end_entry->size;
        }
        uint64 num_pages = required_size % 0x1000 ? (required_size >> 12) + 1 : required_size >> 12;
        PageMapper::mapPages(heap_end, num_pages);
        if(end_entry) {
            end_entry->size += 0x1000 * num_pages;
            prev_entry = end_prev_entry;
            entry = end_entry;
        } else {
            entry = (FreeEntry*) heap_end;
            entry->next = free_list;
            entry->size = 0x1000 * num_pages;
            if(prev_entry)
                prev_entry->next = entry;
            else
                free_list = entry;
        }
        heap_end += 0x1000 * num_pages;
    }

    if(entry->size == (size + sizeof(MallocHeader))) {
        if(prev_entry)
            prev_entry->next = entry->next;
        else
            free_list = entry->next;
    } else {
        FreeEntry* new_entry = (FreeEntry*)(((uint64)entry) + size);
        new_entry->size = entry->size - size;
        new_entry->next = entry->next;
        if(prev_entry)
            prev_entry->next = new_entry;
        else
            free_list = new_entry;
    }
    MallocHeader* hdr = (MallocHeader*)entry;
    hdr->tag = MALLOC_TAG;
    hdr->size = size;
    return ((char*)entry)+sizeof(MallocHeader);
}

#define MERGE_PREV 1
#define MERGE_NEXT 2
#define MERGE_BOTH 3
#define INSERT_START 4
#define INSERT_END 5
#define INSERT 6

void MemAllocator::dealloc(void* p) {
    MallocHeader* hdr = (MallocHeader*)((char*)p)-sizeof(MallocHeader);
    FreeEntry* hdr_as_entry = (FreeEntry*)hdr;
    if(hdr->tag != MALLOC_TAG) {
        return; // TODO: Raise an error
    }

    uint64 addr = (uint64)hdr;
    FreeEntry *prev_entry = 0;
    FreeEntry *entry = free_list;

    int action = 0;
    while(entry) {
        uint64 freeaddr = (uint64)entry;
        uint64 nextaddr = (uint64)(entry->next);
        uint64 prevaddr = (uint64)prev_entry;
        if(addr + hdr->size == freeaddr && prev_entry && (prevaddr+prev_entry->size) == addr) {
            action = MERGE_BOTH;
            break;
        }
        if(addr + hdr->size == freeaddr) {
            action = MERGE_NEXT;
            break;
        }
        if(prev_entry && (prevaddr+prev_entry->size) == addr) {
            action = MERGE_PREV;
            break;
        }

        if(prev_entry && (prevaddr+prev_entry->size < addr) && (addr + hdr->size) < freeaddr) {
            action = INSERT;
            break;
        }

        if(!prev_entry && (addr + hdr->size) < freeaddr) {
            action = INSERT_START;
            break;
        }
        prev_entry = entry;
        entry = entry->next;
    }

    if(!entry && !prev_entry && !action)
        action = INSERT_START; // we can avoid a special case for INSERT_END if this is the only entry
    if(!entry && !action)
        action = INSERT_END;
        

    switch(action) {
        case MERGE_PREV:
            prev_entry->size += hdr->size;
            break;
        case MERGE_NEXT:
            hdr_as_entry->size += entry->size;
            hdr_as_entry->next = entry->next;
            if(prev_entry)
                prev_entry->next = hdr_as_entry;
            else
                free_list = hdr_as_entry;
            break;
        case MERGE_BOTH:
            prev_entry->size += hdr->size;
            prev_entry->size += entry->size;
            prev_entry->next = entry->next;
            break;
        case INSERT_START:
            hdr_as_entry->next = free_list;
            free_list = hdr_as_entry;
            break;
        case INSERT_END:
            hdr_as_entry->next = 0;
            prev_entry->next = hdr_as_entry;
            break;
        case INSERT:
            hdr_as_entry->next = entry;
            prev_entry->next = hdr_as_entry;
            break;
    }
}

void MemAllocator::fixup() {
    free_list = (FreeEntry*)PageMapper::fixupHeap(free_list);
    FreeEntry* e = free_list;
    while(e) {
        if(e->next)
            e->next = (FreeEntry*)PageMapper::fixupHeap(e->next);
        e = e->next;
    }
}

MemAllocator mem_allocator;

void *operator new(uint64 size)
{
    return mem_allocator.alloc(size);
}
 
void *operator new[](uint64 size)
{
    return mem_allocator.alloc(size);
}
  
void operator delete(void *p)
{
    mem_allocator.dealloc(p);
}
   
void operator delete[](void *p)
{
    mem_allocator.dealloc(p);
}