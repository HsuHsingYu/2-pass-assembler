#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_length 17
#define Text_length 100

typedef struct opTable
{
    char mnemonic[MAX_length];
    int opcode; // hex

} opTable;

typedef struct source
{
    int loc;
    char label[MAX_length];
    char operation[MAX_length];
    char operand[MAX_length];
    char opcode[MAX_length];
} source;

typedef struct symTable
{
    char label[MAX_length];
    int loc;
} symTable;

FILE *fp;
int N = 0;      // line of opcode
int NS = 0;     // line of source
int size = 0;   // size of symbol table
int LOCCTR = 0; // location counter
int ProgramLength = 0;

void readOpTable(opTable *opTab);
void readSource(source *statement);
void Pass1(opTable *opTab, source *statement, symTable *symTab);
void Pass2(opTable *opTab, source *statement, symTable *symTab);
void outputIntermediate(source *statement);
void outputsymTab(symTable *symTab);
void outputSourceProgram(source *statement);

int main()
{
    if ((fp = fopen("opcode.txt", "r")) == NULL)
    {
        printf("opcode open failed");
        return -1;
    }
    char c; // get next line

    /* count lines of opcode */
    while (!feof(fp))
    {
        c = fgetc(fp);
        if (c == '\n')
        {
            N++;
        }
    }
    N += 1; // last line

    /* initialize the opTab */
    opTable opTab[N];
    for (int i = 0; i < N; i++)
    {
        opTab[i] = (opTable){"", 0};
    }
    rewind(fp);
    readOpTable(opTab);

    /* open the source file */
    if ((fp = fopen("source.txt", "r")) == NULL)
    {
        printf("source open failed");
        return -1;
    }

    /* count lines of source  */
    while (!feof(fp))
    {
        c = fgetc(fp);
        if (c == '\n')
        {
            NS++;
        }
    }
    NS += 1; // last line

    /* initialize the statement */
    source statement[NS];
    for (int i = 0; i < NS; i++)
    {
        statement[i] = (source){0, "", "", "", ""};
    }
    rewind(fp);
    readSource(statement);
    size -= 1; // - program name

    symTable symTab[size];
    Pass1(opTab, statement, symTab);

    /* write into file */
    if ((fp = fopen("location.txt", "w")) == NULL)
    {
        printf("Can't open file\n");
        return -1;
    }

    outputIntermediate(statement);

    if ((fp = fopen("symbol table.txt", "w")) == NULL)
    {
        printf("Can't open file\n");
        return -1;
    }

    outputsymTab(symTab);

    if ((fp = fopen("final object program.txt", "w")) == NULL)
    {
        printf("Can't open file\n");
        return -1;
    }

    Pass2(opTab, statement, symTab);

    /* write into file */
    if ((fp = fopen("source program.txt", "w")) == NULL)
    {
        printf("Can't open file\n");
        return -1;
    }
    outputSourceProgram(statement);

    return 0;
}

void readOpTable(opTable *opTab)
{
    char c;
    /* read opTable */
    for (int i = 0; i < N - 1; i++)
    {
        for (int j = 0; (c = fgetc(fp)) != ' '; j++) // mnemonic
        {
            if (c == '\t')
            {
                continue;
            }
            opTab[i].mnemonic[j] = c;
        }
        fscanf(fp, "%x", &opTab[i].opcode);
        fgetc(fp); // next line
    }

    /* read the last one (no '\n') */
    for (int j = 0; (c = fgetc(fp)) != ' '; j++)
    {
        if (c == '\t')
        {
            continue;
        }
        opTab[N - 1].mnemonic[j] = c;
    }
    fscanf(fp, "%x", &opTab[N - 1].opcode);
    fclose(fp);
}

void readSource(source *statement)
{
    char c;

    /* read statement and count the size of symbol table */
    for (int i = 0; i < NS - 1; i++)
    {
        for (int j = 0; (c = fgetc(fp)) != '\t'; j++) // label
        {
            if (c == ' ') // extra space
            {
                continue;
            }
            statement[i].label[j] = c;
        }
        if (strcmp(statement[i].label, "") != 0) // size of symbol table
        {
            size++;
        }
        for (int j = 0; ((c = fgetc(fp)) != '\n' && c != '\t'); j++) // operation
        {
            if (c == ' ')
            {
                continue;
            }
            statement[i].operation[j] = c;
        }
        if (c == '\n')
        {
            continue;
        }
        for (int j = 0; (c = fgetc(fp)) != '\n'; j++) // operand
        {
            if (c == ' ')
            {
                continue;
            }
            statement[i].operand[j] = c;
        }
    }

    /* read the last one (no '\n') */
    for (int j = 0; (c = fgetc(fp)) != '\t'; j++)
    {
        if (c == ' ')
        {
            continue;
        }
        statement[NS - 1].label[j] = c;
    }
    for (int j = 0; (c = fgetc(fp)) != '\t' && c != EOF; j++)
    {
        if (c == ' ')
        {
            continue;
        }
        statement[NS - 1].operation[j] = c;
    }
    if (c == EOF)
    {
        return;
    }
    for (int j = 0; (c = fgetc(fp)) != EOF; j++) // operation
    {
        if (c == ' ')
        {
            continue;
        }
        statement[NS - 1].operand[j] = c;
    }
    fclose(fp);
}

void Pass1(opTable *opTab, source *statement, symTable *symTab)
{

    int index = 0; // index of symTable

    if (strcmp(statement[0].operation, "START") == 0) // read the first input line
    {
        statement[0].loc = strtol(statement[0].operand, NULL, 16); // save as starting address
        LOCCTR = statement[0].loc;                                 // initialize LOCCTR tp starting address
    }
    else // if first opcode != start
    {
        LOCCTR = 0;
    }

    for (int i = 1; strcmp(statement[i].operation, "END") != 0; i++) // while opcode != END
    {
        statement[i].loc = LOCCTR; // save the location counter

        if (strcmp(statement[i].label, "") != 0) // there is a symbol in the LABEL field
        {
            for (int j = 0; j < index; j++) // search SYMTAB for LABEL
            {
                if (strcmp(symTab[j].label, statement[i].label) == 0) // if found
                {
                    printf("duplicate symbol %s\n", statement[i].label);
                    exit(-1);
                }
            } // for search SYMTAB for LABEL

            /* insert (LABEL,LOCCTR) into SYMTAB */
            strcpy(symTab[index].label, statement[i].label);
            symTab[index].loc = LOCCTR;
            index++;
        } // if there is a symbol in the LABEL field

        int flag = 0;               // check whether is in the opTab or not
        for (int j = 0; j < N; j++) // search opTab for OPCODE
        {
            if (strcmp(statement[i].operation, opTab[j].mnemonic) == 0) // if found
            {
                flag = 1;
                break;
            }
        }

        if (flag)
        {
            LOCCTR += 3;
        }
        else if (strcmp(statement[i].operation, "WORD") == 0) // OPCODE = "WORD"
        {
            LOCCTR += 3; // word size = 3
        }
        else if (strcmp(statement[i].operation, "RESW") == 0) // OPCODE = "RESW"
        {
            LOCCTR += 3 * strtol(statement[i].operand, NULL, 10);
        }
        else if (strcmp(statement[i].operation, "RESB") == 0) // OPCODE = "RESB"
        {
            LOCCTR += strtol(statement[i].operand, NULL, 10);
        }
        else if (strcmp(statement[i].operation, "BYTE") == 0) // OPCODE = "BYTE"
        {
            if (statement[i].operand[0] == 'X' || statement[i].operand[0] == 'X') // int
            {
                LOCCTR += (strlen(statement[i].operand) - 3) / 2;
            }
            else // char
            {
                LOCCTR += strlen(statement[i].operand) - 3; // - C' '
            }
        }
        else // set error flag
        {
            printf("invalid operation code %s\n", statement[i].operation);
            exit(-1);
        }

    } // for while opcode != end

    ProgramLength = LOCCTR - statement[0].loc;
}

void Pass2(opTable *opTab, source *statement, symTable *symTab)
{
    fprintf(fp, "H%s\t%06X%06X\n", statement[0].label, statement[1].loc, ProgramLength); // write Header record to object program
    fprintf(fp, "T%06X", statement[1].loc);                                              // initialize the first Text record
    int start = 1;                                                                       // start address of the first Text field
    int blank = 0;                                                                       // is blank or not
    char text[Text_length] = {'\0'};
    // is for translate
    for (int i = 1; strcmp(statement[i].operation, "END") != 0; i++) // while opcode != END
    {
        char tmp[MAX_length] = {'\0'};
        int flag = 0;               // whether the operation is in OPTAB or not
        for (int j = 0; j < N; j++) // search OPTAB for OPCODE
        {
            if (strcmp(statement[i].operation, opTab[j].mnemonic) == 0) // if found
            {
                flag = 1;
                sprintf(statement[i].opcode, "%02X", opTab[j].opcode); // store the opcode of operation
                break;
            }
        } // search OPTAB for OPCODE

        if (flag) // if found the opTab for OPCODE
        {
            if (strcmp(statement[i].operand, "") != 0) // if there is a symbol value in OPERAND field
            {
                int found = 0;                 // whether the operand is in SYMTAB or not
                for (int j = 0; j < size; j++) // search SYMTAB for OPERAND
                {
                    int len = strlen(symTab[j].label);
                    if (strncmp(statement[i].operand, symTab[j].label, len) == 0) // if found
                    {
                        if (statement[i].operand[len] != '\0' && statement[i].operand[len] != ',')
                        {
                            continue;
                        }
                        else if (statement[i].operand[len] == ',')
                        {
                            sprintf(tmp, "%04X", symTab[j].loc + 0x8000); // translate the hex value into string
                        }
                        else
                        {
                            sprintf(tmp, "%04X", symTab[j].loc); // translate the hex value into string
                        }

                        /* store symbol value as operand address */
                        strcat(statement[i].opcode, tmp); // append behind
                        found = 1;
                        break;
                    }
                } // search SYMTAB for OPERAND

                if (!found)
                {
                    strcat(statement[i].opcode, "0000");
                    fprintf(fp, "undefined symbol %s\n", statement[i].operand); // set error flag
                    exit(-1);
                }
            } // if there is a symbol value in OPERAND field
            else
            {
                strcat(statement[i].opcode, "0000");
            } // else

            if (blank != 0) // blank blanks in front, and the start value has not been set
            {
                start = i;                              // set the start value
                blank = 0;                              // it's no blank now
                fprintf(fp, "T%06X", statement[i].loc); // initialize new Text field
            }
        }
        else if (strcmp(statement[i].operation, "BYTE") == 0 || strcmp(statement[i].operation, "WORD") == 0) // if OPCODE = BYTE or WORD
        {
            /* convert the consant to object code */
            if (statement[i].operand[0] == 'X' || statement[i].operand[0] == 'x') // is hex
            {

                /* copy the hex number to tmp string */
                for (int j = 0, k = 2; statement[i].operand[k] != '\''; j++, k++)
                {
                    tmp[j] = statement[i].operand[k];
                }
                strcat(statement[i].opcode, tmp);
            }
            else if (statement[i].operand[0] == 'c' || statement[i].operand[0] == 'C') // is char
            {
                for (int k = 2; statement[i].operand[k] != '\''; k++) // copy the char to tmp
                {
                    /* copy the hex number of the char to the string */
                    sprintf(tmp, "%X", statement[i].operand[k]);
                    strcat(statement[i].opcode, tmp);
                } // for
            }     // else if is char
            else  // is word
            {
                int tmp = strtol(statement[i].operand, NULL, 10);
                sprintf(statement[i].opcode, "%06X", tmp);
            }

            if (blank != 0) // blank blanks in front, and the start value has not been set
            {
                start = i;                              // set the start value
                blank = 0;                              // it's no blank now
                fprintf(fp, "T%06X", statement[i].loc); // initialize new Text field
            }
        } // else if  OPCODE = BYTE or WORD

        if (i - start >= 10) // object code will not fit into the current Text record
        {
            /* write Text record to object program initialize new Text record */
            fprintf(fp, "%02X%s\n", statement[i].loc - statement[start].loc, text);
            fprintf(fp, "T%06X", statement[i].loc);
            if (i - start >= 10)
            {
                start = i;
            }
            strcpy(text, "\0");
        }
        else if (strcmp(statement[i].opcode, "\0") == 0)
        {
            if (blank == 0) // first blank
            {
                fprintf(fp, "%02X%s\n", statement[i].loc - statement[start].loc, text);
                strcpy(text, "\0");
            }
            blank++;
            continue;
        }

        /* add object code to Text record */
        strcat(text, statement[i].opcode); // copy to text field
    }                                      // while not END
    fprintf(fp, "%02X%s\n", statement[1].loc + ProgramLength - statement[start].loc, text);
    fprintf(fp, "E%06X\n", statement[1].loc);
    fclose(fp);
}

void outputIntermediate(source *statement)
{
    fprintf(fp, "Loc         Source statement\n\n");
    for (int i = 0; i < NS - 1; i++)
    {
        fprintf(fp, "%04X\t", statement[i].loc);
        fprintf(fp, "%s\t", statement[i].label);
        fprintf(fp, "%s\t", statement[i].operation);
        fprintf(fp, "%s\n", statement[i].operand);
    }
    fprintf(fp, "\t%s\t", statement[NS - 1].label);
    fprintf(fp, "%s\t", statement[NS - 1].operation);
    fprintf(fp, "%s\n", statement[NS - 1].operand);
    fclose(fp);
}

void outputsymTab(symTable *symTab)
{
    fprintf(fp, "Label Name  	Address\n");
    for (int i = 0; i < size; i++)
    {
        fprintf(fp, "%s\t\t", symTab[i].label);
        fprintf(fp, "%04X\n", symTab[i].loc);
    }
    fclose(fp);
}

void outputSourceProgram(source *statement)
{
    fprintf(fp, "Loc         Source statement		Object code\n\n");
    for (int i = 0; i < NS - 1; i++)
    {
        fprintf(fp, "%04X\t", statement[i].loc);
        fprintf(fp, "%s\t", statement[i].label);
        fprintf(fp, "%s\t", statement[i].operation);
        if (statement[i].operand[strlen(statement[i].operand) - 1] == 'X' || statement[i].operand[strlen(statement[i].operand) - 1] == 'x')
        {
            fprintf(fp, "%s\t\t", statement[i].operand);
        }
        else
        {
            fprintf(fp, "%s\t\t", statement[i].operand);
        }
        fprintf(fp, "%s\n", statement[i].opcode);
    }
    fprintf(fp, "\t%s\t", statement[NS - 1].label);
    fprintf(fp, "%s\t", statement[NS - 1].operation);
    fprintf(fp, "%s", statement[NS - 1].operand);
    fclose(fp);
}
