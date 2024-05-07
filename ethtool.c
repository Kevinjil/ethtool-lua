#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#include "lua.h"
#include "lauxlib.h"

#define MODNAME "ethtool"

static int sock_ioctl = -1;

static int ethtool_lua_statistics(lua_State *L)
{
	const char *ifname = luaL_checkstring(L, 1);
	struct ifreq ifr = {0};
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);

	struct ethtool_drvinfo drvinfo;
	struct ethtool_gstrings *strings;
	struct ethtool_stats *stats;
	unsigned int n_stats, sz_str, sz_stats, i;
	int err;

	drvinfo.cmd = ETHTOOL_GDRVINFO;
	ifr.ifr_data = (void *)&drvinfo;
	err = ioctl(sock_ioctl, SIOCETHTOOL, &ifr);
	if (err < 0)
	{
		lua_pushnil(L);
		lua_pushstring(L, "Cannot get driver information");
		return 2;
	}

	n_stats = drvinfo.n_stats;
	if (n_stats < 1)
	{
		lua_pushnil(L);
		lua_pushstring(L, "No stats available");
		return 2;
	}

	sz_str = n_stats * ETH_GSTRING_LEN;
	sz_stats = n_stats * sizeof(uint64_t);

	strings = calloc(1, sz_str + sizeof(struct ethtool_gstrings));
	stats = calloc(1, sz_stats + sizeof(struct ethtool_stats));
	if (!strings || !stats)
	{
		lua_pushnil(L);
		lua_pushstring(L, "No memory available");
		return 2;
	}

	strings->cmd = ETHTOOL_GSTRINGS;
	strings->string_set = ETH_SS_STATS;
	strings->len = n_stats;
	ifr.ifr_data = (void *)strings;
	err = ioctl(sock_ioctl, SIOCETHTOOL, &ifr);
	if (err < 0)
	{
		free(strings);
		free(stats);
		lua_pushnil(L);
		lua_pushstring(L, "Cannot get stats strings information");
		return 2;
	}

	stats->cmd = ETHTOOL_GSTATS;
	stats->n_stats = n_stats;
	ifr.ifr_data = (void *)stats;
	err = ioctl(sock_ioctl, SIOCETHTOOL, &ifr);
	if (err < 0)
	{
		free(strings);
		free(stats);
		lua_pushnil(L);
		lua_pushstring(L, "Cannot get stats information");
		return 2;
	}

	lua_newtable(L);
	for (i = 0; i < n_stats; i++)
	{
		lua_pushinteger(L, stats->data[i]);
		lua_setfield(L, -2, (const char *)&strings->data[i * ETH_GSTRING_LEN]);
	}

	free(strings);
	free(stats);

	return 1;
}

static const luaL_Reg ethtool[] = {
	{"statistics", ethtool_lua_statistics},
	{0, 0}};

int luaopen_ethtool(lua_State *L)
{
	sock_ioctl = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if (sock_ioctl == -1)
	{
		lua_pushnil(L);
		lua_pushstring(L, "Could not open ioctl socket");
		return 2;
	}

	/* create module */
	luaL_register(L, MODNAME, ethtool);

	return 0;
}
