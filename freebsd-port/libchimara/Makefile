# New ports collection makefile for:   libchimara
# Date created:        30 November 2009
# Whom:                W. M. van Vliet
#
# $FreeBSD$
#

PORTNAME=      libchimara
PORTVERSION=   0.1
CATEGORIES=    devel
MASTER_SITES=  http://lassie.student.utwente.nl/chimara/
DISTNAME=     chimara-0.1

MAINTAINER=    w.m.vanvliet@student.utwente.nl
COMMENT=       A GTK widget that implements the GLK interface for interactive fiction.

LIB_DEPENDS=   libchimara.0:${PORTSDIR}/devel/libchimara \
		       gtk-x11-2.0:${PORTSDIR}/x11-toolkits/gtk20 \
			   glib-2.0:${PORTSDIR}/devel/glib20

GNU_CONFIGURE= yes
USE_GMAKE=     yes


.include <bsd.port.mk>
