#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int isUtf8(const char* str)
{
	int n;
	unsigned char *p=(unsigned char *)str;
	for(p=(unsigned char *)str;*p!='\0';++p)
	{
		if(*p<0x80)
			continue;
		n=(*p>>5)==6?2:
		  ((*p>>4)==14?3:
		  ((*p>>3)==30?4:-1));
		if(n<0)
			return 0;
		for(;--n;)
			if((*(++p)>>6)!=2)
				return 0;
	}
	return 1;
}
int isBig5(const char* str)
{//「高位字節」使用了0x81-0xFE，「低位字節」使用了0x40-0x7E，及0xA1-0xFE。
	unsigned char *p=(unsigned char *)str;
	for(;*p!='\0';++p)
	{
		if(*p<0x80)
			continue;
		if(*p<0x81||*p>0xFE)
			return 0;
		++p;
		if(*p<0x40||(0x7E<*p && *p<0xA1)||*p>0xFE)
			return 0;
	}
	return 1;
}

size_t codeConvert(const char* from, //來源編碼
                   const char* to,   //目標編碼
                   const char* src,  //來源字串
                   char* dest,       //目標字串
                   size_t sz)        //目標字串空間大小
{
	size_t sz_src=strlen(src)+1,k;
	char *p=(char*)src;
	iconv_t cd=iconv_open(to,from);
	if(cd==(iconv_t)(-1))
	{
		fprintf(stderr,"iconv init error\n");
		return (size_t)(-1);
	}
	k=iconv(cd,&p,&sz_src,&dest,&sz);
	iconv_close(cd);
	return k;
}
