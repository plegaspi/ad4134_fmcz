#ifndef SHARED_ADDRESSES_H
#define SHARED_ADDRESSES_H

#include <stdint.h>
#include "parameters.h"

/* ---- Shared DDR region you reserved ---- */
#ifndef SHM_BASE
#define SHM_BASE 0x1F800000u
#endif
#ifndef SHM_SIZE
#define SHM_SIZE 0x00800000u   /* 8 MiB */
#endif

/* ---- Frame size that DMA writes & you send ----
 * Prefer the actual words-per-frame over a giant capacity buffer.
 */
#ifndef ADC_WORDS
#define ADC_WORDS   (AD4134_FMC_CH_NO * AD4134_FMC_SAMPLE_NO)
#endif
#ifndef ADC_BYTES
#define ADC_BYTES   ((uint32_t)(ADC_WORDS * sizeof(uint32_t)))
#endif

/* ---- Alignment helpers (DMA-friendly) ---- */
#define ALIGN_UP(x,a)   ( ((uint32_t)(x) + ((uint32_t)(a)-1u)) & ~((uint32_t)(a)-1u) )
#define A1024(x)        ALIGN_UP((x), 1024u)

/* ---- Layout: buf0 | buf1 | selectors/flags ---- */
#define OFFS_BUF0   A1024(0x00000000u)
#define OFFS_BUF1   A1024(OFFS_BUF0 + ADC_BYTES)
#define OFFS_SEL    A1024(OFFS_BUF1 + ADC_BYTES)              /* uint32_t: current DMA target (0/1) */

#ifdef USE_PER_BUFFER_FLAGS
  #define OFFS_RDY0  A1024(OFFS_SEL + sizeof(uint32_t))       /* uint32_t */
  #define OFFS_RDY1  A1024(OFFS_RDY0 + sizeof(uint32_t))      /* uint32_t */
  #define LAST_OFFS  (OFFS_RDY1 + sizeof(uint32_t))
#else
  #define OFFS_READY A1024(OFFS_SEL + sizeof(uint32_t))       /* uint32_t: 0/1 single ready flag */
  #define LAST_OFFS  (OFFS_READY + sizeof(uint32_t))
#endif

/* Bounds check (C11) */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert((uint64_t)LAST_OFFS <= (uint64_t)SHM_SIZE, "Shared layout exceeds SHM_SIZE");
#endif

/* ---- Typed lvalues/pointers into shared DDR ---- */
#define PTR_U32(offs)   ((volatile uint32_t*)(uintptr_t)(SHM_BASE + (uint32_t)(offs)))

#define BUF0_U32        PTR_U32(OFFS_BUF0)
#define BUF1_U32        PTR_U32(OFFS_BUF1)
#define BUFFER_SEL      (*(volatile uint32_t*)(uintptr_t)(SHM_BASE + OFFS_SEL))  /* 0/1 */

#ifndef USE_PER_BUFFER_FLAGS
  /* Single ready-bit scheme */
  #define BUFFER_READY  (*(volatile uint32_t*)(uintptr_t)(SHM_BASE + OFFS_READY)) /* 0/1 */
#else
  /* Per-buffer ready flags */
  #define BUF_READY0    (*(volatile uint32_t*)(uintptr_t)(SHM_BASE + OFFS_RDY0))  /* 0/1 */
  #define BUF_READY1    (*(volatile uint32_t*)(uintptr_t)(SHM_BASE + OFFS_RDY1))  /* 0/1 */
  #define DMA_SEL       BUFFER_SEL  /* alias: which buffer DMA targets next */
#endif

/* ---- Small helpers ---- */
static inline volatile uint32_t* shm_buf_by_sel(uint32_t sel) {
    return sel ? BUF1_U32 : BUF0_U32;
}
static inline uint32_t shm_other(uint32_t sel) { return sel ^ 1u; }

#endif
