
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/icmp.h>

static unsigned int simplefw_net_id;

static unsigned int
simplefw_nfhook_ipv4_handler(void *priv, struct sk_buff *skb,
			     const struct nf_hook_state *state)
{
	struct iphdr *iph;
	__u8 proto;

	if (!skb)
		goto out;

	iph = ip_hdr(skb);
	proto = iph->protocol;
	if (proto == IPPROTO_ICMP) {
		unsigned int *p = NULL;

		/* this is a toy module; IRL we should use something like
		 * net_ratelimit() or net_xxx_ratelimited().
		 */
		pr_info("Drop ICMP packet\n");

		/* panic the kernel by dereferencing a NULL pointer.
		 *
		 * Note that this would only cause a kernel OOPS. To panic the
		 * kernel on OOPS, we have to set the CONFIG_PANIC_ON_OOPS=y
		 * and set the CONFIG_PANIC_ON_OOPS_VALUE=1
		 */
		*p = 0xdeadbeef;

		return NF_DROP;
	}
out:
	return NF_ACCEPT;
}

/* IPv4 handler installed in pre-routing as first rule to be evaluated */
static const struct nf_hook_ops simplefw_nfhook_ops = {
	.hook		= simplefw_nfhook_ipv4_handler,
	.pf		= NFPROTO_IPV4,
	.hooknum	= NF_INET_PRE_ROUTING,
	.priority	= NF_IP_PRI_FIRST,
};

static int __net_init simplefw_netns_init(struct net *net)
{
	int rc;

	rc = nf_register_net_hook(net, &simplefw_nfhook_ops);
	if (rc)
		return rc;

	pr_info("netfilter hook successfully registered\n");
	return 0;
}

static void __net_exit simplefw_netns_exit(struct net *net)
{
	nf_unregister_net_hook(net, &simplefw_nfhook_ops);

	pr_info("netfilter hook successfully un-registered\n");
}

static struct pernet_operations simplefw_netns_ops __net_initdata = {
	.init	= simplefw_netns_init,
	.exit	= simplefw_netns_exit,
	.id	= &simplefw_net_id,
};

int __init simplefw_init(void)
{
	int rc;

	rc = register_pernet_subsys(&simplefw_netns_ops);
	if (rc)
		return rc;

	pr_info("simplefw module loaded\n");
	return 0;
}

void __exit simplefw_exit(void)
{
	unregister_pernet_subsys(&simplefw_netns_ops);

	pr_info("simplefw module un-loaded\n");
}

module_init(simplefw_init);
module_exit(simplefw_exit);

MODULE_AUTHOR("Andrea Mayer <andrea.mayer@uniroma2.it>");
MODULE_DESCRIPTION("Drop ICMP packets on INET_PREROUTING. "
		   "!!! ATTENTION !!! this module crashes the kernel when receiving ICMP packets, on purpose");
MODULE_LICENSE("GPL");
