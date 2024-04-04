#include<common.h>
#include<elf.h>

#define BUFF_NUMS 32




typedef struct itrace_node 
{
    vaddr_t pc;
    uint32_t inst;
}itrace_node;

typedef struct func_info
{
    char name[32];
    paddr_t pc_start;
    paddr_t pc_end;
}func_info;

static Elf32_Shdr *section_head;//节头表指针
static Elf32_Sym* symtab;       //符号节指针
static char* strtab;            //字符串节指针
static func_info * func_tab;    //函数信息执政
static itrace_node iringbuf[BUFF_NUMS] = {};
static int depth = 0;           //调用深度

static FILE *ftrace_log;

static int func_nums = 0;

static int i = 0;
static bool full = false;

void inst_trace(vaddr_t pc, uint32_t inst)
{
    iringbuf[i].pc = pc;
    iringbuf[i].inst = inst;
    i = (i + 1) % BUFF_NUMS;
    full = full || (i == 0);
}

void inst_print()
{
    if(!full)
        return;
    int j = 0;   
    void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
    do{
        char buf[128] = {};
        char *p = buf;
        uint8_t *inst_node = (uint8_t *)&iringbuf[j].inst;
        p += snprintf(p, sizeof(buf), "%s", (j == i - 1)? "-->" : "   ");
        p += snprintf(p, sizeof(buf), FMT_WORD ":", iringbuf[j].pc);
        
        for (int k = 3; k >= 0; k--)
        {
            p += snprintf(p, 4, " %02x", inst_node[k]);
        }

        memset(p, ' ', 1);
        p += 1;
        disassemble(p, buf + sizeof(buf) - p, iringbuf[j].pc, (uint8_t *)&iringbuf[j].inst, 4);

        puts(buf);
        j++;
    }while(j % BUFF_NUMS != 0);


}

void mtrace_read(paddr_t addr, int len)
{
    printf("memory  read at:" FMT_PADDR " len:%d\n", addr, len);
}

void mtrace_write(paddr_t addr, int len, word_t data)
{
    printf("memory write at:" FMT_PADDR " len:%d data:" FMT_WORD, addr, len, data);
}

void init_elf(const char *elf_file)
{

    ftrace_log = fopen("/home/fgx/Desktop/ysyx-workbench/am-kernels/tests/cpu-tests/ftrace.log", "w");//funtion trace记录文件
    Assert(ftrace_log, "ftrace日志文件创建失败");



    FILE *elf_fp = fopen(elf_file, "rb");//读取elf文件
    Assert(elf_fp, "elf文件读取失败");


    Elf32_Ehdr elf_head;
    if(fread(&elf_head, sizeof(Elf32_Ehdr), 1, elf_fp) != 1)//读取elf头
    {
        Assert(0,"elf头读取错误");
    }

    section_head = (Elf32_Shdr*)malloc(elf_head.e_shnum * elf_head.e_shentsize);//为节头表的存储分配内存
    if(section_head == NULL)
    {
        Assert(0, "section_head分配失败");
    }
    fseek(elf_fp, elf_head.e_shoff, SEEK_SET);//定位节头表的位置
    
    if(fread(section_head, elf_head.e_shentsize, elf_head.e_shnum, elf_fp) != elf_head.e_shnum)//读取节头表的内容
    {
        Assert(0,"节头表读取错误");
    }
    Elf32_Shdr sec_symtab;
    Elf32_Shdr sec_strtab;

    for(int i = 0; i < elf_head.e_shnum; i++)
    {
        if(section_head[i].sh_type == SHT_SYMTAB)
            sec_symtab = section_head[i];//读节头表中符号表内容
        else if(section_head[i].sh_type == SHT_STRTAB && elf_head.e_shstrndx != i)
            sec_strtab = section_head[i];//读节头表中字符串表内容
    }

    int symbol_nums = sec_symtab.sh_size / sec_symtab.sh_entsize;//计算符号表中表项个数

    symtab = (Elf32_Sym*)malloc(sec_symtab.sh_size);//读取符号段
    if(symtab == NULL)
    {
        Assert(0, "symtab内存分配失败");
    }
    fseek(elf_fp, sec_symtab.sh_offset, SEEK_SET);
    if(fread(symtab, sec_symtab.sh_size, 1, elf_fp) != 1)
    {
        Assert(0,"符号表读取错误");
    }

    strtab = (char*)malloc(sec_strtab.sh_size);//读取字符段
    if(strtab == NULL)
    {
        Assert(0, "strtab内存分配失败");
    }
    fseek(elf_fp, sec_strtab.sh_offset, SEEK_SET);
    if(fread(strtab, sec_strtab.sh_size, 1, elf_fp) != 1)
    {
        Assert(0, "符号表读取错误");
    }

    func_tab = (func_info*)malloc(symbol_nums * sizeof(func_info));
    if(func_tab == NULL)
    {
        Assert(0, "func_tab内存分配失败");
    }

    

    for(int i = 0; i < symbol_nums; i++)
    {
        if(ELF32_ST_TYPE(symtab[i].st_info) == STT_FUNC)
        {
            strcpy(func_tab[func_nums].name, strtab + symtab[i].st_name);
            func_tab[func_nums].pc_start = symtab[i].st_value;
            func_tab[func_nums].pc_end   = symtab[i].st_value + symtab[i].st_size;
            func_nums++;
        }
    }

    //test
    printf("Ftrace:\n");
    for(int i = 0; i < func_nums; i++)
    {
        printf("Name: %10s, start:" FMT_PADDR "end:" FMT_PADDR "\n", func_tab[i].name, func_tab[i].pc_start, func_tab[i].pc_end);
    }


}

static int search_symbol(paddr_t pc)
{
    for(int i = 0; i < func_nums; i++)
    {
        if(pc < func_tab[i].pc_end && pc >= func_tab[i].pc_start)
            return i;
    }
    return -1;
}

void ftrace_call(paddr_t pc, paddr_t dest)
{
    depth++;
    int i = search_symbol(dest);
    fprintf(ftrace_log, FMT_PADDR ":%*s call [%s@" FMT_PADDR "]\n", pc, depth*2, " ", i >= 0? func_tab[i].name : "???", dest);
    fflush(ftrace_log);
}

void ftrace_return(paddr_t pc)
{
    int i = search_symbol(pc);
    fprintf(ftrace_log, FMT_PADDR ":%*s ret [%s]\n", pc, depth*2, " ", i >= 0? func_tab[i].name : "???");
    fflush(ftrace_log);
    depth--;
}

void free_elf(void)
{
    free(section_head);
    free(symtab);
    free(strtab);
    free(func_tab);
    fclose(ftrace_log);
}