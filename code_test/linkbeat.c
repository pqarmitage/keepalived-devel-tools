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

	if (bmsr & BMSR_LSTATUS)
		return IFF_UP;
	else if (new_bmsr & BMSR_LSTATUS)
		return IFF_UP;
	else
		return 0;
}

static int
if_mii_probe(const char *ifname)
{
	struct mii_ioctl_data *data = (struct mii_ioctl_data *)&ifr.ifr_data;
	uint16_t phy_id;
	int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	int status;

	if (fd < 0)
		return -1;

	memset(&ifr, 0, sizeof (struct ifreq));
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(fd, SIOCGMIIPHY, &ifr) < 0) {
		close(fd);
		return -1;
	}

	/* check if the driver reports BMSR using the MII interface, as we
	 * will need this and we already know that some don't support it.
	 */
	phy_id = data->phy_id; /* save it in case it is overwritten */
	data->reg_num = MII_BMSR;
	if (ioctl(fd, SIOCGMIIREG, &ifr) < 0) {
		close(fd);
printf("BMSR not supported\n");
		return -1;
	}
	data->phy_id = phy_id;

	/* Dump the MII transceiver */
	status = if_mii_status(fd);
	close(fd);
	return status;
}

static int
if_ethtool_status(const int fd)
{
	struct ethtool_value edata;

	edata.cmd = ETHTOOL_GLINK;
	ifr.ifr_data = (caddr_t) & edata;
	if (!ioctl(fd, SIOCETHTOOL, &ifr)) {
		printf("ethtool data 0x%x\n", edata.data);
		return (edata.data) ? 1 : 0;
	}
	printf("SIOCETHTOOL error %d - %m\n", errno);
	return -1;
}

static int
if_ethtool_probe(const char *ifname)
{
	int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	int status;

	if (fd < 0)
		return -1;

	memset(&ifr, 0, sizeof (struct ifreq));
	strcpy(ifr.ifr_name, ifname);

	status = if_ethtool_status(fd);
	close(fd);
	return status;
}

/* Returns false if interface is down */
static bool
if_ioctl_flags(char *ifname)
{
	int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);

	if (fd < 0)
		return true;

	memset(&ifr, 0, sizeof (struct ifreq));
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
		printf("SIOCGIFFFLAGS failed\n - %d (%m)", errno);
		close(fd);
		return true;
	}

	close(fd);

	printf("ioctl flags 0x%x\n", ifr.ifr_flags);
	return FLAGS_UP(ifr.ifr_flags);
}

int main(int argc, char **argv)
{
        bool if_up = true, was_up;
	bool use_mii = true;
	char *iface = "eth0";

	if (argc > 1)
		iface = argv[1];

//        was_up = IF_FLAGS_UP(ifp);

	while (true) {
		printf("mii reports 0x%x\n", if_mii_probe(iface));
		printf("ethtool reports 0x%x\n", if_ethtool_probe(iface));

		/*
		 * update ifp->flags to get the new IFF_RUNNING status.
		 * Some buggy drivers need this...
		 */
		printf("ioctl returned 0x%x\n", if_ioctl_flags(iface));

		sleep(3);
		printf("\n\n\n");
	}
}
