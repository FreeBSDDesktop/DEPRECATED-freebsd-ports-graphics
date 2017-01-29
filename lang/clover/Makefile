# $FreeBSD$

PORTNAME=	clover
PORTVERSION=	${MESAVERSION}
PORTREVISION=	4
CATEGORIES=	lang

COMMENT=	Mesa "Clover" OpenCL library

BUILD_DEPENDS=	opencl>=0:devel/opencl
LIB_DEPENDS=	libdrm.so:graphics/libdrm \
		libOpenCL.so:devel/ocl-icd \
		libexpat.so:textproc/expat2
RUN_DEPENDS=	opencl>=0:devel/opencl

USES=		ssl
USE_XORG=	glproto x11 xext xxf86vm xdamage xfixes dri2proto \
		presentproto xvmc xshmfence dri3proto

ONLY_FOR_ARCHS=	i386 amd64
ONLY_FOR_ARCHS_REASON=	Clover needs a graphics driver supported by the Radeon KMS driver


#BROKEN= Does not link. To be fixed.


.include <bsd.port.options.mk>
.include "${.CURDIR}/../../graphics/libGL/Makefile.common"

MESA_INSTALL_WRKSRC=	src/gallium/targets/opencl src/gallium/targets/pipe-loader

.if ${OPSYS} == FreeBSD && ${OSVERSION} < 1001000
IGNORE=		Clover is only supported on FreeBSD 10.1 and newer
.endif

.include "${.CURDIR}/../../graphics/libGL/Makefile.targets"

post-configure:
	@${CP} ${.CURDIR}/files/git_sha1.h  ${WRKSRC}/src/gallium/state_trackers/clover/.

post-install:
	@${MV} ${STAGEDIR}/etc/OpenCL ${STAGEDIR}${PREFIX}/etc/

.include <bsd.port.mk>
