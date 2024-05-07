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
#define METANAME MODNAME ".meta"

#define ETHTOOL_ERROR_UNKNOWN -1

struct ethtool_lua_socket
{
	int sock_ioctl;
};

static inline int ethtool_lua_error(lua_State *L, const char *message)
{
	lua_pushnil(L);
	lua_pushstring(L, message);
	return 2;
}

static int ethtool_lua_open(lua_State *L)
{
	struct ethtool_lua_socket *s;

	if ((s = lua_newuserdata(L, sizeof(*s))) == NULL)
		return ethtool_lua_error(L, "Could not allocate userdata");
	if ((s->sock_ioctl = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)
		return ethtool_lua_error(L, "Could not open ioctl socket");

	luaL_getmetatable(L, METANAME);
	lua_setmetatable(L, -2);
	return 1;
}

static int ethtool_lua_gc(lua_State *L)
{
	struct ethtool_lua_socket *s = luaL_checkudata(L, 1, METANAME);

	if (s->sock_ioctl != -1)
	{
		shutdown(s->sock_ioctl, 2);
		s->sock_ioctl = -1;
	}

	return 0;
}

static int ethtool_lua_statistics(lua_State *L)
{
	struct ethtool_lua_socket *s = luaL_checkudata(L, 1, METANAME);
	const char *ifname = luaL_checkstring(L, 2);

	struct ifreq ifr = {0};
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);

	struct ethtool_drvinfo drvinfo;
	struct ethtool_gstrings *strings;
	struct ethtool_stats *stats;
	unsigned int n_stats, sz_str, sz_stats, i;
	int err;

	drvinfo.cmd = ETHTOOL_GDRVINFO;
	ifr.ifr_data = (void *)&drvinfo;
	err = ioctl(s->sock_ioctl, SIOCETHTOOL, &ifr);
	if (err < 0)
	{
		return ethtool_lua_error(L, "Cannot get driver information");
	}

	n_stats = drvinfo.n_stats;
	if (n_stats < 1)
	{
		return ethtool_lua_error(L, "No stats available");
	}

	sz_str = n_stats * ETH_GSTRING_LEN;
	sz_stats = n_stats * sizeof(uint64_t);

	strings = calloc(1, sz_str + sizeof(struct ethtool_gstrings));
	stats = calloc(1, sz_stats + sizeof(struct ethtool_stats));
	if (!strings || !stats)
	{
		return ethtool_lua_error(L, "No memory available");
	}

	strings->cmd = ETHTOOL_GSTRINGS;
	strings->string_set = ETH_SS_STATS;
	strings->len = n_stats;
	ifr.ifr_data = (void *)strings;
	err = ioctl(s->sock_ioctl, SIOCETHTOOL, &ifr);
	if (err < 0)
	{
		free(strings);
		free(stats);
		return ethtool_lua_error(L, "Cannot get stats strings information");
	}

	stats->cmd = ETHTOOL_GSTATS;
	stats->n_stats = n_stats;
	ifr.ifr_data = (void *)stats;
	err = ioctl(s->sock_ioctl, SIOCETHTOOL, &ifr);
	if (err < 0)
	{
		free(strings);
		free(stats);
		return ethtool_lua_error(L, "Cannot get stats information");
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
	{"open", ethtool_lua_open},
	{0, 0}};

static const luaL_Reg ethtool_meta[] = {
	{"__gc", ethtool_lua_gc},
	{"close", ethtool_lua_gc},
	{"statistics", ethtool_lua_statistics},
	{0, 0}};

int luaopen_ethtool(lua_State *L)
{
	/* create metatable */
	luaL_newmetatable(L, METANAME);

	/* metatable.__index = metatable */
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	/* fill metatable */
	luaL_register(L, NULL, ethtool_meta);
	lua_pop(L, 1);

	/* create module */
	luaL_register(L, MODNAME, ethtool);

	return 0;
}
