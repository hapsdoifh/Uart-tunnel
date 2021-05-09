#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=mips-openwrt-linux-gcc
CCC=mips-openwrt-linux-g++
CXX=mips-openwrt-linux-g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU_GWS5000_4_4_14-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/c-serial-com.o \
	${OBJECTDIR}/src/instance_ctrl.o \
	${OBJECTDIR}/src/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../libgws

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uart-tunnel

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uart-tunnel: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uart-tunnel ${OBJECTFILES} ${LDLIBSOPTIONS} -lpthread -lgws_sk

${OBJECTDIR}/src/c-serial-com.o: src/c-serial-com.c
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I. -Iinclude -I../libgws -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/c-serial-com.o src/c-serial-com.c

${OBJECTDIR}/src/instance_ctrl.o: src/instance_ctrl.c
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I. -Iinclude -I../libgws -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/instance_ctrl.o src/instance_ctrl.c

${OBJECTDIR}/src/main.o: src/main.c
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I. -Iinclude -I../libgws -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/main.o src/main.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
