#include <stdlib.h>

#ifdef _DEBUG
	#include <string.h>
	#include <assert.h>
	#include "crc.h"
	#include "list.h"
	#include "log.h"
#endif

#ifndef NULL
	#define NULL	((void*)0)
#endif

#ifdef _DEBUG

typedef struct _mem_head
{
	unsigned int   len;	// 数据部分长度
	unsigned int   line;
	const char*	   file;
}mem_head;

typedef struct _mem_tail
{
	unsigned int   check[4];
}mem_tail;

#define mem_lock	while ( s_lock ) ;\
							s_lock = 1;

#define mem_unlock	s_lock = 0; 

#define recrc16(node)	do{\
							if ( node )\
							{\
								mem_head* _mh = (mem_head*)((node)+1);\
								_mh->crc_head = crc16(0, (unsigned char*)node, MEM_HEAD_CRC_SIZE );\
							}\
						}while(0);

#define mem_list_add_tail(head,node)	list_add_tail(head,node);

#define mem_list_del( head,node )		list_del(head,node);

/**
  内存布局: 链表结构
  			数据部分长度(4字节) 头部crc(2字节) 尾部crc(2字节)
			数据部分(n 字节) 
			文件名长度(4字节) 行(4字节) 文件名( 直到 \0 )
**/
#define MEM_HEAD_SIZE		( sizeof(list_node) + sizeof(mem_head) + sizeof(mem_tail) )
#define MEM_HEAD_CRC_SIZE	( sizeof(list_node) + sizeof(unsigned int))
#define MEM_TAIL_CRC_SIZE	( sizeof(mem_tail) )

#define MEM_SIZE_CAL(n,align)	( (n+align-1) & (~(align-1)) )
#define MEM_SIZE(n)		((n+7) & (~7))
#define MEM_ALIGN_VALUE	0x96

#define MEM_PRINT_BUF_SIZE	(1024)
#define MEM_PRINT_DATA_SIZE	(16)

static LOG_V_MODULE(log_cbv, "memory", g_global_log_output, 0);

static list_head	s_head = {NULL};
static volatile int s_lock = 0;

void* 	__dbg_alloc_( unsigned int n, const char* file, unsigned int line )
{
	list_node* pnode = NULL;
	mem_head*  pmhead = NULL;
	mem_tail*  pmtail = NULL;
	
	char* m 	= NULL;
	char* p     = NULL;
	unsigned int len = MEM_SIZE(n);
	m = (char*)malloc(MEM_HEAD_SIZE + len);
	
	if (m)
	{	
		pnode = (list_node*)m;
		pmhead = (mem_head*)(pnode + 1);
		p = (char*)(pmhead + 1);
		pmtail = (mem_tail*)(p + len);

		mem_lock;
		mem_list_add_tail(&s_head, pnode );
		mem_unlock;
		
		pmhead->len = n;
		pmhead->file = file;
		pmhead->line = line;
		memset(p + n, MEM_ALIGN_VALUE , len - n);
		memset(pmtail, MEM_ALIGN_VALUE, sizeof(mem_tail));
		
		return p;
	}
	return NULL;
}

static int __dbg_mem_check(const char* ptr, char value, int size)
{
	int i;
	for (i = 0; i < size; ++i){
		if (ptr[i] != value)
			return 0;
	}
	return 1;
}	

void __dbg_free_( void* ptr )
{
	char *p = (char*)ptr;
	list_node* pnode = NULL;
	mem_head*  pmhead = NULL;
	mem_tail*  pmtail = NULL;

	pmhead = (mem_head*)(p - sizeof(mem_head));
	pnode = (list_node*)pmhead - 1;
	pmtail = (mem_tail*)(p + MEM_SIZE(pmhead->len));
	
	if (!__dbg_mem_check(p+pmhead->len, MEM_ALIGN_VALUE, MEM_SIZE(pmhead->len)-pmhead->len+sizeof(mem_tail)))
		assert(0);

	mem_lock;
	mem_list_del(&s_head, pnode );
	mem_unlock;
	
	free(pnode);
}

static int __dbg_print_node(const void* p)
{
	char str[MEM_PRINT_BUF_SIZE];
	char* pdata = NULL;
	unsigned int str_len = 0;
	unsigned int len = 0;
	mem_head* pmhead = (mem_head*)((list_node*)p + 1);
	mem_tail* pmtail = NULL;
	int flag = 0;

	pdata = (char*)(pmhead + 1);
	len = MEM_SIZE(pmhead->len);
	pmtail = (mem_tail*)( pdata + len );

	if (!__dbg_mem_check(pdata+pmhead->len, MEM_ALIGN_VALUE, len - pmhead->len + sizeof(mem_tail))){
		flag = -1;
	}

	if (!flag)
		 LOG_CB_V(log_cbv, LOG_DEBUG, "memory check OK   : %s(%d) %p->%p %d",
		 			pmhead->file, pmhead->line, pdata, pdata+pmhead->len, pmhead->len);
	else
		 LOG_CB_V(log_cbv, LOG_DEBUG, "memory check ERROR: %s(%d) %p->%p %d", 
		 			pmhead->file, pmhead->line, pdata, pdata+pmhead->len, pmhead->len);
	return flag;
}

void	__dbg_print_()
{
	unsigned int count = 0;
	list_node* pnode;

	LOG_CB_V ( log_cbv, LOG_DEBUG, "==================== Memory Dump =========================" );
	mem_lock;
	for ( pnode = s_head.head; pnode ; pnode = pnode->next )
	{
		count++;
		if ( -1 == __dbg_print_node( pnode ))
		{
			LOG_CB_V ( log_cbv, LOG_DEBUG, "内存链表被破坏,退出打印 > ERROR" );
			break;
		}
	}
	
	mem_unlock;
	LOG_CB_V ( log_cbv, LOG_DEBUG,  "==================== Dumped count=%d =====================", count );
}

void*	__dbg_binit( unsigned int n, const char* file, unsigned int line )
{
	return NULL;
}
void	__dbg_bdelete( void* h )
{
	assert( NULL != h );
}

void*	__dbg_balloc( void* h, unsigned int n )
{
	assert( NULL != h);
	return NULL;
}

void	__dbg_bfree( void* h, void* p )
{
	assert( NULL != h && NULL != p);
}

#else

void* __alloc( unsigned int n )
{
	return malloc(n);
}

void  __free( void* p )
{
	return free(p);
}

void*	__binit( unsigned int n )
{
	return NULL;
}

void	__bdelete( void* h )
{
	;
}

void*	__balloc( void* h, void* p)
{
	return NULL;
}

void	__bfree( void* h, void* p)
{
	;
}

#endif