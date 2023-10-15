#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//全域變數
#define MAXOP 27

char name[16];
FILE *f;
FILE *fobj;

char line[80];
char label[7];
char op[7];
char operand[10];
char new_operand[7];
int indexed = 0;
char hex[16];

char prog_name[7];
int  start_addr = 0;
int  start_text=0;//記text部分的起使位置
int  end_text=0;//記text部分的結束位置
int  prog_len = 0;
char obj_line[70];
char obj_code[7];
int  locctr = 0;
int  textpos = 0;

const char a_start[] = "START";
const char a_end[] = "END";
const char a_byte[] = "BYTE";
const char a_word[] = "WORD";
const char a_resb[] = "RESB";
const char a_resw[] = "RESW";

const char optab[26][2][6] = {{"ADD", "18"}, {"AND", "40"}, {"COMP", "28"}, {"DIV", "24"}, {"J", "3C"}, {"JEQ", "30"}, {"JGT", "34"}, {"JLT", "38"}, {"JSUB", "48"}, {"LDA", "00"}, {"LDCH", "50"}, {"LDL", "08"}, {"LDX", "04"}, {"MUL", "20"}, {"OR", "44"}, {"RD", "D8"}, {"RSUB", "4C"}, {"STA", "0C"}, {"STCH", "54"}, {"STL", "14"}, {"STSW", "E8"}, {"STX", "10"}, {"SUB", "1C"}, {"TD", "E0"}, {"TIX", "2C"}, {"WD", "DC"}};
const char hex_c[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void deci_to_hex(int num)//10進位轉成16進位
{
    char temp[16];
    int i = 0;
    int j = 0;
    while(num)
    {
        temp[i] = hex_c[num % 16];//對十進位制數求餘並最終與hexc陣列中的字元匹配
        i++;
        num=num/16;
    }
    j=i-1;
    for (i=0; j >= 0; j--, i++)//倒過來放好
        hex[i]=temp[j];
    hex[i]='\0';
}

typedef struct sym_node
{
    char l[7];
    int v;
    struct sym_node * next;
} symNODE;

symNODE *symtab = NULL;

symNODE* insert(char *s, int r)
{
    symNODE *t = NULL;
    t = (struct sym_node *)malloc(sizeof(struct sym_node));
    if( t )
    {
        strcpy(t->l, s);
        t->v = r;
        t->next=symtab;
    }
    return t;
}

symNODE* search(symNODE *t, char *s)//有起點
{
    if(t)
    {
        if (strcmp(s, t->l) == 0)
        {
            return t;
        }
        else
        {
            return search(t->next, s);
        }
    }
    else
    {
        return NULL;
    }
}

symNODE* new_search( char *s)//從頭找
{
    int i;
    strcpy(new_operand, s );
    for (i=strlen(new_operand); i<6; i++)
        new_operand[i] = ' ';
    new_operand[6] = '\0';
    return search( symtab, new_operand );
}

char* lookup (char *s)//找 op code
{
    int i = 0;
    int nf = 1;
    while ((i < MAXOP) && (nf))
    {
        if (strcmp(s, optab[i][0]) == 0)
            nf=0;
        else i++;
    }
    if (i >= MAXOP)
        return NULL;
    else
        return (char*)optab[i][1];
}

int operand_len ()//operand長度
{
    int i, l;
    l = strlen(operand);
    if (operand[0] == 'C')
        l -= 3;
    else if (operand[0] == 'X')
        l = (l-3) / 2;
    return l;
}

int readline()//讀一行
{
    int i, j, l, x;

    fgets(line, 80, f);
    l = strlen(line);
    if ((l>0) && (line[0]!='.'))
    {
        for (i = 0; i < 6; i++)//記label
        {
            label[i] = line[i];
        }
        label[i] = '\0';
        while(line[i]==' ') i++;
        j = 0;
        while ((line[i]!=' ') && (line[i]!='\0') && (line[i]!='\n') && (i < l))//記operation
        {
            op[j] = line[i];
            i++;
            j++;
        }
        op[j] = '\0';
        while(line[i]==' ') i++;
        j = 0;
        while ((line[i]!=' ') && (line[i]!='\0') && (line[i]!='\n') && (i < l))//記operand
        {
            operand[j] = line[i];
            i++;
            j++;
        }
        operand[j] = '\0';
        indexed = 0;
        x = strlen(operand);
        if((x>2) && (operand[x-2]==',') && (operand[x-1]=='X'))
        {
            operand[x-2] = '\0';
            indexed = 1;
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

void pass1 ()
{
    readline();
    int l;
    if(strcmp(op,"START")==0)//從START開始
    {
        l = strlen(operand);
        int i, T;
        for(i=l-1, T=1; i>=0; i--, T*=16)
            locctr+=(operand[i]-'0')*T;
        start_addr=locctr;
    }
    else
        locctr=0;

    while(1)
    {
        readline();
        if(strcmp(op,"END")==0)//一直讀到END
            break;
        if(line[0]!='.')//跳過有.的那幾行
        {
            symNODE *temp=search(symtab, label);//檢查是否有重複的label
            if(line[0]!=' ')//label有東西的話
            {
                if(temp!=NULL)
                printf("have duplicate label [%s]\n", label);
            else
                symtab=insert(label, locctr);//加入label表格
            }
            char *n=lookup(op);//找op表
            if(n!=NULL)//正常的op
                locctr+=3;
            else if(strcmp(op,"WORD")==0)
                locctr+=3;
            else if(strcmp(op,"RESW")==0)
                locctr+=3*atoi(operand);//operand轉成數字
            else if(strcmp(op,"RESB")==0)
                locctr+=atoi(operand);
            else if(strcmp(op,"BYTE")==0)
            {
                locctr+=operand_len(operand);
            }
            else
                printf("invalid opcode [%s]\n", op);
        }
    }
    prog_len=locctr-start_addr;
}

void print_symtab (symNODE * t)//印symtab
{
    if (t)
    {
        print_symtab( t->next );
        printf("[%s] = [%5X]\n", t->l, t->v);
    }
}

void init_obj_line()//初始化obj_line
{
    int i;
    for (i=0; i<70; i++)
        obj_line[i] = ' ';
    obj_line[i] = '\0';
}

void wr_header()//寫Head
{
    init_obj_line();
    obj_line[0]='H';//0
    deci_to_hex(prog_len);
    int i, j;
    for(i=1, j=0; i<=6; i++, j++)//1-6程式名稱
        obj_line[i]=label[j];
    for(i=7, j=0; i<=12; i++)//7-12起始位置
    {
        if(i<=12-operand_len())//前面補0
            obj_line[i]='0';
        else
        {
            obj_line[i]=operand[j];
            j++;
        }
    }
    deci_to_hex(prog_len);
    for(i=13, j=0; i<=18; i++)//13-18總長度
    {
        if(i<=18-strlen(hex))//前面補0
            obj_line[i]='0';
        else
        {
            obj_line[i]=hex[j];
            j++;
        }
    }
    printf(obj_line);
    printf("\n");
    fprintf(fobj, obj_line);
    fprintf(fobj, "\n");
}

void init_obj_code ()
{
    int i;
    for (i=0; i<6; i++)
        obj_code[i] = ' ';
    obj_code[6] = '\0';
}

void conv_byte ( int l, char *p, char *q )
{
    int i, j, k, max, c, m, n;
    if (p[0] == 'X')
    {
        max = 2 * l;
        for (i=2, j=0, k=0; k < max&&p[i]!='\''; i++, j++, k++)
            q[j] = p[i];

        q[j] = '\0';
    }
    else if (p[0] == 'C')
    {
        max = l;
        for (i=2, j=0, k=0; k < max; i++, k++)
        {
            c = (int)p[i];
            m = c / 16;
            q[j++] = hex_c[m];
            n = c % 16;
            q[j++] = hex_c[n];
        }
        q[j] = '\0';
    }
    else
    {
        printf("Error: wrong operand of BYTE!\n");
    }
}

void init_text ()
{
    init_obj_line();
    sprintf( obj_line, "T%6X  ", start_text);//obj_line的格式:T+起始位置
    int i;
    for (i=1; i<7; i++)
        if (obj_line[i] == ' ') obj_line[i] = '0';//空白補0
    textpos = 9;
}

void wr_text ()
{
    if((end_text-start_text)>=0)//若長度為正
    {
        deci_to_hex(end_text-start_text);
        if(textpos<999)
            start_text=end_text;//現在的結尾是下一次的開頭
        if(strlen(hex)==2)
        {
            obj_line[7]=hex[0];
            obj_line[8]=hex[1];
        }
        else//長度不足2的話要補0
        {
            obj_line[7]='0';
            obj_line[8]=hex[0];
        }
        printf(obj_line);
        printf("\n");
        fprintf(fobj, obj_line);
        fprintf(fobj, "\n");
    }
}

void add_text (int k)
{

    int const max = 69;
    int i;
    if ((textpos+k)>max)//當無法再加的時候就開始寫入Text
    {
        wr_text();
        init_text();
    }
    for (i=0; i<k; i++)//把code家道陣列尾端
    {
        obj_line[textpos] = obj_code[i];
        textpos++;
    }
    end_text=locctr;//更新結束位置
}

void wr_end ()
{
    init_obj_line();
    obj_line[0]='E';//0
    deci_to_hex(start_addr);
    int i, j;
    for(i=1, j=0; i<=6; i++)
    {
        if(i<=6-strlen(hex))
            obj_line[i]='0';
        else
        {
            obj_line[i]=hex[j];
            j++;
        }
    }
    printf(obj_line);
    printf("\n");
    fprintf(fobj, obj_line);
    fprintf(fobj, "\n");
}


void pass2 ()
{
    readline();
    if(strcmp(op,"START")==0)//START開始
    {
        locctr=start_addr;
        start_text=locctr;
        wr_header();
    }
    else
        locctr=0;

    init_text ();
    while(1)
    {
        init_obj_code();
        readline();
        if(line[0]!='.')
        {
            char *n=lookup(op);//找op表
            if(n!=NULL)
            {
                obj_code[0]=n[0];
                obj_code[1]=n[1];
                if(strlen(line)>=16)//operand是有東西的
                {
                    int i;
                    if(indexed==1)//有","的是間接定址模式
                        deci_to_hex(new_search(operand)->v+32768);
                    else
                        deci_to_hex(new_search(operand)->v);
                    int j;
                    for(i=2, j=0; i<6; i++, j++)
                        obj_code[i]=hex[j];
                    obj_code[i]='\0';

                }
                else
                {
                    int i, j;
                    for(i=2, j=0; i<6; i++, j++)
                        obj_code[i]='0';
                    obj_code[i]='\0';
                }
                locctr+=3;
                add_text(strlen(obj_code));//累加進陣列裡
            }
            else if(strcmp(op,"WORD")==0)
            {
                obj_code[0]='0';
                obj_code[1]='0';
                int i, j;
                int nn=atoi(operand);//轉成整數
                deci_to_hex(nn);
                for(i=2, j=0; i<6; i++)
                {
                    if(i>=6-strlen(hex))
                    {
                        obj_code[i]=hex[j];
                        j++;
                    }
                    else
                        obj_code[i]='0';
                }
                obj_code[i]='\0';
                locctr+=3;
                add_text(strlen(obj_code));//累加進陣列裡
            }
            else if(strcmp(op,"RESW")==0)
            {
                locctr+=3*atoi(operand);
                if(textpos!=999)
                {
                    wr_text();
                    init_text();
                }
                textpos=999;//直接開新的一行Text
                start_text=locctr;
            }
            else if(strcmp(op,"RESB")==0)
            {
                locctr+=atoi(operand);
                if(textpos!=999)
                {
                    wr_text();
                    init_text();
                }
                textpos=999;
                start_text=locctr;
            }
            else if(strcmp(op,"BYTE")==0)
            {
                obj_code[0]='0';
                obj_code[1]='0';
                conv_byte(strlen(operand)-3, operand, obj_code);
                locctr+=operand_len(operand);
                add_text(strlen(obj_code));
            }
            else if(strcmp(op,"END")==0)
            {
                wr_text();
                break;
            }
            else
            {
                printf("invalid opcode [%s]\n", op);
            }
        }
    }
    wr_end();
}

int main(int argc, char*argv[])
{
    int t = argc;
    char fname[20];
    int i = 0;
    if (t == 2 )
    {
        f = fopen(argv[1], "r");
        if (f)
        {
            printf("... Assembling %s!\n", argv[1]);
            pass1();
            printf("...... End of Pass 1; Program length = %6X.\n", prog_len);
            printf("...... Contents in SymbTab:\n");
            print_symtab(symtab);
            fclose( f );
            strcpy( fname, argv[1] );
            for (i=0; (fname[i]!='.') && (fname[i]!='\0'); i++);
            fname[i++] = '.';
            fname[i++] = 'o';
            fname[i++] = 'b';
            fname[i++] = 'j';
            fname[i] = '\0';
            f = fopen(argv[1], "r");
            fobj = fopen("ans.txt", "w+");
            printf("...... Start of Pass 2.\n");
            pass2();
            printf("Assembling succeeded.  %s is generated.\n", fname);
            fclose( f );
            fclose( fobj );
        }
        else
        {
            printf("Assemble syntax: [assemble soure_file_name]\n");
        } // f
    }
    else
    {
        printf("Assemble syntax: [assemble soure_file_name]\n");
    } // t
}

