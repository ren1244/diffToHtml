#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "vector.h"

#include "outputObj.c"

#define SYS_FOPEN fopen_utf8

void print_err(const char* errlog)
{
	fputs((char*)errlog,stderr);
}
FILE *fopen_utf8(const char* fname,const char *mode)
{
	wchar_t ws1[128],ws2[128];
	MultiByteToWideChar(CP_UTF8, 0, fname, -1, ws1, 128);
	MultiByteToWideChar(CP_UTF8, 0, mode, -1, ws2, 128);
	FILE *fp=_wfopen(ws1,ws2);
	return fp;
}
int getFileSize(FILE *fp,int *line)
{
	fseek(fp,0,SEEK_SET);
	int i,ch,last;
	*line=0;
	for(i=0;(ch=fgetc(fp))!=EOF;++i)
	{
		if(ch=='\n')
			++ *line;
		last=ch;
	}
	if(last!='\n')
		++ *line;
	return i;
}

char** readFile(const char* filename,int *line)
{
	//open file
	FILE *fp;
	fp=SYS_FOPEN(filename,"r");
	if(fp==NULL)
	{
		print_err("readFile:fopen return NULL\n");
		return NULL;
	}
	//計算行數與檔案大小
	int sz=getFileSize(fp,line);
	printf("%d %d",sz,*line);
	//動態宣告記憶體
	char *buf=(char*)malloc(sizeof(char*)*(*line)+sz+1);
	if(buf==NULL)
	{
		fclose(fp);
		print_err("readFile:malloc return NULL\n");
		return NULL;
	}
	//將檔案資料寫入
	char *data=buf+sizeof(char*)*(*line);
	char **arr=(char**)buf;
	fseek(fp,0,SEEK_SET);
	int i,ch,last;
	*(arr++)=data;
	last=' ';
	for(i=0;(ch=fgetc(fp))!=EOF;++i)
	{
		if(last=='\0')
			*(arr++)=data;
		*(data++)=(last=ch=='\n'?'\0':ch);
	}
	fclose(fp);
	return (char**)buf;
}

char *readAsString(FILE* fp)
{//讀取 stream 直到 eof，儲存在buffer中並回傳
	int capacity=1024;
	int n;
	char *buf=malloc(capacity);
	
	if(buf==NULL)
	{
		print_err("readStdin:malloc return NULL\n");
		return NULL;
	}
	int ch;
	char *p;
	n=0;
	while((ch=getc(fp))!=EOF)
	{
		if(n+1>=capacity) //+1是預留 '\0'
		{
			p=realloc(buf,capacity*=2);
			if(p==NULL)
			{
				free(buf);
				print_err("readStdin:realloc return NULL\n");
				return NULL;
			}
			else
				buf=p;
		}
		buf[n++]=ch;
	}
	buf[n++]='\0';
	return buf;
}

char* readData(const char *fname)
{
	FILE* fp;
	char *str;
	if(fname==NULL)
		str=readAsString(stdin);
	else
	{
		fp=SYS_FOPEN(fname,"r");
		if(fp==NULL)
		{
			print_err("fopen return NULL\n");
			return NULL;
		}
		str=readAsString(fp);
		fclose(fp);
	}
	if(str==NULL)
	{
		print_err("readAsString return NULL\n");
		return NULL;
	}
	return str;
}

char** convertToArray(char* src,int *arrlen)
{
	int i,n,k;
	n=strlen(src);
	if(src[n-1]=='\n')
		src[n-1]='\0';
	for(i=0,n=1;src[i]!='\0';++i)
		if(src[i]=='\n')
			++n;
	char **A=(char**)malloc(sizeof(char*)*n);
	if(A==NULL)
	{
		print_err("convertToArray:malloc return NULL\n");
		return NULL;
	}
	for(i=0,A[k=0]=src;src[i]!='\0';++i)
	{
		if(src[i]=='\n')
		{
			A[++k]=src+i+1;
			src[i]='\0';
		}
	}
	*arrlen=n;
	return A;
}

char* getfname(char* str)
{
	str+=strspn(str," \t\"");
	char *end=strpbrk(str," \t\"");
	if(end!=NULL)
		*end='\0';
	int i,j,N,ch;
	for(j=i=0,N=strlen(str);i<N;++i,++j)
	{
		if(str[i]=='\\')
		{
			ch=(str[++i]-'0');
			ch=ch*8+(str[++i]-'0');
			ch=ch*8+(str[++i]-'0');
			*(unsigned char*)(str+j)=(unsigned char)(ch&0xFF);
		}
		else
			str[j]=str[i];
	}
	str[j]='\0';
	return str+2;
}
void getVar(int *var,char *str)
{
	char *a=strpbrk(str,"0123456789");
	char *c=strchr(a,'@');
	*c='\0';
	int i,count,k=0;
	for(i=count=0;a[i]!='\0';++i)
	{
		if('0'<=a[i] && a[i]<='9')
			k=k*10+a[i]-'0';
		else
		{
			var[count++]=k;
			k=0;
			if(count==4)
				return;
			for(;a[i]!='\0' && !('0'<=a[i] && a[i]<='9');++i);
			--i;
		}
	}
	if(count==2)
	{
		var[2]=var[1];
		var[1]=var[3]=1;
	}
}
int f2(char **A,int N,int rd,const char* fname,output* data)
{
	char *p;
	int x,la,lb; // la lb 為行號
	char *content=NULL,**B=NULL;
	int M;
	
	fprintf(stderr,"fname:%s\n",fname);
	output_pushSep(data,fname);
	//開啟檔案
	if((content=readData(fname))!=NULL)
	{
		if((B=convertToArray(content,&M))==NULL)
		{
			free(content);
			content=NULL;
		}
	}
	if(content==NULL)
	{
		for(;rd<N && strstr(A[rd],"diff --git")!=A[rd];++rd);
		return rd-1;
	}
	
	for(la=lb=0;rd<N;++rd)
	{
		if(strstr(A[rd],"diff --git")==A[rd])
			break;
		if(strstr(A[rd],"@@ -")==A[rd])
		{
			p=strstr(A[rd]+4," +")+2;
			for(x=0;'0'<=*p && *p<='9';++p)
				x=x*10+*p-'0';
			output_balance(data);
			for(;lb+1<x;)
			{
				++la;++lb;
				//printf("#%d/%d:%s\n",la,lb,B[lb-1]);
				output_pushData(data,B[lb-1],3,la,lb);
			}
			//printf("@%d@:%s\n",x,A[rd]);
			continue;
		}
		if(A[rd][0]=='+')
		{
			//printf("+:%s\n",A[rd]);
			++lb;
			output_pushData(data,A[rd]+1,2,la,lb);
			continue;
		}
		if(A[rd][0]=='-')
		{
			//printf("-:%s\n",A[rd]);
			++la;
			output_pushData(data,A[rd]+1,1,la,lb);
			continue;
		}
		if(A[rd][0]==' ')
		{
			//printf(" :%s\n",A[rd]);
			output_balance(data);
			++la;++lb;
			output_pushData(data,A[rd]+1,3,la,lb);
			continue;
		}	
	}
	//printf("output all\n");
	output_balance(data);
	for(;lb<M;)
	{
		++lb;++la;
		output_pushData(data,B[lb-1],3,la,lb);
	}
	free(content);
	free(B);
	return rd-1;
}
char *html(const char* s,char *buf)
{
	int i,n;
	char *p,ch;
	p=buf;
	for(i=0;(ch=s[i])!='\0';++i)
	{
		if(ch=='<')
		{
			strcpy(p,"&lt;");
			p+=4;
		}
		else if(ch=='>')
		{
			strcpy(p,"&gt;");
			p+=4;
		}
		else if(ch=='&')
		{
			strcpy(p,"&amp;");
			p+=5;
		}
		else
			*(p++)=ch;
	}
	*p='\0';
	return buf;
}

void f3(output* data)
{
	char buf[2][1024];
	printf("<!DOCTYPE html>\n\
<html>\n\
<head>\n\
	<meta charset=\"utf-8\">\n\
	<title>diff</title>\n\
	<style type='text/css'>\n\
		tr,td,table{\n\
	margin:0;\n\
	padding:0;\n\
	border:0;\n\
	border-collapse: collapse;\n\
	border-spacing: 0;\n\
	font-family: Inconsolata,Consolas,Monaco;\n\
	vertical-align:top;\n\
	font: normal 112.5%/1.65 Helvetica Neue,Helvetica,Arial;\n\
	font-size:14px;\n\
}\n\
tr,table{\n\
	width:100%%;\n\
}\n\
.line_number{\n\
	padding-left:4px;\n\
	padding-right:4px;\n\
	color:#666;\n\
	text-align:right;\n\
}\n\
.normal{\n\
	white-space: pre-wrap;\n\
	word-break: break-all;\n\
	width:50%%;\n\
}\n\
.gray{\n\
	white-space: pre-wrap;\n\
	word-break: break-all;\n\
	background-color:#ccc;\n\
	width:50%%;\n\
}\n\
.green{\n\
	white-space: pre-wrap;\n\
	word-break: break-all;\n\
	background-color:#9f9;\n\
	width:50%%;\n\
}\n\
.red{\n\
	white-space: pre-wrap;\n\
	word-break: break-all;\n\
	background-color:#fcd8d9;\n\
	width:50%%;\n\
}\n\
	</style>\n\
</head>\n\
<body>\n");
	
	int i,ca,cb;
	char *sa,*sb;
	for(i=0;i<data->list_a.N && i<data->list_b.N;++i)
	{
		ca=data->list_a.A[i];
		cb=data->list_b.A[i];
		sa=data->buf.A+(ca&0xFFFF);
		sb=data->buf.A+(cb&0xFFFF);
		ca>>=16;
		cb>>=16;
		if(ca==0 && strlen(sa)!=0)
			printf("%s<h3>%s</h3>\n<table>\n",i==0?"":"</table>\n",sa);
		else
		{
			if(ca==0 && cb!=0)
				printf("<tr><td class='line_number'></td><td class='gray'>%s</td><td class='line_number'>%d.</td><td class='green'>%s</td></tr>\n",html(sa,buf[0]),cb,html(sb,buf[1]));
			else if(cb==0 && ca!=0)
				printf("<tr><td class='line_number'>%d.</td><td class='red'>%s</td><td class='line_number'></td><td class='gray'>%s</td></tr>\n",ca,html(sa,buf[0]),html(sb,buf[1]));
			else if(strcmp(sa,sb)!=0)
				printf("<tr><td class='line_number'>%d.</td><td class='red'>%s</td><td class='line_number'>%d.</td><td class='green'>%s</td></tr>\n",ca,html(sa,buf[0]),cb,html(sb,buf[1]));
			else
				printf("<tr><td class='line_number'>%d.</td><td class='normal'>%s</td><td class='line_number'>%d.</td><td class='normal'>%s</td></tr>\n",ca,html(sa,buf[0]),cb,html(sb,buf[1]));
		}
	}
	printf("</table>\n</body>\n\
</html>");
}
void proc(char **A,int N)
{
	const char *keys[]={
		"diff --git",
		"---",
		"+++"
	};
	int m=sizeof(keys)/sizeof(*keys);
	int reader=0; //讀取到第幾行
	int i,j;
	int stat=0;
	char *fname_1=NULL,*fname_2=NULL;
	output data;
	output_init(&data);
	for(;reader<N;++reader)
	{
		for(j=0;j<m && strstr(A[reader],keys[j])!=A[reader];++j);
		if(j==0)
			stat=0;
		else if(j==1 && stat==0)
		{
			fprintf(stderr,"f1:%s\n",A[reader]);
			fname_1=getfname(A[reader]+3);
			++stat;
		}
		else if(j==2 && stat==1)
		{
			fprintf(stderr,"f2:%s\n",A[reader]);
			fname_2=getfname(A[reader]+3);
			if(strcmp(fname_1,fname_2)==0)
			{
				reader=f2(A,N,reader+1,fname_1,&data);
			}
		}
	}
	f3(&data);
	output_free(&data);
}

int main(int argc,char* argv[])
{
	char* buf=readData(argc==1?NULL:argv[1]);
	if(buf==NULL)
		return -1;
	int n;
	char **A=convertToArray(buf,&n);
	if(A==NULL)
	{
		free(buf);
		return -2;
	}
	//已讀取 A,n，對此處理
	/*int i;
	for(i=0;i<n;++i)
		printf("[%3d/%d]%s\n",i+1,n,A[i]);*/
	proc(A,n);
	//處理結束
	free(A);
	free(buf);
	return 0;
}
