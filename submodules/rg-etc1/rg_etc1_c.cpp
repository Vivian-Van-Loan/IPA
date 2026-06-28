#include "rg_etc1_c.h"

#include "rg-etc1-true/rg_etc1.h"

constexpr rg_etc1::etc1_quality QUAL_MAP[] = {rg_etc1::cLowQuality, rg_etc1::cMediumQuality, rg_etc1::cHighQuality};

#ifdef __cplusplus
extern "C" {
#endif

bool unpack_etc1_block(const void *pETC1_block, unsigned int* pDst_pixels_rgba) {
    return rg_etc1::unpack_etc1_block(pETC1_block, pDst_pixels_rgba, false);
}

bool unpack_etc1_block_preserve_alpha(const void *pETC1_block, unsigned int* pDst_pixels_rgba) {
    return rg_etc1::unpack_etc1_block(pETC1_block, pDst_pixels_rgba, true);
}

void pack_etc1_block_init() {
    rg_etc1::pack_etc1_block_init();
}

unsigned int pack_etc1_block(void* pETC1_block, const unsigned int* pSrc_pixels_rgba, etc1_quality_t qual, bool dithering) {
    rg_etc1::etc1_pack_params params;
    params.m_dithering = dithering;
    params.m_quality = QUAL_MAP[qual];
    return rg_etc1::pack_etc1_block(pETC1_block, pSrc_pixels_rgba, params);
}

#ifdef __cplusplus
}
#endif
