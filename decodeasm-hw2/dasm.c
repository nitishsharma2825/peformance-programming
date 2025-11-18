#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

// masks
#define MOV_INSTRUCTION_MASK        0x88
#define IMM_MOV_INSTRUCTION_MASK    0xB0
#define IMM_DEST_REGISTER_MASK      0x07

#define REGISTER_MASK               0x38
#define RM_MASK                     0x07
#define MOD_MASK                    0xC0

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
        return 0;
    }

    char* byteRegisterTable[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
    char* wordRegisterTable[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
    char* mod00Table[] = {"(BX) + (SI)", "(BX) + (DI)", "(BP) + (SI)", "(BP) + (DI)", "(SI)", "(DI)", "", "(BX)"};

    int fd = open(argv[1], O_RDONLY);

    unsigned char buffer[1];
    char ans[1024];
    while(1)
    {
        int n = read(fd, &buffer, sizeof(unsigned char));
        if (n < 1)
        {
            printf("Nothing to read\n");
            break;
        }

        if ((buffer[0] & IMM_MOV_INSTRUCTION_MASK) == IMM_MOV_INSTRUCTION_MASK)
        {
            printf("Mov Imm Instruction\n");
            int datalen = 1;
            if ((buffer[0] >> 3) & 1)
            {
                printf("16 bit data\n");
                datalen = 2;
            }
            unsigned char data[datalen];
            n = read(fd, data, datalen * sizeof(unsigned char));
            if (n < datalen) fprintf(stderr, "Wrong format, expected %d bytes, got %d bytes\n", datalen, n);
            int imm_val;
            if (datalen == 2)
            {
                unsigned int u = ((unsigned int)data[1] << 8) | (unsigned int)data[0];
                imm_val = (short)u; /* interpret as signed 16-bit */
            }
            else
            {
                unsigned int u = data[0];
                imm_val = (signed char)u; /* interpret as signed 8-bit */
            }
            int destRegisterIndex = buffer[0] & IMM_DEST_REGISTER_MASK;
            char* destReg = datalen == 2 ? wordRegisterTable[destRegisterIndex] : byteRegisterTable[destRegisterIndex];
            sprintf(ans, "mov %s %d\n", destReg, imm_val);
        }
        else
        {
            unsigned char nextBuffer[1];
            n = read(fd, &nextBuffer, sizeof(unsigned char));
            if (n < 1) fprintf(stderr, "Wrong format, expected 1 byte\n");

            if ((nextBuffer[0] & MOD_MASK) >> 6 == 3)
            {
                // register mode
                printf("Register Mode \n");
                char* srcReg;
                char* destReg;
                int destRegTableIndex;
                int srcRegTableIndex;
                if ((buffer[0] >> 1) & 1)
                {
                    destRegTableIndex = (nextBuffer[0] & REGISTER_MASK) >> 3;
                    srcRegTableIndex =  nextBuffer[0] & RM_MASK;
                }
                else
                {
                    srcRegTableIndex = (nextBuffer[0] & REGISTER_MASK) >> 3;
                    destRegTableIndex =  nextBuffer[0] & RM_MASK;
                }
                srcReg = (buffer[0] & 1) ? wordRegisterTable[srcRegTableIndex] : byteRegisterTable[srcRegTableIndex];
                destReg = (buffer[0] & 1) ? wordRegisterTable[destRegTableIndex] : byteRegisterTable[destRegTableIndex];
                
                sprintf(ans, "mov %s %s\n", destReg, srcReg);
            }
            else if ((nextBuffer[0] & MOD_MASK) >> 6 == 0)
            {
                // memory mode, no displacement
                printf("Memory mode, no displacement\n");
                int index = nextBuffer[0] & RM_MASK;
                char* srcMem = mod00Table[index];
                int destRegTableIndex = (nextBuffer[0] & REGISTER_MASK) >> 3;
                char* destReg = (buffer[0] & 1) ? wordRegisterTable[destRegTableIndex] : byteRegisterTable[destRegTableIndex];
                sprintf(ans, "mov %s %s\n", destReg, srcMem);
            }
            else if ((nextBuffer[0] & MOD_MASK) >> 6 == 1)
            {
                // 8 bit displacement
                printf("8 bit displacement\n");
                unsigned dispL[1];
                n = read(fd, &dispL, sizeof(unsigned char));
            }
            else
            {
                // 16 bit displacement
                printf("16 bit displacement\n");
                unsigned disp[2];
                n = read(fd, &disp, 2 * sizeof(unsigned char));
            }
            
        }
        printf("%s\n", ans);
    }

    close(fd);
    return 0;
}