/* Verify an actual native module against its source DOL. Not a boot test. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#define RTLD_NOW 0
#define RTLD_LOCAL 0
static void *dlopen(const char *p, int flags) { (void)flags; return (void *)LoadLibraryA(p); }
static void *dlsym(void *h, const char *n) { return (void *)GetProcAddress((HMODULE)h, n); }
static void dlclose(void *h) { FreeLibrary((HMODULE)h); }
static const char *dlerror(void) { return "Windows loader failed; verify module architecture and installed runtime dependencies"; }
#else
#include <dlfcn.h>
#endif
#include "StaticRecompABI.h"

static uint32_t be32(const unsigned char *p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}
static void fail(const char *message) { fprintf(stderr, "FAIL: %s\n",message); exit(1); }
static unsigned char *read_all(const char *path, size_t *size_out) {
    FILE *f=fopen(path,"rb");
    unsigned char *data;
    long measured;
    if(!f) return NULL;
    if(fseek(f,0,SEEK_END)) { fclose(f); return NULL; }
    measured=ftell(f);
    if(measured<0 || fseek(f,0,SEEK_SET)) { fclose(f); return NULL; }
    data=(unsigned char*)malloc((size_t)measured);
    if(!data) { fclose(f); return NULL; }
    if(fread(data,1,(size_t)measured,f)!=(size_t)measured) {
        free(data); fclose(f); return NULL;
    }
    fclose(f);
    *size_out=(size_t)measured;
    return data;
}
static int join_rel_text_path(char *out, size_t out_size, const char *dir,
                              uint32_t section_index) {
    int wrote;
#if defined(_WIN32)
    const char *slash="\\";
#else
    const char *slash="/";
#endif
    size_t len=strlen(dir);
    if(len && (dir[len-1]=='/' || dir[len-1]=='\\')) slash="";
    wrote=snprintf(out,out_size,"%s%srel_text_section_%u.bin",dir,slash,section_index);
    return wrote>0 && (size_t)wrote<out_size;
}
int main(int argc, char **argv) {
    const char *generated_dir = NULL;
    if (argc!=3 && argc!=4) {
        fprintf(stderr,"usage: verify_module module.so main.dol [generated-dir]\n");
        return 2;
    }
    if (argc==4)
        generated_dir=argv[3];
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
    uint64_t dol_covered=0;
    uint64_t rel_covered=0;
    for(uint32_t i=0;i<m->num_chunk_ranges;i++) {
        uint32_t start=m->chunk_ranges[i].start,end=m->chunk_ranges[i].end;
        if(end<=start || (i && start<m->chunk_ranges[i-1].end)) fail("overlapping/empty chunks");
        const unsigned char *bytes=NULL;
        unsigned char *rel_bytes_owned=NULL;
        size_t rel_size=0;
        for(unsigned j=0;j<7;j++) {
            uint32_t off=be32(dol+4*j),address=be32(dol+0x48+4*j),length=be32(dol+0x90+4*j);
            if(start>=address && (uint64_t)end<=(uint64_t)address+length && (uint64_t)off+length<=size) {
                bytes=dol+off+(start-address);
                dol_covered+=end-start;
            }
        }
        if(!bytes && generated_dir) {
            for(uint32_t module_index=0; module_index<m->num_rel_modules && !bytes; ++module_index) {
                const StaticRecompRelModule *rel=&m->rel_modules[module_index];
                for(uint32_t section_index=0; section_index<rel->num_sections && !bytes; ++section_index) {
                    const StaticRecompRelSection *section=&rel->sections[section_index];
                    uint64_t section_end=(uint64_t)section->linked_start+section->size;
                    if(start>=section->linked_start && (uint64_t)end<=section_end) {
                        char path[1200];
                        if(!join_rel_text_path(path,sizeof(path),generated_dir,section->section_index))
                            fail("REL text path too long");
                        rel_bytes_owned=read_all(path,&rel_size);
                        if(!rel_bytes_owned) fail("REL text open");
                        if(rel_size<section->size) fail("REL text too small");
                        bytes=rel_bytes_owned+(start-section->linked_start);
                        rel_covered+=end-start;
                    }
                }
            }
        }
        if(!bytes) fail("chunk outside original DOL/REL text");
        uint64_t hash=UINT64_C(0xcbf29ce484222325);
        for(uint32_t j=0;j<end-start;j++) hash=(hash^bytes[j])*UINT64_C(0x100000001b3);
        if(hash!=m->chunk_hashes[i]) fail("chunk hash mismatch");
        covered+=end-start;
        free(rel_bytes_owned);
    }
    uint64_t expected=0; for(unsigned j=0;j<7;j++) expected+=be32(dol+0x90+4*j);
    if(m->num_rel_modules==0 && covered!=expected) fail("incomplete text tiling");
    if(m->num_rel_modules!=0 && dol_covered!=expected) fail("incomplete DOL text tiling");
    CPUState state={0}; state.pc=0xdeadbeef;
    if(m->dispatch(&state,state.pc)!=0 || state.pc!=0xdeadbeef) fail("uncovered dispatch changed state");
    printf("{\n  \"status\": \"PASS\",\n  \"game_id\": \"%.6s\",\n  \"module_abi\": %u,\n  \"cpu_abi\": %u,\n  \"cpu_state_size\": %u,\n  \"entry_point\": %u,\n  \"code_ranges\": %u,\n  \"verified_chunk_hashes\": %u,\n  \"covered_text_bytes\": %llu,\n  \"covered_rel_text_bytes\": %llu,\n  \"rel_modules\": %u,\n  \"uncovered_dispatch_test\": true,\n  \"game_boot_test\": false\n}\n",m->game_id,m->abi_version,m->cpu_abi_version,m->cpu_state_size,m->entry_point,m->num_code_ranges,m->num_chunk_ranges,(unsigned long long)covered,(unsigned long long)rel_covered,m->num_rel_modules);
    dlclose(lib); free(dol); return 0;
}
