#include <linux/mii.h>
#include <linux/sockios.h>      /* needed to get correct values for SIOC* */
#include <linux/ethtool.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/if.h>

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#define FLAGS_UP(x)	(((x) & (IFF_UP | IFF_RUNNING)) == (IFF_UP | IFF_RUNNING))
#define LINK_UP		1
#define	LINK_DOWN	0

static struct ifreq ifr;

/* MII Transceiver Registers poller functions */
static uint16_t
if_mii_read(int fd, uint16_t phy_id, uint16_t reg_num)
{
	struct mii_ioctl_data *data = (struct mii_ioctl_data *)&ifr.ifr_data;

	data->phy_id = phy_id;
	data->reg_num = reg_num;

	if (ioctl(fd, SIOCGMIIREG, &ifr) < 0) {
		printf("MIIREG %s\n", strerror(errno));
		return 0xffff;
	}
printf("mii read reg %d returns 0x%x\n", reg_num, data->val_out);
	if (reg_num == MII_BMCR) {
		printf("\tBMCR:");
		if (data->val_out & BMCR_SPEED1000) printf(" MSB of Speed (1000),");
		if (data->val_out & BMCR_CTST) printf(" Collision test,");
		if (data->val_out & BMCR_FULLDPLX) printf(" Full duplex,");
		if (data->val_out & BMCR_ANRESTART) printf(" Auto negotiation restart,");
		if (data->val_out & BMCR_ISOLATE) printf(" Isolate data paths from MII,");
		if (data->val_out & BMCR_PDOWN) printf(" Enable low power state,");
		if (data->val_out & BMCR_ANENABLE) printf(" Enable auto negotiation,");
		if (data->val_out & BMCR_SPEED100) printf(" Select 100Mbps,");
		if (data->val_out & BMCR_LOOPBACK) printf(" TXD loopback bits,");
		if (data->val_out & BMCR_RESET) printf(" Reset to default state,");
		if (data->val_out & BMCR_SPEED10) printf(" Select 10Mbps,");
		printf("\n");
	}
	else if (reg_num == MII_BMSR) {
		printf("\tBMSR:");
		if (data->val_out & BMSR_ERCAP) printf(" Ext-reg capability,");
		if (data->val_out & BMSR_JCD) printf(" Jabber detected,");
		if (data->val_out & BMSR_LSTATUS) printf(" Link status,");
		if (data->val_out & BMSR_ANEGCAPABLE) printf(" Able to do auto-negotiation,");
		if (data->val_out & BMSR_RFAULT) printf(" Remote fault detected,");
		if (data->val_out & BMSR_ANEGCOMPLETE) printf(" Auto-negotiation complete,");
		if (data->val_out & BMSR_RESV) printf(" Unused...,");
		if (data->val_out & BMSR_ESTATEN) printf(" Extended Status in R15,");
		if (data->val_out & BMSR_100HALF2) printf(" Can do 100BASE-T2 HDX,");
		if (data->val_out & BMSR_100FULL2) printf(" Can do 100BASE-T2 FDX,");
		if (data->val_out & BMSR_10HALF) printf(" Can do 10mbps, half-duplex,");
		if (data->val_out & BMSR_10FULL) printf(" Can do 10mbps, full-duplex,");
		if (data->val_out & BMSR_100HALF) printf(" Can do 100mbps, half-duplex,");
		if (data->val_out & BMSR_100FULL) printf(" Can do 100mbps, full-duplex,");
		if (data->val_out & BMSR_100BASE4) printf(" Can do 100mbps, 4k packets,");
		printf("\n");
	}
	return data->val_out;
}

#if 0
static void if_mii_dump(const uint16_t *mii_regs, size_t num_regs, unsigned phy_id)
{
	int mii_reg;

	printf(" MII PHY #%d transceiver registers:", phy_id);
	for (mii_reg = 0; mii_reg < num_regs; mii_reg++)
		printf("%s %4.4x\n", (mii_reg % 8) == 0 ? "\n ":"", mii_regs[mii_reg]);
	printf("\n");
}
#endif

static int
if_mii_status(const int fd)
{
	struct mii_ioctl_data *data = (struct mii_ioctl_data *)&ifr.ifr_data;
	uint16_t phy_id = data->phy_id;
	uint16_t bmsr, new_bmsr;

	if (if_mii_read(fd, phy_id, MII_BMCR) == 0xffff ||
	    (bmsr = if_mii_read(fd, phy_id, MII_BMSR)) == 0) {
		printf("mii status failed %s\n", ifr.ifr_name);
		return -1;
	}

// if_mii_dump(mii_regs, sizeof(mii_regs)/ sizeof(mii_regs[0], phy_id);

	/*
	 * For Basic Mode Status Register (BMSR).
	 * Sticky field (Link established & Jabber detected), we need to read
	 * a second time the BMSR to get current status.
	 */
	new_bmsr = if_mii_read(fd, phy_id, MII_BMSR);

 printf("Basic Mode Status Register 0x%4.4x ... 0x%4.4x\n", bmsr, new_bmsr);

	if (bmsr & BMSR_LSTATUS ||
	    new_bmsr & BMSR_LSTATUS)
		return LINK_UP;

	return LINK_DOWN;
}

static int
if_mii_probe(const int fd, const char *ifname)
{
	struct mii_ioctl_data *data = (struct mii_ioctl_data *)&ifr.ifr_data;
	uint16_t phy_id;
	int status;

	memset(&ifr, 0, sizeof (struct ifreq));
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(fd, SIOCGMIIPHY, &ifr) < 0) {
		return -1;
	}

	/* check if the driver reports BMSR using the MII interface, as we
	 * will need this and we already know that some don't support it.
	 */
	phy_id = data->phy_id; /* save it in case it is overwritten */
	data->reg_num = MII_BMSR;
	if (ioctl(fd, SIOCGMIIREG, &ifr) < 0) {
printf("BMSR not supported\n");
		return -1;
	}
	data->phy_id = phy_id;

	/* Dump the MII transceiver */
	status = if_mii_status(fd);
	return status;
}

static int
if_ethtool_status(const int fd)
{
	struct ethtool_value edata;

	edata.cmd = ETHTOOL_GLINK;
	ifr.ifr_data = (caddr_t) & edata;
	if (ioctl(fd, SIOCETHTOOL, &ifr)) {
		printf("SIOCETHTOOL error %d - %m\n", errno);
		return -1;
	}

	printf("ethtool data 0x%x\n", edata.data);
	return edata.data ? LINK_UP : LINK_DOWN;
}

static int
if_ethtool_probe(const int fd, const char *ifname)
{
	int status;

	memset(&ifr, 0, sizeof (struct ifreq));
	strcpy(ifr.ifr_name, ifname);

	status = if_ethtool_status(fd);

	return status;
}

/* Returns false if interface is down */
static bool
if_ioctl_flags(int fd, char *ifname)
{
	memset(&ifr, 0, sizeof (struct ifreq));
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
		printf("SIOCGIFFFLAGS failed\n - %d (%m)", errno);
		return true;
	}

	printf("ioctl flags 0x%x:", ifr.ifr_flags);
	if (ifr.ifr_flags & IFF_UP) printf(" UP,");
	if (ifr.ifr_flags & IFF_BROADCAST) printf(" BROADCAST,");
	if (ifr.ifr_flags & IFF_DEBUG) printf(" DEBUG,");
	if (ifr.ifr_flags & IFF_LOOPBACK) printf(" LOOPBACK,");
	if (ifr.ifr_flags & IFF_POINTOPOINT) printf(" POINTOPOINT,");
	if (ifr.ifr_flags & IFF_NOTRAILERS) printf(" NOTRAILERS,");
	if (ifr.ifr_flags & IFF_RUNNING) printf(" RUNNING,");
	if (ifr.ifr_flags & IFF_NOARP) printf(" NOARP,");
	if (ifr.ifr_flags & IFF_PROMISC) printf(" PROMISC,");
	if (ifr.ifr_flags & IFF_ALLMULTI) printf(" ALLMULTI,");
	if (ifr.ifr_flags & IFF_MASTER) printf(" MASTER,");
	if (ifr.ifr_flags & IFF_SLAVE) printf(" SLAVE,");
	if (ifr.ifr_flags & IFF_MULTICAST) printf(" MULTICAST,");
	if (ifr.ifr_flags & IFF_PORTSEL) printf(" PORTSEL,");
	if (ifr.ifr_flags & IFF_AUTOMEDIA) printf(" AUTOMEDIA,");
	if (ifr.ifr_flags & IFF_DYNAMIC) printf(" DYNAMIC,");
	if (ifr.ifr_flags & IFF_LOWER_UP) printf(" LOWER_UP,");
	if (ifr.ifr_flags & IFF_DORMANT) printf(" DORMANT,");
	if (ifr.ifr_flags & IFF_ECHO) printf(" ECHO,");
	printf("\n");

	return FLAGS_UP(ifr.ifr_flags) ? LINK_UP : LINK_DOWN;
}

int main(int argc, char **argv)
{
        bool if_up = true, was_up;
	bool use_mii = true;
	char *iface = "eth0";
	int fd = -1;

	if (argc > 1)
		iface = argv[1];

//        was_up = IF_FLAGS_UP(ifp);

	while (true) {
		if (fd == -1 && (fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0)) == -1) {
			printf("socket failed - errno %d - %m\n", errno);
		} else {
			printf("mii reports 0x%x\n", if_mii_probe(fd, iface));
			printf("ethtool reports 0x%x\n", if_ethtool_probe(fd, iface));

			/*
			 * update ifp->flags to get the new IFF_RUNNING status.
			 * Some buggy drivers need this...
			 */
			printf("ioctl returned 0x%x\n", if_ioctl_flags(fd, iface));
		}

		sleep(3);
		printf("\n\n\n");
	}
}
