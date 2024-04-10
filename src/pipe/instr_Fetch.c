/**************************************************************************
 * C S 429 system emulator
 * 
 * instr_Fetch.c - Fetch stage of instruction processing pipeline.
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

extern uint64_t F_PC;

/*
 * Select PC logic.
 * STUDENT TO-DO:
 * Write the next PC to *current_PC.
 */

static comb_logic_t 
select_PC(uint64_t pred_PC,                                       // The predicted PC
          opcode_t D_opcode, uint64_t val_a, uint64_t D_seq_succ, // Possible correction from RET
          opcode_t M_opcode, bool M_cond_val, uint64_t seq_succ,  // Possible correction from B.cond
          uint64_t *current_PC) {
    /* 
     * Students: Please leave this code
     * at the top of this function. 
     * You may modify below it. 
     */
    if (D_opcode == OP_RET && val_a == RET_FROM_MAIN_ADDR) {
        *current_PC = 0; // PC can't be 0 normally.
        return;
    }
    if (M_opcode == OP_B_COND && M_cond_val) {
        *current_PC = seq_succ; // This should be the correct sequential successor address
        return;
    }

    if (D_opcode == OP_RET) {
        *current_PC = val_a; // The return address is in val_a
        return;
    }

    *current_PC = pred_PC;
}

/*
 * Predict PC logic. Conditional branches are predicted taken.
 * STUDENT TO-DO:
 * Write the predicted next PC to *predicted_PC
 * and the next sequential pc to *seq_succ.
 */

static comb_logic_t 
predict_PC(uint64_t current_PC, uint32_t insnbits, opcode_t op, 
           uint64_t *predicted_PC, uint64_t *seq_succ) {
    /* 
     * Students: Please leave this code
     * at the top of this function. 
     * You may modify below it. 
     */
    if (!current_PC) {
        return; // We use this to generate a halt instruction.
    }
    *seq_succ = current_PC + 4;
    int64_t offset = 0;
    switch (op) {
        case OP_B:
        case OP_BL:
            offset = bitfield_s64(insnbits, 0, 26);
            *predicted_PC = current_PC + offset;
            break;
        case OP_B_COND:
            offset = bitfield_s64(insnbits, 5, 19);
            *predicted_PC = current_PC + offset;
            break;
                
        default:
            *predicted_PC = *seq_succ;
            break;
    }
}

/*
 * Helper function to recognize the aliased instructions:
 * LSL, LSR, CMP, and TST. We do this only to simplify the 
 * implementations of the shift operations (rather than having
 * to implement UBFM in full).
 * STUDENT TO-DO
 */

static
void fix_instr_aliases(uint32_t insnbits, opcode_t *op) {
    unsigned sf = (insnbits >> 31) & 1;
    unsigned immr = (insnbits >> 16) & 0x3F;
    unsigned imms = (insnbits >> 10) & 0x3F;
    unsigned Rd = insnbits & 0x1F;
    unsigned N = (insnbits >> 22) & 1;
    
    // ChArm-v2 only uses x registers 
    if (sf == 1) {
        if (*op == OP_UBFM) {
            if (N == 1) {
                unsigned shift_amount = 63 - imms;
                if (imms != 0b111111 && ((imms + 1) & 0x3F) == immr) {
                    *op = OP_LSL;
                } else if (imms == 0b111111 && ((shift_amount & 0x3F) == immr)) {
                    *op = OP_LSR;
                }
            }
        }
        else if (*op == OP_CMP_RR && Rd == 0b11111) {
            *op = OP_SUBS_RR;
        }
        else if (*op == OP_TST_RR && Rd == 0b11111) {
            *op = OP_ANDS_RR;
        }
    }
}

/*
 * Fetch stage logic.
 * STUDENT TO-DO:
 * Implement the fetch stage.
 * 
 * Use in as the input pipeline register,
 * and update the out pipeline register as output.
 * Additionally, update F_PC for the next
 * cycle's predicted PC.
 * 
 * You will also need the following helper functions:
 * select_pc, predict_pc, and imem.
 */

comb_logic_t fetch_instr(f_instr_impl_t *in, d_instr_impl_t *out) {
    bool imem_err = 0;
    uint64_t current_PC;
    select_PC(in->pred_PC, X_out->op, X_out->val_a, X_out->seq_succ_PC,
              M_out->op, M_out->cond_holds, M_out->seq_succ_PC, 
              &current_PC);
    /* 
     * Students: This case is for generating HLT instructions
     * to stop the pipeline. Only write your code in the **else** case. 
     */
    if (!current_PC || F_in->status == STAT_HLT) {
        out->insnbits = 0xD4400000U;
        out->op = OP_HLT;
        out->print_op = OP_HLT;
        imem_err = false;
    }
    else {
        // fetch the instruction bits for the current PC from the instruction memory
        uint32_t insnbits;
        bool imem_err;
        imem(current_PC, &insnbits, &imem_err);
        if (!imem_err) {
            const uint32_t OPCODE_MASK = 0b11;  
            const uint32_t OPCODE_SHIFT = 29;  
            opcode_t opcode = (insnbits >> OPCODE_SHIFT) & OPCODE_MASK;

            fix_instr_aliases(insnbits, &opcode);

            uint64_t predicted_PC;
            uint64_t seq_succ;
            predict_PC(current_PC, insnbits, opcode, &predicted_PC, &seq_succ);

            // update the output pipeline register with the fetched instruction
            out->insnbits = insnbits;
            out->op = opcode;  
            out->print_op = opcode;  
            out->this_PC = current_PC;  
            out->seq_succ_PC = seq_succ;  
            out->status = STAT_AOK;
            in->pred_PC = predicted_PC;  
        } else {
            out->status = STAT_INS;  
            out->op = OP_ERROR;      
            out->print_op = OP_ERROR;
        }
    }

    if (imem_err || out->op == OP_ERROR) {
        in->status = STAT_INS;
        F_in->status = in->status;
    }
    else if (out->op == OP_HLT) {
        in->status = STAT_HLT;
        F_in->status = in->status;
    }
    else {
        in->status = STAT_AOK;
    }
    out->status = in->status;

    return;
}
