#define max_len 256
uint64_t ls_source[max_len/64] = {0};
uint64_t ls_res[max_len/64] = {0};
uint64_t ls_ldst = insn.vectorls_ldst();
uint64_t ls_mode = insn.vectorls_mode();
uint64_t ls_base = READ_REG(insn.vectorls_base());
uint64_t ls_offset= READ_REG(insn.vectorls_offset());
uint64_t ls_elen = insn.vectorls_elen();
uint64_t ls_xlen= insn.vectorls_xlen();

//address
uint64_t addr = ls_base;
if(ls_mode == 0){
    addr = ls_base + ls_offset;
}

//step
uint64_t step = ls_offset;
if(ls_mode == 0){
    step = 1 <<  ls_elen;
}

//times
uint64_t total_times = 1 << (3+ls_xlen-ls_elen);

if(ls_ldst == 1){ //store

    //prepare ls_source
    for(uint64_t ls_index = 0;ls_index<max_len/64;++ls_index){
        ls_source[ls_index] = READ_REG((insn.rd()+ls_index)%32);
    }

    //store to mem
    for(uint64_t ls_index = 0;ls_index<total_times;++ls_index){
        uint64_t data = (ls_source[ls_index/(64/(1 << (3+ls_elen)))] >> ((1 << (3+ls_elen)) * (ls_index%(64/(1 << (3+ls_elen))))));
        if(ls_elen == 0){
            MMU.store_uint8(addr + ls_index*step,data);
        }else if(ls_elen == 1){
            MMU.store_uint16(addr + ls_index*step,data);
        }else if(ls_elen == 2){
            MMU.store_uint32(addr + ls_index*step,data);
        }else{
            MMU.store_uint64(addr + ls_index*step,data);
        }
    }

}else{ //load

    //load from mem
    for(uint64_t ls_index = 0;ls_index<total_times;++ls_index){
        uint64_t data = 0;
        if(ls_elen == 0){
            data = MMU.load_uint8(addr + ls_index*step);
        }else if(ls_elen == 1){
            data = MMU.load_uint16(addr + ls_index*step);
        }else if(ls_elen == 2){
            data = MMU.load_uint32(addr + ls_index*step);
        }else{
            data = MMU.load_uint64(addr + ls_index*step);
        }
        ls_res[ls_index/(64/(1<<(3+ls_elen)))] = ls_res[ls_index/(64/(1<<(3+ls_elen)))] | (data << ((1<<(3+ls_elen)) * (ls_index%(64/(1<<(3+ls_elen))))));

    }
    //writeback to gpr
    for(uint64_t ls_index = 0;ls_index < (1<< ls_xlen);++ls_index){
        WRITE_REG((insn.vectorls_rd()+ls_index)%32,ls_res[ls_index]);
    }

}