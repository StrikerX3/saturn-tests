#include <yaul.h>

#define BUF_SIZE 256
#define XFER_SIZE 12

uint8_t src_buf[BUF_SIZE];
uint8_t dst_buf[BUF_SIZE];

int main(void)
{
    dbgio_init();
    dbgio_dev_default_init(DBGIO_DEV_VDP2_ASYNC);
    dbgio_dev_font_load();

    const uintptr_t wramSrc = src_buf;
    const uintptr_t wramDst = dst_buf;
    const uintptr_t bbusSrc = CPU_CACHE_THROUGH | VDP1_VRAM(BUF_SIZE);
    const uintptr_t bbusDst = CPU_CACHE_THROUGH | VDP1_VRAM(0);

    const uint8_t fillData = 0xFF;

    for (int i = 0; i < BUF_SIZE; i++)
    {
        *(char *)(wramSrc + i) = i;
        *(char *)(bbusSrc + i) = i;
    }

    // TODO: test different strides
    //   reads: 0 or 4 bytes
    //   writes: 0, 2, 4, 8, 16, 32, 64 or 128 bytes
    // TODO: unaligned reads and/or writes

    scu_dma_handle_t dma_handle_no_inc = {
        //.dnr = CPU_CACHE_THROUGH | (uintptr_t)src,
        //.dnw = (uintptr_t)dst,
        .dnc = XFER_SIZE,
        .dnad = SCU_DMA_STRIDE_0_BYTES | (SCU_DMA_STRIDE_0_BYTES << 8),
        .dnmd = 0x00010100};

    for (int dmach = 0; dmach <= 2; dmach++)
    {
        dbgio_printf("DMA level %d\n", dmach);

        // WRAM->WRAM
        memset(wramDst, fillData, BUF_SIZE);
        scu_dma_transfer(dmach, wramDst, wramSrc, XFER_SIZE);
        scu_dma_transfer_wait(dmach);
        cpu_cache_area_purge(wramDst, XFER_SIZE);

        dbgio_puts("WRAM->WRAM +4: ");
        for (int i = 0; i < XFER_SIZE; i++)
        {
            dbgio_printf("%02X", *(uint8_t *)(wramDst + i));
        }
        dbgio_puts("\n");

        dbgio_flush();

        vdp2_sync();
        vdp2_sync_wait();

        // WRAM->BBus
        memset(bbusDst, fillData, BUF_SIZE);
        scu_dma_transfer(dmach, bbusDst, bbusSrc, XFER_SIZE);
        scu_dma_transfer_wait(dmach);
        cpu_cache_area_purge(bbusDst, XFER_SIZE);

        dbgio_puts("WRAM->BBus +4: ");
        for (int i = 0; i < XFER_SIZE; i++)
        {
            dbgio_printf("%02X", *(uint8_t *)(bbusDst + i));
        }
        dbgio_puts("\n");

        dbgio_flush();

        vdp2_sync();
        vdp2_sync_wait();

        // WRAM->CS2
        // TODO: CDB put sector transfer from WRAM

        // BBus->WRAM
        memset(wramDst, fillData, BUF_SIZE);
        scu_dma_transfer(dmach, wramDst, bbusSrc, XFER_SIZE);
        scu_dma_transfer_wait(dmach);
        cpu_cache_area_purge(wramDst, XFER_SIZE);

        dbgio_puts("BBus->WRAM +4: ");
        for (int i = 0; i < XFER_SIZE; i++)
        {
            dbgio_printf("%02X", *(uint8_t *)(wramDst + i));
        }
        dbgio_puts("\n");

        dbgio_flush();

        vdp2_sync();
        vdp2_sync_wait();

        // BBus->BBus
        memset(bbusDst, fillData, BUF_SIZE);
        scu_dma_transfer(dmach, bbusDst, bbusSrc, XFER_SIZE);
        scu_dma_transfer_wait(dmach);
        cpu_cache_area_purge(bbusDst, XFER_SIZE);

        dbgio_puts("BBus->BBus +4: ");
        for (int i = 0; i < XFER_SIZE; i++)
        {
            dbgio_printf("%02X", *(uint8_t *)(bbusDst + i));
        }
        dbgio_puts("\n");

        dbgio_flush();

        vdp2_sync();
        vdp2_sync_wait();

        // BBus->CS2
        // TODO: CDB put sector transfer from VDP1/2 VRAM

        // CS2->WRAM
        // TODO: CDB transfer to WRAM buffer

        // CS2->BBus
        // TODO: CDB transfer to VDP1/2 VRAM

        // CS2->CS2
        // TODO: CDB put sector transfer from CDB regs (unpredictable/inconsistent)

        // -----------------------------------

        // WRAM->WRAM
        memset(wramDst, fillData, BUF_SIZE);
        dma_handle_no_inc.dnr = wramSrc;
        dma_handle_no_inc.dnw = wramDst;
        scu_dma_config_set(dmach, SCU_DMA_START_FACTOR_ENABLE, &dma_handle_no_inc, NULL);
        scu_dma_level_end_set(dmach, NULL, NULL);
        scu_dma_level_fast_start(dmach);
        scu_dma_transfer_wait(dmach);
        cpu_cache_area_purge(wramDst, XFER_SIZE);
        dbgio_puts("WRAM->WRAM +0: ");
        for (int i = 0; i < XFER_SIZE; i++)
        {
            dbgio_printf("%02X", *(uint8_t *)(wramDst + i));
        }
        dbgio_puts("\n");

        dbgio_flush();

        vdp2_sync();
        vdp2_sync_wait();

        // WRAM->BBus
        memset(bbusDst, fillData, BUF_SIZE);
        dma_handle_no_inc.dnr = wramSrc;
        dma_handle_no_inc.dnw = bbusDst;
        scu_dma_config_set(dmach, SCU_DMA_START_FACTOR_ENABLE, &dma_handle_no_inc, NULL);
        scu_dma_level_end_set(dmach, NULL, NULL);
        scu_dma_level_fast_start(dmach);
        scu_dma_transfer_wait(dmach);
        cpu_cache_area_purge(bbusDst, XFER_SIZE);
        dbgio_puts("WRAM->BBus +0: ");
        for (int i = 0; i < XFER_SIZE; i++)
        {
            dbgio_printf("%02X", *(uint8_t *)(bbusDst + i));
        }
        dbgio_puts("\n");

        dbgio_flush();

        vdp2_sync();
        vdp2_sync_wait();

        // BBus->WRAM
        memset(wramDst, fillData, BUF_SIZE);
        dma_handle_no_inc.dnr = bbusSrc;
        dma_handle_no_inc.dnw = wramDst;
        scu_dma_config_set(dmach, SCU_DMA_START_FACTOR_ENABLE, &dma_handle_no_inc, NULL);
        scu_dma_level_end_set(dmach, NULL, NULL);
        scu_dma_level_fast_start(dmach);
        scu_dma_transfer_wait(dmach);
        cpu_cache_area_purge(wramDst, XFER_SIZE);
        dbgio_puts("BBus->WRAM +0: ");
        for (int i = 0; i < XFER_SIZE; i++)
        {
            dbgio_printf("%02X", *(uint8_t *)(wramDst + i));
        }
        dbgio_puts("\n");

        dbgio_flush();

        vdp2_sync();
        vdp2_sync_wait();

        // BBus->BBus
        memset(bbusDst, fillData, BUF_SIZE);
        dma_handle_no_inc.dnr = bbusSrc;
        dma_handle_no_inc.dnw = bbusDst;
        scu_dma_config_set(dmach, SCU_DMA_START_FACTOR_ENABLE, &dma_handle_no_inc, NULL);
        scu_dma_level_end_set(dmach, NULL, NULL);
        scu_dma_level_fast_start(dmach);
        scu_dma_transfer_wait(dmach);
        cpu_cache_area_purge(bbusDst, XFER_SIZE);
        dbgio_puts("BBus->BBus +0: ");
        for (int i = 0; i < XFER_SIZE; i++)
        {
            dbgio_printf("%02X", *(uint8_t *)(bbusDst + i));
        }
        dbgio_puts("\n");

        dbgio_flush();

        vdp2_sync();
        vdp2_sync_wait();
    }

    dbgio_flush();

    vdp2_sync();
    vdp2_sync_wait();

    while (true)
    {
    }

    return 0;
}

void user_init(void)
{
    vdp2_tvmd_display_res_set(VDP2_TVMD_INTERLACE_NONE, VDP2_TVMD_HORZ_NORMAL_A,
                              VDP2_TVMD_VERT_224);

    vdp2_scrn_back_color_set(VDP2_VRAM_ADDR(3, 0x01FFFE),
                             RGB1555(1, 0, 3, 15));

    vdp2_tvmd_display_set();
}
