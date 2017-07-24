declare_vector(dyBuf,char)
implement_vector(dyBuf,char)
declare_vector(posArray,int)
implement_vector(posArray,int)

typedef struct{
	posArray list_a;
	posArray list_b;
	dyBuf buf;
} output;
void output_init(output *obj)
{
	posArray_init(&(obj->list_a),16);
	posArray_init(&(obj->list_b),16);
	dyBuf_init(&(obj->buf),1024);
	dyBuf_push(&(obj->buf),'\0');
}
void output_free(output *obj)
{
	posArray_free(&(obj->list_a));
	posArray_free(&(obj->list_b));
	dyBuf_free(&(obj->buf));
}
int output_isOK(output *obj)
{
	return (obj->list_a.A!=NULL && obj->list_b.A!=NULL && obj->buf.A!=NULL)?1:0;
}
void output_pushData(output *obj,const char* s,int LRmask,int line_a,int line_b) //bit0:L bit1:R
{
	if(!output_isOK(obj))
	{
		output_free(obj);
		return;
	}
	int n=strlen(s);
	int i,ss;
	if(LRmask&3)
	{
		ss=obj->buf.N;//dyBuf_memPush(&(obj->buf),s,n+1);
		for(i=0;i<n;++i)
		{
			dyBuf_push(&(obj->buf),s[i]);
		}
		dyBuf_push(&(obj->buf),'\0');
	}
	if(LRmask&1) //push to list_a
		posArray_push(&(obj->list_a),ss|line_a<<16);
	if(LRmask&2) //push to list_a
		posArray_push(&(obj->list_b),ss|line_b<<16);
}
void output_pushSep(output *obj,const char* filename)
{
	if(!output_isOK(obj))
	{
		output_free(obj);
		return;
	}
	int ss;
	ss=dyBuf_memPush(&(obj->buf),filename,strlen(filename)+1);
	posArray_push(&(obj->list_a),ss);
	posArray_push(&(obj->list_b),ss);
}
void output_balance(output *obj)
{
	for(;obj->list_a.N<obj->list_b.N;)
		posArray_push(&(obj->list_a),0);
	for(;obj->list_b.N<obj->list_a.N;)
		posArray_push(&(obj->list_b),0);
}
