/**************************************************************************
 * C S 429 system emulator
 * 
 * instr_Memory.c - Memory stage of instruction processing pipeline.
 **************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "err_handler.h"
#include "instr.h"
#include "instr_pipeline.h"
#include "machine.h"
#include "hw_elts.h"

extern machine_t guest;
extern mem_status_t dmem_status;

extern comb_logic_t copy_w_ctl_sigs(w_ctl_sigs_t *, w_ctl_sigs_t *);

/*
 * Memory stage logic.
 * STUDENT TO-DO:
 * Implement the memory stage.
 * 
 * Use in as the input pipeline register,
 * and update the out pipeline register as output.
 * 
 * You will need the following helper functions:
 * copy_w_ctl_signals and dmem.
 */

comb_logic_t memory_instr(m_instr_impl_t *in, w_instr_impl_t *out) {
    
    bool *memError;

    copy_w_ctl_sigs(&out->W_sigs, &in->W_sigs);
    if (in->M_sigs.dmem_read) {
        dmem(in->val_ex, 0, true, false, &out->val_mem, &memError);
    }
    else if (in->M_sigs.dmem_write) {
        dmem(in->val_ex, in->val_b, false, true, NULL, &memError);
    }

    //check for memory errors and update status 
    if (memError && (in->op == OP_LDUR || in->op == OP_STUR)){
        dmem_status = ERROR;
        out->status = STAT_ADR;
    } else{
        out->status = in->status;
    }

    out->op = in->op;
    out->print_op = in->print_op;
    out->dst = in->dst;
    out->W_sigs = in->W_sigs;
    out->val_ex = in->val_ex;
    out->val_b = in->val_b;
    


    return;
}
