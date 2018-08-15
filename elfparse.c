/*
 * 　编写一个64位的elf解析器
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <elf.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdint.h>

#include <sys/stat.h>
#include <fcntl.h>


void shdr_list(Elf64_Word type);

int main(int argc,char *argv[])
{
	int fd,i;
	uint8_t *mem;
	struct stat st;

	char *StringTable,*interp;

	Elf64_Ehdr *ehdr;
	Elf64_Phdr *phdr;
	Elf64_Shdr *shdr;

	if(argc < 2){
		printf("Usage:%s <executable>\n",argv[0]);
		exit(EXIT_SUCCESS);
	}

	fd = open(argv[1],O_RDONLY);
	if(fd < 0){
		perror("open");
		exit(EXIT_FAILURE);
	}
	if(fstat(fd,&st) < 0){
		perror("fstat");
		exit(EXIT_FAILURE);
	}
	/* 将从elf读取的数据映射到mem */
	mem = mmap(NULL,st.st_size,PROT_READ,MAP_PRIVATE,fd,0);
	if(mem == MAP_FAILED){
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	/*
	 * 检测前两个字节是否满足elf魔数
	 */
	if(mem[0] != 0x7f && strcmp(&mem[1],"ELF")){
		fprintf(stderr,"%s is not an ELF file\n",argv[1]);
		exit(EXIT_FAILURE);
	}

	/*
	 * 将分配的内存转换为Elf64_Ehdr指针（mem本质不止一个Elf64_Ehdr大小）
	 */
	ehdr = (Elf64_Ehdr *)mem;
	if(ehdr->e_type != ET_EXEC){
		fprintf(stderr,"%s is not an executable\n",argv[1]);
		exit(EXIT_FAILURE);
	}
	/*
	 * 从mem中得到该elf程序的程序头指针和程序节头指针.
	 */
	phdr = (Elf64_Phdr *)&mem[ehdr->e_phoff];
	shdr = (Elf64_Shdr *)&mem[ehdr->e_shoff];

	printf("Program Entry point:0x%x\n",ehdr->e_entry);
	/*
	 * 获取string table
	 */
	StringTable = &mem[shdr[ehdr->e_shstrndx].sh_offset];

	/*
	 * 打印段：
	 *	如果e_shnum==0:可能是使用的laptop系统架构与
	 *	程序中的不相符
	 */
	printf("Section header list:\n");
	printf("ehdr->e_shnum = %d\n",ehdr->e_shnum);
	printf("[index]\t名称\t\t\t类型\t\t地址\n");
	for(i=1;i<ehdr->e_shnum;i++){
		printf("[%2d]\t%-20s\t",i-1,&StringTable[shdr[i].sh_name]);
		shdr_list(shdr[i].sh_type);
		printf("\t0x%x\n",shdr[i].sh_addr);
	}
	/*
	 *　打印出每一个段的名字和地址
	 */
	printf("\nProgram header list\n");
	printf("ehdr->e_phnum = %d\n",ehdr->e_phnum);
	printf("[index]\t名称\t\t\t类型\t\t地址\n");
	for(i=0;i<ehdr->e_phnum;i++){
		printf("[%d]",i);
		switch(phdr[i].p_type){
			case PT_LOAD:
				if(phdr[i].p_offset == 0)
					printf("\tPT_LOAD\t\t\tText segment\t0x%x\n",phdr[i].p_vaddr);
				else
					printf("\tPT_LOAD\t\t\tData segment\t0x%x\n",phdr[i].p_vaddr);
				break;
			case PT_INTERP:
				interp = strdup((char *)&mem[phdr[i].p_offset]);
				printf("\tPT_INTERP\t\tInterpreter\t%s\n",interp);
				break;
			case PT_NOTE:
				printf("\tPT_NOTE\t\t\tNote segment\t0x%x\n",phdr[i].p_vaddr);
				break;
			case PT_DYNAMIC:
				printf("\tPT_DYNAMIC\t\tDynamic segment\t0x%x\n",phdr[i].p_vaddr);
				break;
			case PT_PHDR:
				printf("\tPT_PHDR\t\t\tPhdr segment\t0x%x\n",phdr[i].p_vaddr);
				break;
			case PT_GNU_EH_FRAME:
				printf("\tPT_GNU_EH_FRAME\t\tsegment\t\t0x%x\n",phdr[i].p_vaddr);
				break;
			case PT_GNU_STACK:
				printf("\tPT_GNU_STACK\t\tsegment\t\t0x%x\n",phdr[i].p_vaddr);
				break;
			case PT_GNU_RELRO:
				printf("\tPT_GNU_RELRO\t\tsegment\t\t0x%x\n",phdr[i].p_vaddr);
				break;
			default:
				printf("some segment can not parse\n");
				break;
		}
	}
	exit(EXIT_SUCCESS);
}


void shdr_list(Elf64_Word type)
{
	switch(type){
		case SHT_NULL:
			printf("SHT_NULL");
			break;
		case SHT_PROGBITS:
			printf("SHT_PROGBITS");
			break;
		case SHT_SYMTAB:
			printf("SHT_SYMTAB");
			break;
		case SHT_STRTAB:
			printf("SHT_STRTAB");
			break;
		case SHT_RELA:
			printf("SHT_RELA");
			break;
		case SHT_HASH:
			printf("SHT_HASH");
			break;
		case SHT_DYNAMIC:
			printf("SHT_DYNAMIC");
			break;
		case SHT_NOTE:
			printf("SHT_NOTE");
			break;
		case SHT_NOBITS:
			printf("SHT_NOBITS");
			break;
		case SHT_REL:
			printf("SHT_REL");
			break;
		case SHT_SHLIB:
			printf("SHT_SHLIB");
			break;
		case SHT_DYNSYM:
			printf("SHT_DYNSYM");
			break;
		case SHT_INIT_ARRAY:
			printf("SHT_INIT_ARRAY");
			break;
		case SHT_FINI_ARRAY:
			printf("SHT_FINI_ARRAY");
			break;
		case SHT_PREINIT_ARRAY:
			printf("SHT_PREINIT_ARRAY");
			break;
		case SHT_GROUP:
			printf("SHT_GROUP");
			break;
		case SHT_SYMTAB_SHNDX:
			printf("SHT_SYMTAB_SHNDX");
			break;
		case SHT_LOOS:
			printf("SHT_LOOP");
			break;
		//case SHT_LOSUNW:
		//	printf("SHT_NULL");
		//	break;
		//case SHT_SUNW_dof:
		//	printf("SHT_NULL");
		//	break;
		//case SHT_SUNW_cap:
		//	printf("SHT_NULL");
		//	break;
		//case SHT_SUNW_SIGNATURE:
		//	printf("SHT_NULL");
		//	break;
		//case SHT_SUNW_ANNOTATE:
		//	printf("SHT_NULL");
		//	break;
		//case SHT_SUNW_DEBUGSTR:
		//	printf("SHT_NULL");
		//	break;
		//case SHT_SUNW_DEBUG:
		//	printf("SHT_NULL");
		//	break;
		//case SHT_SUNW_move:
		//	printf("SHT_NULL");
		//	break;
		case SHT_SUNW_COMDAT:
			printf("SHT_SUNW_COMDAT");
			break;
		case SHT_SUNW_syminfo:
			printf("SHT_SUNW_syminfo");
			break;
		//case SHT_SUNW_verdef:
		//	printf("SHT_NULL");
		//	break;
		//case SHT_SUNW_verneed:
		//	printf("SHT_NULL");
		//	break;
		//case SHT_SUNW_versym:
		//	printf("SHT_NULL");
		//	break;
		//case SHT_HISUNW:
		//	printf("SHT_NULL");
		//	break;
		//case SHT_HIOS:
		//	printf("SHT_NULL");
		//	break;
		case SHT_LOPROC:
			printf("SHT_LOPROC");
			break;
		//case SHT_SPARC_GOTDATA:
		//	printf("SHT_NULL");
		//	break;
		//case SHT_AMD64_UNWIND:
		//	printf("SHT_NULL");
		//	break;
		case SHT_HIPROC:
			printf("SHT_HITROC");
			break;
		case SHT_LOUSER:
			printf("SHT_LOUSER");
			break;
		case SHT_HIUSER:
			printf("SHT_HITUSER");
			break;
		default:
			printf("NOT_FIND");
			break;
	}

}




