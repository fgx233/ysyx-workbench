#include <elf.h>
#include "common.hpp"
#include "cpu.hpp"
#include "utils.hpp"

#ifdef CONFIG_FTRACE

#define JALR_MASK     0x0000707f 
#define JALR_KEY      0x00000067
#define JAL_MASK      0x0000007f
#define JAL_KEY       0x0000006f

typedef struct func_info {
  paddr_t addr;
  word_t  size;
  char name[64];
} func_info;
static int func_num = 0;
static func_info *funcs = NULL;
static int call_depth = 0;

void init_ftrace(const char *elf_flie) {
  if (elf_flie == NULL) {
    return;
  }
  //打开文件
  FILE *fp = fopen(elf_flie, "rb");
  Assert(fp, "打开elf文件失败，请检查elf目录(%s)是否正确", elf_flie);
  //读入整个elf头
  Elf32_Ehdr ehdr;
  Assert(fread(&ehdr, sizeof(Elf32_Ehdr), 1, fp) == 1, "读取elf头出错");
  //读入整个section head table
  Elf32_Shdr shdrs[ehdr.e_shnum];
  fseek(fp, ehdr.e_shoff, SEEK_SET);
  Assert(fread(shdrs, sizeof(Elf32_Shdr), ehdr.e_shnum, fp) == ehdr.e_shnum, "读取节头表出错");

  //读取符号表头，字符表头
  Elf32_Shdr *sym_hdr = NULL;
  Elf32_Shdr *str_hdr = NULL;

  for (int i = 0; i < ehdr.e_shnum; i++) {
    if (shdrs[i].sh_type == SHT_SYMTAB) {
      sym_hdr = &shdrs[i];
      str_hdr = &shdrs[sym_hdr->sh_link];
      break;
    }
  }

  //若没读取成功，直接返回
  if (sym_hdr == NULL || str_hdr == NULL) {
    fclose(fp);
    return;
  }

  //读取字符表
  char *str_section = (char *)malloc(str_hdr->sh_size);
  fseek(fp, str_hdr->sh_offset, SEEK_SET);
  Assert(fread(str_section, str_hdr->sh_size, 1, fp) == 1, "读取字符表失败");

  //读取符号表
  int symbol_cnt = sym_hdr->sh_size / sizeof(Elf32_Sym);
  Elf32_Sym sym_section[symbol_cnt];
  fseek(fp, sym_hdr->sh_offset, SEEK_SET);
  Assert((int)fread(sym_section, sizeof(Elf32_Sym), symbol_cnt, fp) == symbol_cnt, "读取符号表失败");

  for (int i = 0; i < symbol_cnt; i++) {
    if (ELF32_ST_TYPE(sym_section[i].st_info) == STT_FUNC) {
      func_num++;
    }
  }

  funcs = (func_info *)malloc(func_num * sizeof(func_info));

  int ind = 0;
  for (int i = 0; i < symbol_cnt; i++) {
    if (ELF32_ST_TYPE(sym_section[i].st_info) == STT_FUNC) {
      funcs[ind].addr = sym_section[i].st_value;
      funcs[ind].size = sym_section[i].st_size;
      strncpy(funcs[ind].name, str_section + sym_section[i].st_name, 63);
      funcs[ind].name[63] = '\0';
      ind++;
    }
  }

  free(str_section);
  fclose(fp);

}

static const char *find_func(paddr_t addr) {
  for (int i = 0; i < func_num; i++) {
    if (addr >= funcs[i].addr && addr < funcs[i].addr + funcs[i].size) {
      return funcs[i].name;
    }
  }
  return "Unknown function";
}

static void ftrace_call(paddr_t pc, paddr_t dest) {
  log_write(FMT_PADDR":",pc);
  for (int i = 0; i < call_depth; i++) {
    log_write("  ");
  }
  log_write("call [%s@" FMT_PADDR "]\n", find_func(dest), dest);
  call_depth++;
}

static void ftrace_ret(paddr_t pc) {
  if (call_depth > 0) {
    call_depth--;
  }
  log_write(FMT_PADDR":",pc);
  for (int i = 0; i < call_depth; i++) {
    log_write("  ");
  }
  log_write("ret  [%s]\n", find_func(pc));
}

static void jal_check(paddr_t pc, paddr_t dest, int rd) {
  if (rd == 1 || rd == 5) {
    ftrace_call(pc, dest);
  }
}

static void jalr_check(paddr_t pc, paddr_t dest, int rd, uint32_t inst) {
  int rs1 = BITS(inst, 19, 15);
  bool rd_islink = (rd == 1 || rd == 5);
  bool rs1_islink = (rs1 == 1 || rs1 == 5);
  bool rd_isequal_rs1 = (rd == rs1);
  if (!rd_islink && !rs1_islink) {
    return;
  } else if (!rd_islink && rs1_islink) {
    ftrace_ret(pc);
  } else if (rd_islink && !rs1_islink) {
    ftrace_call(pc, dest);
  } else if (rd_islink && rs1_islink && !rd_isequal_rs1) {
    ftrace_ret(pc);
    ftrace_call(pc, dest);
  } else if (rd_islink && rs1_islink && rd_isequal_rs1) {
    ftrace_call(pc, dest);
  }
}

void ftrace_check(Decode *s) {
  uint32_t inst = s->isa.inst;
  int rd = BITS(inst, 11, 7);
  if ((inst & JAL_MASK) == JAL_KEY) {
    jal_check(s->pc, s->dnpc, rd);
  } else if ((inst & JALR_MASK) == JALR_KEY) {
    jalr_check(s->pc, s->dnpc, rd, inst);
  }
}
#endif