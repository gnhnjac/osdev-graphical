
#define RTL_VID 0x10EC
#define RTL_DID 0x8139

#define RBSTRT_REG 0x30
#define CFG1_REG 0x52 
#define CMD_REG 0x37
#define IMR_REG 0x3C
#define ISR_REG 0x3E
#define RCR_REG 0x44
#define CAP_REG 0x38

#define ROK     (1<<0)
#define TOK     (1<<2)

#define RX_BUF_SIZE 8192

#define RX_READ_POINTER_MASK (~3)
#define CR_BUFE 0x01

typedef struct rtl8139_dev {
    uint16_t io_base;
    uint8_t mac_addr[6];
    char * rx_buffer;
    int tx_cur;
}rtl8139_dev_t;

//refs
// Four TXAD register, you must use a different one to send packet each time(for example, use the first one, second... fourth and back to the first);
void install_nic();
void send_packet(char *packet,int len);
void rtl8139_handler(struct regs *r);
void receive_packet();
char *get_mac_addr();