/*
 * canopen_client.c - SocketCAN skeleton for Project 4, Part 3.2
 *
 * Minimal CANopen client helpers for a single node:
 *   - open and bind a CAN_RAW socket with receive filters
 *   - send NMT commands
 *   - wait for the Boot-up message
 *   - expedited SDO upload (read) and download (write) with timeout
 *   - receive a TPDO with timeout
 *
 * Multi-byte values are packed explicitly in little-endian order,
 * as required by CANopen, independent of the host byte order.
 *
 * Build:  gcc -Wall -O2 -o canopen_client canopen_client.c
 * Run:    ./canopen_client can0
 *
 * The parts marked TODO are for you to write.
 */

#include <errno.h>
#include <net/if.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#define NODE_ID 1

/* COB-IDs for the node */
#define COB_NMT          0x000
#define COB_TPDO1(id)    (0x180 + (id))
#define COB_RPDO1(id)    (0x200 + (id))
#define COB_SDO_TX(id)   (0x580 + (id)) /* node -> client */
#define COB_SDO_RX(id)   (0x600 + (id)) /* client -> node */
#define COB_BOOTUP(id)   (0x700 + (id))

/* NMT command specifiers */
#define NMT_START        0x01
#define NMT_STOP         0x02
#define NMT_PREOP        0x80
#define NMT_RESET_NODE   0x81
#define NMT_RESET_COMM   0x82

/* Return codes in addition to SDO abort codes */
#define ERR_TIMEOUT      (-1)
#define ERR_IO           (-2)
#define ERR_PROTOCOL     (-3)

/* Configuration written at start-up */
#define CTRL_TYPE    1       /* 0 = P, 1 = PI */
#define KP           34.0f
#define TI           0.03f    /* s */
#define PROFILE_LIMIT 1000   /* rpm/s, acc and dec, default if no third argument */

#define STEP_RPM     40     /* target after the step */
#define STEP_AT_MS   4000     /* time of the step, gives a baseline before it */
#define STEP_DOWN_MS 8000    /* target back to 0 */
#define RUN_MS       12000    /* length of the test */


/* ------------------------------------------------------------------ */
/* Little-endian helpers                                                */
/* ------------------------------------------------------------------ */

static void put_u16(uint8_t *p, uint16_t v) { p[0] = v & 0xFF; p[1] = v >> 8; }
static void put_u32(uint8_t *p, uint32_t v)
{
    p[0] = v & 0xFF; p[1] = (v >> 8) & 0xFF; p[2] = (v >> 16) & 0xFF; p[3] = v >> 24;
}
static uint16_t get_u16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static uint32_t get_u32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* Elapsed-time helper for timeouts */
static long ms_since(const struct timespec *t0)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (t.tv_sec - t0->tv_sec) * 1000 + (t.tv_nsec - t0->tv_nsec) / 1000000;
}

/* ------------------------------------------------------------------ */
/* Socket                                                              */
/* ------------------------------------------------------------------ */

/* Open a CAN_RAW socket on ifname, receiving only the given CAN ids. */
int can_open(const char *ifname, const canid_t *ids, int n_ids)
{
    int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        perror("socket");
        return -1;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        close(s);
        return -1;
    }

    struct can_filter filters[16];
    if (n_ids > 16) n_ids = 16;
    for (int i = 0; i < n_ids; i++) {
        filters[i].can_id = ids[i];
        filters[i].can_mask = CAN_SFF_MASK;
    }
    if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, filters,
                   n_ids * sizeof(struct can_filter)) < 0) {
        perror("CAN_RAW_FILTER");
        close(s);
        return -1;
    }

    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(s);
        return -1;
    }
    return s;
}

static int can_send(int s, canid_t id, const uint8_t *data, uint8_t len)
{
    struct can_frame f;
    memset(&f, 0, sizeof(f));
    f.can_id = id;
    f.can_dlc = len;
    memcpy(f.data, data, len);
    return write(s, &f, sizeof(f)) == sizeof(f) ? 0 : ERR_IO;
}

/* Receive the next frame with the given id, or time out. */
static int can_recv_id(int s, canid_t id, struct can_frame *f, int timeout_ms)
{
    struct timespec t0;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (;;) {
        int left = timeout_ms - (int)ms_since(&t0);
        if (left <= 0) return ERR_TIMEOUT;

        struct pollfd pfd = { .fd = s, .events = POLLIN };
        int r = poll(&pfd, 1, left);
        if (r == 0) return ERR_TIMEOUT;
        if (r < 0) {
            if (errno == EINTR) return ERR_IO; /* e.g. Ctrl-C */
            return ERR_IO;
        }
        if (read(s, f, sizeof(*f)) != sizeof(*f)) return ERR_IO;
        if ((f->can_id & CAN_SFF_MASK) == id) return 0;
        /* other ids passing the filter are skipped here */
    }
}

/* ------------------------------------------------------------------ */
/* CANopen services                                                     */
/* ------------------------------------------------------------------ */

int nmt_send(int s, uint8_t command, uint8_t node)
{
    uint8_t d[2] = { command, node };
    return can_send(s, COB_NMT, d, 2);
}

/* Wait for the Boot-up message (0x700 + node, one byte 0x00). */
int wait_bootup(int s, uint8_t node, int timeout_ms)
{
    struct can_frame f;
    int r = can_recv_id(s, COB_BOOTUP(node), &f, timeout_ms);
    if (r) return r;
    return (f.can_dlc == 1 && f.data[0] == 0x00) ? 0 : ERR_PROTOCOL;
}

/*
 * Expedited SDO upload (read).
 * Returns 0 on success, an SDO abort code (> 0), or a negative error.
 */
long sdo_upload(int s, uint8_t node, uint16_t index, uint8_t sub,
                uint32_t *value, int timeout_ms)
{
    uint8_t d[8] = { 0x40 };
    put_u16(&d[1], index);
    d[3] = sub;
    if (can_send(s, COB_SDO_RX(node), d, 8)) return ERR_IO;

    struct can_frame f;
    int r = can_recv_id(s, COB_SDO_TX(node), &f, timeout_ms);
    if (r) return r;
    if (f.can_dlc != 8 || get_u16(&f.data[1]) != index || f.data[3] != sub)
        return ERR_PROTOCOL;

    uint8_t cs = f.data[0];
    if (cs == 0x80) return (long)get_u32(&f.data[4]); /* abort */
    if ((cs & 0xE3) != 0x43) return ERR_PROTOCOL;     /* expedited, size indicated */

    *value = get_u32(&f.data[4]); /* unused bytes are zero */
    return 0;
}

/*
 * Expedited SDO download (write) of 1, 2 or 4 bytes.
 * Returns 0 on success, an SDO abort code (> 0), or a negative error.
 */
long sdo_download(int s, uint8_t node, uint16_t index, uint8_t sub,
                  uint32_t value, int size, int timeout_ms)
{
    static const uint8_t cs_for_size[5] = { 0, 0x2F, 0x2B, 0x27, 0x23 };
    if (size < 1 || size > 4) return ERR_PROTOCOL;

    uint8_t d[8] = { cs_for_size[size] };
    put_u16(&d[1], index);
    d[3] = sub;
    put_u32(&d[4], value);
    if (can_send(s, COB_SDO_RX(node), d, 8)) return ERR_IO;

    struct can_frame f;
    int r = can_recv_id(s, COB_SDO_TX(node), &f, timeout_ms);
    if (r) return r;
    if (f.can_dlc != 8 || get_u16(&f.data[1]) != index || f.data[3] != sub)
        return ERR_PROTOCOL;

    if (f.data[0] == 0x80) return (long)get_u32(&f.data[4]); /* abort */
    return f.data[0] == 0x60 ? 0 : ERR_PROTOCOL;
}

/* REAL32 values travel as their IEEE 754 bit pattern. */
long sdo_download_float(int s, uint8_t node, uint16_t index, uint8_t sub,
                        float value, int timeout_ms)
{
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    return sdo_download(s, node, index, sub, bits, 4, timeout_ms);
}

/* Print a readable message for a return code from the functions above. */
void report(const char *what, long r)
{
    if (r == 0) return;
    if (r == ERR_TIMEOUT)       fprintf(stderr, "%s: timeout\n", what);
    else if (r == ERR_IO)       fprintf(stderr, "%s: I/O error\n", what);
    else if (r == ERR_PROTOCOL) fprintf(stderr, "%s: unexpected response\n", what);
    else fprintf(stderr, "%s: SDO abort 0x%08lX\n", what, (unsigned long)r);
}

/* ------------------------------------------------------------------ */
/* Example                                                             */
/* ------------------------------------------------------------------ */

static uint32_t float_bits(float f)
{
    uint32_t b;
    memcpy(&b, &f, sizeof(b));
    return b;
}

/* Write one OD entry by SDO and read it back to verify. Returns 0 on success. */
static long write_verify(int s, uint16_t index, uint32_t value, int size)
{
    char what[32];

    snprintf(what, sizeof(what), "write 0x%04X", index);
    long r = sdo_download(s, NODE_ID, index, 0, value, size, 500);
    if (r) { report(what, r); return r; }

    uint32_t readback;
    snprintf(what, sizeof(what), "read 0x%04X", index);
    r = sdo_upload(s, NODE_ID, index, 0, &readback, 500);
    if (r) { report(what, r); return r; }

    if (readback != value) {
        fprintf(stderr, "0x%04X: wrote 0x%08X, read back 0x%08X\n", index, value, readback);
        return ERR_PROTOCOL;
    }
    printf("0x%04X = 0x%08X verified\n", index, value);
    return 0;
}

/* RPDO1 bytes 0-3: target velocity, INTEGER32 rpm, little-endian */
static int rpdo1_send(int s, int32_t rpm)
{
    uint8_t d[4];
    put_u32(d, (uint32_t)rpm);
    return can_send(s, COB_RPDO1(NODE_ID), d, 4);
}

static double s_since(const struct timespec *t0)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (t.tv_sec - t0->tv_sec) + (t.tv_nsec - t0->tv_nsec) / 1e9;
}

/* Set by Ctrl-C, checked by the main loop */
static volatile sig_atomic_t stop_requested = 0;

static void on_sigint(int sig)
{
    (void)sig;
    stop_requested = 1;
}



int main(int argc, char *argv[])
{
    const char *ifname = argc > 1 ? argv[1] : "can0";

    const canid_t ids[] = { COB_BOOTUP(NODE_ID), COB_SDO_TX(NODE_ID), COB_TPDO1(NODE_ID) };
    int s = can_open(ifname, ids, 3);
    if (s < 0) return EXIT_FAILURE;

    /* Reset the node and wait for it to report Pre-operational */
    nmt_send(s, NMT_RESET_NODE, NODE_ID);
    long r = wait_bootup(s, NODE_ID, 3000);
    if (r) {
        report("boot-up", r);
        close(s);
        return EXIT_FAILURE;
    }
    printf("Node %d booted\n", NODE_ID);

    /* Read the device type */
    uint32_t device_type;
    r = sdo_upload(s, NODE_ID, 0x1000, 0, &device_type, 500);
    if (r) report("read 0x1000", r);
    else   printf("Device type: 0x%08X\n", device_type);

    /* Profile acceleration and deceleration from the third argument, 0 = true step */
    uint32_t profile = argc > 3 ? strtoul(argv[3], NULL, 10) : PROFILE_LIMIT;

    /* 1. Configure the controller and setpoint profile, verify by read-back */
    if (write_verify(s, 0x2000, CTRL_TYPE, 1) ||
        write_verify(s, 0x2001, float_bits(KP), 4) ||
        write_verify(s, 0x2002, float_bits(TI), 4) ||
        write_verify(s, 0x6083, profile, 4) ||
        write_verify(s, 0x6084, profile, 4)) {
        close(s);
        return EXIT_FAILURE;
    }

    /* CSV log of TPDO1, file name from the second argument */
    const char *csv_name = argc > 2 ? argv[2] : "tpdo1_log.csv";
    FILE *csv = fopen(csv_name, "w");
    if (!csv) {
        perror(csv_name);
        close(s);
        return EXIT_FAILURE;
    }
    fprintf(csv, "time_s,target_rpm,speed_rpm,duty_pct\n");


    /* Ctrl-C from here on runs the stop sequence instead of killing the program */
    struct sigaction sa = { .sa_handler = on_sigint };
    sigaction(SIGINT, &sa, NULL);

    /* 2. Set the node operational */
    r = nmt_send(s, NMT_START, NODE_ID);
    if (r) {
        report("NMT start", r);
        fclose(csv);
        close(s);
        return EXIT_FAILURE;
    }

    /* 3. Target 0 -> STEP_RPM -> 0 on RPDO1 and log TPDO1 to CSV */
    struct timespec t0;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    int32_t target = 0;
    while (!stop_requested && ms_since(&t0) < RUN_MS) {
        long ms = ms_since(&t0);
        int32_t want = (ms >= STEP_AT_MS && ms < STEP_DOWN_MS) ? STEP_RPM : 0;
        if (want != target) {
            target = want;
            report("RPDO1 target", rpdo1_send(s, target));
        }
        struct can_frame f;
        r = can_recv_id(s, COB_TPDO1(NODE_ID), &f, 100);
        if (r) {
            if (!stop_requested) report("TPDO1", r);
            break;
        }
        double t = s_since(&t0); /* time-stamp right after reception */
        int32_t speed = (int32_t)get_u32(&f.data[0]);
        int16_t duty = (int16_t)get_u16(&f.data[4]);
        fprintf(csv, "%.6f,%d,%d,%.1f\n", t, target, speed, duty / 10.0);
    }
    fclose(csv);
    printf("Logged to %s\n", csv_name);

    /* Stop, also on Ctrl-C: zero target while still Operational, then Pre-operational */
    if (stop_requested) printf("Ctrl-C, stopping node\n");
    report("RPDO1 zero", rpdo1_send(s, 0));
    report("NMT pre-operational", nmt_send(s, NMT_PREOP, NODE_ID));

    close(s);
    return EXIT_SUCCESS;
}
