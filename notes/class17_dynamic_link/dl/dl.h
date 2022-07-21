#define REC_SZ 32
#define DL_MAGIC "\x01\x14\x05\x14"

#ifdef __ASSEMBLER__
  #define DL_HEAD     __hdr: \
                      /* magic */    .ascii DL_MAGIC; \
                      /* file_sz */  .4byte (__end - __hdr); \
                      /* code_off */ .4byte (__code - __hdr)
  #define DL_CODE     .fill REC_SZ - 1, 1, 0; \
                      .align REC_SZ, 0; \
                      __code:
  #define DL_END      __end:

  #define RECORD(sym, off, name) \
    .align REC_SZ, 0; \
    sym .8byte (off); .ascii name

  #define IMPORT(sym) RECORD(sym:,           0, "?" #sym "\0")//创建了一个符号，但地址未知,先填0
  #define EXPORT(sym) RECORD(    , sym - __hdr, "#" #sym "\0")//得到了sym符号的地址
  #define LOAD(lib)   RECORD(    ,           0, "+" lib  "\0")
  #define DSYM(sym)   *sym(%rip)
#else
  #include <stdint.h>

  //自定义的elf文件格式
  struct dl_hdr {
    char magic[4];//magic占4字节
    uint32_t file_sz, code_off;//file_sz和code_off各4字节
  };

  struct symbol {
    int64_t offset;//前8字节是一个offset指针
    char type, name[REC_SZ - sizeof(int64_t) - 1];//一个char字符串，第一个字节是type，后面是符号名
  };
#endif
