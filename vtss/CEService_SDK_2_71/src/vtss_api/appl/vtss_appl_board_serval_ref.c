/*

 Vitesse API software.

 Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
 Rights Reserved.

 Unpublished rights reserved under the copyright laws of the United States of
 America, other countries and international treaties. Permission to use, copy,
 store and modify, the software and its source code is granted. Permission to
 integrate into other products, disclose, transmit and distribute the software
 in an absolute machine readable format (e.g. HEX file) is also granted.  The
 source code of the software may not be disclosed, transmitted or distributed
 without the written permission of Vitesse. The software and its source code
 may only be used in products utilizing the Vitesse switch products.

 This copyright notice must appear in any copy, modification, disclosure,
 transmission or distribution of the software. Vitesse retains all ownership,
 copyright, trade secret and proprietary rights in the software.

 THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
 INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR USE AND NON-INFRINGEMENT.
 
 $Id$
 $Revision$

*/

#include "vtss_api.h"  /* Defines BOARD_SERVAL_REF if applicable */

#if defined(BOARD_SERVAL_REF)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>

#include <dirent.h>
#include <endian.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/if_tun.h>
#include <linux/limits.h>

#include <asm/byteorder.h>

#define TUNDEV "/dev/net/tun"

/* ================================================================= *
 *  Register access
 * ================================================================= */

#if 0
static void memdump(const unsigned char      *byte_p, 
                    const int                byte_cnt)
{
    int i;
    for (i= 0; i < byte_cnt; i += 16) {
        int j = 0;
        printf("%04x:", i);
        while (j+i < byte_cnt && j < 16) {
            printf(" %02x", byte_p[i+j]); 
            j++;
        }
        putchar('\n');
    }
}
#endif

#if defined (IO_METHOD_UIO)

#define ENABLE_DMA_PACKET_MODE

#include "vtss_api.h"
#include "vtss_appl.h"

#define MMAP_SIZE   0x02000000
#define CHIPID_OFF (0x01070000 >> 2)
#define ENDIAN_OFF (0x01000000 >> 2)

#define HWSWAP_BE 0x81818181       /* Big-endian */
#define HWSWAP_LE 0x00000000       /* Little-endian */

int irq_fd;
#ifdef __BYTE_ORDER
#define IS_BIG_ENDIAN (__BYTE_ORDER == __BIG_ENDIAN)
#else
# error Unable to determine byteorder!
#endif

#if (__BYTE_ORDER == __BIG_ENDIAN)
#define CPU_TO_LE32(x) __cpu_to_le32(x)
#define LE32_TO_CPU(x) __le32_to_cpu(x)
#else
#define CPU_TO_LE32(x) (x)
#define LE32_TO_CPU(x) (x)
#endif

static BOOL use_tap;
static int  tap_fd;
static const char *tap_ifname;
vtss_packet_tx_ifh_t tx_ifh;

static char iodev[64];
static volatile u32 *base_mem;

static vtss_rc reg_read(const vtss_chip_no_t chip_no,
                        const u32            addr,
                        u32                  *const value)
{
    *value = LE32_TO_CPU(base_mem[addr]);
    return VTSS_RC_OK;
}

static vtss_rc reg_write(const vtss_chip_no_t chip_no,
                         const u32            addr,
                         const u32            value)
{
    base_mem[addr] = CPU_TO_LE32(value);
    return VTSS_RC_OK;
}

static BOOL find_dev(void)
{
    const char *top = "/sys/class/uio";
    DIR *dir;
    struct dirent *dent;
    char fn[PATH_MAX], devname[128];
    FILE *fp;
    BOOL found = FALSE;

    if (!(dir = opendir (top))) {
        perror(top);
        exit (1);
    }

    while((dent = readdir(dir)) != NULL) {
        if (dent->d_name[0] != '.') {
            snprintf(fn, sizeof(fn), "%s/%s/name", top, dent->d_name);
            if ((fp = fopen(fn, "r"))) {
                fgets(devname, sizeof(devname), fp);
                fclose(fp);
                if (strstr(devname, "Serval1")) {
                    snprintf(iodev, sizeof(iodev), "/dev/%s", dent->d_name);
                    found = TRUE;
                    break;
                }
            }
        }
    }

    closedir(dir);

    return found;
}

static void board_io_init(void)
{
    u32 value, value1;
    const int enable = 1;

    if(!find_dev()) {
        fprintf(stderr, "Unable to locate UIO device, please check 'lspci -nn' and 'lsmod'\n");
        exit(1);
    }

    /* Open the UIO device file */
    fprintf(stderr, "Using UIO: %s\n", iodev);
    irq_fd = open(iodev, O_RDWR);
    if (irq_fd < 1) {
        perror(iodev);
        exit(1);
    }

    /* mmap the UIO device */
    base_mem = mmap(NULL, MMAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, irq_fd, 0);
    if(base_mem != MAP_FAILED) {
        fprintf(stderr, "Mapped register memory @ %p\n", base_mem);
        value = HWSWAP_LE;
        fprintf(stderr, "UIO: Using normal PCIe byteorder value %08x\n", value);
        reg_write(0, ENDIAN_OFF, value);
        reg_read(0, ENDIAN_OFF, &value);
        reg_read(0, CHIPID_OFF, &value);
        fprintf(stderr, "Chipid: %08x\n", value);
        value = (value >> 12) & 0xffff;
        if (value != (value1 = 0x7418)) {
            fprintf(stderr, "Unexpected Chip ID, expected 0x%08x\n", value1);
            exit(1);
        }
    } else {
        perror("mmap");
    }

    if(write(irq_fd, &enable, sizeof(enable)) != sizeof(enable)) {
        perror("write() failed, unable to enable IRQs");
    }
}

#else

#include <linux/vitgenio.h>

#undef __USE_EXTERN_INLINES /* Avoid warning */
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>

#include "vtss_api.h"
#include "vtss_appl.h"

static int irq_fd;

/* Global frame buffers */
#define VRAP_LEN  28
#define FRAME_MAX 2000
static u8 req_frame[VRAP_LEN];
static u8 rep_frame[FRAME_MAX];

static struct sockaddr_ll sock_addr;

static void frame_dump(const char *name, u8 *frame, int len)
{
    u32  i;
    char buf[100], *p = &buf[0];
    
    T_D("%s:", name);
    for (i = 0; i < len; i++) {
        if ((i % 16) == 0) {
            p = &buf[0];
            p += sprintf(p, "%04x: ", i);
        }
        p += sprintf(p,"%02x%c", frame[i], ((i + 9) % 16) == 0 ? '-' : ' ');
        if (((i + 1) % 16) == 0 || (i + 1) == len) {
            T_D("%s", buf);
        }
    }
}

static vtss_rc board_rd_wr(const u32 addr, u32 *const value, int write)
{
    u32     val = (write ? *value : 0);
    ssize_t n;
    BOOL    dump = 0;

    /* Address */
    req_frame[20] = ((addr >> 22) & 0xff);
    req_frame[21] = ((addr >> 14) & 0xff);
    req_frame[22] = ((addr >> 6) & 0xff);
    req_frame[23] = (((addr << 2) & 0xff) | (write ? 2 : 1));
    
    //dump = (req_frame[21] == 7 ? 1 : 0); /* DEVCPU_GCB */
    
    if (dump)
        T_D("%s, addr: 0x%08x, value: 0x%08x", write ? "WR" : "RD", addr, val);

    /* Value, only needed for write operations */
    req_frame[24] = ((val >> 24) & 0xff);
    req_frame[25] = ((val >> 16) & 0xff);
    req_frame[26] = ((val >> 8) & 0xff);
    req_frame[27] = (val & 0xff);
    
    n = VRAP_LEN;
    if (sendto(irq_fd, req_frame, n, 0, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) < 0) {
        T_E("sendto failed");
        return VTSS_RC_ERROR;
    }
    if (dump)
        frame_dump("VRAP Request", req_frame, n);

    if ((n = recvfrom(irq_fd, rep_frame, FRAME_MAX, 0, NULL, NULL)) < 0) {
        T_E("recvfrom failed");
        return VTSS_RC_ERROR;
    }
    if (dump)
        frame_dump("VRAP Reply", rep_frame, n);
    
    if (!write) {
        *value = ((rep_frame[20] << 24) | (rep_frame[21] << 16) | 
                  (rep_frame[22] << 8) | rep_frame[23]);
    }

    return VTSS_RC_OK;
}

static vtss_rc reg_read(const vtss_chip_no_t chip_no,
                        const u32            addr,
                        u32                  *const value)
{
    return board_rd_wr(addr, value, 0);
}

static vtss_rc reg_write(const vtss_chip_no_t chip_no,
                         const u32            addr,
                         const u32            value)
{
    u32 val = value;
    return board_rd_wr(addr, &val, 1);
}

static void board_io_init(void)
{
    struct ifreq ifr;
    int    protocol = htons(0x8880);

    /* Open socket */
    if ((irq_fd = socket(AF_PACKET, SOCK_RAW, protocol)) < 0) {
        T_E("socket create failed");
        return;
    }

    /* Get ifIndex */
    strcpy(ifr.ifr_name, "eth0");
    if (ioctl(irq_fd, SIOCGIFINDEX, &ifr) < 0) {
        T_E("SIOCGIFINDEX failed");
        return;
    }

    /* Initialize socket address */
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sll_family = htons(AF_PACKET);
    sock_addr.sll_protocol = protocol;
    sock_addr.sll_halen = ETH_ALEN;
    sock_addr.sll_ifindex = ifr.ifr_ifindex;

    /* Get SMAC */
    if (ioctl(irq_fd, SIOCGIFHWADDR, &ifr) < 0) {
        T_E("SIOCGIFHWADDR failed");
        return;
    }

    /* Initialize request frame */
    req_frame[0] = 0x00;
    req_frame[1] = 0x01; /* DMAC 00-01-00-00-00-00 */
    req_frame[2] = 0x00;
    req_frame[3] = 0x00;
    req_frame[4] = 0x00;
    req_frame[5] = 0x00;
    memcpy(&req_frame[6], ifr.ifr_hwaddr.sa_data, ETH_ALEN);
    req_frame[12] = 0x88;
    req_frame[13] = 0x80; 
    req_frame[15] = 0x04; /* EPID 0x0004 */
    req_frame[16] = 0x10; /* VRAP request */
    
    /* Copy DMAC */
    memcpy(&sock_addr.sll_addr, req_frame, ETH_ALEN);
}

#endif

/* Board port map */
static vtss_port_map_t port_map[VTSS_PORT_ARRAY_SIZE];

static vtss_port_interface_t port_interface(vtss_port_no_t port_no)
{
    return (port_map[port_no].miim_controller == VTSS_MIIM_CONTROLLER_NONE ? 
            VTSS_PORT_INTERFACE_SERDES : VTSS_PORT_INTERFACE_SGMII);
}

static BOOL port_poll(vtss_port_no_t port_no)
{
#if defined (IO_METHOD_UIO)
    return 1;                   /* Also has NPI port here */
#else
    /* Avoid polling NPI port */
    return (port_no == 10 ? 0 : 1);
#endif
}

#if defined (IO_METHOD_UIO)

static void word_memcpy(volatile u32 *dst, volatile u32 *src, int n_words)
{
    if ((((u64)dst) & 0x3) || (((u64)dst) & 0x3)) {
        printf("Unaligned addresses: dst %p, src %p\n", dst, src);
    }
#if 1
    int i;
    for(i = 0; i < n_words; i++) {
        dst[i] = src[i];
    }
#else
    memcpy((void*) dst, (void*) src, n_words * sizeof(u32));
#endif
}

vtss_rc dma_transmit(struct vtss_board_t        *board, 
                     const vtss_packet_tx_ifh_t *const ifh,
                     const u8                   *const frame,
                     const u32                   length)
{
    u32 dma_offset;
    if (vtss_packet_dma_offset(NULL, FALSE, &dma_offset) == VTSS_RC_OK) {
        u32 data_words = (length + 3) >> 2; /* Whole words */
        /* Write control word */
        base_mem[dma_offset] = CPU_TO_LE32(0x30000 | (ifh->length + length));
        /* Tx IFH - offset just > control */
        word_memcpy(&base_mem[dma_offset + 1], (volatile u32*)ifh->ifh, ifh->length / sizeof(u32));
        //memdump((u8*)ifh->ifh, ifh->length);
        /* Tx frame data - offset just > control (add dummy CRC word)*/
        word_memcpy(&base_mem[dma_offset + 1], (volatile u32*)frame, data_words+1);
        //memdump((u8*)frame, length);
        T_I("DMA: Sent %d bytes - offset 0x%0x, status 0x%0x", ifh->length + length, dma_offset, LE32_TO_CPU(base_mem[dma_offset]));
    }
    return VTSS_RC_OK;
}

/* NB: Inserts DMA status word after last u32 word in buffer to avoid
 * copying. If using real DMA controller this can probably be
 * avoided. IFH msut be inserted properly in start of frame. */
static void tx_frame_dma(u32 *frame, size_t frame_len)
{
    u32 dma_offset;
    if (vtss_packet_dma_offset(NULL, FALSE, &dma_offset) == VTSS_RC_OK) {
        u32 data_words = (frame_len + 3) >> 2; /* Whole words */
        base_mem[dma_offset] = CPU_TO_LE32(0x30000 | (frame_len));
        word_memcpy(&base_mem[dma_offset + 1], frame, data_words);
        //memdump((u8*)frame, frame_len);
        T_I("TAP: Sent %zd bytes - offset 0x%0x", frame_len, dma_offset);
    }
}

static void rx_frames_fdma(vtss_board_t *board)
{
    u32 frame[1024];
    vtss_packet_rx_meta_t pmeta;
    vtss_packet_rx_info_t info;
    u32 dma_offset;
    memset(frame, 0xce, sizeof(frame));
    if (vtss_packet_dma_offset(NULL, TRUE, &dma_offset) == VTSS_RC_OK) {
        u32 frame_len = 0, chunk = 128 /* words */, status;
        u32 control_bits = 0;
        do {
            u32 chunk_bytes;
            u32 *ptr = &frame[frame_len>>2];
            word_memcpy(ptr, &base_mem[dma_offset - chunk], chunk + 1);
            status = LE32_TO_CPU(ptr[chunk]); /* Get last dword = status */
            control_bits |= status;
            chunk_bytes = status & 0xFFFF; /* Bits[0;15] */
            if(frame_len&0x3) {
                T_E("Unaligned data at %d", frame_len);
            }
            frame_len += chunk_bytes;
            T_D("Chunk %d bytes, total %d, status 0x%08x", chunk_bytes, frame_len, status);
        } while((control_bits & 0x60000) == 0); /* Exit on abort, eof */
        T_D("XTR got %d bytes, control_bits %08x", frame_len, control_bits & 0x70000);
        memset(&pmeta, 0, sizeof(pmeta));
        pmeta.length = frame_len - 16 - 4; /* Ex FCS ex IFH */
        if (vtss_packet_rx_hdr_decode(NULL, &pmeta, (u8*) frame, &info) == VTSS_RC_OK) {
            T_I("PCIe: Received %d bytes on port %d cos %d", info.length, iport2uport(info.port_no), info.cos);
            if (use_tap) {
                /* IFH is 16 bytes with default settings */
                write(tap_fd, &frame[4], info.length); /* info.length = Ex FCS */
            }
        } else {
            T_E("vtss_packet_rx_hdr_decode error");
        }
    }
}

static void rx_frames_register(vtss_board_t *board)
{
    u8                      frame[2000];
    vtss_packet_rx_header_t header;
    vtss_packet_rx_queue_t  queue;
    u32                     total, frames;
                
    total = 0;
    do {
        frames = 0;
        for (queue = VTSS_PACKET_RX_QUEUE_START; queue < VTSS_PACKET_RX_QUEUE_END; queue++) {
            if (vtss_packet_rx_frame_get(board->inst, queue, &header, frame, sizeof(frame)) == VTSS_RC_OK) {
                T_I("received frame on port_no: %u, queue_mask: 0x%08x, length: %u",
                    header.port_no, header.queue_mask, header.length);
                total++, frames++;
            }
        }
    } while(frames);
    T_I("Extracted %d frames", total);
}

static void irq_poll(vtss_board_t *board)
{
    vtss_irq_status_t status;
    fd_set rfds;
    struct timeval tv;
    int ret, nfds = 0;

    FD_ZERO(&rfds);
    FD_SET(irq_fd, &rfds);
    nfds = MAX(nfds, irq_fd);
    if (use_tap) {
        FD_SET(tap_fd, &rfds);
        nfds = MAX(nfds, tap_fd);
    }
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    if((ret = select(nfds + 1, &rfds, NULL, NULL, &tv)) > 0) {

        if (FD_ISSET(irq_fd, &rfds)) {
            const int enable = 1;
            int n_irq;
            read(irq_fd, &n_irq, sizeof(n_irq));

            if (vtss_irq_status_get_and_mask(NULL, &status) == VTSS_RC_OK) {
                T_D("IRQ %d - status: 0x%08x (raw 0x%08x)", n_irq, status.active, status.raw_ident);
                if (status.active & (1 << VTSS_IRQ_FDMA_XTR)) {
                    rx_frames_fdma(board);
                    vtss_irq_enable(NULL, VTSS_IRQ_FDMA_XTR, TRUE);
                }
                if (status.active & (1 << VTSS_IRQ_XTR)) {
                    rx_frames_register(board);
                    vtss_irq_enable(NULL, VTSS_IRQ_XTR, TRUE);
                }
                if (status.active & (1 << VTSS_IRQ_SOFTWARE)) {
                    T_I("Software INTR");
                    vtss_irq_enable(NULL, VTSS_IRQ_SOFTWARE, FALSE);
                }
            }
            if(write(irq_fd, &enable, sizeof(enable)) != sizeof(enable)) {
                perror("write() failed, unable to enable IRQs");
            }
        }
        if (use_tap && FD_ISSET(tap_fd, &rfds)) {
            u32 buf[1024];
            int n_read;
            memcpy(buf, tx_ifh.ifh, tx_ifh.length);
            n_read = read(tap_fd, ((u8*) buf) + tx_ifh.length, sizeof(buf) - tx_ifh.length);
            if(n_read > 0) {
                T_I("TAP: NPI->switch inject %d bytes", n_read);
                tx_frame_dma(buf, tx_ifh.length + n_read + 4); /* + 4 = Dummy FCS */
            }
        }
    } else {
        if (ret < 0) {
            perror("select()");
        }
    }
}
#endif

/* ================================================================= *
 *  Board init.
 * ================================================================= */

static void board_init_post(vtss_board_t *board)
{
    /* NPI port is always forwarding */
    vtss_port_state_set(NULL, 10, 1);

    /* Enable GPIO_9 and GPIO_10 as MIIM controllers */
    vtss_gpio_mode_set(NULL, 0, 9, VTSS_GPIO_ALT_1);
    vtss_gpio_mode_set(NULL, 0, 10, VTSS_GPIO_ALT_1);

    /* Tesla pre reset */
    vtss_phy_pre_reset(NULL, 0);
}

static void board_init_done(vtss_board_t *board)
{
    /* Tesla post reset */
    vtss_phy_post_reset(NULL, 0);

#if defined(ENABLE_DMA_PACKET_MODE)
    {
        vtss_packet_dma_conf_t mode;
        if (vtss_packet_dma_conf_get(NULL, &mode) == VTSS_RC_OK) {
            memset(mode.dma_enable, TRUE, sizeof(mode.dma_enable));
            if (vtss_packet_dma_conf_set(NULL, &mode)) {
                T_E("vtss_packet_dma_conf_set fails");
            }
        } else {
            T_E("vtss_packet_dma_conf_get fails");
        }
    }
#endif
    {
        vtss_irq_conf_t conf;

        memset(&conf, 0, sizeof(conf));
        conf.external = TRUE;

        vtss_irq_conf_set(NULL, VTSS_IRQ_XTR, &conf);
        vtss_irq_conf_set(NULL, VTSS_IRQ_FDMA_XTR, &conf);
        vtss_irq_conf_set(NULL, VTSS_IRQ_SOFTWARE, &conf);
        vtss_irq_enable(NULL, VTSS_IRQ_XTR, TRUE);
        vtss_irq_enable(NULL, VTSS_IRQ_FDMA_XTR, TRUE);
        vtss_irq_enable(NULL, VTSS_IRQ_SOFTWARE, TRUE);
    }
}

static int maketap (const char *ifname)
{
    struct ifreq ifr;
    int fd, err;

    if( (fd = open(TUNDEV, O_RDWR)) < 0 ) {
        return fd;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

    if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
        close(fd);
        return err;
    }

    return fd;
}

static int board_init(int argc, const char **argv, vtss_board_t *board)
{
    vtss_port_no_t  port_no;
    vtss_port_map_t *map;

    if(argc > 0) {
        if (strcmp(argv[0],"--tap") == 0) {
            use_tap = TRUE;
            tap_ifname = argc > 1 ? argv[1] : "npi";
        } else {
            printf("Unknown option %s, use:\n"
                   " --tap [<ifname>]: Create TAP inteface with given name.\n", argv[0]);
            return(1);
        }
    }

    board_io_init();
    board->board_init_post = board_init_post;
    board->board_init_done = board_init_done;
    board->port_map = port_map;
    board->port_interface = port_interface;
    board->port_poll = port_poll;
    board->init.init_conf->reg_read = reg_read;
    board->init.init_conf->reg_write = reg_write;

    /* Setup port map and calculate port count */
    board->port_count = VTSS_PORTS;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        map = &port_map[port_no];

        if (port_no < 4) {
            /* Port 0-3: Copper ports */
            map->chip_port = (7 - port_no);
            map->miim_controller = VTSS_MIIM_CONTROLLER_1;
            map->miim_addr = (16 + port_no);
        } else if (port_no < 8) {
            /* Port 4-7: 1G SFP */
            map->chip_port = (7 - port_no);
            map->miim_controller = VTSS_MIIM_CONTROLLER_NONE;
        } else if (port_no < 10) {
            /* Port 8-9: 2.5G SFP */
            map->chip_port = port_no;
            map->miim_controller = VTSS_MIIM_CONTROLLER_NONE;
        } else {
            /* Port 10: NPI port */
            map->chip_port = 10;
            map->miim_controller = VTSS_MIIM_CONTROLLER_1;
            map->miim_addr = 28;
        }
    }

    if (use_tap) {
        vtss_packet_tx_info_t tx_info;
        tap_fd = maketap(tap_ifname);
        if (tap_fd < 0) {
            printf("TAP device %s creation failed, disabling TAP.\n", tap_ifname);
            use_tap = FALSE;
        } else {
            printf("Created '%s' TAP interface, td %d\n", tap_ifname, tap_fd);
        }
        vtss_packet_tx_info_init(NULL, &tx_info);
        tx_info.switch_frm = TRUE;
        vtss_packet_tx_hdr_compile(NULL, &tx_info, &tx_ifh);
    }

    return 0;
}

void vtss_board_serval_ref_init(vtss_board_t *board)
{
    board->descr = "Serval";
    board->target = VTSS_TARGET_SERVAL;
    board->feature.port_control = 1;
    board->feature.layer2 = 1;
    board->feature.packet = 1;
    board->board_init = board_init;
#if defined (IO_METHOD_UIO)
    board->board_irq_poll = irq_poll;
    board->board_transmit = dma_transmit;
#endif
}
#endif /* BOARD_SERVAL_REF */
