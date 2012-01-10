#include "pageallocator.h"

#include "pagemapper.h"

#include "earlyprint.h"

namespace PageAllocator {
struct Entry {
    uint64 start;
    uint64 end;
    Entry* next;
};

Entry* head;
}

void PageAllocator::init() {
    head = 0;
}
 
uint64 PageAllocator::requestPage() {
    uint64 result = head->start;
    head->start += 0x1000;
    if(head->start == head->end) {
        Entry* next = head->next;
        delete head;
        head = next;
    }
    return result;
}


void PageAllocator::addPageRange(uint64 start, uint64 end) {
    Entry *e = new Entry;
    e->start = start;
    e->end   = end;

    Entry *entry = head;
    Entry *prev_entry = 0;

    while(entry) {
        if(entry->start == end) {
            entry->start = start;
            return;
        }
        if(entry->end == start) {
            entry->end = end;
            return;
        }
        if(entry->start > end) {
            // This entry goes after us. We need to insert ourselves here
            e->next = entry;
            if(prev_entry)
                prev_entry->next = e;
            else
                head = e;
            return;
        }
        prev_entry = entry;
        entry = entry->next;
    }
    // We've reached the end, and every entry came before us
    // stick ourselves at the end of the list
    e->next = 0;
    if(prev_entry)
        prev_entry->next = e;
    else
        head = e;
}

void PageAllocator::reservePageRange(uint64 start, uint64 end) {
    Entry *prev_entry = 0;
    Entry *entry = head;
    while(entry) {
        if(entry->start < start) {
            if(entry->end < end && entry->end > start) {
                // The start of our range is contained within this chunk
                // Truncate it and keep looking
                uint64 tmp = entry->end;
                entry->end = start;
                start = tmp;
            }
            if(entry->end > end) {
                // Our entire range is contained inside this entry
                // We split it in two, then return
                Entry* new_entry = new Entry;
                new_entry->start = end;
                new_entry->end = entry->end;
                new_entry->next = entry->next;
                entry->end = start;
                entry->next = new_entry;
                return;
            }
            if(entry->end == end) {
                // We're contained entirely in the tail of this entry.
                // truncate it.
                entry->end = start;
                return;
            }
        } else if(entry->start == start) {
            if(entry->end < end) {
                // This entry covers only part of the range we want to use
                // We blow it away, set our start value to the end of the
                // chunk from this entry, and keep looking.
                if(prev_entry) {
                    prev_entry->next = entry->next;
                } else {
                    head = entry->next;
                }
                start = entry->end;
                entry = entry->next;
                continue;
            } else if (entry->end == end) {
                // This entry covers the entire range we want to use.
                // We blow it away and return
                if(prev_entry) {
                    prev_entry->next = entry->next;
                } else {
                    head = entry->next;
                }
                return;
            } else {
                // This entry covers more than the range we want to use
                // We truncate it and return
                entry->start = end;
                return;
            }
        } else if(entry->start > start) {
            // Because the list of chunks is kept sorted, if we get here
            // we know the start of the reserved chunk is in unmanaged
            // memory... so we push it up to where we actually manage
            // memory and keep going.
            start = entry->start;
            if(start >= end)
                return;
            else
                continue;
        }
        prev_entry = entry;
        entry = entry->next;
    }
    // By the end, any sections of the reserved range not removed from a chunk were
    // never under our control to begin with.
}

void PageAllocator::fixup() {
    head = (Entry*)PageMapper::fixupHeap(head);
    Entry* e = head;
    while(e) {
        if(e->next)
            e->next = (Entry*)PageMapper::fixupHeap(e->next);
        e = e->next;
    }
}

void PageAllocator::printDebugInfo() {
    Entry *e = head;
    while(e) {
        earlyprint(e->start);
        earlyprint(" : ");
        earlyprint(e->end);
        earlyprint("\n");
        e = e->next;
    }
}
