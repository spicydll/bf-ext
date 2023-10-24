#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>

#define SIZE_MEM 30000

typedef struct brainfuck
{
    u_int8_t memory[SIZE_MEM];
    int mem_index;
    char *code;
    int code_index;
    int debug;
    int line;
    int col;
} BrainFuck;

void read_file(FILE *srcfile, BrainFuck *bf)
{
    char* buffer = NULL;
    size_t len;
    ssize_t bytes_read = getdelim( &buffer, &len, '\0', srcfile);

    if (bytes_read == -1)
    {
        printf("fuck u git good\n");
        exit(-1);
    }

    bf->code = buffer;
}

void init_bf(BrainFuck *bf)
{
    memset(bf->memory, 0, SIZE_MEM);
    bf->mem_index = 0;
    bf->code_index = 0;
    bf->line = 1;
    bf->col = 1;
}

void add_mem_index(int *index)
{
    (*index)++;
    if (*index >= SIZE_MEM)
    {
        *index = 0;
    }
}

void sub_mem_index(int *index)
{
    (*index)--;
    if (*index < 0)
    {
        *index = SIZE_MEM - 1;
    }
}

void ptr_up(BrainFuck *bf)
{
    add_mem_index(&(bf->mem_index));
}

void ptr_down(BrainFuck *bf)
{
    sub_mem_index(&(bf->mem_index));
}

void val_add(BrainFuck *bf)
{
    bf->memory[bf->mem_index]++;
}

void val_sub(BrainFuck *bf)
{
    bf->memory[bf->mem_index]--;
}

void get(BrainFuck *bf)
{
    bf->memory[bf->mem_index] = (u_int8_t)getchar();
}

void put(BrainFuck *bf)
{
    putchar(bf->memory[bf->mem_index]);
}

void enter_loop(BrainFuck *bf)
{
    // nop
}

void set_col(BrainFuck *bf)
{
    int index = bf->code_index - 1;
    int col = 1;

    do 
    {
        if (index < 0)
        {
            printf("Bad Loop\n");
            exit(-1);
        }

        col++;
    }
    while (bf->code[--index] != '\n');

    bf->col = col;
}

void exit_loop(BrainFuck *bf)
{
    int close_count = 0;
    if (bf->memory[bf->mem_index] != 0)
    {
        // loop again
        while (bf->code[bf->code_index] != '[' || close_count > 0)
        {
            if (bf->code[bf->code_index] == '[')
            {
                close_count--;
            }

            bf->code_index--;
            
            if (bf->code_index < 0)
            {
                printf("Bad Loop\n");
                exit(-1);
            }

            switch (bf->code[bf->code_index])
            {
            case ']':
                close_count++;
                break;
            case '\n':
                bf->line--;
                set_col(bf);
                break;

            default:
                break;
            }

            bf->col--;
        }
    }
}

void do_syscall(BrainFuck *bf)
{
    u_int64_t return_val = 0;
    u_int64_t args[6];
    u_int8_t long_temp[8];
    int tmp_index = bf->mem_index;
    
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 8; j++)
        {               
            long_temp[j] = bf->memory[tmp_index];
            add_mem_index(&tmp_index);
        }

        memcpy(&(args[i]), long_temp, 8);
        if (bf->debug)
            printf("DEBUG: args[%d] = 0x%016lx (%lu)\n", i, args[i], args[i]);
    }

    //printf("DEBUG: %s\n", (char *)args[1]);

    *long_temp = syscall(args[0], args[1], args[2], args[3], args[4], args[5]);

    tmp_index = bf->mem_index;
    for (int j = 0; j < 8; j++)
    {    
        bf->memory[tmp_index] = long_temp[j];
        add_mem_index(&tmp_index);
    }
}

void load_effective_address(BrainFuck *bf)
{
    u_int8_t index[4], addr_bits[8];
    u_int8_t *addr;
    int temp_index = bf->mem_index;

    for (int i = 0; i < 4; i++)
    {
        index[i] = bf->memory[temp_index];
        add_mem_index(&temp_index);
    }

    addr = &(bf->memory[(int)*index]);
    memcpy(addr_bits, &addr, 8);

    temp_index = bf->mem_index;
    for (int i = 0; i < 8; i++)
    {
        bf->memory[temp_index] = addr_bits[i];
        add_mem_index(&temp_index);
    }
}

void address_to_index(BrainFuck *bf)
{
    u_int8_t index_bits[4], addr_bits[8];
    u_int8_t *addr;
    int index;
    int temp_index = bf->mem_index;

    for (int i = 0; i < 8; i++)
    {
        addr_bits[i] = bf->memory[temp_index];
        add_mem_index(&temp_index);
    }

    memcpy(&addr, addr_bits, 8);
    index = addr - bf->memory;
    memcpy(index_bits, &index, 4);

    temp_index = bf->mem_index;
    for (int i = 0; i < 4; i++)
    {
        bf->memory[temp_index] = index_bits[i];
        add_mem_index(&temp_index);
    }
}

void print_debug_state(BrainFuck *bf)
{
    printf("DEBUG: Ln %1$3d, Col %2$3d: '%3$c' %4$d -> 0x%5$02x (%5$3u", bf->line, bf->col, bf->code[bf->code_index], bf->mem_index, bf->memory[bf->mem_index]);
    if (bf->memory[bf->mem_index] >= ' ')
        printf(": '%c'", bf->memory[bf->mem_index]);
    printf(")\n");
}

void interpret(BrainFuck *bf)
{
    int is_token;
    while (bf->code[bf->code_index] != '\0')
    {
        is_token = 1;
        switch (bf->code[bf->code_index])
        {
        case '>':
            ptr_up(bf);
            break;
        case '<':
            ptr_down(bf);
            break;
        case '+':
            val_add(bf);
            break;
        case '-':
            val_sub(bf);
            break;
        case '[':
            enter_loop(bf);
            break;
        case ']':
            exit_loop(bf);
            break;
        case ',':
            get(bf);
            break;
        case '.':
            put(bf);
            break;
        case '$':
            do_syscall(bf);
            break;
        case '@':
            load_effective_address(bf);
            break;
        case '!':
            address_to_index(bf);
            break;
        case '\n':
            bf->line++;
            bf->col = 0;
        default:
            is_token = 0;
            break;
        }

        if (bf->debug && is_token)
            print_debug_state(bf);
        bf->code_index++;
        bf->col++;
    }
}

void handle_args(int argc, char *argv[], int *file_index, int *debug)
{
    *debug = 0;
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (argv[i][1] == 'd')
                *debug = 1;
        }
        else
        {
            *file_index = i;
        }
    }
}

int main(int argc, char *argv[])
{
    FILE *srcfile;
    BrainFuck bf;
    int file_index;

    if (argc < 2)
    {
        printf("wrong\n");
        return -1;
    }

    handle_args(argc, argv, &file_index, &(bf.debug));
    srcfile = fopen(argv[file_index], "r");

    read_file(srcfile, &bf);
    fclose(srcfile);
    init_bf(&bf);
    interpret(&bf);
}