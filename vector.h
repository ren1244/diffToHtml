#ifndef CVECTOR_H
#include <stdlib.h>
#include <string.h>
#define CVECTOR_H

#define declare_vector(cvector_struct_name,cvector_type) \
typedef struct\
{\
	cvector_type *A;\
	int N;\
	int C;\
} cvector_struct_name; \
void cvector_struct_name##_init(cvector_struct_name* v,int sz); \
cvector_struct_name* cvector_struct_name##_push(cvector_struct_name* v,cvector_type x); \
void cvector_struct_name##_free(cvector_struct_name* v);\
cvector_type* cvector_struct_name##_realloc(cvector_struct_name* v,int sz);\
int cvector_struct_name##_memPush(cvector_struct_name* buf,const void* src,int sz);



#define implement_vector(cvector_struct_name,cvector_type) \
void cvector_struct_name##_init(cvector_struct_name* v,int sz)\
{\
	v->A=malloc(sz*sizeof(cvector_type));\
	v->N=0;\
	v->C=sz;\
}\
 \
cvector_struct_name* cvector_struct_name##_push(cvector_struct_name* v,cvector_type x)\
{\
	if(v->A==NULL)\
		return NULL;\
	cvector_type *p;\
	if(v->N >= v->C)\
		if(cvector_struct_name##_realloc(v,v->C+1)==NULL)\
			return NULL;\
	v->A[v->N++]=x;\
	return v;\
}\
 \
void cvector_struct_name##_free(cvector_struct_name* v)\
{\
	free(v->A);\
	v->A=NULL;\
} \
\
cvector_type* cvector_struct_name##_realloc(cvector_struct_name* v,int sz)\
{\
	cvector_type *p;\
	for(;v->C<sz;v->C*=2);\
	if(v->A==NULL)\
		return NULL;\
	p=realloc(v->A,v->C*sizeof(cvector_type));\
	if(p==NULL)\
	{\
		free(v->A);\
		v->A=NULL;\
		return NULL;\
	}\
	return v->A=p;\
}\
 \
int cvector_struct_name##_memPush(cvector_struct_name* buf,const void* src,int sz)\
{\
	int i;\
	int ss=buf->N;\
\
	cvector_struct_name##_realloc(buf,buf->N+sz);\
	if(buf->A==NULL)\
		return -1;\
\
	memcpy(buf->A+buf->N,src,sz);\
	buf->N+=sz;\
	return ss;\
}


#endif
