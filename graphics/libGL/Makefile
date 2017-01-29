# Created by: Eric Anholt <anholt@FreeBSD.org>
# $FreeBSD$

PORTNAME=	libGL
PORTVERSION=	${MESAVERSION}
PORTREVISION=	6
CATEGORIES=	graphics

COMMENT=	OpenGL library that renders using GLX or DRI

LIB_DEPENDS+=	libdrm.so:graphics/libdrm \
		libglapi.so:graphics/libglapi \
		libexpat.so:textproc/expat2

OPTIONS_DEFINE_amd64=	DRI3
OPTIONS_DEFINE_i386=	DRI3
DRI3_DESC=	Experimental: Enable DRI3 support

OPTIONS_DEFAULT_amd64=	DRI3

USE_XORG=	glproto x11 xext xxf86vm xdamage xfixes dri2proto:both \
		presentproto xshmfence xcb

.include <bsd.port.options.mk>
.include "${.CURDIR}/Makefile.common"

MESA_BUILD_WRKSRC=	src/mapi src/glx
MESA_INSTALL_WRKSRC=	src/glx

.include "${.CURDIR}/Makefile.targets"

post-install:
	@${MKDIR} ${STAGEDIR}${PREFIX}/lib/.mesa
	@${MV} ${STAGEDIR}${PREFIX}/lib/libGL* \
		${STAGEDIR}${PREFIX}/lib/.mesa/
	@cd ${WRKSRC}/src && ${SETENV} DESTDIR=${STAGEDIR} \
		${MAKE_CMD} install-pkgconfigDATA install-glHEADERS \
		install-glxHEADERS

.include <bsd.port.mk>
