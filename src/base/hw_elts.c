/**************************************************************************
 * C S 429 system emulator
 * 
 * Student TODO: AE
 * 
 * hw_elts.c - Module for emulating hardware elements.
 * 
 * Copyright (c) 2022, 2023. 
 * Authors: S. Chatterjee, Z. Leeper. 
 * All rights reserved.
 * May not be used, modified, or copied without permission.
 **************************************************************************/ 

#include <assert.h>
#include "hw_elts.h"
#include "mem.h"
#include "machine.h"
#include "forward.h"
#include "err_handler.h"

extern machine_t guest;

comb_logic_t 
imem(uint64_t imem_addr,
     uint32_t *imem_rval, bool *imem_err) {
    // imem_addr must be in "instruction memory" and a multiple of 4
    *imem_err = (!addr_in_imem(imem_addr) || (imem_addr & 0x3U));
    *imem_rval = (uint32_t) mem_read_I(imem_addr);
}

/*
 * Regfile logic.
 * STUDENT TO-DO:
 * Read from source registers and write to destination registers when enabled.
 */
comb_logic_t
regfile(uint8_t src1, uint8_t src2, uint8_t dst, uint64_t val_w,
        // bool src1_31isSP, bool src2_31isSP, bool dst_31isSP, 
        bool w_enable,
        uint64_t *val_a, uint64_t *val_b) {
}

/*
 * cond_holds logic.
 * STUDENT TO-DO:
 * Determine if the condition is true or false based on the condition code values
 */
static bool 
cond_holds(cond_t cond, uint8_t ccval) {
    return false;
}

/*
 * alu logic.
 * STUDENT TO-DO:
 * Compute the result of a bitwise or mathematial operation (all operations in alu_op_t).
 * Additionally, apply hw or compute condition code values as necessary.
 * Finally, compute condition values with cond_holds.
 */
comb_logic_t 
alu(uint64_t alu_vala, uint64_t alu_valb, uint8_t alu_valhw, alu_op_t ALUop, bool set_CC, cond_t cond, 
    uint64_t *val_e, bool *cond_val, uint8_t *nzcv) {
    uint64_t res = 0xFEEDFACEDEADBEEF;  // To make it easier to detect errors.

}

comb_logic_t 
dmem(uint64_t dmem_addr, uint64_t dmem_wval, bool dmem_read, bool dmem_write, 
     uint64_t *dmem_rval, bool *dmem_err) {
    // dmem_addr must be in "data memory" and a multiple of 8
    *dmem_err = (!addr_in_dmem(dmem_addr) || (dmem_addr & 0x7U));
    if (is_special_addr(dmem_addr)) *dmem_err = false;
    if (dmem_read) *dmem_rval = (uint64_t) mem_read_L(dmem_addr);
    if (dmem_write) mem_write_L(dmem_addr, dmem_wval);
}