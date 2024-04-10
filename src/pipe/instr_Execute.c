/**************************************************************************
 * C S 429 system emulator
 * 
 * instr_Execute.c - Execute stage of instruction processing pipeline.
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
#include "instr_Decode.c"

extern machine_t guest;
extern mem_status_t dmem_status;

extern comb_logic_t copy_m_ctl_sigs(m_ctl_sigs_t *, m_ctl_sigs_t *);
extern comb_logic_t copy_w_ctl_sigs(w_ctl_sigs_t *, w_ctl_sigs_t *);

/*
 * Execute stage logic.
 * STUDENT TO-DO:
 * Implement the execute stage.
 * 
 * Use in as the input pipeline register,
 * and update the out pipeline register as output.
 * 
 * You will need the following helper functions:
 * copy_m_ctl_signals, copy_w_ctl_signals, and alu.
 */

comb_logic_t execute_instr(x_instr_impl_t *in, m_instr_impl_t *out) {

    uint8_t nzcv = 0;

    //calling generate from decode - is this enough? updates x_sigs to be correct for call to alu
    d_ctl_sigs_t local_D_sigs;
    generate_DXMW_control(in->op, &local_D_sigs, &in->X_sigs, &in->M_sigs, &in->W_sigs);
    
    uint64_t aluOperandB = in->X_sigs.valb_sel ? in->val_b : in->val_imm; 
    bool *condVal;
    alu(in->val_a, aluOperandB, in->val_hw, in->ALU_op, in->X_sigs.set_CC, in->cond, out->val_ex, &condVal, &nzcv);

    // Copy control signals for the next pipeline stages.
    copy_m_ctl_sigs(&out->M_sigs, &in->M_sigs);
    copy_w_ctl_sigs(&out->W_sigs, &in->W_sigs);

    //preparing data to be sent to memory
    out->op = in->op;
    out->print_op = in->print_op;
    out->status = in->status;
    out->seq_succ_PC = in->seq_succ_PC;
    out->dst = in->dst;
    
    return;
}
