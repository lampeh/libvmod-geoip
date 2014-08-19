/**
 * libvmod-geoip - varnish interface to MaxMind's GeoIP library
 * GeoIP API: http://www.maxmind.com/app/c
 *
 * See file README.rst for usage instructions
 * 
 * This code is licensed under a MIT-style License, see file LICENSE
*/


#include <stdlib.h>
#include <GeoIP.h>
#include <GeoIPCity.h>
#include "vrt.h"
#include "vrt_obj.h"
#include "bin/varnishd/cache.h"

#include "vcc_if.h"


// The default string in case the GeoIP lookup fails
#define GI_UNKNOWN_STRING "Unknown"


int
init_function(struct vmod_priv *pp, const struct VCL_conf *conf)
{
	// first call to lookup functions initializes pp
	return (0);
}

static void
init_priv(struct vmod_priv *pp, int type, int flags) {
	//GeoIPCity.dat
	//GeoIP.dat
	// The README says:
	// If GEOIP_MMAP_CACHE doesn't work on a 64bit machine, try adding
	// the flag "MAP_32BIT" to the mmap call. MMAP is not avail for WIN32.
   	pp->priv = GeoIP_open_type(type, flags);
	if (pp->priv != NULL) {
		pp->free = (vmod_priv_free_f *)GeoIP_delete;
		GeoIP_set_charset((GeoIP *)pp->priv, GEOIP_CHARSET_UTF8);
	}
}



const char *
vmod_country_code(struct sess *sp, struct vmod_priv *pp, const char *ip)
{
	const char* country = NULL;

	//GeoIP.dat is the default free database from MaxMind. It only maps country names to IP Addresses
	//http://dev.maxmind.com/geoip/legacy/geolite/
	if (!pp->priv) {
		init_priv(pp,1,GEOIP_MMAP_CACHE);
	}

	if (ip) {
		country = GeoIP_country_code_by_addr((GeoIP *)pp->priv, ip);
	}

	return(WS_Dup(sp->wrk->ws, (country ? country : GI_UNKNOWN_STRING)));
}

const char *
vmod_client_country_code(struct sess *sp, struct vmod_priv *pp) {
	return vmod_country_code(sp, pp, VRT_IP_string(sp, VRT_r_client_ip(sp)));
}

const char *
vmod_ip_country_code(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
	return vmod_country_code(sp, pp, VRT_IP_string(sp, ip));
}


const char *
vmod_country_name(struct sess *sp, struct vmod_priv *pp, const char *ip)
{
	const char* country = NULL;

	//GeoIP.dat is the default free database from MaxMind. It only maps country names to IP Addresses
	//http://dev.maxmind.com/geoip/legacy/geolite/
	if (!pp->priv) {
		init_priv(pp,1,GEOIP_MMAP_CACHE);
	}

	if (ip) {
		country = GeoIP_country_name_by_addr((GeoIP *)pp->priv, ip);
	}

	return(WS_Dup(sp->wrk->ws, (country ? country : GI_UNKNOWN_STRING)));
}

const char *
vmod_client_country_name(struct sess *sp, struct vmod_priv *pp) {
	return vmod_country_name(sp, pp, VRT_IP_string(sp, VRT_r_client_ip(sp)));
}

const char *
vmod_ip_country_name(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
	return vmod_country_name(sp, pp, VRT_IP_string(sp, ip));
}

const char *
vmod_city_region_name(struct sess *sp, struct vmod_priv * pp, const char *ip)
{
	GeoIPRecord *gir;
	const char* region = NULL;

	//The GeoIPCity.dat file is a subscription file from MaxMind
	//The free version, GeoCityLite.dat, can be downloaded from  http://dev.maxmind.com/geoip/legacy/geolite/ 
	if (!pp->priv) {
		init_priv(pp,2,GEOIP_MMAP_CACHE);
	}

	if (ip) {		
		if (gir = GeoIP_record_by_addr((GeoIP *)pp->priv, ip)) {
			region = gir->region;
			GeoIPRecord_delete(gir);
		}
	}

	return(WS_Dup(sp->wrk->ws, (region ? region : GI_UNKNOWN_STRING)));
}

const char *
vmod_client_city_region_name(struct sess *sp, struct vmod_priv *pp) 
{
	return vmod_city_region_name(sp, pp, VRT_IP_string(sp, VRT_r_client_ip(sp)));
}


const char *
vmod_ip_city_region_name(struct sess * sp, struct vmod_priv *pp, struct sockaddr_storage *ip)
{
	return vmod_city_region_name(sp, pp, VRT_IP_string(sp, ip));
}

const char *
vmod_region_name(struct sess *sp, struct vmod_priv *pp, const char *ip)
{
	GeoIPRegion *gir;
	const char* region = NULL;

	//The GeoIPRegion.dat is a subscription service from MaxMind
	if (!pp->priv) { 
		init_priv(pp,3,GEOIP_MMAP_CACHE);
	}

	if (ip) {
		if (gir = GeoIP_region_by_addr((GeoIP *)pp->priv, ip)) {
			region = GeoIP_region_name_by_code(gir->country_code, gir->region);
			// TODO: is gir* a local copy or the actual record?
			GeoIPRegion_delete(gir);
		}
	}

	return(WS_Dup(sp->wrk->ws, (region ? region : GI_UNKNOWN_STRING)));
}

const char *
vmod_client_region_name(struct sess *sp, struct vmod_priv *pp) {
	return vmod_region_name(sp, pp, VRT_IP_string(sp, VRT_r_client_ip(sp)));
}

const char *
vmod_ip_region_name(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
	return vmod_region_name(sp, pp, VRT_IP_string(sp, ip));
}
