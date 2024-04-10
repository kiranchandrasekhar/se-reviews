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
        if(src1 < 32){
            *val_a = guest.proc->GPR[src1];
        } else {
            *val_a = 0;
        }
        *val_b = guest.proc->GPR[src2];
        if (w_enable){
            guest.proc->GPR[dst] = val_w;
        }
}

/*
 * cond_holds logic.
 * STUDENT TO-DO:
 * Determine if the condition is true or false based on the condition code values
 */
static bool 
cond_holds(cond_t cond, uint8_t ccval) {
    //ccval = ccval >> 4;
    switch (cond){
        case C_EQ:
            if (GET_ZF(ccval) == 1){
                return true;
                break;
            }
            return false;
            break;
        case C_NE:
            if (GET_ZF(ccval) == 0){
                return true;
                break;
            }
            return false;
            break;
        case C_CS:
            if (GET_CF(ccval) == 1){
                return true;
                break;
            }
            return false;
            break;
        case C_CC:
            if (GET_CF(ccval) == 0){
                return true;
                break;
            }
            return false;
            break;
        case C_MI:
            if (GET_NF(ccval) == 1){
                return true;
                break;
            }
            return false;
            break;
        case C_PL:
            if (GET_NF(ccval) == 0){
                return true;
                break;
            }
            return false;
            break;
        case C_VS:
            if (GET_VF(ccval) == 1){
                return true;
                break;
            }
            return false;
            break;
        case C_VC:
            if (GET_VF(ccval) == 0){
                return true;
                break;
            }
            return false;
            break;
        case C_HI:
            if ( (GET_CF(ccval) == 1) && (GET_ZF(ccval) == 0)){
                return true;
                break;
            }
            return false;
            break;
        case C_LS:
            if (!((GET_CF(ccval) == 1) && (GET_ZF(ccval) == 0))){
                return true;
                break;
            }
            return false;
            break;
        case C_GE:
            if (GET_NF(ccval) == GET_VF(ccval)){
                return true;
                break;
            }
            return false;
            break;
        case C_LT:
            if (GET_NF(ccval) != GET_VF(ccval)){
                return true;
                break;
            }
            return false;
            break;
        case C_GT:
            if ((GET_ZF(ccval) == 0) && (GET_NF(ccval) == GET_VF(ccval))){
                return true;
                break;
            }
            return false;
            break;
        case C_LE:
            if (!((GET_ZF(ccval) == 0) && (GET_NF(ccval) == GET_VF(ccval)))){
                return true;
                break;
            }
            return false;
            break;
        default:
            return true;
            break;
    }
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

    uint8_t n = 0;
    uint8_t c = 0;
    uint8_t z = 0;
    uint8_t v = 0;

    uint64_t vala_msb;
    uint64_t valb_msb;
    uint64_t res_msb;

    switch(ALUop){
        case PLUS_OP:
            res = alu_vala + (alu_valb << alu_valhw);
            if (res < alu_vala || res < alu_valb){
                c = 1;
            }
            valb_msb = (alu_valb >> 63) && 0x1;
            vala_msb = (alu_vala >> 63) && 0x1;
            res_msb = (res >> 63) && 0x1;
            if ((vala_msb == 0 && valb_msb == 0 && res_msb == 1) || (vala_msb == 1 && valb_msb == 1 && res_msb == 0)){
                v = 1;
            }
            break;
        case MINUS_OP:
            res = alu_vala - (alu_valb << alu_valhw);
            vala_msb = (alu_vala >> 63) && 0x1;
            res_msb = (res >> 63) && 0x1;
            if (alu_vala >= alu_valb){
                c = 1;
            }else{
                c = 0;
            }
            valb_msb = (alu_valb >> 63) && 0x1;
            if ((vala_msb == 0 && valb_msb == 1 && res_msb == 0 ) || (vala_msb == 1 && valb_msb == 0 && res_msb == 1)){
                v = 1;
            }
            break;
        case INV_OP:
            res = alu_vala | (~alu_valb);
            break;
        case OR_OP:
            res = alu_vala | alu_valb;
            break;
        case EOR_OP:
            res = alu_vala ^ alu_valb;
            break;
        case AND_OP:
            res = alu_vala & alu_valb;
            break;
        case MOV_OP:
            res = alu_vala | (alu_valb << alu_valhw);
            break;
        case LSL_OP:
            res = alu_vala << (alu_valb & 0x3FUL);
            break;
        case LSR_OP:
            res = alu_vala >> (alu_valb & 0x3FUL);
            break;
        case ASR_OP:
            res = alu_vala >> (alu_valb & 0x3FUL);
            break;
        case PASS_A_OP:
            res = alu_vala;
            break;
        default:
            break;
    }
    // valb_msb = (alu_valb >> 63) && 0x1;
    // vala_msb = (alu_vala >> 63) && 0x1;
    // res_msb = (res >> 63) && 0x1;
    // if ((vala_msb == 0 && valb_msb == 1 && res_msb == 0 ) || (vala_msb == 1 && valb_msb == 0 && res_msb == 1)){
    //      v = 1;
    // }
    if (res == 0){
        z = 1;
    }if (res_msb == 1){
        n = 1;
    }
    if (set_CC){
        *nzcv = PACK_CC(n, z, c, v);
    }
    *cond_val = cond_holds(cond, *nzcv);
    *val_e = res;
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