#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>


#ifdef __linux__
#include <sys/ioctl.h>
#include <bits/ioctls.h>
#include <sys/utsname.h>
#include <stdio.h>
#endif

#include <errno.h>
#include <sys/socket.h>


static char *
inet_sockaddrtos2(struct sockaddr_storage *addr, char *addr_str)
{
        void *addr_ip;

        if (addr->ss_family == AF_INET6) {
                struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) addr;
                addr_ip = &addr6->sin6_addr;
        } else {
                struct sockaddr_in *addr4 = (struct sockaddr_in *) addr;
                addr_ip = &addr4->sin_addr;
        }

        if (!inet_ntop(addr->ss_family, addr_ip, addr_str, INET6_ADDRSTRLEN))
                return NULL;

        return addr_str;
}

char *
inet_sockaddrtos(struct sockaddr_storage *addr)
{
        static char addr_str[INET6_ADDRSTRLEN];
        inet_sockaddrtos2(addr, addr_str);
        return addr_str;
}

int main ( int argc, char **argv ) {

        // sock = openSocket(env, AF_INET);
        // static int openSocket(JNIEnv *env, int proto){
        // sock = JVM_Socket(AF_INET, SOCK_DGRAM, 0)

        // JVM_Socket(jint domain, jint type, jint protocol)
        // result = socket(domain, type, protocol);
        int sock  = socket(AF_INET, SOCK_DGRAM, 0);
		int ret;

        struct ifreq if2;

        memset((char *) &if2, 0, sizeof(if2));
        strcpy(if2.ifr_name, argv[1]);

        if ((ret =ioctl(sock, SIOCGIFFLAGS, (char *)&if2))  < 0) {
                printf("IOCTL  SIOCGIFFLAGS failed %s, ret %d, errno %d - %m\n",argv[1], ret, errno);
                return 0;
        }

        if (if2.ifr_flags & IFF_BROADCAST) {
                /* It does, let's retrieve it*/
		if (argc > 2) {
			memset((char *) &if2, 0, sizeof(if2));
			strcpy(if2.ifr_name, argv[2]);
		}

                if ((ret = ioctl(sock, SIOCGIFBRDADDR, (char *)&if2)) < 0) {
                        printf("IOCTL SIOCGIFBRDADDR failed %s, ret 0x%x, errno %d - %m\n",argv[1], ret, errno);
                        return 0;
                }
		else
			printf("Brd: %s\n", inet_sockaddrtos((struct sockaddr_storage *)&if2.ifr_broadaddr));
        }
	else
		printf("No broadcast address\n");
        

        return 0;

}
