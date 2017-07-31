#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"

declare_vector(charbuf,char)
implement_vector(charbuf,char)


char* get_exec(const char *cmd,int *r,char* buffer) // 取得命令列的stdout
{//注意：回傳的char*是動態宣告的，要free
	FILE *fp=freopen("tmp.txt","w",stdout);
	if(fp==NULL)
	{
		fprintf(stderr,"NULL");
		return NULL;
	}
	fprintf(stderr,"%s ... ",cmd);
	*r=system(cmd);
	fclose(fp);
	fprintf(stderr,"%d\n",*r);
	
	fp=fopen("tmp.txt","r");
	if(fp==NULL)
	{
		fprintf(stderr,"NULL 2");
		return NULL;
	}
	charbuf buf;
	if(buffer==NULL)
		charbuf_init(&buf,32);
	else
	{
		buf.A=buffer;
		buf.N=0;
		buf.C=strlen(buffer)+1;
	}
	int ch;
	while((ch=fgetc(fp))!=EOF)
		charbuf_push(&buf,(char)ch);
	charbuf_push(&buf,'\0');
	fclose(fp);
	return buf.A;
}

enum{
	GIT_STASH_BIT=1,
	GIT_CHECKOUT_BIT=2
};

int gitStatusFlag=0;
char gitCheckBack[128];

void recovery(int e)
{
	char *s=NULL;
	int k=0;
	if(gitStatusFlag & GIT_CHECKOUT_BIT)
		s=get_exec(gitCheckBack,&k,s);
	if(k==0 && gitStatusFlag & GIT_STASH_BIT)
		s=get_exec("git stash pop",&k,s);
	free(s);
	if(e)
		exit(e);
}
