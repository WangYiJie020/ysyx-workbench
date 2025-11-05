#include <stdio.h>
#include <memory.h>
#include <elf.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <debug.h>

#include "elf_tool.h"

#define ensure_frd_one(item) ensure_frd(&(item), 1)

#define ensure_frd(ptr,cnt) do{\
	int _fread_result=fread((ptr),sizeof(*ptr),cnt,fp);\
	Assert(_fread_result==(cnt),"expected %d(siz %d) get %d\n",(int)(cnt),(int)sizeof(*ptr),_fread_result);\
}while(0)

void* ensure_malloc(size_t size){
	void* res=malloc(size);
	Assert(res, "malloc failed with siz %zu",size);
	return res;
}
void* ensure_calloc(size_t nmemb,size_t size){
	void* res=calloc(nmemb, size);
	Assert(res, "calloc failed with %zu * %zu = %zu",nmemb,size,nmemb*size);
	return res;
}

char* create_strbuf(FILE* fp,const Elf32_Shdr* shdr){
	Assert(shdr->sh_type==SHT_STRTAB,"try read strtable but get wrong header type");
	fseek(fp, shdr->sh_offset, SEEK_SET);
	char* buf=ensure_malloc(shdr->sh_size);
	ensure_frd(buf, shdr->sh_size);
	return buf;
}


void load_elf(const char* filename){
	unsigned char e_ident[EI_NIDENT];	
	FILE* fp=fopen(filename,"rb");
	Assert(fp,"open file failed");

	fseek(fp, 0, SEEK_END);
	int elfsize=ftell(fp);
	fseek(fp, 0, SEEK_SET);

	Log("open elf file %s size %d\n",filename,elfsize);

	ensure_frd_one(e_ident);

	Assert(memcmp(e_ident, ELFMAG, sizeof(ELFMAG)-1)==0,"not elf file");

	Assert(e_ident[EI_CLASS]==ELFCLASS32,"not support non 32bit elf");

	// EI_PAD said the e_ident end pos
	// printf("EI_PAD : %d\n",e_ident[EI_PAD]);

	Elf32_Ehdr hdr;

	fseek(fp, 0, SEEK_SET);
	ensure_frd_one(hdr);

	//for(int i=0;i<sizeof(buf);i++){
	//	putchar(buf[i]);
	//}


	//printf("shoff %d shnum %d\n",hdr.e_shoff,hdr.e_shnum);
   	fseek(fp, hdr.e_shoff, SEEK_SET);
   	Elf32_Shdr* shdr=ensure_calloc(hdr.e_shnum,sizeof(Elf32_Shdr));
	Assert(hdr.e_shstrndx<hdr.e_shnum,"section header str table idx out of header tot num");

	Elf32_Shdr* sh_shstrtab=&shdr[hdr.e_shstrndx];
	Elf32_Shdr* sh_symtab=NULL;
	for(int i=0;i<hdr.e_shnum;i++){
		ensure_frd_one(shdr[i]);
		if(shdr[i].sh_type==SHT_SYMTAB)
			sh_symtab=&shdr[i];
	}
	Assert(sh_symtab,"can not find sym table");

	char* shstr_buf=create_strbuf(fp, sh_shstrtab);

	//for(int i=0;i<hdr.e_shnum;i++){
	//	printf("[%2d] %s\n",i,&shstr_buf[shdr[i].sh_name]);
	//}

	Assert(sh_symtab->sh_entsize==sizeof(Elf32_Sym),"sym table entry size inconsist with 32b sym structure");
	Assert(sh_symtab->sh_size%sh_symtab->sh_entsize==0,"sym tot size is not multiples of entry size");
	size_t sym_num=sh_symtab->sh_size/sh_symtab->sh_entsize;
	fseek(fp, sh_symtab->sh_offset, SEEK_SET);

	Elf32_Sym* syms=ensure_calloc(sym_num,sizeof(Elf32_Sym));
	ensure_frd(syms, sym_num);

	Assert(sh_symtab->sh_link<hdr.e_shnum,"sym table linked strtab idx out of tot num");
	Elf32_Shdr* sh_syms_strtab=&shdr[sh_symtab->sh_link];

	char* symstr_buf=create_strbuf(fp, sh_syms_strtab);

	for(int i=0;i<sym_num;i++){
		int type=ELF32_ST_TYPE(syms[i].st_info);
		if(type!=STT_FUNC)continue;
		printf("SYM %d: %08X %5d FUNC %s\n",
				i,syms[i].st_value,syms[i].st_size, &symstr_buf[syms[i].st_name]);
	}

	free(shdr);
	free(shstr_buf);
	free(symstr_buf);
	free(syms);
	fclose(fp);
}
