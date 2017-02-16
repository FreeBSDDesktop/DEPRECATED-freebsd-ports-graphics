# Created by: Michael Gmelin <freebsd@grem.de>
# $FreeBSD$

PORTNAME=		intel-backlight
PORTVERSION=	20170215
CATEGORIES=		graphics

MAINTAINER=		freebsd@grem.de
COMMENT=		Control backlight on various modern Intel(R) GPUs

LIB_DEPENDS=	libdrm.so:graphics/libdrm \
			libpciaccess.so:devel/libpciaccess

USE_GITHUB=		yes
GH_ACCOUNT=		FreeBSDDesktop
GH_PROJECT=		intel_backlight_fbsd
GH_TAGNAME=		9cc6d99

OPTIONS_DEFINE=	SUID
SUID_DESC=	Install intel_backlight binary with setuid bit set
OPTIONS_DEFAULT= SUID

PLIST_FILES=		bin/intel_backlight
PORTEXAMPLES=		isl_backlight.sh asmc_backlight.sh

ONLY_FOR_ARCHS=		amd64 i386
ONLY_FOR_ARCHS_REASON=	This hardware is only found on Intel x86 systems

do-build:
	cd ${WRKSRC} && \
	${CC} ${CFLAGS} -o intel_backlight -I${PREFIX}/include \
		-I${PREFIX}/include/libdrm  -L${PREFIX}/lib -ldrm_intel \
		-lpciaccess intel_reg_map.c intel_mmio.c intel_backlight.c \
		intel_drm.c intel_pci.c

.include <bsd.port.options.mk>

do-install:
	${INSTALL_PROGRAM} ${WRKSRC}/intel_backlight ${STAGEDIR}${PREFIX}/bin
.if ${PORT_OPTIONS:MSUID}
	chmod u+s ${STAGEDIR}${PREFIX}/bin/intel_backlight
.endif
	${MKDIR} ${STAGEDIR}${EXAMPLESDIR}
	${INSTALL_DATA} ${WRKSRC}/isl_backlight.sh ${STAGEDIR}${EXAMPLESDIR}
	${INSTALL_DATA} ${WRKSRC}/asmc_backlight.sh ${STAGEDIR}${EXAMPLESDIR}

.include <bsd.port.mk>
