#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

// masks
#define MOV_INSTRUCTION         0x88

#define MOV_INSTRUCTION_MASK    0xFC
#define FULL_WORD_MASK          0x01
#define DEST_REGISTER_MASK      0x38
#define SRC_REGISTER_MASK       0x07


void print_binary(char item)
{
    for(int i = 7; i >=0 ; i--)
    {
        printf("%d", (item >> i) & 1);
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: dasm <binary-file>\n");
    }

    static const char* byteRegisterTable[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
    static const char* wordRegisterTable[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};

    int n;
    char inst[2];
    FILE* file = fopen(argv[1], "rb");
    char ans[1024];

    while ((n = fread(inst, 1, sizeof(inst), file)) == 2)
    {
        if ((inst[0] & MOV_INSTRUCTION_MASK) == MOV_INSTRUCTION)
        {
            // full word or half?
            bool useWord = ((inst[0]) & FULL_WORD_MASK) == FULL_WORD_MASK;

            // Assuming always Register to Register mov
            int destRegTableIndex = (inst[1] & DEST_REGISTER_MASK) >> 3;
            int srcRegTableIndex =  inst[1] & SRC_REGISTER_MASK;

            const char* destRegister = useWord ? wordRegisterTable[destRegTableIndex] : byteRegisterTable[destRegTableIndex];
            const char* srcRegister = useWord ? wordRegisterTable[srcRegTableIndex] : byteRegisterTable[srcRegTableIndex];

            sprintf(ans, "mov %s %s\n", srcRegister, destRegister);
        }
        else
        {
            sprintf(ans, "Error with instruction\n");
        }
        printf("%s", ans);
    }

    return 0;
}