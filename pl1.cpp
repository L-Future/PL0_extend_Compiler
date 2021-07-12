// pl/0 compiler with code generation
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fstream>  //对文件的操作
#include "pl0.h"
using namespace std;

void error(long n){
    long i;

    printf(" error：");  //error行
    for (i=1; i<=cc-1; i++){
		printf(" ");   //控制输出之间的间隔 将错误显示在出错字符的下方
    }
    printf("^%2d\n",n);
    err++;
}


/* 读取原程序中下一个字符过程getch( );*/
void getch() {          //输出源程序
    if(cc==ll){  //行缓冲区指针指向行缓冲区最后一个字符 从文件读一行到行缓冲区
	if(feof(infile)){    //用来侦测是否读取到了文件尾 为fopen()所返回之文件指针 返回非零值代表已到达文件尾 
	    printf("************************************\n");
	    printf("      program incomplete\n");
	    printf("************************************\n");
	    exit(1);
	}
	ll=0; //行缓冲区长度置于行首 
	cc=0;  //行缓冲区指针置行首 
	printf("%5d", cx);  //输出代码行编号
	while((!feof(infile))&&((ch=getc(infile))!='\n')){     //一行结束 //输出一行原代码
	    printf("%c",ch);
	    ll=ll+1; line[ll]=ch;
	}
	printf("\n");
	ll=ll+1; line[ll]=' ';
    }
    cc=cc+1; ch=line[cc];
}

/* 词法分析过程getsym */
void getsym(){
    long i,j,k;
    int flag=0;
	double t=1.0;     //标记小数点后的位数
	double numt=0,z=0;   //小数部分的值
    while(ch==' '||ch=='\t'){ //去除多余的空格和回车，读一个有效的字符
	getch();
    }
    if(isalpha(ch)){ //测试是否为字母 返回TRUE，否则返回NULL(0)  说明是保留字或者标识符
		// identified OR reserved  开始进行词法分析
	k=0;
	do{  //依次读出源文件中的字符构成标识符 
	    if(k<al){  //标识符长度<最大标识符长度，如超过，取前面一部分多余的抛弃
			a[k]=ch; k=k+1;
	    }
	    getch();
	}while(isalpha(ch)||isdigit(ch));  //#include <ctype.h> 是否为字母|数字
	//直到不是字母或数字，以字母/_开头，后面跟若干个字母或数字
	//对齐  出于程序性能的上考虑
	if(k>=kk){ //标识符长度大于等于kk 
	    kk=k;  //令kk为当前标识符长度
	}else{   // 把标识符缓冲后部没有填入相应字母或空格的空间用空格补足
	    do{
			kk=kk-1; a[kk]=' ';
	    }while(k<kk);
	}
	strcpy(id,a); i=0; j=norw-1;     //把符号串（如var）放入id中
	do{   //二分法查找保留字
	    k=(i+j)/2;
	    if(strcmp(id,word[k])<=0){
		j=k-1;
	    }
	    if(strcmp(id,word[k])>=0){
		i=k+1;
	    }
	}while(i<=j);      //此循环用于在保留字符表中找到对应的保留字
	if(i-1>j){
	    sym=wsym[k];  //  sym返回词法单元的类型 如果i-1》j是保留字
	}else{
	    sym=ident;    //否则是定义的标识符

	}
    }else if(isdigit(ch)){ // number
	k=0;   //数字位数
	num=0;   //数字置为0
	do{  //将读入的数字字符转化成数字
	    num=num*10+(ch-'0'); //num * 10加上最近读出的字符ASCII减'0'的ASCII得到相应的数值
	    k=k+1; getch();
	}while(isdigit(ch)); //依次从源文件中读出字符，组成数字 
	if(ch=='.'){   //遇到小数点说明是实型数据
		getch();
		if(isdigit(ch)){   //处理小数点后的数字
			while(isdigit(ch)){
			t/=10;   //转化为小数
			num=num+(ch-'0')*t;
		    k++;
			getch();
			}
		    sym=realsym;  //实型
		}
		else if(ch=='.'){     //又遇到一个点说明是数组的定义
            sym=intersym;   //数组
			cc=cc-2;
		}
		else{
			error(57);
		}
	}
	else{
		sym=intersym;
	}
	if(k>nmax){  //组成的数字位数大于最大允许的数字位数 
	    error(31);
	}
    }else if(ch==':'){  //读出是冒号 
	getch();
	if(ch=='='){  //变量赋值号
	    sym=becomes;  //sym的类型为赋值号becomes 
		getch();
	}else{
	    sym=nul;  //单独的冒号什么也不是 
	}
    }else if(ch=='/'){   //新增注释
		getch();
		if(ch=='*'){
			flag++;   //注释开始
			getch();
			while(flag>0){
				while(ch!='*'){
					getch();
				}
				getch();
				if(ch=='/')
					flag--;  //注释结束
			}
			getch();
			getsym();
		}
		else{
			sym=ssym[(unsigned char)'/'];  //单字符  多余/
		}
	}
	else if(ch=='*'){
		getch();
		if(ch=='/'){
			getch();
			if(ch=='*'){
				flag=0;
				sym=ssym[(unsigned char)'*'];
				flag++;
				getch();
			    while(flag>0){
					while(ch!='*'){
						getch();
					}
					getch();
				    if(ch=='/')
					    flag--;
				}
				getch();
			}
			else{
			printf("a superflous note symbol ");  //多余的符号
			sym=nul;
			}
		}
		else{
			sym=ssym[(unsigned char)'*'];
		}
	}
	else if(ch=='<'){
	getch();
    if(ch=='='){
	    sym=leq; //<=
		getch();
	}else if(ch=='>'){
	    sym=neq; //<>
		getch();
	}else{
	    sym=lss;  //<
	}
    }else if(ch=='>'){
	getch();
	if(ch=='='){  //>=
	    sym=geq; getch();
	}else{
	    sym=gtr;  //>
	}
    }else if(ch=='='){
		getch();
		if(ch=='='){
			getch();
		}
		sym=eql;  //==
	}
	else if(ch=='.'){
		getch();
		if(ch=='.'){
			sym=dotdot;   //数组？
			getch();
		}
		else{
			sym=period;    //.
		}
	}
	else{
	sym=ssym[(unsigned char)ch]; //未定义的符号
	getch();
    }
}


//目标代码生成
//把生成的目标代码写入目标代码数组，供后面的解释器解释执行 
void gen(enum fct x, long y, double z){
    if(cx>cxmax){
	printf("program too long\n");
	exit(1);
    }
    code[cx].f=x;  //一行代码的助记符 
	code[cx].l=y; code[cx].a=z; //操作数 
    cx=cx+1; //cx指针指向下一个空位 
}

//测试当前单词是否合法过程test 
void test(__int64 s1, __int64 s2, long n){   //-------------语法恢复
//s1:当语法分析进入或退出某一语法单元时当前单词符合应属于的集合  
//s2:在某一出错状态下，可恢复语法分析正常工作的补充单词集合  
//n:出错信息编号，当当前符号不属于合法的s1集合时发出的出错信息 
    if (!(sym & s1)){
	error(n);
	s1=s1|s2;  //把s2集合补充进s1集合
	while(!(sym & s1)){   //找到下一个合法的符号，恢复语法分析工作
	    getsym();
	}
    }
}


/* test */;
//登陆符号表
//k:欲登陆到符号表的符号类型
void enter(enum object k){		// enter object into table
    tx=tx+1;
    strcpy(table[tx].name,id);
    table[tx].kind=k;
    switch(k){  //根据不同的类型进行不同的操作
	case constant:  // 常量名
	    if(num>amax){
		error(31);   // 常数过大
		num = 0;
	    }
	    table[tx].val=num;  // 实际登陆的数字以0代替 
		table[tx].type1=sym;     //记录下常量是实型还是整型
	    break;
	case variable:  //变量名
	    table[tx].level=lev; table[tx].addr=dx; dx=dx+1;
	    break;
	case proc:    //过程名
	    table[tx].level=lev;dx=dx+1;
	    break;
	case type:  //  非终结符
		table[tx].level=lev; /*table[tx].addr=dx; dx=dx+1;*/
		table[tx].type1=sym;
		break;
	case func:   //函数名
		table[tx].level=lev;
		table[tx].funcaddr=dx;dx=dx+1;
		break;
	}
}// 出现重复定义，以最后一次的定义为准

// 在符号表中查找指定符号所在位置
long position(char* id){	// find identifier id in table    // id:要找的符号 
    long i;

    strcpy(table[0].name,id);
    i=tx;  //从符号表中当前位置也即最后一个符号开始找 
    while(strcmp(table[i].name,id)!=0){ //当前的符号与要找的不一致 
	i=i-1;
    }
    return i;  //符号在符号表中的位置，找不到就返回0
}


// 常量声明处理
void constdeclaration(){         
    if(sym==ident){   //开始遇到的第一个符号为标识符
	getsym();      //取得const常量的下一个词的类型   获取下一个字符
	if(sym==eql||sym==becomes){  // = || ：=
	    if(sym==becomes){   //   变量赋值号
		error(1);  //应为=而不是:=
		 //自动进行了错误纠正 赋值号当作等号处理-----------------自动纠错功能
	    }
	    getsym();   //获取下一个token，接上数字  
	    if(sym==intersym||sym==realsym){
		enter(constant);  //把常量登陆到符号表
		getsym();

	    }else{
		error(2);  //后接不是数字 报错
	    }
	}else{
	    error(3); //常量标识符后接不是等号=或赋值号:=，报错
	}
    }else{ 
	error(4);  //const后应为标识符
    }
}

//变量声明
void vardeclaration(){
	long i;
	int identnum=0;      //同类型的变量个数
	char sameid[10][al+1];   //存放同类型的变量的名字
	long size;
	int j;      
	int k=0;
	int t;
    if(sym==ident){  //标识符
		identnum++;
		strcpy(sameid[k],id);
		getsym();
		while(sym==comma){   // , 意味着多个变量名
			getsym();  //读取下一个变量名
			if(sym==ident){
				for(j=0;j<identnum;j++){  //检查变量是否定义重复
					if(strcmp(sameid[j],id)==0){
						error(48);    //重复声明 报错
						break;
					}
				}
				identnum++;
				k++; strcpy(sameid[k],id);
				getsym();  
			
			}
		}
 		if(sym==nul){   //说明遇到：后面应该是指示变量的类型
			getsym();
			if(sym==intersym||sym==realsym||sym==Boolsym){  //类型
				for(j=0,k=0;j<identnum;j++){       //之前的变量全部是相同的类型
					strcpy(id,sameid[k]);

					enter(variable);    //加入符号表
					//tx1++;
					table[tx].type1=sym;  //加入类型
					
					k++;
				}
			}else if(sym==ident){   //用变量定义相同的类型
				i=position(id);        //在符号表中找到Id的位置 返给下表付给i
	            if(i==0){
	            error(11);   //标识符未声明
				}
				sym=table[i].type1;
				if(sym==intersym||sym==realsym||sym==Boolsym){
				for(j=0,k=0;j<identnum;j++){       //之前的变量全部是相同的类型
					strcpy(id,sameid[k]);
					enter(variable);
					//tx1++;
					k++;
					table[tx].type1=sym;
				}
				}
				else if(sym==arraysym){
					for(j=0;j<identnum;j++){       //之前的变量全部是相同的类型
						strcpy(id,sameid[j]);
						enter(variable);
						table[tx].type1=arraysym;
						table[tx].drt=table[i].drt;    //数组维数
						table[tx].size=table[i].size;
						size=table[tx].size;
						for(t=1;t<=table[tx].drt;t++){
						table[tx].low[t]=table[i].low[t];
						table[tx].high[t]=table[i].high[t];
						}
						sym=table[i].type2;
					  	table[tx].type2=sym;
				    	dx=dx+2*table[i].drt+1;         //存放上下界的空间
						dx+=size;
					}
				}
			}else{
				error(36);  //未声明过的变量
			}
			getsym();
		}
		else{
			error(55);  //未声明变量类型
		}
	}else{
	error(4);   //后面应接标识符
    }
}

//标识符表达式 ？？
void typeexpression(){
	    __int64 sym1;       
		long num3,num4,size;          //记录数组的上下界以及大小
		long tx1;
		int drtnum=1;                   
		int j;
		if(sym==intersym||sym==realsym||sym==Boolsym){
			enter(type); getsym();
		}
		else if(sym==arraysym){
			
			sym1=sym;
			getsym();
			if(sym==lmparen){   // [ ??? ？/左括号
				getsym();
				if(sym==intersym){
					num3=num;
			    	getch();
				    getsym();
					if(sym==dotdot){  // ..
						getsym();
						sym=sym1;
						num4=num;
						size=num4-num3+1;
                        enter(type);
						table[tx].low[drtnum]=num3;
						table[tx].high[drtnum]=num4;
						table[tx].drt++;
						getsym();   //取得右中括号
						getsym();   //取得of
						if(sym==ofsym){
							getsym();
							if(sym==intersym||sym==realsym||sym==Boolsym){
								table[tx].type2=sym;
								table[tx].drt=1;
								table[tx].size=size;
							}else{
								while(sym==arraysym){  //多维数组
									table[tx].drt++;  //增加数组维数
									drtnum++;
									getsym();
								    if(sym==lmparen){
										getsym();
										table[tx].low[drtnum]=num;
										getch();
										getsym();
					                    getsym();
										table[tx].high[drtnum]=num;
										getsym();
										getsym();
										getsym();
										size=size*(table[tx].high[drtnum]-table[tx].low[drtnum]+1);
									}
								    else{
										error(32);
									}
									table[tx].size=size;
								}
						    	if(sym==intersym||sym==realsym||sym==Boolsym){
									table[tx].type2=sym;
								}

							}
							getsym();
						}else{
							error(34);
						}
					}else{
						error(34);
					}
				}else{
					error(34);
				}
			}
			else{
				error(34);
			}
		}else
			error(33);
}


//标识符声明
void typedeclaration(){
	char id1[al+1];
	if(sym==ident){
        strcpy(id1,id);
		getsym();
		if(sym==eql||sym==becomes){
	    if(sym==becomes){
		error(1);  //自动进行了错误纠正 把赋值号当作等号处理-----------------自动纠错功能
	    }
		getsym();
		strcpy(id,id1);
		typeexpression();   //传递标识符的名字
		}else{
			error(3);
		}
	}else{
		error(4);
	}
}


//代码指令	列出类PCODE代码 		
void listcode(long cx0){	// list code generated for this block
    long i;
    for(i=cx0; i<=cx-1; i++){
			printf("%10d%5s%3d%10.5f\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}

}
void expre(__int64);
void arraydo(enum fct x, int i){
	long d,t;
	switch(x){
	case sto:
	    gen(lit,0,table[i].drt);
		gen(sto,lev-table[i].level,table[i].addr+1);
		for(t=1,d=1;t<=table[i].drt;t++){    //产生指令将上下界存入,还有存数组的维数
			d++;
			gen(lit,0,table[i].low[t]);	
			gen(sto,lev-table[i].level,table[i].addr+d);
			d++;
			gen(lit,0,table[i].high[t]);
			gen(sto,lev-table[i].level,table[i].addr+d);
		}
		gen(say,lev-table[i].level,table[i].addr);
		break;
	case lod:
	    gen(lit,0,table[i].drt);
		gen(sto,lev-table[i].level,table[i].addr+1);
		for(t=1,d=1;t<=table[i].drt;t++){    //产生指令将上下界存入,还有存数组的维数
			d++;
			gen(lit,0,table[i].low[t]);	
			gen(sto,lev-table[i].level,table[i].addr+d);
			d++;
			gen(lit,0,table[i].high[t]);
			gen(sto,lev-table[i].level,table[i].addr+d);
		}
		gen(lay,lev-table[i].level,table[i].addr);
		break;
	}
}


//因子处理 
//fsys: 如果出错可用来恢复语法分析的符号集合
void factor(__int64 fsys){
    long i;
	long drtnum,away;
	long k;
	long j;
    test(facbegsys,fsys,24);
	//检查当前token是否在facbegsys集合中  //facbegsys开始符号集合
	//如果不是合法的token，报错24,通过fsys集恢复使语法处理可以继续进行------------恢复
    while(sym & facbegsys){
	if(sym==ident){
	    i=position(id);  //返回在符号表中的位置
	    if(i==0){
		error(11);
	    }else{
		switch(table[i].kind){
		    case constant:  //标识符对应的是常量
				gen(lit,0,table[i].val);    //生成lit指令，把值val放到栈顶
				lastsym=table[i].type1;
		        getsym();
			break;
		    case variable:  // 如果标识符是变量名，生成lod指令
					//把位于距离当前层level的层的偏移地址为adr的变量放到栈顶
				if(table[i].type1==intersym||table[i].type1==realsym||table[i].type1==Boolsym)
				{gen(lod,lev-table[i].level,table[i].addr);  //
				 
				 lastsym=table[i].type1;
				 getsym();
				}
				else{
					if(table[i].type1==arraysym){     //数组类型
						away=0;
						getsym();
						for(j=0;j<table[i].drt;j++){
                            if(sym==lmparen)
							{
							getsym();
							expre(fsys|rmparen);
							if(lastsym!=intersym){
								lastsym=typeerror;
								error(49);
							}
							if(sym==rmparen)
								getsym();
							else
								error(40);
							}
							else{
								error(46);
							}
						}
						if(lastsym!=typeerror){
							lastsym=table[i].type2;
						}
                        arraydo(lod,i);

					}
				}	     	  
			break;
		    case proc:    
			error(21); //在因子处理中遇到过程名 报错21号
			break;
			case func:  //
				getsym();
				k=0;
				if(sym==lparen){  //左括号
					getsym();
					expre(fsys|comma|rparen);
					if(lastsym!=table[i].paral[k]){  //参数类型
						error(49);   //类型不匹配
						lastsym=typeerror;
					}
					while(sym==comma){  // ","
						k++;
						getsym();
						expre(fsys|comma|rparen);
				    	if(lastsym!=table[i].paral[k]){
						error(49);
						lastsym=typeerror;
					}
					}
					if(sym==rparen){  // ")"
						gen(cal,lev-table[i].level,table[i].addr);   //
						getsym();
					}
					for(k=0;k<table[i].n;k++){
					   gen(opr,0,7);    //把实参弹出栈
					}
					gen(lod,lev-table[i].level,table[i].funcaddr);
					if(lastsym!=typeerror){
						lastsym=table[i].type1;
					}
				}
				else if(sym==becomes){      //说明是函数中的返回语句
      				gen(lod,lev-table[i].level,table[i].funcaddr);        //记录函数在符号表中的地址偏移量 //将函数返回值放入栈顶
					lastsym=table[i].type1;
				}
				else {       //无参函数
					gen(cal,lev-table[i].level,table[i].addr);  //调用地址为table[i].addr
					gen(lod,lev-table[i].level,table[i].funcaddr);  //取到栈顶 
					lastsym=table[i].type1;
				}

				break;
		}
		}
	}else if(sym==intersym||sym==realsym){
	    if(num>amax){
		error(31); num=0;
	    }
		gen(lit,0,num);
		lastsym=sym;
	    getsym();
	}else if(sym==NOT){
		getsym();
		factor(fsys);
		gen(opr,0,22);
	}else if(sym==truesym){
		gen(lit,0,1);
		lastsym=Boolsym;
		getsym();
	}else if(sym==falsesym){
		gen(lit,0,0);
		lastsym=Boolsym;
		getsym();
	}else if(sym==lparen){
	    getsym();
	    expre(rparen|fsys);
	    if(sym==rparen){
		getsym();
	    }else{
		error(22);
	    }
	}
	test(fsys,lparen,23);
    }
}


// 项处理过程term
//fsys: 如果出错可用来恢复语法分析的符号集合
void term(__int64 fsys){
    __int64 mulop;
	__int64 lasttype;
	long cx1,cx2;
    factor(fsys|times|slash|div|mod|AND);
	lasttype=lastsym;
    while(sym==times||sym==slash||sym==div||sym==mod||sym==AND){
	mulop=sym; getsym();
	if(mulop==AND){
		cx1=cx;gen(jpc,0,0);
	}
	factor(fsys|times|slash|div|mod|AND);
	if(mulop==times){
		if(lasttype==intersym&&lastsym==intersym){
			lastsym=intersym;
		}
		else if(lasttype==intersym&&lastsym==realsym){
			lastsym=realsym;
		}
		else if(lasttype==realsym&&lastsym==realsym){
			lastsym=realsym;
		}
		else if(lasttype==realsym&&lastsym==intersym){
			lastsym=realsym;
		}
		else{
			lastsym=typeerror;
			error(50);
		}
	    gen(opr,0,4);
	}else if(mulop==slash){
		if(lasttype==intersym&&lastsym==intersym){
			lastsym=intersym;
		}
		else if(lasttype==intersym&&lastsym==realsym){
			lastsym=realsym;
		}
		else if(lasttype==realsym&&lastsym==realsym){
			lastsym=realsym;
		}
		else if(lasttype==realsym&&lastsym==intersym){
			lastsym=realsym;
		}
		else{
			lastsym=typeerror;
			error(50);
		}
	    gen(opr,0,5);
	}else if(mulop==div){
		if(lasttype==intersym&&lastsym==intersym){
			lastsym=intersym;
		}
		else if(lasttype==intersym&&lastsym==realsym){
			lastsym=realsym;
		}
		else if(lasttype==realsym&&lastsym==realsym){
			lastsym=realsym;
		}
		else if(lasttype==realsym&&lastsym==intersym){
			lastsym=realsym;
		}
		else{
			lastsym=typeerror;
			error(50);
		}
		gen(opr,0,18);
    }else if(mulop==mod){
		if(lasttype==intersym&&lastsym==intersym){
			lastsym=intersym;
		}
		else{
			lastsym=typeerror;
			error(50);
		}
		gen(opr,0,19);
	}else if(mulop==AND){
		if(lasttype==Boolsym&&lastsym==Boolsym){
			lastsym=Boolsym;
		}
		else{
			lastsym=typeerror;
			error(50);
		}
	     //and短路计算：如果第一个值为0，则不需要判断后面的，直接跳转到factor执行完后的下一条语句，值栈顶值为0；
	    gen(opr,0,20);
	    cx2=cx; gen(jmp,0,0);  
		code[cx1].a=cx;
		gen(lit,0,0);
		code[cx2].a=cx;
	}
	}
}

void simpexp(__int64 fsys){
   __int64 addop;
   __int64 lasttype;
   long cx1,cx2;
    if(sym==plus||sym==minus){
	addop=sym; getsym();
	term(fsys|plus|minus|OR);
	if(addop==minus){
	    gen(opr,0,1);
	}
    }else{
	term(fsys|plus|minus|OR);
    }
    while(sym==plus||sym==minus||sym==OR){
	addop=sym; getsym();
	lasttype=lastsym;
	if(addop==OR){
		cx1=cx; gen(jpq,0,0);       //or指令相关的跳转地址
	}
	term(fsys|plus|minus|OR);
	if(addop==plus){
		if(lasttype==intersym&&lastsym==intersym){
			lastsym=intersym;
		}
		else if(lasttype==intersym&&lastsym==realsym){
			lastsym=realsym;
		}
		else if(lasttype==realsym&&lastsym==realsym){
			lastsym=realsym;
		}
		else if(lasttype==realsym&&lastsym==intersym){
			lastsym=realsym;
		}
		else{
			lastsym=typeerror;
			error(50);
		}
	    gen(opr,0,2);
	}else if(addop==minus){
		if(lasttype==intersym&&lastsym==intersym){
			lastsym=intersym;
		}
		else if(lasttype==intersym&&lastsym==realsym){
			lastsym=realsym;
		}
		else if(lasttype==realsym&&lastsym==realsym){
			lastsym=realsym;
		}
		else if(lasttype==realsym&&lastsym==intersym){
			lastsym=realsym;
		}
		else{
			lastsym=typeerror;
			error(50);
		}
	    gen(opr,0,3);
	}else{
		if(lasttype==Boolsym&&lastsym==Boolsym){
			lastsym=Boolsym;
		}
		else{
			lastsym=typeerror;
			error(50);
		}
        gen(opr,0,21);
	    cx2=cx; gen(jmp,0,0);  
		code[cx1].a=cx;
		gen(lit,0,1);
		code[cx2].a=cx;
	}
	}
}

void expre(__int64 fsys){
    __int64 relop;
	__int64 lasttype;
    if(sym==oddsym){
	getsym(); simpexp(fsys);
	if(lastsym=intersym){
		lastsym=Boolsym;
	}
	else{
		lastsym=typeerror;
		error(50);
	}
	gen(opr,0,6);
    }else{
		simpexp(fsys|eql|neq|lss|gtr|leq|geq|comma|rparen|rmparen);
	}
	if(sym&(eql|neq|lss|gtr|leq|geq)){
	    relop=sym; getsym();
		lasttype=lastsym;
	    simpexp(fsys);
		if(lasttype==intersym&&lastsym==intersym){
			lastsym=Boolsym;
		}
		else if(lasttype==intersym&&lastsym==realsym){
			lastsym=Boolsym;
		}
		else if(lasttype==realsym&&lastsym==realsym){
			lastsym=Boolsym;
		}
		else if(lasttype==realsym&&lastsym==intersym){
			lastsym=Boolsym;
		}
		else if(lasttype==Boolsym&&lastsym==Boolsym){
			lastsym=Boolsym;
		}
		else{
			lastsym=typeerror;
			error(50);
		}
	    switch(relop){
		case eql:
		    gen(opr,0,8);
		    break;
		case neq:
		    gen(opr,0,9);
		    break;
		case lss:
		    gen(opr,0,10);
		    break;
		case geq:
		    gen(opr,0,11);
		    break;
		case gtr:          //大于
		    gen(opr,0,12);
		    break;
		case leq:
		    gen(opr,0,13);
		    break;
	    }
	}    
}


// 语句处理
//fsys: 如果出错可用来恢复语法分析的符号集合
void statement(__int64 fsys){
	__int64 lasttype;
    long i,cx1,cx2;
	long num1[now];
	int drtnum=0;
	long away=0;      //数组元素的偏移量
	long ii;
    if(sym==ident){
		i=position(id);
		if(table[i].type1==arraysym){
			getsym();
	    	for(ii=0;ii<table[i].drt;ii++){
			    if(sym==lmparen){
				    getsym();
				    expre(fsys|rmparen);
					if(lastsym!=intersym){
						error(46);
					}
					lasttype=lastsym;
				    if(sym==rmparen){
					   getsym();
					}
				}
				else{
					error(46);
				}
			}
			lasttype=table[i].type2;
		}
		else{
			lasttype=table[i].type1;
			getsym();
		}
	if(sym==becomes){
	    getsym();
	}else{
	    error(13);    
	}
	expre(fsys);
	if(lasttype==intersym&&lastsym==intersym){
			lastsym=voiderror;
		}
	else if(lasttype==intersym&&lastsym==realsym){
			lastsym=voiderror;
		}
	else if(lasttype==realsym&&lastsym==realsym){
			lastsym=voiderror;
		}
	else if(lasttype==realsym&&lastsym==intersym){
			lastsym=voiderror;
		}
	else if(lastsym==Boolsym&&lasttype==lastsym){
			lastsym=voiderror;
		}
	else
	{
		lastsym=typeerror;
		error(51);
	}
	if(i!=0){
		switch(table[i].kind){
		case variable:
		if(table[i].type1==intersym||table[i].type1==realsym||table[i].type1==Boolsym)
	    gen(sto,lev-table[i].level,table[i].addr);
		else if(table[i].type1=arraysym){
			arraydo(sto,i);
		}
		break;
		case func:
			gen(sto,lev-table[i].level,table[i].funcaddr);
			break;
		case type:
			error(56);
			break;
		}
	}
    }else if(sym==callsym){
		int k=0;
		getsym();
	    if(sym!=ident){   // call的过程名
			error(14);   //call后缺少标识符
		}else{
			i=position(id);
	        if(i==0){
				error(11); //未定义标识符
			}else 
			{
				if(table[i].kind==proc){
					getsym();
					if(sym==lparen)      //左括号  有参数
					{
						do
						{
							getsym();
							expre(fsys|comma|rparen);  //   ， 右括号
							k++;
						}while(sym==comma);
						if(sym==rparen)
						{
							getsym();
						}
						else
						{
							error(40);
						}
					    if(k!=table[i].n)
						{
							printf("the parameter cannot match\n");
						    error(41);
						}
					    gen(cal,lev-table[i].level,table[i].addr);
			     	   // gen(opr,0,7);
						for(k=0;k<table[i].n;k++){
						   gen(opr,0,7);    //把实参弹出栈
						}
						if(sym==funcsym)
						gen(lod,lev-table[i].level,table[i].addr);   //把返回值压入栈顶
					}
				    else{         //没有参数的过程
					    gen(cal,lev-table[i].level,table[i].addr);
					}
				}
				else{
					error(15);
				}
			}
		}
	}else if(sym==ifsym){
	getsym(); 
	expre(fsys|thensym|dosym);
	if(lastsym!=Boolsym){
        lastsym=typeerror;
		error(52);
	}
	if(sym==thensym){
	    getsym();
	}else{
	    error(16);
	}
	cx1=cx;	gen(jpc,0,0);
	statement(fsys|semicolon|endsym|elsesym);

    if(sym==semicolon){
		getsym();
	}
	if(sym==elsesym){
		getsym();
		cx2=cx;gen(jmp,0,0);
	    code[cx1].a=cx;	
	    statement(fsys|semicolon|endsym);
     	code[cx2].a=cx;
	}
	else{
		code[cx1].a=cx;
        statement(fsys|semicolon|endsym);
	}
	}else if(sym==elsesym){
		//cx1=cx;gen(jm p,0,0);
		getsym();
		statement(fsys|semicolon|endsym);
     	//code[cx2].a=cx;
		//code[cx1].a=cx;
    }else if(sym==beginsym){
	getsym(); 
	statement(fsys|semicolon|endsym);
	while(sym==semicolon||(sym&statbegsys)){
	    if(sym==semicolon){
		getsym();
	    }else{
    		error(10);
	    }
  	    statement(fsys|semicolon|endsym);

	}
	if(sym==endsym){                
	    getsym();
	}else{
	    error(17);
	}
    }else if(sym==whilesym){
	whilenum++;
	cx1=cx; getsym();
	expre(fsys|dosym);
	if(lastsym!=Boolsym){
		lastsym=typeerror;
		error(52);
	}
	cx2=cx;	gen(jpc,0,0);
	if(sym==dosym){
	    getsym();
	}else{
	    error(18);
	}
	statement(fsys); gen(jmp,0,cx1);
	code[cx2].a=cx;
	if(exitcx!=0){             //修改exit语句的跳转地址
		code[exitcx].a=cx;
		exitcx=0;
	}
	whilenum--;
	}else if(sym==exitsym){
		if(whilenum==0){
			error(47);
		}
		else{
		exitcx=cx;
		gen(jmp,0,0);
		}
		getsym();
	}

	else if(sym==readsym){
	getsym();
	if(sym!=lparen){
		error(35);
	}else{
		do{
			getsym();
			if(sym==ident)
				i=position(id);
			else i=0;
			if(i==0) error(36);
			else if(table[i].kind==constant||table[i].kind==proc||table[i].type1==Boolsym){	// 不能往常量或过程以及布尔类型读入数据
				error(12); i=0;
				getsym();
				continue;
			}
			else{
				if(table[i].type1==intersym||table[i].type1==realsym){ //说明要读的只是一个简单变量
					getsym();
					gen(opr,0,14);
					gen(sto, lev-table[i].level, table[i].addr); 
				}
				else{
					if(table[i].type1==arraysym&&(table[i].type2&(intersym|realsym))){     //数组类型
						//drtnum=0;
						//away=0;
						getsym();
						for(ii=0;ii<table[i].drt;ii++){
			               if(sym==lmparen){
				              getsym();
				              expre(fsys|rmparen);
						   	  if(lastsym!=intersym){
						        lastsym=typeerror;
							    error(46);
							  }
				              if(sym==rmparen){
					            getsym();
							  } 
						   }
			                else{
				            	error(46);
							}
						}
						gen(opr,0,14);
						arraydo(sto,i);
					}
					else
						error(39);
				}
			}
		}while(sym==comma);
		gen(opr,0,15);    //取得换行符
		if(sym==rparen)
			getsym();
	}
	}
	else if(sym==writesym){
		getsym();
		if(sym==lparen){
			do{
				getsym();
				expre(fsys|rparen|comma);
				if(lastsym!=intersym&&lastsym!=realsym&&lastsym!=constsym){
					lastsym=typeerror;
					error(53);
				}
				if(lastsym==intersym){  //输出整型
				gen(opr,0,16);    //生成输出指令，输出栈顶的值
				}
				else if(lastsym==realsym){
					gen(opr,0,24);
				}
			}while(sym==comma);
			if(sym!=rparen){
				error(35);
			}
			else
				getsym();
			    gen(opr,0,17);            //输出换行符
		}else
			error(35);
	}
    test(fsys,0,19);
}
			
void block(__int64 fsys){
	int j;
	long i,k=0;
    long tx0;		// initial table index
    long cx0; 		// initial code index
    long tx1;		// save current table index before processing nested procedures
    long dx1;		// save data allocation index
	long tx2;

    dx=3; tx0=tx; table[tx].addr=cx; gen(jmp,0,0);
	table[tx].n=prodn;
    for(j=0;j<prodn;j++)                 //参数
    {   
        tx++;   
        strcpy(table[tx].name,pnow[j].id);   
        table[tx].kind=variable;   
        table[tx].level=lev;   

        table[tx].addr=dx;   
		table[tx].type1=pnow[j].sym;
        dx++;   
    }
	tx2=tx;    //后来加的

    if(lev>levmax){
	error(32);   //嵌套层数过多
    }
    do{
		if(sym==constsym){         
	    getsym();
	    do{
		constdeclaration();     //如果是const常量 则调用这个
		while(sym==comma){
		    getsym(); 
			constdeclaration();
		}
		if(sym==semicolon){
		    getsym();
		}else{
		    error(5);
		}
	    }while(sym==ident);
		}
	if(sym==typesym){
		getsym();
		do{
			typedeclaration();
  			if(sym==semicolon)
				getsym();
			else
				error(33);
		}while(sym==ident);
	}
	if(sym==varsym){
	    getsym();
	    do{
		vardeclaration();

		if(sym==semicolon) {
		    getsym();
		}else{
		    error(5);
		}
	    }while(sym==ident);
	}
	while(sym==procsym||sym==funcsym){  //判断为 procedure  function
		prodn=0;    //记录过程或函数的数目
		if(sym==procsym){  // procedure
			getsym(); 
	        if(sym==ident){   // procedure名
				enter(proc);  //加入符号表 
				getsym();
			}else{
				error(4);   //未添加标识符
			}
			if(sym==semicolon){   // ";"
				getsym();
			}   //常规操作
			else if(sym==lparen){    //有左括号，说明是有参数的过程
				getsym();
				while(sym==ident){    //读取标识符，为参数名  存储参数名和类型
					 strcpy(pnow[prodn].id,id);   //记录参数名
					 prodn++;
					 if(ch==':'){   //类型声明
						 getch();
					     getsym();
					 }else{
						 error(38);   //未声明类型
					 }
					 if(sym==intersym||sym==realsym||sym==Boolsym)   
					 {   
						 pnow[prodn-1].sym=sym;
						 getsym();   
					 }else{
					     error(39);   //声明的类型不正确 ????
					 }
					 if(sym==semicolon||sym==rparen){
						 getsym();
					 }else{
						 error(5);   //遗漏,或 ]   	-----------语法恢复
					 }
				}
				if(sym==semicolon){    // ";"   结束  ???
					getsym();
				}		
			}else{
				error(5);	//遗漏括号 	 -----------语法恢复
			} 
			
		}// end-procedure
		else if(sym==funcsym){   //处理函数  function
			k=0;
			getsym();
		    if(sym==ident){    // 函数名 标识符
				enter(func); getsym();
				i=position(id);
			}else{
				error(4);  //缺少标识符
			}
            if(sym==lparen){    //有左括号，说明是有参数的函数
				getsym();
				while(sym==ident){   //  参数名
					strcpy(pnow[prodn].id,id);   //记录参数名
				    prodn++;
					if(ch==':'){
						 getch();
					     getsym();
					}else{
						 error(38);   //缺少类型声明
					}
					if(sym==intersym||sym==realsym)   
					{   
						table[i].paral[k]=sym;
						k++;
						pnow[prodn-1].sym=sym;
					    getsym();   

					}else{
					     error(39);   //类型声明不正确
					}
					if(sym==semicolon||sym==rparen){
						 getsym();
					}else{
						error(5);  //遗漏 , 或 )
					}
				}
			}
			if(sym==nul){   // 不能识别的符号
				getsym();
		        if(sym==intersym||sym==realsym||sym==Boolsym){     //声明返回值的类型
					  table[i].type1=sym;
				      getsym();
				}
				 else{
					   error(45);  //函数返回值的类型不正确
				}
			}
			else{
				error(44);  //没有声明函数返回值的类型
			}
			if(sym==semicolon)     //   ";"     ----------缺少 ";" 自动恢复
				getsym();
		    else{
			error(5);		
			} 
		}//end function
	    lev=lev+1; tx1=tx; dx1=dx;    //嵌套层数+1
   	    block(fsys|semicolon);         // 因子开始符号集合 | ";"  第2层嵌套的子程序  //处理 procedure  function 过程/函数语句
	    lev=lev-1; tx=tx1; dx=dx1;    //嵌套执行完 当前层数减少一层
	    if(sym==semicolon){  // ";"
		getsym();
		test(statbegsys|ident|procsym|funcsym,fsys,6);
	    }else{
		error(5);
	    }
	}  //procedure function end.
	test(statbegsys|ident,declbegsys,7);
    }while(sym&declbegsys);  //end  do
    code[table[tx0].addr].a=cx;   //将跳转地址改为正确的CX以保证调用返回的正确性
    table[tx0].addr=cx;		// start addr of code
    cx0=cx;
	for(j=0;j<table[tx0].n;j++){
	gen(sto,lev-table[tx2].level,table[tx2].addr-j);
//	tx0--;
	}
	gen(Int,0,dx+table[tx0].n);
	if(sym==beginsym){
    statement(fsys|semicolon|endsym);
	}
	else{
		error(54);
		getsym();
	}
    gen(opr,0,0); // return
    test(fsys,0,8);
    //listcode(cx0);
}

long base(long b, long l){   //返回访问链的下标
    long b1;
    b1=b;
    while (l>0){	// find base l levels down
	b1=s[b1]; l=l-1;     //根据访问链往上回溯找到变量
    }
    return b1;
}

void interpret(){
//	FILE *infile=fopen("out.txt","rb");
    long p,b,t;		// 指令计数器-,栈基址寄存器,栈顶指针
    instruction i;	// instruction register
//	instruction code[200];
    int k=0,kk;
	int a,j;
	int n;
	double getnum=0;
	double tt=1.0;
	long ls=0;
	long away=0;
	long adr;
	long d;
	char ch/*[nmax]*/;
    printf("start PL/0\n");
    t=0; b=1; p=0;
    s[1]=0; s[2]=0; s[3]=0;
    do{
		if(t>stacksize){
			printf("overflow running_stack\n");
			exit(0);
		}
	i=code[p]; p=p+1;
	switch(i.f){
	    case lit:
		t=t+1; 
		s[t]=i.a;
		break;
	/*	case lir:
			t=t+1;
			s[t]=i.a;
			break;*/
	    case opr:
		switch((long)i.a){ 	// operator
		    case 0:	// return
			t=b-1; p=s[t+3]; b=s[t+2];
			break;
		    case 1:
			s[t]=-s[t];
			break;
		    case 2:
			t=t-1; s[t]=s[t]+s[t+1];
			break;
		    case 3:
			t=t-1; s[t]=s[t]-s[t+1];
			break;
		    case 4:
			t=t-1; s[t]=s[t]*s[t+1];
			break;
		    case 5:     //slash
			t=t-1; s[t]=(long)s[t]/s[t+1];
			break;
		    case 6:
			s[t]=(long)s[t]%2;
			break;
			case 7:
			t--;
			break;
		    case 8:
			t=t-1; s[t]=(s[t]==s[t+1]);
			break;
		    case 9:
			t=t-1; s[t]=(s[t]!=s[t+1]);
			break;
		    case 10:
			t=t-1; s[t]=(s[t]<s[t+1]);
			break;
		    case 11:
			t=t-1; s[t]=(s[t]>=s[t+1]);
			break;
		    case 12:
			t=t-1; s[t]=(s[t]>s[t+1]);
			break;
		    case 13:
			t=t-1; s[t]=(s[t]<=s[t+1]);
			break;
			case 14:   //read指令,读整形3
				j=0;
				getnum=0;
				a=0;
				t++;
				//printf("输入要读的数: ");
			//	scanf("%c",&ch[j]);
				ch=getchar();
				while(ch!=' '&&ch!='.'){
					getnum=getnum*10+(ch-'0');
					j++;
					//scanf("%c",&ch[j]);
					ch=getchar();
				}
				if(ch=='.'){       //说明读入的数是实型 
					tt=1.0;
				//	scanf("%c",&ch[j]);
					ch=getchar();
					while(ch!='\n'){
						tt/=10;
					    getnum=getnum+(ch-'0')*tt;
					    j++;
					    //scanf("%c",&ch[j]);
						ch=getchar();
					}
				}
				s[t]=getnum;
				break;
			case 15:   
				ch=getchar();   //读取换行符
				break;
			case 16:     //write指令
					printf("%5d",(long)s[t]);
					t--;
				break;
			case 17:
				printf("\n");
				break;
			case 18:     //div
				t=t-1; s[t]=s[t]/s[t+1];
				break;
			case 19:
				t=t-1; s[t]=(long)s[t]%(long)s[t+1];
				break;
			case 20:
				t=t-1; s[t]=(long)s[t]&&(long)s[t+1];
				break;
			case 21:
				t=t-1; s[t]=(long)s[t]||(long)s[t+1];
				break;
			case 22:
				s[t]=!(long)s[t];
				break;
			case 23:
				t++;
				break;
			case 24:
				printf("%5.2f  ",s[t]);
				t--;
				break;

		}
		break;
	    case lod:
		t=t+1; 
		s[t]=s[base(b,i.l)+(long)i.a];
		break;
		case lay:
		//	adr=table[i.a].drt;
			adr=0;
			for(k=0,d=0;k<s[base(b,i.l)+(long)i.a+1];k++)   
                {   
                    ls=s[t-(long)(s[base(b,i.l)+(long)i.a+1]-1)+k];
					d++;
                    if(ls<s[base(b,i.l)+(long)i.a+1+d])  
                    {   
                        printf("array overflow\n");   
                        error(40);   
                        break;   
                    }
					d++;
					if(ls>s[base(b,i.l)+(long)i.a+1+d]){
			            printf("array overflow\n");   
                        error(40);   
                        break;  
					}	
					if(s[base(b,i.l)+(long)i.a+1]==1||k==0){      //第一维情况
						away=ls-s[base(b,i.l)+(long)i.a+1+1];
					}
					else{
						away=away*(s[base(b,i.l)+(long)i.a+1+d]-s[base(b,i.l)+(long)i.a+1+d-1]+1)+ls-s[base(b,i.l)+(long)i.a+1+d-1]+1;      
						}
                } 
				adr=adr+away;
			    t=t+1-s[base(b,i.l)+(long)i.a+1];
                s[t]=s[base(b,i.l)+(long)i.a+1+2*(long)s[base(b,i.l)+(long)i.a+1]+1+adr];   
				break;
	    case sto:
		s[base(b,i.l)+(long)i.a]=s[t];/* printf("%10d\n", s[t]);*/t=t-1;
		break;
		case say:
          //      adr=table[i.a.longnum].drt;   
		     	adr=0;
				away=0;
                for(k=0,d=0;k<s[base(b,i.l)+(long)i.a+1];k++)  
                {    
					ls=s[t-(long)s[base(b,i.l)+(long)i.a+1]+k];  
                    d++;
                    if(ls<s[base(b,i.l)+(long)i.a+1+d])  
                    {   
                        printf("array overflow\n");   
                        error(40);   
                        break;   
                    }
					d++;
					if(ls>s[base(b,i.l)+(long)i.a+1+d]){
			            printf("array overflow\n");   
                        error(40);   
                        break;  
					}	
					
					if(s[base(b,i.l)+(long)i.a+1]==1||k==0){
							away=ls-s[base(b,i.l)+(long)i.a+1+1];;
					}
					else{
						away=away*(s[base(b,i.l)+(long)i.a+1+d]-s[base(b,i.l)+(long)i.a+1+d-1]+1)+ls-s[base(b,i.l)+(long)i.a+1+d-1]+1; 
						}
  
                }  
		        adr=adr+away; 
              //  t--;   
                s[base(b,i.l)+(long)i.a+1+2*(long)s[base(b,i.l)+(long)i.a+1]+1+adr]=s[t];   
				t=t-1-s[base(b,i.l)+(long)i.a+1]; //把要赋的值以及下标出栈
                break;   
	    case cal:		// generate new block mark
		s[t+1]=base(b,i.l);    //将父过程基地址入栈  ,访问链
		s[t+2]=b;              //本过程基地址   控制链
		s[t+3]=p;              //返回地址
		b=t+1; p=i.a;
		break;
	    case Int:
		t=t+i.a;
		break;
	    case jmp:
		p=i.a;
		break;
	    case jpc:
		if(s[t]==0){
		    p=i.a;
		}
		//t=t-1;     
		break;
		case jpq:
			if(s[t]==1){
		    p=i.a;
			}
			break;
	}
    }while(p!=0);
    printf("end PL/0\n");
}

    int main(){
    long i;
    for(i=0; i<256; i++){
	ssym[i]=nul;
    }
    for(i=0;i<norw;i++)
		wsym[i]=nul;
	strcpy(word[0],  "Boolean   ");
	strcpy(word[1],  "AND       "); 
	strcpy(word[2],  "array     ");
    strcpy(word[3],  "begin     ");
    strcpy(word[4],  "call      ");
    strcpy(word[5],  "const     ");
	strcpy(word[6],  "div       ");
    strcpy(word[7],  "do        ");
	strcpy(word[8],  "else      ");
    strcpy(word[9],  "end       ");
	strcpy(word[10], "exit      ");	
	strcpy(word[11], "false     ");
    strcpy(word[12], "function  ");
    strcpy(word[13], "if        ");	
	strcpy(word[14], "integer   ");
	strcpy(word[15], "mod       ");
	strcpy(word[16], "NOT       ");
    strcpy(word[17], "odd       ");
    strcpy(word[18], "of        ");
	strcpy(word[19], "OR        ");
    strcpy(word[20], "procedure ");
	strcpy(word[21], "read      ");
	strcpy(word[22], "real      ");
    strcpy(word[23], "then      ");
	strcpy(word[24], "true      ");
	strcpy(word[25], "type      ");
    strcpy(word[26], "var       ");
    strcpy(word[27], "while     ");
	strcpy(word[28], "write     ");	
    strcpy(mnemonic[lit],"lit");
    strcpy(mnemonic[opr],"opr");
    strcpy(mnemonic[lod],"lod");
    strcpy(mnemonic[sto],"sto");
    strcpy(mnemonic[cal],"cal");
    strcpy(mnemonic[Int],"int");   
    strcpy(mnemonic[jmp],"jmp");
    strcpy(mnemonic[jpc],"jpc");
	strcpy(mnemonic[say],"say");   //数组的存指令
	strcpy(mnemonic[lay],"lay");    //数组的取指令
	strcpy(mnemonic[jpq],"jpq");
	wsym[0]=Boolsym;
	wsym[1]=AND;
	wsym[2]=arraysym;
	wsym[3]=beginsym;	

    wsym[4]=callsym;
    wsym[5]=constsym;	
	wsym[6]=div;
    wsym[7]=dosym;	
	wsym[8]=elsesym;
    wsym[9]=endsym;	
	wsym[10]=exitsym;	
	wsym[11]=falsesym;
	wsym[12]=funcsym;
    wsym[13]=ifsym;	
	wsym[14]=intersym;	
	wsym[15]=mod;	
	wsym[16]=NOT;
    wsym[17]=oddsym;	
	wsym[18]=ofsym;	
	wsym[19]=OR;
    wsym[20]=procsym;
	wsym[21]=readsym;
	wsym[22]=realsym;
    wsym[23]=thensym;	
	wsym[24]=truesym;
	wsym[25]=typesym;
    wsym[26]=varsym;
    wsym[27]=whilesym;	
	wsym[28]=writesym;
    ssym['+']=plus;
    ssym['-']=minus;
    ssym['*']=times;
    ssym['/']=slash;
    ssym['(']=lparen;
    ssym[')']=rparen;
    //ssym['=']=eql;
    ssym[',']=comma;
    ssym['.']=period;
    ssym[';']=semicolon;
	ssym['[']=lmparen;
	ssym[']']=rmparen;

    declbegsys=constsym|typesym|varsym|procsym|funcsym;   //声明开始符号集: const、type、var、proc、func;
    statbegsys=beginsym|callsym|ifsym|whilesym|exitsym|writesym|readsym;   //语句开始符号集: begin、call、if、while、exit、write、read;
    facbegsys=ident|intersym|realsym|lparen|NOT|truesym|falsesym;   //因子开始符号集: ident int real ( not true false

    printf("please input source program file name: ");
    scanf("%s",infilename);
    printf("\n");
    if((infile=fopen(infilename,"r"))==NULL){
	printf("File %s can't be opened.\n", infilename);
	exit(1);
    }
    
    err=0;
    cc=0; cx=0; ll=0; ch=' '; kk=al; getsym();
    lev=0; tx=0;  
    block(declbegsys|statbegsys|period);
    if(sym!=period){
	error(9);
    }
	if(err==0){
	if((outfile=fopen("out.txt","wb"))==NULL){
	printf("File <out.txt> can't be opened.\n");
	exit(1);
    }    //  code的地址  指令的大小  指令数目 写入指针
    fwrite(&code,sizeof(instruction),cxmax,outfile);
	fclose(outfile);
    if((outfile=fopen("code.txt","w"))==NULL){
	printf("File <out.txt> can't be opened.\n");
	exit(1);
    }
	for(i=0;i<=cx;i++){
			fprintf(outfile,"%10d%5s%3d%10.5f\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	fclose(outfile);
    interpret();
    }else{
	printf("errors in PL/0 program\n");
    }
    fclose(infile);
    system("pause");
	return(0);
}

