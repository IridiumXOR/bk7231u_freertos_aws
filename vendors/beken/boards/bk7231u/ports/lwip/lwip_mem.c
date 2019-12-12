#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/sys.h"
#include "str_pub.h"

#if LWIP_STATS
#include "lwip/stats.h"
extern struct stats_ lwip_stats;
#endif

#if defined(LWIP_USING_SYS_MEM)
#include "mem_pub.h"
void mem_init(void)
{
}

void *mem_trim(void *mem, mem_size_t size)
{
    // return rt_realloc(mem, size);
    /* not support trim yet */
    return mem;
}

void *mem_calloc(mem_size_t count, mem_size_t size)
{
    void *p;
    size_t len = count * size;

    p = os_malloc(len);
#if 0//OSMALLOC_STATISTICAL
    if (64 == len)
    {
        bk_printf("mem_calloc=%p\n", p);
        rtos_print_stack(NULL);
    }
#endif
    if (p) os_memset(p, 0, len);

    return p;
}

void *mem_malloc(mem_size_t size)
{
#if 0//OSMALLOC_STATISTICAL
    void *p = os_malloc(size);
    if (64 == size)
    {
        bk_printf("mem_malloc=%p\n", p);
        rtos_print_stack(NULL);
    }
    return p;
#else
    return os_malloc(size);
#endif
}

void  mem_free(void *mem)
{
    os_free(mem);
}
#endif

extern const struct memp_desc* const memp_pools[MEMP_MAX];

#define cmd_printf(...) do{\
                                if (xWriteBufferLen > 0) {\
                                    snprintf(pcWriteBuffer, xWriteBufferLen, __VA_ARGS__);\
                                    xWriteBufferLen-= os_strlen(pcWriteBuffer);\
                                    pcWriteBuffer+= os_strlen(pcWriteBuffer);\
                                }\
                             }while(0)

void memp_dump_Command( char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv )
{
    int i;
	struct memp_desc* tmp;
	
    SYS_ARCH_DECL_PROTECT(old_level);    
    SYS_ARCH_PROTECT(old_level);
    
    cmd_printf("%-16s total used addr       size\r\n", "Name");
    cmd_printf("----------------------------------------------------\r\n");
    for(i=0; i<MEMP_MAX; i++) {
		tmp = (struct memp_desc*)memp_pools[i];
#if defined(LWIP_DEBUG) || MEMP_OVERFLOW_CHECK || LWIP_STATS_DISPLAY
        cmd_printf("%-16s %-5d %-4d 0x%08x %-4d\r\n", 
			tmp->desc, tmp->num, tmp->stats->used,
			tmp->base, tmp->size);
#elif MEMP_STATS && !MEMP_MEM_MALLOC
        cmd_printf("%-5d %-4d 0x%08x %-4d\r\n", 
			tmp->num, tmp->stats->used,
			tmp->base, tmp->size);
#elif !MEMP_MEM_MALLOC
        cmd_printf("%-5d 0x%08x %-4d\r\n", 
			tmp->num, tmp->base, tmp->size);
#endif
    }

#if LWIP_STATS
#if MEM_STATS
	cmd_printf("===== MEM ======\r\n");
	cmd_printf("avail %d, used %d, max %d\r\n", 
					lwip_stats.mem.avail, 
					lwip_stats.mem.used, 
					lwip_stats.mem.max);
#endif /* MEM_STATS */
#endif /* LWIP_STATS */

    SYS_ARCH_UNPROTECT(old_level);
}

void mem_stat()
{
    extern unsigned char _empty_ram;
    extern unsigned char _stack_unused;
#if LWIP_STATS
    bk_printf("memory: free %d min %d total %d\r\n", xPortGetFreeHeapSize(), xPortGetMinimumEverFreeHeapSize(), &_stack_unused - &_empty_ram);
	bk_printf("heap  : used %d max %d total %d\r\n", lwip_stats.mem.used, lwip_stats.mem.max, lwip_stats.mem.avail);
#endif
}
