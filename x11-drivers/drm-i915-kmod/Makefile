# Created by:	Jean-Sebastien Pedron <dumbbell@FreeBSD.org>
# $FreeBSD$

PORTNAME=	drm-i915-kmod
DISTVERSION=	9.275699
CATEGORIES=	x11-drivers
#MASTER_SITES=	LOCAL/dumbbell
MASTER_SITES=	http://people.freebsd.org/~dumbbell/
DISTNAME=	drm-i915-${DISTVERSION}

MAINTAINER=	x11@FreeBSD.org
COMMENT=	FreeBSD 9.3 Kernel DRM driver for i915 GPUs

LICENSE=	MIT

USES=		tar:xz kmod
ONLY_FOR_ARCHS=	i386 amd64

PLIST_FILES=	${KMODDIR}/drm2_port.ko \
		${KMODDIR}/i915kms_port.ko

.include <bsd.port.options.mk>

.if (${OSVERSION} < 903000 || ${OSVERSION} > 1000000)
IGNORE=		This port is only for 9.3 systems.
.endif

.if ${PORT_OPTIONS:MDEBUG}
MAKE_ARGS+=" HAVE_DEBUG=YES"
.endif

.include <bsd.port.mk>
