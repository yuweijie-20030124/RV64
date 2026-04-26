//#include <device/map.h>
#include <fcntl.h>
#include <elf.h>
#include <unistd.h>
#include <common.h>

typedef struct SymbolEntry {
	char name[128];	//函数名
	unsigned char info;     //ELF符号类型信息
	paddr_t address;       //函数起始地址
	word_t size;        //函数大小
} SymbolEntry;

static SymbolEntry* sym_entrys = NULL;
static uint32_t sym_num = 0;
static uint32_t call_depth = 0;
static uint32_t trace_func_call_flag = 0;

void init_symtab_entrys(FILE *elf_file) {
	if (elf_file == NULL) assert(0);

	Elf32_Ehdr ehdr;
	int result = fread(&ehdr, sizeof(Elf32_Ehdr), 1, elf_file);
	assert(&ehdr != NULL && result == 1);

    if (ehdr.e_ident[0] != 0x7F ||
        ehdr.e_ident[1] != 'E' ||
        ehdr.e_ident[2] != 'L' ||
        ehdr.e_ident[3] != 'F') {
        printf("Not a ELF file\n");
        exit(0);
    }


	//  ehdr.e_shnum是节头表中的项数
	Elf32_Shdr *shdrs = malloc(sizeof(Elf32_Shdr) * ehdr.e_shnum);
    assert(shdrs != 0);
	result = fseek(elf_file, ehdr.e_shoff, SEEK_SET); //节头表偏移
	assert(result == 0);
	result = fread(shdrs, sizeof(Elf32_Shdr), ehdr.e_shnum, elf_file);
	assert(result != 0);

    //遍历节头表，查找符号表和字符串表，用偏移赋值给他
	//符号表：保存符号的 地址 size 类型 名字（在字符串的便宜）。
	//字符串表：保存符号名的字符数据。
	Elf32_Shdr *symtab = NULL;
    Elf32_Shdr *strtab = NULL;

    // 只寻找符号表，找到后通过 sh_link 定位对应的字符串表
    for (int i = 0; i < ehdr.e_shnum; i++) {
        if (shdrs[i].sh_type == SHT_SYMTAB) {
            symtab = &shdrs[i];
            break;
        }
    }
    assert(symtab != NULL);

    // sh_link 是目标字符串表在节头表中的索引
    uint32_t strtab_idx = symtab->sh_link;
    assert(strtab_idx < (uint32_t)ehdr.e_shnum);
    strtab = &shdrs[strtab_idx];
    assert(strtab->sh_type == SHT_STRTAB);

	//计算符号表中条目数量 shsize是符号表的大小   shentsize是每个符号条目的大小
    //两者相除得到符号表中包含的符号总数量，赋值给全局变量symnum
    //offset是符号表在ELF文件中的偏移量
	uint32_t entry_num = symtab->sh_size / symtab->sh_entsize;
	sym_num = entry_num;	
	uint32_t offset = symtab->sh_offset; //符号数据在文件的起始位置的偏移量多少，用于后面进来读取具体内容。


	//把符号表的内容读取到symbol tables中，从 ELF 文件中读取 entry_num 个符号，存入 symbol_tables 数组中。
	Elf32_Sym *symbol_tables = malloc(sizeof(Elf32_Sym) * entry_num);
	result = fseek(elf_file, offset, SEEK_SET);
	assert(result == 0);
	result = fread(symbol_tables, sizeof(Elf32_Sym), entry_num, elf_file);
	assert(result != 0);

	// 初始化自定义符号表 SymbolEntry 是 符号表条目
	sym_entrys = malloc(sizeof(SymbolEntry) * entry_num);
    char *str = malloc(strtab -> sh_size);
	//字符串表的总字节数
    int str_result = fseek(elf_file, strtab -> sh_offset, SEEK_SET);
    assert(str_result == 0);
	printf("sh_size = ");
    str_result = fread(str, 1, strtab -> sh_size, elf_file);
    assert(str_result != 0);
	assert(str != NULL);
    //把strtab中的str解析出来。

	for (int i = 0; i < entry_num; i++) {
		strcpy(sym_entrys[i].name, str + symbol_tables[i].st_name);	//name是内部偏移，要加上str
		sym_entrys[i].info = symbol_tables[i].st_info;
		sym_entrys[i].address = (paddr_t) symbol_tables[i].st_value;
		sym_entrys[i].size = (word_t) symbol_tables[i].st_size;
	}
        
	free(shdrs);
	free(symbol_tables);
	free(str);
}

void parse_elf(const char *elf_file) {
	if (elf_file == NULL) {
		return;
	}	
	Log("The elf file is %s\n", elf_file);
	trace_func_call_flag = 1;
	FILE *file = fopen(elf_file, "rb");
	assert(file != NULL);
	init_symtab_entrys(file);
}

char *get_function_name_by_addres(paddr_t addr) {
	for (int i = 0; i < sym_num; i++) {
		if (ELF32_ST_TYPE(sym_entrys[i].info) == STT_FUNC) {
			if (addr >= sym_entrys[i].address && addr < 
                (sym_entrys[i].size + sym_entrys[i].address)) {
				return sym_entrys[i].name;
			}
		}
	}
	return NULL;
}

void call_trace(paddr_t pc, paddr_t target) {
    if (trace_func_call_flag == 0) return;
    ++call_depth;
    char *name  = get_function_name_by_addres(target);
    Log(FMT_PADDR ":%*scall [%s@" FMT_PADDR "] (depth=%u)\n", pc, call_depth , "", name, target, call_depth);
}

void ret_trace(paddr_t pc) {
    if (trace_func_call_flag == 0) return;
    char *name = get_function_name_by_addres(pc);
    Log(FMT_PADDR ":%*sret [%s] (depth=%u)\n", pc, call_depth , "", name, call_depth);
    --call_depth;
}

