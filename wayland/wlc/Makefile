# $FreeBSD$

PORTNAME=	wlc
PORTVERSION=0.0.5
CATEGORIES=	wayland

MAINTAINER=	x11@FreeBSD.org
COMMENT=	Wayland compositor library

USE_GITHUB=	yes
GH_ACCOUNT=	cloudef
GH_TAGNAME=	fa780ce

GH_TUPLE=	cloudef:chck:3f27d2a:chck

USES=		cmake pkgconfig pathfix
USE_XORG=	x11 xcb xcursor
USE_GL=		egl gbm glesv2

LIB_DEPENDS+= \
		libxkbcommon.so:${PORTSDIR}/x11/libxkbcommon		\
		libX11.so:${PORTSDIR}/x11/libX11					\
		libxcb.so:${PORTSDIR}/x11/libxcb					\
		libxcb-ewmh.so:${PORTSDIR}/x11/xcb-util-wm			\
		libpixman-1.so:${PORTSDIR}/x11/pixman				\
		libcairo.so:${PORTSDIR}/graphics/cairo				\
		libwayland-server.so:${PORTSDIR}/wayland/wayland	\
		libwayland-client.so:${PORTSDIR}/wayland/wayland	\
		libwayland-cursor.so:${PORTSDIR}/wayland/wayland	\
		libdrm.so:${PORTSDIR}/graphics/libdrm				\
		libudev.so:${PORTSDIR}/wayland/libudev-devd			\
		libinput.so:${PORTSDIR}/wayland/libinput			\
		libxcb-image.so:${PORTSDIR}/x11/xcb-util-image

BUILD_DEPENDS+= \
		${NONEXISTENT}:${PORTSDIR}/wayland/wayland-protocols \
		wayland-scanner:${PORTSDIR}/wayland/wayland

RUN_DEPENDS+= \
		${LOCALBASE}/lib/libglapi.so.0:${PORTSDIR}/graphics/libglapi \
		${LOCALBASE}/share/X11/xkb/rules/base:${PORTSDIR}/x11/xkeyboard-config

CONFIGURE_ENV+=	\
		PKG_CONFIG_PATH=${STAGEDIR}${PREFIX}/libdata/pkgconfig

CFLAGS+=	-fcolor-diagnostics

CPPFLAGS+=	-I${LOCALBASE}/include

post-extract:
	@${RM} -r ${WRKSRC}/lib/chck
	@${LN} -s ${WRKSRC_chck} ${WRKSRC}/lib/chck

.include <bsd.port.mk>
