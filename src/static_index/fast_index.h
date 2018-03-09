/*
  Fast Architecture Sensitive Tree layout for binary search trees
  (Kim et. al, SIGMOD 2010)

  implementation by Viktor Leis, TUM, 2012

  notes:
  -keys are 4 byte integers
  -SSE instructions are used for comparisons
  -huge memory pages (2MB)
  -page blocks store 4 levels of cacheline blocks
  -cacheline blocks store 15 keys and are 64-byte aligned
  -the parameter K results in a tree size of (2^(16+K*4))
 */

/*
  modified by Yingjun Wu, IBM Almaden Research Center, 2018.
*/

#include <sys/mman.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>
#include <iostream>
#include <emmintrin.h>
#include <cassert>
#include <string.h>
#include <algorithm>

const unsigned K=2;

struct LeafEntry {
   int32_t key;
   uint64_t value;
};

void* malloc_huge(size_t size) {
   void* p=mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
   madvise(p,size,MADV_HUGEPAGE);
   return p;
}

inline unsigned pow16(unsigned exponent) {
   // 16^exponent
   return 1<<(exponent<<2);
}

inline unsigned median(unsigned i,unsigned j) {
   return i+(j-1-i)/2;
}

inline void storeSIMDblock(int32_t v[],unsigned k,LeafEntry l[],unsigned i,unsigned j) {
   unsigned m=median(i,j);
   v[k+0]=l[m].key;
   v[k+1]=l[median(i,m)].key;
   v[k+2]=l[median(1+m,j)].key;
}

inline unsigned storeCachelineBlock(int32_t v[],unsigned k,LeafEntry l[],unsigned i,unsigned j) {
   storeSIMDblock(v,k+3*0,l,i,j);
   unsigned m=median(i,j);
   storeSIMDblock(v,k+3*1,l,i,median(i,m));
   storeSIMDblock(v,k+3*2,l,median(i,m)+1,m);
   storeSIMDblock(v,k+3*3,l,m+1,median(m+1,j));
   storeSIMDblock(v,k+3*4,l,median(m+1,j)+1,j);
   return k+16;
}

unsigned storeFASTpage(int32_t v[],unsigned offset,LeafEntry l[],unsigned i,unsigned j,unsigned levels) {
   for (unsigned level=0;level<levels;level++) {
      unsigned chunk=(j-i)/pow16(level);
      for (unsigned cl=0;cl<pow16(level);cl++)
         offset=storeCachelineBlock(v,offset,l,i+cl*chunk,i+(cl+1)*chunk);
   }
   return offset;
}

int32_t* buildFAST(LeafEntry l[],unsigned len) {
   // create array of appropriate size
   unsigned n=0;
   for (unsigned i=0; i<K+4; i++)
      n+=pow16(i);
   n=n*64/4;
   int32_t* v=(int32_t*)malloc_huge(sizeof(int32_t)*n);

   // build FAST
   unsigned offset=storeFASTpage(v,0,l,0,len,4);
   unsigned chunk=len/(1<<16);
   for (unsigned i=0;i<(1<<16);i++)
      offset=storeFASTpage(v,offset,l,i*chunk,(i+1)*chunk,K);
   assert(offset==n);

   return v;
}

inline unsigned maskToIndex(unsigned bitmask) {
   static unsigned table[8]={0,9,1,2,9,9,9,3};
   return table[bitmask&7];
}

unsigned scale=0;

unsigned search(int32_t v[],int32_t key_q) {
   __m128i xmm_key_q=_mm_set1_epi32(key_q);

   unsigned page_offset=0;
   unsigned level_offset=0;

   // first page
   for (unsigned cl_level=1; cl_level<=4; cl_level++) {
      // first SIMD block
      __m128i xmm_tree=_mm_loadu_si128((__m128i*) (v+page_offset+level_offset*16));
      __m128i xmm_mask=_mm_cmpgt_epi32(xmm_key_q,xmm_tree);
      unsigned index=_mm_movemask_ps(_mm_castsi128_ps(xmm_mask));
      unsigned child_index=maskToIndex(index);

      // second SIMD block
      xmm_tree=_mm_loadu_si128((__m128i*) (v+page_offset+level_offset*16+3+3*child_index));
      xmm_mask=_mm_cmpgt_epi32(xmm_key_q,xmm_tree);
      index=_mm_movemask_ps(_mm_castsi128_ps(xmm_mask));

      unsigned cache_offset=child_index*4 + maskToIndex(index);
      level_offset=level_offset*16 + cache_offset;
      page_offset+=pow16(cl_level);
   }

   unsigned pos=level_offset;
   unsigned offset=69904+level_offset*scale;
   page_offset=0;
   level_offset=0;

   // second page
   for (unsigned cl_level=1; cl_level<=K; cl_level++) {
      // first SIMD block
      __m128i xmm_tree=_mm_loadu_si128((__m128i*) (v+offset+page_offset+level_offset*16));
      __m128i xmm_mask=_mm_cmpgt_epi32(xmm_key_q,xmm_tree);
      unsigned index=_mm_movemask_ps(_mm_castsi128_ps(xmm_mask));
      unsigned child_index=maskToIndex(index);

      // second SIMD block
      xmm_tree=_mm_loadu_si128((__m128i*) (v+offset+page_offset+level_offset*16+3+3*child_index));
      xmm_mask=_mm_cmpgt_epi32(xmm_key_q,xmm_tree);
      index=_mm_movemask_ps(_mm_castsi128_ps(xmm_mask));

      unsigned cache_offset=child_index*4 + maskToIndex(index);
      level_offset=level_offset*16 + cache_offset;
      page_offset+=pow16(cl_level);
   }

   return (pos<<(K*4))|level_offset;
}

static double gettime(void) {
   struct timeval now_tv;
   gettimeofday (&now_tv,NULL);
   return ((double)now_tv.tv_sec) + ((double)now_tv.tv_usec)/1000000.0;
}

// int main(int argc,char** argv) {
//    unsigned n=(1<<(16+(K*4)));

//    for (unsigned i=0;i<K;i++)
//       scale+=pow16(i);
//    scale*=16;

//    int32_t* keys=(int32_t*)malloc_huge(sizeof(int32_t)*(n));
//    LeafEntry* leaves=new LeafEntry[n];
//    for (unsigned i=0;i<n;i++) {
//       keys[i]=i;
//       leaves[i].value=i;
//       leaves[i].key=i;
//    }

//    int32_t* fast=buildFAST(leaves,n);
//    std::random_shuffle(keys,keys+n);
//    unsigned repeat=n>10000000 ? 1 : 10000000/n;

//    double start=gettime();
//    for (unsigned r=0; r<repeat; r++)
//       for (unsigned i=0;i<n;i++)
//          assert(search(fast,keys[i])+1);
//    std::cout << "searchnoleaf:" << (n*repeat/1000000.0)/(gettime()-start) << std::endl;

//    start=gettime();
//    for (unsigned r=0; r<repeat; r++)
//       for (unsigned i=0;i<n;i++)
//          assert(leaves[search(fast,keys[i])].key==keys[i]);
//    std::cout << "search:" << (n*repeat/1000000.0)/(gettime()-start) << std::endl;

//    return 0;
// }
