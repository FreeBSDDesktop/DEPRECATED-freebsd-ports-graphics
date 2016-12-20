# Created by: Eric Anholt <anholt@FreeBSD.org>
# $FreeBSD$

PORTNAME?=	xorg-server
PORTVERSION=	1.18.4
PORTREVISION?=	1
PORTEPOCH?=	1
CATEGORIES=	x11-servers
MASTER_SITES=	XORG/individual/xserver
DISTNAME=	xorg-server-${PORTVERSION}

MAINTAINER=	x11@FreeBSD.org
COMMENT?=	X.Org X server and related programs

LICENSE=	MIT

LIB_DEPENDS+=	libdrm.so:graphics/libdrm \
		libepoxy.so:graphics/libepoxy
RUN_DEPENDS+=	xkeyboard-config>=2.5:x11/xkeyboard-config \
		xkbcomp:x11/xkbcomp

XORG_CAT=	xserver
SLAVE_PORT?=	no

USES=		gmake libtool pkgconfig perl5 ssl tar:bzip2
USE_PERL5=	build
USE_GL=		gbm egl gl
USE_XORG?=	xf86driproto glproto xdmcp x11 xkbfile xxf86misc xxf86vm xaw7 \
		xmu xt xpm xext randrproto renderproto fixesproto damageproto \
		dri2proto dri3proto xcmiscproto xextproto xproto xtrans \
		xf86miscproto xf86vidmodeproto xf86bigfontproto scrnsaverproto \
		bigreqsproto resourceproto fontsproto inputproto xf86dgaproto \
		videoproto compositeproto trapproto presentproto recordproto \
		xineramaproto xinerama evieproto xfont fontenc xkbui pixman \
		pciaccess xshmfence
CONFIGURE_ARGS?= --disable-dmx --disable-xvfb --disable-xnest \
		--without-xmlto --disable-docs --disable-devel-docs \
		--localstatedir=/var --without-dtrace --disable-xephyr \
		--enable-record=yes --enable-glamor --disable-config-udev \
		--disable-config-udev-kms

INSTALL_TARGET=	install-strip

OPTIONS_SUB=	yes
OPTIONS_DEFINE=	SUID XWL
OPTIONS_RADIO=	CONF
OPTIONS_RADIO_CONF=	HAL DEVD UDEV
OPTIONS_DEFINE_amd64=	DRI3
OPTIONS_DEFINE_i386=	DRI3
SUID_DESC=	Install the Xorg server with setuid bit set
HAL_DESC=	Compile with HAL config support
DEVD_DESC=	Use devd for autoconfiguration of input devices
DRI3_DESC=	Experimental: Enable DRI3 support
XWL_DESC=	XWayland support.
UDEV_DESC=	Use udev for input devices.
OPTIONS_DEFAULT=SUID DEVD
# CONFIGURE_ENABLE write to CONFIGURE_ARGS so we have to move it
# below CONFIGURE_ARGS?=... otherwise args are never assigned!!
XWL_CONFIGURE_ENABLE=	xwayland
OPTIONS_EXCLUDE_sparc64=	HAL

.include <bsd.port.options.mk>

.if ${SLAVE_PORT} == "no"
SUB_FILES=	pkg-install pkg-deinstall
.else
# for slave ports we need to overwrite PLIST, so it doesn't overwrite
# PLIST_FILES, with the masterport plist.
PLIST=		${.CURDIR}/pkg-plist
.endif

.if ${OPSYS} == FreeBSD
USE_XORG+=	dri3proto presentproto xshmfence
CONFIGURE_ARGS+=	--enable-dri3
.else
CONFIGURE_ARGS+=	--disable-dri3
.endif

.if ${PORT_OPTIONS:MUDEV}
LIB_DEPENDS+=	libudev.so:${PORTSDIR}/wayland/libudev-devd
CONFIGURE_ARGS+= --enable-config-udev --disable-config-udev-kms --disable-systemd-logind
.endif

.if ${PORT_OPTIONS:MXWL}
BUILD_DEPENDS+=	${NONEXISTENT}:${PORTSDIR}/wayland/wayland-protocols
BUILD_DEPENDS+=	wayland-scanner:${PORTSDIR}/wayland/wayland
LIB_DEPENDS+=	libwayland-client.so:${PORTSDIR}/wayland/wayland
.endif

.include <bsd.port.pre.mk>

.if ${SSL_DEFAULT} == base
# The reason why I use this is cause openssl from base doesn't install a .pc file
# and configure will fail trying to find it. Setting both of those variables to
# a *non-empty* value by-passes the pkg-config check.
CONFIGURE_ENV+=	SHA1_LIB="-L/usr/lib -lcrypto" SHA1_CFLAGS="-I/usr/include"
.endif

.if ${PORT_OPTIONS:MHAL}
LIB_DEPENDS+=	libhal.so:sysutils/hal
CONFIGURE_ARGS+=	--enable-config-hal=yes
.else
CONFIGURE_ARGS+=	--enable-config-hal=no
.endif

# We handle Xorg setuid in the plist. This allows to build xorg-server as a user.
CONFIGURE_ARGS+=--disable-install-setuid

.if ${ARCH} == ia64
EXTRA_PATCHES+=	${FILESDIR}/extra-arch-ia64
.endif

.if ${ARCH} == powerpc || ${ARCH} == powerpc64
EXTRA_PATCHES+=	${FILESDIR}/extra-arch-powerpc
.endif

.if ${ARCH} == sparc64
PLIST_SUB+=	SPARC64=""
EXTRA_PATCHES+=	${FILESDIR}/extra-arch-sparc64
.else
PLIST_SUB+=	SPARC64="@comment "
.endif

.if ${PORT_OPTIONS:MSUID}
pre-everything::
	@${ECHO_MSG} "By default, the X Server installs as a set-user-id root binary. When run by"
	@${ECHO_MSG} "a normal user, it checks arguments and environment as done in the x11/wrapper"
	@${ECHO_MSG} "port before handling them normally.  If you are concerned about the security"
	@${ECHO_MSG} "of this, but still want to run an X Server (for example using xdm/kdm/gdm,"
	@${ECHO_MSG} "which will still run the server as root), you can cancel the build and set"
	@${ECHO_MSG} "xorg-server_UNSET=SUID in /etc/make.conf."
.endif

post-patch:
	@${REINPLACE_CMD} 's/test.*-traditional.*;/true;/' \
		${WRKSRC}/configure
# build libglx.so but don't install it yet. which is done in pre-install.
	@${REINPLACE_CMD} -e 's|@GLX_TRUE@GLXMODS =|@GLX_BOGUS@GLXMODS =|g' \
		-e 's|^LTLIBRARIES = |LTLIBRARIES = libglx.la |g' \
		${WRKSRC}/hw/xfree86/dixmods/Makefile.in

post-configure:
.if ${PORT_OPTIONS:MDEVD}
	@${REINPLACE_CMD} -e 's|config\.c|config.c devd.c|g' \
		-e 's|config\.lo|config.lo devd.lo|g' \
		${WRKSRC}/config/Makefile
	@${REINPLACE_CMD} -e 's|^/\* #undef CONFIG_UDEV \*/|#define CONFIG_DEVD 1|' \
		${WRKSRC}/include/dix-config.h
.endif

.if ${SLAVE_PORT} == "no"
post-install:
# The .xorg dir because else the xorg-server might not load the correct
# libglx module.
	@${MKDIR} ${STAGEDIR}${PREFIX}/lib/xorg/modules/extensions/.xorg
	${INSTALL_LIB} ${WRKSRC}/hw/xfree86/dixmods/.libs/libglx.so \
		${STAGEDIR}${PREFIX}/lib/xorg/modules/extensions/.xorg/
	@${MKDIR} ${STAGEDIR}${PREFIX}/etc/X11/xorg.conf.d
.endif # ! SLAVE_PORT

.include <bsd.port.post.mk>
