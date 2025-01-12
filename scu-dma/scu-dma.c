#include <yaul.h>

#define BUF_SIZE 4096

static smpc_peripheral_digital_t _digital;

__aligned(4) uint8_t src_buf[BUF_SIZE];
__aligned(4) uint8_t dst_buf[BUF_SIZE];

typedef enum Region
{
    RGN_WRAM,
    RGN_BBUS,
} Region;

static void _vblank_out_handler(void *work);

void run_test(int level, int xferLen,
              Region srcRegion, Region dstRegion,
              scu_dma_stride_t srcStride, scu_dma_stride_t dstStride,
              int srcOffset, int dstOffset,
              int outputOffset);

int get_stride_bytes(scu_dma_stride_t stride)
{
    switch (stride)
    {
    case SCU_DMA_STRIDE_0_BYTES:
        return 0;
    case SCU_DMA_STRIDE_2_BYTES:
        return 2;
    case SCU_DMA_STRIDE_4_BYTES:
        return 4;
    case SCU_DMA_STRIDE_8_BYTES:
        return 8;
    case SCU_DMA_STRIDE_16_BYTES:
        return 16;
    case SCU_DMA_STRIDE_32_BYTES:
        return 32;
    case SCU_DMA_STRIDE_64_BYTES:
        return 64;
    case SCU_DMA_STRIDE_128_BYTES:
        return 128;
    default:
        assert(false);
    }
}

int main(void)
{
    dbgio_init();
    dbgio_dev_default_init(DBGIO_DEV_VDP2_ASYNC);
    dbgio_dev_font_load();

    const uintptr_t wramSrc = CPU_CACHE_THROUGH | (uintptr_t)src_buf;
    const uintptr_t bbusSrc = CPU_CACHE_THROUGH | VDP1_VRAM(BUF_SIZE);

    for (int i = 0; i < BUF_SIZE; i++)
    {
        *(char *)(wramSrc + i) = i;
        *(char *)(bbusSrc + i) = i;
    }

    // TODO: test CS2 area
    //   reads: CDB read sector transfer
    //   writes: CDB put sector transfer
    // TODO: large transfers

    scu_dma_stride_t srcStride = SCU_DMA_STRIDE_4_BYTES;
    scu_dma_stride_t dstStride = SCU_DMA_STRIDE_4_BYTES;
    int srcOffset = 0;
    int dstOffset = 0;

    int xferLen = 12;
    int outputOffset = 0;

    int select = 0;

    while (true)
    {
        smpc_peripheral_process();
        smpc_peripheral_digital_port(1, &_digital);

        if (_digital.held.button.down)
        {
            if (select == 0)
                srcStride ^= SCU_DMA_STRIDE_4_BYTES;
            else
                dstStride = (dstStride == SCU_DMA_STRIDE_0_BYTES) ? SCU_DMA_STRIDE_128_BYTES : dstStride - 1;
        }

        if (_digital.held.button.up)
        {
            if (select == 0)
                srcStride ^= SCU_DMA_STRIDE_4_BYTES;
            else
                dstStride = (dstStride == SCU_DMA_STRIDE_128_BYTES) ? SCU_DMA_STRIDE_0_BYTES : dstStride + 1;
        }

        if (_digital.held.button.left)
        {
            if (select == 0)
                srcOffset = (srcOffset == 0) ? 3 : srcOffset - 1;
            else
                dstOffset = (dstOffset == 0) ? 3 : dstOffset - 1;
        }

        if (_digital.held.button.right)
        {
            if (select == 0)
                srcOffset = (srcOffset == 3) ? 0 : srcOffset + 1;
            else
                dstOffset = (dstOffset == 3) ? 0 : dstOffset + 1;
        }

        if (_digital.held.button.b && xferLen > 1)
            xferLen--;

        if (_digital.held.button.c && xferLen < 32)
            xferLen++;

        if (_digital.held.button.x)
            outputOffset = (outputOffset >= 12) ? outputOffset - 12 : 0;

        if (_digital.held.button.l && outputOffset > 0)
            outputOffset--;

        if (_digital.held.button.y)
            outputOffset = 0;

        if (_digital.held.button.r && outputOffset < BUF_SIZE - 12)
            outputOffset++;

        if (_digital.held.button.z && outputOffset <= BUF_SIZE - 12)
            outputOffset = (outputOffset <= BUF_SIZE - 24) ? outputOffset + 12 : BUF_SIZE - 12;

        if (_digital.held.button.a)
            select ^= 1;

        int srcStrideBytes = get_stride_bytes(srcStride);
        int dstStrideBytes = get_stride_bytes(dstStride);

        dbgio_printf("[H[2J");
        if (select == 0)
        {
            dbgio_puts("        [A] <src> dst\n");
        }
        else
        {
            dbgio_puts("        [A]  src <dst>\n");
        }
        dbgio_printf("[\x1e\x1f] Stride  %3d  %3d bytes\n", srcStrideBytes, dstStrideBytes);
        dbgio_printf("[\x11\x10] Offset  %3d  %3d bytes\n", srcOffset, dstOffset);
        dbgio_printf("[BC] Transfer length  %3d bytes\n", xferLen);

        dbgio_printf("lv src-dst [XL] output offset=%-4d  [RZ]\n", outputOffset);
        for (int level = 0; level <= 2; level++)
        {
            // same-bus transfers are ignored
            // run_test(level, xferLen, RGN_WRAM, RGN_WRAM, srcStride, dstStride, srcOffset, dstOffset, outputOffset);
            run_test(level, xferLen, RGN_WRAM, RGN_BBUS, srcStride, dstStride, srcOffset, dstOffset, outputOffset);
            run_test(level, xferLen, RGN_BBUS, RGN_WRAM, srcStride, dstStride, srcOffset, dstOffset, outputOffset);
            // run_test(level, xferLen, RGN_BBUS, RGN_BBUS, srcStride, dstStride, srcOffset, dstOffset, outputOffset);
        }

        dbgio_flush();

        vdp2_sync();
        vdp2_sync_wait();
    }

    return 0;
}

void user_init(void)
{
    vdp2_tvmd_display_res_set(VDP2_TVMD_INTERLACE_NONE, VDP2_TVMD_HORZ_NORMAL_A,
                              VDP2_TVMD_VERT_224);

    vdp2_scrn_back_color_set(VDP2_VRAM_ADDR(3, 0x01FFFE),
                             RGB1555(1, 0, 3, 15));

    vdp_sync_vblank_out_set(_vblank_out_handler, NULL);

    vdp2_tvmd_display_set();

    smpc_peripheral_init();
}

void run_test(int level, int xferLen,
              Region srcRegion, Region dstRegion,
              scu_dma_stride_t srcStride, scu_dma_stride_t dstStride,
              int srcOffset, int dstOffset,
              int outputOffset)
{
    const uintptr_t wramSrc = CPU_CACHE_THROUGH | (uintptr_t)src_buf;
    const uintptr_t wramDst = CPU_CACHE_THROUGH | (uintptr_t)dst_buf;
    const uintptr_t bbusSrc = CPU_CACHE_THROUGH | VDP1_VRAM(BUF_SIZE);
    const uintptr_t bbusDst = CPU_CACHE_THROUGH | VDP1_VRAM(0);

    const uint8_t fillData = 0xFF;

    scu_dma_handle_t dma_handle = {
        .dnc = xferLen,
        .dnad = dstStride,
        .dnmd = 0x00010100};

    if (srcStride != SCU_DMA_STRIDE_0_BYTES)
        dma_handle.dnad |= 1 << 8;

    uintptr_t srcBuf;
    uintptr_t dstBuf;
    const char *srcName;
    const char *dstName;

    switch (srcRegion)
    {
    case RGN_WRAM:
        srcBuf = wramSrc;
        srcName = "WRAM";
        break;
    case RGN_BBUS:
        srcBuf = bbusSrc;
        srcName = "BBus";
        break;
    default:
        assert(false);
    }

    switch (dstRegion)
    {
    case RGN_WRAM:
        dstBuf = wramDst;
        dstName = "WRAM";
        break;
    case RGN_BBUS:
        dstBuf = bbusDst;
        dstName = "BBus";
        break;
    default:
        assert(false);
    }

    dma_handle.dnr = srcBuf + srcOffset;
    dma_handle.dnw = dstBuf + dstOffset;

    memset((void *)dstBuf, fillData, BUF_SIZE);

    scu_dma_config_set(level, SCU_DMA_START_FACTOR_ENABLE, &dma_handle, NULL);
    scu_dma_level_end_set(level, NULL, NULL);
    scu_dma_level_fast_start(level);
    scu_dma_transfer_wait(level);

    cpu_cache_area_purge((void *)dstBuf, xferLen);

    dbgio_printf("%d %s-%s ", level, srcName, dstName);
    dbgio_puts(outputOffset > 0 ? "\x11" : " ");
    for (int i = 0; i < 12; i++)
        dbgio_printf("%02X", *(uint8_t *)(dstBuf + i + outputOffset));
    dbgio_puts(outputOffset < BUF_SIZE - 12 ? "\x10" : " ");
    dbgio_puts("\n");
}

static void _vblank_out_handler(void *work __unused)
{
    smpc_peripheral_intback_issue();
}
