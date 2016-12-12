# Created by: Koop Mast <kwm@FreeBSD.org>
# $FreeBSD: head/devel/libdevq/Makefile 378958 2015-02-14 11:19:40Z kwm $

PORTNAME=	libdevq
PORTVERSION=	0.0.4
CATEGORIES=	devel
MASTER_SITES=	https://github.com/freebsd/${PORTNAME}/releases/download/${PORTVERSION}/

MAINTAINER=	x11@FreeBSD.org
COMMENT=	Generic Device Query and Monitor interface

LICENSE=	BSD2CLAUSE
LICENSE_FILE=	${WRKSRC}/COPYING

GNU_CONFIGURE=	yes
USES=		libtool pathfix tar:xz
USE_LDCONFIG=	yes
INSTALL_TARGET=	install-strip

OPTIONS_DEFINE=	PROGRAMS
OPTIONS_SUB=	yes
PROGRAMS_DESC=	Build diagnostic programs
PROGRAMS_CONFIGURE_ENABLE=programs

.include <bsd.port.mk>
