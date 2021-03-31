#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
using namespace std;

ifstream infile;                    //讀取檔案
fstream outfile;                    //輸出檔案
int reg[10]={0,9,5,7,1,2,3,4,5,6};  //register
int memory[5]={5,9,4,8,7};          //memory
int cycle=1;                        //clock cycle
string input[100];                  //把指令存入
int pos=0;                          //算input的位置

/**IF/ID用到的變數**/
struct IFID{
    int pc;
    string instruction;
    IFID(){//初始化
        pc=0;
    }
};

/**ID/EX用到的變數**/
struct IDEX{
    int data1,data2;
    int signext;    //sign_ext
    int rs,rt,rd;
    int address;    //branch的位置
    string control;
    string opcode;
    string fun;
    string instruction;
    int hazard;
    int lwdata1;
    int lwdata2;
    IDEX(){
        hazard=0;
        address=0;
        rs=rd=rt=data1=data2=0;
        signext=0;
        fun="000000";
        control="000000000";
        instruction="00000000000000000000000000000000";
    }
};

/**EX/MEM用到的變數**/
struct EXMEM{
    int aluout;
    int writedata;
    int rt,rd,rs;
    string control;
    string fun;
    int signext;    //sign_ext
    int data1;
    int data2;
    EXMEM(){
        signext=0;
        rd=rt=aluout=data1=data2=0;
        writedata=0;
        control="000000000";
    }
};

/**MEM/WB用到的變數**/
struct MEMWB{
    int readdata;
    int aluout;
    int rt,rs,rd;
    string control;
    int signext;    //sign_ext
    MEMWB(){
        signext=0;
        rs=rt=rd=readdata=aluout=0;
        control="000000000";
    }
};

/**將二進位轉乘十進位**/
int binary(string s){//R Type
    int num=0,site=0;
    for(int i=s.length()-1;i>=0;i--){
        if(s[i]=='1')
            num+=pow(2,site);
            site++;
    }
    return num;
}

//可能有負數的
int binary2(string s){//I Type (andi, addi)
    int num=0,site=0;
    int sign=0;
    if(s[0]=='1'){ //如果是負數的話 就把0轉成1 1轉成0
        sign=1;
        for(int i=s.length()-1;i>=0;i--){
            if(s[i]=='0'){
                s[i]='1';
            }else{
                s[i]='0';
            }
        }
        int add=0;
        if(s[s.length()-1]=='0'){
            s[s.length()-1]='1';
        }else{
            s[s.length()-1]=0;
            add=1;
            for(int i=s.length()-2;i>=0;i--){
                if(s[i]=='1'&&add==1){
                    s[i]='0';
                }else{
                    s[i]='1';
                    add=0;
                    break;
                }
            }
        }
    }
    for(int i=s.length()-1;i>=0;i--){
        if(s[i]=='1')
            num+=pow(2,site);
            site++;
    }
    if(sign==0)
        return num;
    else
        return -num;
}


void printf(IFID *,IDEX*,EXMEM*,MEMWB*);//宣告輸出變數function

/**********跑pipeline**********/

void run(IFID *state1,IDEX *state2,EXMEM *state3,MEMWB *state4){

/************************IF**************************/
        state1->pc+=4;                  //把IF/ID的pc+4
        string s;
        s=input[state1->pc/4-1];
        if(s==""){  //如果沒有指令的時候全都是0
            state1->instruction="00000000000000000000000000000000";
        }else{
            state1->instruction=s;
        }

/**********************ID*************************/
    int branchhazard=0;                                                //偵測有沒有beq
    state2->opcode=state2->instruction.substr(0,6);                    //取0~5的位置算opcode
    state2->rs=binary(state2->instruction.substr(6,5));                //取6~10的位置算RS
    state2->rt=binary(state2->instruction.substr(11,5));               //取11~15的位置算RT
    state2->data1=reg[state2->rs];                                 // 第1個值是 RS的值
    state2->data2=reg[state2->rt];                                 // 第2個值是 RT的值
    state2->signext=binary(state2->instruction.substr(16,16));         //算16~31的位置
    if(state2->opcode=="000000"){                                      //R type
        state2->rd=binary(state2->instruction.substr(16,5));           //取16~20的位置算RT
        state2->fun=state2->instruction.substr(26,6);                  //取26~31的位置算 FUNCTION
        if(state2->fun=="000000"){                                     //bubble
            state2->control="000000000";
        }else{                                                         // R type
            state2->control="110000010";
        }
    }else{                                       //I type
        state2->rd=0;                            //I type的RT是0
        if(state2->opcode=="000100"){            //beq
            state2->address=state2->signext*4+4; //算跳的位置
            state2->control="X0101000X";
            if(state2->data1==state2->data2){    //如果值一樣的話就branch(branchhazard=1)
                branchhazard=1;
            }
        }else if(state2->opcode=="100011"){      //lw
            state2->control="000101011";
        }else if(state2->opcode=="101011"){      //sw
            state2->control="X0010010X";
        }else if(state2->opcode=="001000"){      //addi
            state2->signext=binary2(state2->instruction.substr(16,16));
            state2->control="000100010";
        }else{                                   //andi
            state2->signext=binary2(state2->instruction.substr(16,16));
            state2->control="111000010";
        }
    }

/***********************EXE*************************/
    if(state3->control=="000000000")                        //bubble
        state3->aluout=0;
    else if(state3->control=="110000010"){                  //R type
        if(state3->fun=="100000"){                          //add
            state3->aluout=state3->data1+state3->data2;
        }else if(state3->fun=="100010"){                    //sub
            state3->aluout=state3->data1-state3->data2;
        }else if(state3->fun=="100100"){                    //and
            state3->aluout=state3->data1&state3->data2;
        }else if(state3->fun=="100101"){                    //or
            state3->aluout=state3->data1|state3->data2;
        }else if(state3->fun=="101010"){                    //slt
            if(state3->data1<state3->data2)
                state3->aluout=1;
            else
                state3->aluout=0;
        }
    }else if(state3->control=="000100010"){                 //addi
        state3->rd=state3->rt;
        state3->aluout=state3->signext+state3->data1;
    }else if (state3->control=="000101011"){                //lw
        state3->rd=state3->rt;
        state3->aluout=state3->rs;
    }else if(state3->control=="X0101000X"){                 //beq
        state3->aluout=state3->data1-state3->data2;
        state3->rd=state3->rt;
    }else if(state3->control=="111000010"){                 //andi
        state3->rd=state3->rt;
        state3->aluout=state3->data1&state3->signext;
    }else{                                                  //sw
        state3->aluout=state3->data1+state3->signext;
        state3->rd=state3->rt;
    }

/**********************MEM*************************/
    if(state4->control=="000101011"){                       //lw
        state4->readdata=memory[(state4->signext+reg[state4->aluout])/4];
    }else if(state4->control=="X0010010X"){                 //sw
        memory[state4->aluout/4]=reg[state4->rt];
    }else {
        state4->readdata=0;
    }

/***********MEMhazard**************/
    int hazard=0;
    if(state4->rd!=0 && state3->rd!=state2->rs && (state4->rd==state2->rs || state4->rd==state2->rt)&&state4->control=="000101011"){
        hazard=1;
       if(state4->rd==state2->rs){
            state2->data1=state4->aluout;
       }
       if(state4->rd==state2->rt){
           state2->data2=state4->aluout;
       }
    }

/*******EXhazard**********/

    if(state3->rd!=0 && (state2->rs==state3->rd || state3->rd==state2->rt )&&state3->control!="000101011"){
        hazard=1;
        if(state2->rs==state3->rd){
            state2->data1=state3->aluout;
        }if(state2->rt==state3->rd){
            state2->data2=state3->aluout;
        }
    }

    if(hazard==1&&state4->control=="000101011"){
        if(state4->rd==state2->rs){
            state2->data1=memory[(state4->signext+reg[state4->rs])/4];
        }else if(state4->rd==state2->rt){
                state2->data2=memory[(state4->signext+reg[state4->rs])/4];
        }
    }

/**********lwhazard*************/

    int loadhazard=0;
    if(state2->control=="000101011"){
        if(state2->rt==binary(state1->instruction.substr(6,5))||state2->rt==binary(state1->instruction.substr(11,5))){
            loadhazard=1;
        }
    }

/**********輸出****************/

    printf(state1,state2,state3,state4);

/**********偵測要不要下一個cycle***********/

    int next=-1;        //如果IF的指令是bubble + ID的control全是0 + EXE的control全是0 + MEM的control全是0
    if(cycle!=1&&state1->instruction=="00000000000000000000000000000000"&&state2->control=="000000000"&&state3->control=="000000000"&&state4->control=="000000000"){
        next=0;
    }else{
        next=1;
    }
    if(next==1){        //如果要到下一個cycle就轉換
        cycle++;

    /************************WB************************/

        if(state4->rd!=0){ //暫存器不為0
            if(state4->control=="110000010" ||state4->control=="000100010" ||state4->control=="111000010"){//R type + addi + andi
                reg[state4->rd]=state4->aluout;
            }else if(state4->control=="000101011"){//lw
                    reg[state4->rd]=state4->readdata;
            }
        }

    /********state轉換***********/
        if(loadhazard==1){
            //如果lwhazard的話就增加一個bubble 原來的指令就繼續IF ID
            /*if(state3->rt==state2->rs){
                state2->lwdata1=memory[(state3->signext+reg[state3->rs])/4];
                state2->lwdata2=state2->data2;
            }else if(state3->rt==state2->rt){
                state2->lwdata1=state2->data1;
                state2->lwdata2=memory[(state3->signext+reg[state3->rs])/4];
            }
            state2->hazard=1;*/
            state4->rd=state3->rd;            //把state3的指令給state4
            state4->control=state3->control;
            state4->aluout=state3->aluout;
            state4->signext=state3->signext;
            state3->signext=state2->signext;
            state3->rd=state2->rd;
            state3->rt=state2->rt;
            state3->rs=state2->rs;
            state3->data1=state2->data1;
            state3->data2=state2->data2;
            state3->control=state2->control;
            state2->control="000000000";
            state2->instruction="00000000000000000000000000000000";
            state1->pc-=4;
        }else{                                //沒有load hazard的話
            state2->instruction=state1->instruction;
            if(branchhazard==1){             //如果branchhazard的話 就增加一個bubble給state2
                state1->pc=state2->address;  //跑到beq的地方
                state2->instruction="00000000000000000000000000000000";
            }
            state3->writedata=state2->data2;    //data部分
            state3->data1=state2->data1;
            state3->data2=state2->data2;
            state4->control=state3->control;    //control部分
            state3->control=state2->control;
            state4->rd=state3->rd;              //暫存器部分
            state3->rd=state2->rd;
            state4->rs=state3->rs;
            state3->rs=state2->rs;
            state3->rt=state2->rt;
            state4->aluout=state3->aluout;      //ALUOUT部分
            state3->fun=state2->fun;
            state4->signext=state3->signext;    //sign_ext部分
            state3->signext=state2->signext;
        }
        run(state1,state2,state3,state4);       //繼續跑
    }
}

void refresh(){//將暫存器和記憶體初始化
    cycle=1;
    reg[0]=0;
    reg[1]=9;
    reg[2]=5;
    reg[3]=7;
    reg[4]=1;
    reg[5]=2;
    reg[6]=3;
    reg[7]=4;
    reg[8]=5;
    reg[9]=6;
    memory[0]=5;
    memory[1]=9;
    memory[2]=4;
    memory[3]=8;
    memory[4]=7;
    for(int i=pos-1;i>=0;i--){//陣列初始化
        input[i]="";
    }
    pos=0;
}


int main()
{
    string s;
//cout<<"file 1"<<endl;
    /**讀第一個檔案**/
    infile.open("General.txt",ios::in);
    while(infile>>s){
        input[pos]=s;
        pos++;
    }
    infile.close();
    IFID state1;
    IDEX state2;
    EXMEM state3;
    MEMWB state4;
    outfile.open("genResult.txt",ios::out);
    run(&state1,&state2,&state3,&state4);
    outfile.close();
//cout<<"file2"<<endl;
    /**讀第二個檔案**/
    refresh();
    infile.open("Datahazard.txt",ios::in);
    while(infile>>s){
        input[pos]=s;
        pos++;
    }
    infile.close();
    IFID state5;
    IDEX state6;
    EXMEM state7;
    MEMWB state8;
    outfile.open("dataResult.txt",ios::out);
    run(&state5,&state6,&state7,&state8);
    outfile.close();
//cout<<"file3"<<endl;
    /**讀第三個檔案**/
    refresh();
    //infile.open("SampleInput.txt");
    infile.open("Lwhazard.txt",ios::in);
    while(infile>>s){
        input[pos]=s;
        pos++;
    }
    infile.close();
    IFID state9;
    IDEX state10;
    EXMEM state11;
    MEMWB state12;
    outfile.open("loadResult.txt",ios::out);
    run(&state9,&state10,&state11,&state12);
    outfile.close();
//cout<<"file 4"<<endl;
    /**讀第四個檔案**/
    refresh();
    infile.open("Branchhazard.txt",ios::in);
    while(infile>>s){
        input[pos]=s;
        pos++;
    }
    infile.close();
    IFID state13;
    IDEX state14;
    EXMEM state15;
    MEMWB state16;
    outfile.open("branchResult.txt",ios::out);
    run(&state13,&state14,&state15,&state16);
    outfile.close();

    return 0;
}

void printf(IFID *state1,IDEX *state2,EXMEM *state3,MEMWB *state4){//輸出結果
    outfile<<"CC"<<":"<<cycle<<endl<<endl;

    outfile<<"Registers:"<<endl;
    outfile<<"$0: "<<reg[0]<<endl;
    outfile<<"$1: "<<reg[1]<<endl;
    outfile<<"$2: "<<reg[2]<<endl;
    outfile<<"$3: "<<reg[3]<<endl;
    outfile<<"$4: "<<reg[4]<<endl;
    outfile<<"$5: "<<reg[5]<<endl;
    outfile<<"$6: "<<reg[6]<<endl;
    outfile<<"$7: "<<reg[7]<<endl;
    outfile<<"$8: "<<reg[8]<<endl;
    outfile<<"$9: "<<reg[9]<<endl<<endl;

    outfile<<"Data memory:"<<endl;
    outfile<<"0x00: "<<memory[0]<<endl;
    outfile<<"0x04: "<<memory[1]<<endl;
    outfile<<"0x08: "<<memory[2]<<endl;
    outfile<<"0x0C: "<<memory[3]<<endl;
    outfile<<"0x10: "<<memory[4]<<endl<<endl;

    outfile<<"IF/ID :"<<endl;
    outfile<<"PC"<<"\t"<<"\t"<<state1->pc<<endl;
    outfile<<"Instruction"<<"\t"<<state1->instruction<<endl<<endl;

    outfile<<"ID/EX:"<<endl;
    outfile<<"ReadData1"<<"\t"<<state2->data1<<endl;
    outfile<<"ReadData2"<<"\t"<<state2->data2<<endl;
    outfile<<"sign_ext"<<"\t"<<state2->signext<<endl;
    outfile<<"Rs"<<"\t"<<"\t"<<state2->rs<<endl;
    outfile<<"Rt"<<"\t"<<"\t"<<state2->rt<<endl;
    outfile<<"Rd"<<"\t"<<"\t"<<state2->rd<<endl;
    outfile<<"Control signals "<<state2->control<<endl<<endl;

    outfile<<"EX/MEM :"<<endl;
    outfile<<"ALUout"<<"\t"<<"\t"<<state3->aluout<<endl;
    outfile<<"WriteData"<<"\t"<<state3->writedata<<endl;
    outfile<<"Rt/Rd"<<"\t"<<"\t"<<state3->rd<<endl;
    outfile<<"Control signals "<<state3->control.substr(4,5)<<endl<<endl;

    outfile<<"MEM/WB :"<<endl;
    outfile<<"ReadData"<<"\t"<<state4->readdata<<endl;
    outfile<<"ALUout"<<"\t"<<"\t"<<state4->aluout<<endl;
    outfile<<"Rt/Rd"<<"\t"<<"\t"<<state4->rd<<endl;
    outfile<<"Control signals "<<state4->control.substr(7,2)<<endl;

    outfile<<"================================================================="<<endl;
}
