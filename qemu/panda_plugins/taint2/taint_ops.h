/* PANDABEGINCOMMENT
 *
 * Authors:
 *  Tim Leek               tleek@ll.mit.edu
 *  Ryan Whelan            rwhelan@ll.mit.edu
 *  Joshua Hodosh          josh.hodosh@ll.mit.edu
 *  Michael Zhivich        mzhivich@ll.mit.edu
 *  Brendan Dolan-Gavitt   brendandg@gatech.edu
 *
 * This work is licensed under the terms of the GNU GPL, version 2.
 * See the COPYING file in the top-level directory.
 *
PANDAENDCOMMENT */

// taint2_memlog
//
// This will replace the dynamic log, since we now need to track values for
// a much shorter period of time. Instead of full-fledged file logging, we're
// just going to use a ring buffer.

// Initialize this to 0.
#define TAINT2_MEMLOG_SIZE 2
typedef struct taint2_memlog {
    uint64_t ring[TAINT2_MEMLOG_SIZE];
    uint64_t idx;
} taint2_memlog;

uint64_t taint2_memlog_pop(uint64_t memlog_ptr);

void taint2_memlog_push(uint64_t memlog_ptr, uint64_t val);

// taint_pop_frame; taint_push_frame
//
// Functions for dealing with function frames. We'll just advance and retract
// the label array pointer to make a new frame.
void taint_push_frame(uint64_t shad_ptr);
void taint_pop_frame(uint64_t shad_ptr);

// Bookkeeping.
void taint_breadcrumb(uint64_t dest_ptr, uint64_t bb_slot);

// Taint operations
//
// These are all the taint operations which we will inline into the LLVM code
// as it JITs.
void taint_copy(
        uint64_t shad_dest_ptr, uint64_t dest,
        uint64_t shad_src_ptr, uint64_t src,
        uint64_t size);

void taint_move(
        uint64_t shad_dest_ptr, uint64_t dest,
        uint64_t shad_src_ptr, uint64_t src,
        uint64_t size);

// Two compute models: parallel and mixed. Parallel for bitwise, mixed otherwise.
// Parallel compute: take labelset vectors [1,2,3] + [4,5,6] -> [14,25,36]
void taint_parallel_compute(
        uint64_t shad_ptr,
        uint64_t dest, uint64_t ignored,
        uint64_t src1, uint64_t src2, uint64_t src_size);

// Mixed compute: [1,2] + [3,4] -> [1234,1234]
// Note that dest_size and src_size can differ.
void taint_mix_compute(
        uint64_t shad_ptr,
        uint64_t dest, uint64_t dest_size,
        uint64_t src1, uint64_t src2, uint64_t src_size);

// Clear taint.
void taint_delete(uint64_t shad_ptr, uint64_t dest, uint64_t size);

// Copy a single value to multiple destinations. (i.e. memset)
void taint_set(
        uint64_t shad_dest_ptr, uint64_t dest, uint64_t dest_size,
        uint64_t shad_src_ptr, uint64_t src);

// Union all labels within here: [1,2,3] -> [123,123,123]
// A mixed compute becomes two mixes followed by a parallel.
void taint_mix(
        uint64_t shad_ptr,
        uint64_t dest, uint64_t dest_size,
        uint64_t src, uint64_t src_size);

// Only generate when signed and dest_size > src_size.
// Otherwise it should just be a copy.
void taint_sext(
        uint64_t shad_ptr,
        uint64_t dest, uint64_t dest_size,
        uint64_t src, uint64_t src_size);

// Takes a NULL-terminated list of (value, select) pairs.
void taint_select(
        uint64_t shad_ptr,
        uint64_t dest, uint64_t size, uint64_t selector,
        ...);