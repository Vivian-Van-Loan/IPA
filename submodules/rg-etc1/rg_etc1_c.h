#ifndef MATRIX_CLIENT_RG_ETC1_C_H
#define MATRIX_CLIENT_RG_ETC1_C_H

typedef enum etc1_quality_t {
    ETC_LOW_QUALITY,
    ETC_MEDIUM_QUALITY,
    ETC_HIGH_QUALITY,
} etc1_quality_t;

#ifdef __cplusplus
extern "C" {
#endif
    bool unpack_etc1_block(const void* pETC1_block, unsigned int* pDst_pixels_rgba);
    bool unpack_etc1_block_preserve_alpha(const void *pETC1_block, unsigned int* pDst_pixels_rgba);
    void pack_etc1_block_init();
    unsigned int pack_etc1_block(void* pETC1_block, const unsigned int* pSrc_pixels_rgba, etc1_quality_t qual, bool dithering);
#ifdef __cplusplus
}
#endif

#endif //MATRIX_CLIENT_RG_ETC1_C_H
