/* Verify an actual native module against its source DOL. Not a boot test. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dlfcn.h>
#include "StaticRecompABI.h"

static uint32_t be32(const unsigned char *p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}
static void fail(const char *message) { fprintf(stderr, "FAIL: %s\n",message); exit(1); }
int main(int argc, char **argv) {
    if (argc!=3) { fprintf(stderr,"usage: verify_module module.so main.dol\n"); return 2; }
    FILE *f=fopen(argv[2],"rb"); if(!f) fail("DOL open");
    if(fseek(f,0,SEEK_END)) fail("DOL seek");
    long measured=ftell(f); if(measured<0x100) fail("DOL size"); rewind(f);
    size_t size=(size_t)measured; unsigned char *dol=malloc(size); if(!dol) fail("allocation");
    if(fread(dol,1,size,f)!=size) fail("DOL read");
    fclose(f);
    void *lib=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL); if(!lib) { fprintf(stderr,"%s\n",dlerror()); return 1; }
    StaticRecompGetModuleFn get_module=(StaticRecompGetModuleFn)dlsym(lib,STATICRECOMP_GET_MODULE_SYMBOL);
    if(!get_module) fail("missing module export");
    const StaticRecompModuleDesc *m=get_module(); if(!m) fail("null descriptor");
    if(m->abi_version!=STATICRECOMP_ABI_VERSION || m->cpu_abi_version!=GXRUNTIME_CPU_ABI_VERSION || m->cpu_state_size!=sizeof(CPUState)) fail("ABI mismatch");
    if(memcmp(m->game_id,"RMSE52\0",7)) fail("wrong game ID");
    if(m->entry_point!=be32(dol+0xe0)) fail("entry mismatch");
    if(!m->dispatch || !m->code_ranges || !m->chunk_ranges || !m->chunk_hashes || !m->num_chunk_ranges || m->num_chunk_ranges>1000000) fail("bad descriptor tables");
    uint64_t covered=0;
    for(uint32_t i=0;i<m->num_chunk_ranges;i++) {
        uint32_t start=m->chunk_ranges[i].start,end=m->chunk_ranges[i].end;
        if(end<=start || (i && start<m->chunk_ranges[i-1].end)) fail("overlapping/empty chunks");
        const unsigned char *bytes=NULL;
        for(unsigned j=0;j<7;j++) {
            uint32_t off=be32(dol+4*j),address=be32(dol+0x48+4*j),length=be32(dol+0x90+4*j);
            if(start>=address && (uint64_t)end<=(uint64_t)address+length && (uint64_t)off+length<=size) bytes=dol+off+(start-address);
        }
        if(!bytes) fail("chunk outside original DOL text");
        uint64_t hash=UINT64_C(0xcbf29ce484222325);
        for(uint32_t j=0;j<end-start;j++) hash=(hash^bytes[j])*UINT64_C(0x100000001b3);
        if(hash!=m->chunk_hashes[i]) fail("chunk hash mismatch");
        covered+=end-start;
    }
    uint64_t expected=0; for(unsigned j=0;j<7;j++) expected+=be32(dol+0x90+4*j);
    if(covered!=expected) fail("incomplete text tiling");
    CPUState state={0}; state.pc=0xdeadbeef;
    if(m->dispatch(&state,state.pc)!=0 || state.pc!=0xdeadbeef) fail("uncovered dispatch changed state");
    printf("{\n  \"status\": \"PASS\",\n  \"game_id\": \"%.6s\",\n  \"module_abi\": %u,\n  \"cpu_abi\": %u,\n  \"cpu_state_size\": %u,\n  \"entry_point\": %u,\n  \"code_ranges\": %u,\n  \"verified_chunk_hashes\": %u,\n  \"covered_text_bytes\": %llu,\n  \"rel_modules\": %u,\n  \"uncovered_dispatch_test\": true,\n  \"game_boot_test\": false\n}\n",m->game_id,m->abi_version,m->cpu_abi_version,m->cpu_state_size,m->entry_point,m->num_code_ranges,m->num_chunk_ranges,(unsigned long long)covered,m->num_rel_modules);
    dlclose(lib); free(dol); return 0;
}
