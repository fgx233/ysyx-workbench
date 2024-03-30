#include<common.h>

#define BUFF_NUMS 32

typedef struct itrace_node 
{
    vaddr_t pc;
    uint32_t inst;
}itrace_node;

static itrace_node iringbuf[BUFF_NUMS] = {};

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