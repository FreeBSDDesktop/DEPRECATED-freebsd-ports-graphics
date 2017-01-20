# Created by: Johannes Lundberg <johalun0@gmail.com>
# $FreeBSD$

PORTNAME=	meslo
DISTVERSION=	1.2.1
PORTVERSION=	20170120
PORTREVISION=	1
CATEGORIES=	x11-fonts
MASTER_SITES=	https://github.com/andreberg/Meslo-Font/raw/master/dist/v${DISTVERSION}/
DISTNAME=		Meslo%20LG%20v${DISTVERSION}

MAINTAINER=	x11@FreeBSD.org
COMMENT=	Customized version of Apple Menlo font.

EXTRACT_SUFX=	.zip
LICENSE=	APACHE20
USES=		fonts zip
FONTNAME=	TTF
NO_BUILD=	yes
WRKSRC=		${WRKDIR}/Meslo\ LG\ v${DISTVERSION}

do-install:
	@${MKDIR} ${STAGEDIR}${FONTSDIR}
	${INSTALL_DATA} ${WRKSRC}/*.ttf ${STAGEDIR}${FONTSDIR}

.include <bsd.port.mk>
