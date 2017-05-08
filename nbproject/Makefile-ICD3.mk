#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-ICD3.mk)" "nbproject/Makefile-local-ICD3.mk"
include nbproject/Makefile-local-ICD3.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=ICD3
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/SmartModule.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/SmartModule.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=--mafrlcsj
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS

else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=config.c main.c interrupt.c button.c sram.c serial_comm.c wifi.c linked_list.c buffer.c system.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/config.p1 ${OBJECTDIR}/main.p1 ${OBJECTDIR}/interrupt.p1 ${OBJECTDIR}/button.p1 ${OBJECTDIR}/sram.p1 ${OBJECTDIR}/serial_comm.p1 ${OBJECTDIR}/wifi.p1 ${OBJECTDIR}/linked_list.p1 ${OBJECTDIR}/buffer.p1 ${OBJECTDIR}/system.p1
POSSIBLE_DEPFILES=${OBJECTDIR}/config.p1.d ${OBJECTDIR}/main.p1.d ${OBJECTDIR}/interrupt.p1.d ${OBJECTDIR}/button.p1.d ${OBJECTDIR}/sram.p1.d ${OBJECTDIR}/serial_comm.p1.d ${OBJECTDIR}/wifi.p1.d ${OBJECTDIR}/linked_list.p1.d ${OBJECTDIR}/buffer.p1.d ${OBJECTDIR}/system.p1.d

# Object Files
OBJECTFILES=${OBJECTDIR}/config.p1 ${OBJECTDIR}/main.p1 ${OBJECTDIR}/interrupt.p1 ${OBJECTDIR}/button.p1 ${OBJECTDIR}/sram.p1 ${OBJECTDIR}/serial_comm.p1 ${OBJECTDIR}/wifi.p1 ${OBJECTDIR}/linked_list.p1 ${OBJECTDIR}/buffer.p1 ${OBJECTDIR}/system.p1

# Source Files
SOURCEFILES=config.c main.c interrupt.c button.c sram.c serial_comm.c wifi.c linked_list.c buffer.c system.c


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-ICD3.mk dist/${CND_CONF}/${IMAGE_TYPE}/SmartModule.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=18F27J13
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/config.p1: config.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/config.p1.d 
	@${RM} ${OBJECTDIR}/config.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=icd3  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/config.p1  config.c 
	@-${MV} ${OBJECTDIR}/config.d ${OBJECTDIR}/config.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/config.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/main.p1: main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.p1.d 
	@${RM} ${OBJECTDIR}/main.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=icd3  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/main.p1  main.c 
	@-${MV} ${OBJECTDIR}/main.d ${OBJECTDIR}/main.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/main.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/interrupt.p1: interrupt.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/interrupt.p1.d 
	@${RM} ${OBJECTDIR}/interrupt.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=icd3  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/interrupt.p1  interrupt.c 
	@-${MV} ${OBJECTDIR}/interrupt.d ${OBJECTDIR}/interrupt.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/interrupt.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/button.p1: button.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/button.p1.d 
	@${RM} ${OBJECTDIR}/button.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=icd3  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/button.p1  button.c 
	@-${MV} ${OBJECTDIR}/button.d ${OBJECTDIR}/button.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/button.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/sram.p1: sram.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/sram.p1.d 
	@${RM} ${OBJECTDIR}/sram.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=icd3  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/sram.p1  sram.c 
	@-${MV} ${OBJECTDIR}/sram.d ${OBJECTDIR}/sram.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/sram.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/serial_comm.p1: serial_comm.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/serial_comm.p1.d 
	@${RM} ${OBJECTDIR}/serial_comm.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=icd3  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/serial_comm.p1  serial_comm.c 
	@-${MV} ${OBJECTDIR}/serial_comm.d ${OBJECTDIR}/serial_comm.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/serial_comm.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/wifi.p1: wifi.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/wifi.p1.d 
	@${RM} ${OBJECTDIR}/wifi.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=icd3  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/wifi.p1  wifi.c 
	@-${MV} ${OBJECTDIR}/wifi.d ${OBJECTDIR}/wifi.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/wifi.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/linked_list.p1: linked_list.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/linked_list.p1.d 
	@${RM} ${OBJECTDIR}/linked_list.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=icd3  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/linked_list.p1  linked_list.c 
	@-${MV} ${OBJECTDIR}/linked_list.d ${OBJECTDIR}/linked_list.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/linked_list.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/buffer.p1: buffer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/buffer.p1.d 
	@${RM} ${OBJECTDIR}/buffer.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=icd3  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/buffer.p1  buffer.c 
	@-${MV} ${OBJECTDIR}/buffer.d ${OBJECTDIR}/buffer.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/buffer.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/system.p1: system.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/system.p1.d 
	@${RM} ${OBJECTDIR}/system.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=icd3  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/system.p1  system.c 
	@-${MV} ${OBJECTDIR}/system.d ${OBJECTDIR}/system.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/system.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
else
${OBJECTDIR}/config.p1: config.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/config.p1.d 
	@${RM} ${OBJECTDIR}/config.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/config.p1  config.c 
	@-${MV} ${OBJECTDIR}/config.d ${OBJECTDIR}/config.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/config.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/main.p1: main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.p1.d 
	@${RM} ${OBJECTDIR}/main.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/main.p1  main.c 
	@-${MV} ${OBJECTDIR}/main.d ${OBJECTDIR}/main.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/main.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/interrupt.p1: interrupt.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/interrupt.p1.d 
	@${RM} ${OBJECTDIR}/interrupt.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/interrupt.p1  interrupt.c 
	@-${MV} ${OBJECTDIR}/interrupt.d ${OBJECTDIR}/interrupt.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/interrupt.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/button.p1: button.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/button.p1.d 
	@${RM} ${OBJECTDIR}/button.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/button.p1  button.c 
	@-${MV} ${OBJECTDIR}/button.d ${OBJECTDIR}/button.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/button.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/sram.p1: sram.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/sram.p1.d 
	@${RM} ${OBJECTDIR}/sram.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/sram.p1  sram.c 
	@-${MV} ${OBJECTDIR}/sram.d ${OBJECTDIR}/sram.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/sram.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/serial_comm.p1: serial_comm.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/serial_comm.p1.d 
	@${RM} ${OBJECTDIR}/serial_comm.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/serial_comm.p1  serial_comm.c 
	@-${MV} ${OBJECTDIR}/serial_comm.d ${OBJECTDIR}/serial_comm.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/serial_comm.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/wifi.p1: wifi.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/wifi.p1.d 
	@${RM} ${OBJECTDIR}/wifi.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/wifi.p1  wifi.c 
	@-${MV} ${OBJECTDIR}/wifi.d ${OBJECTDIR}/wifi.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/wifi.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/linked_list.p1: linked_list.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/linked_list.p1.d 
	@${RM} ${OBJECTDIR}/linked_list.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/linked_list.p1  linked_list.c 
	@-${MV} ${OBJECTDIR}/linked_list.d ${OBJECTDIR}/linked_list.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/linked_list.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/buffer.p1: buffer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/buffer.p1.d 
	@${RM} ${OBJECTDIR}/buffer.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/buffer.p1  buffer.c 
	@-${MV} ${OBJECTDIR}/buffer.d ${OBJECTDIR}/buffer.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/buffer.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/system.p1: system.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/system.p1.d 
	@${RM} ${OBJECTDIR}/system.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=350    -o${OBJECTDIR}/system.p1  system.c 
	@-${MV} ${OBJECTDIR}/system.d ${OBJECTDIR}/system.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/system.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/SmartModule.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) --chip=$(MP_PROCESSOR_OPTION) -G -mdist/${CND_CONF}/${IMAGE_TYPE}/SmartModule.X.${IMAGE_TYPE}.map  -D__DEBUG=1 --debugger=icd3  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=1090        $(COMPARISON_BUILD) --memorysummary dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml -odist/${CND_CONF}/${IMAGE_TYPE}/SmartModule.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}     
	@${RM} dist/${CND_CONF}/${IMAGE_TYPE}/SmartModule.X.${IMAGE_TYPE}.hex 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/SmartModule.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) --chip=$(MP_PROCESSOR_OPTION) -G -mdist/${CND_CONF}/${IMAGE_TYPE}/SmartModule.X.${IMAGE_TYPE}.map  --double=32 --float=24 --emi=byteselect --opt=none --addrqual=require -P -N255 --warn=0 --asmlist -DXPRJ_ICD3=$(CND_CONF)  --summary=default,+psect,-class,+mem,-hex,+file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,-config,+clib,+plib --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s" --MSGDISABLE=1090     $(COMPARISON_BUILD) --memorysummary dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml -odist/${CND_CONF}/${IMAGE_TYPE}/SmartModule.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}     
	
	@echo Normalizing hex file
	@"C:/Program Files (x86)/Microchip/MPLABX/v3.60/mplab_ide/platform/../mplab_ide/modules/../../bin/hexmate" --edf="C:/Program Files (x86)/Microchip/MPLABX/v3.60/mplab_ide/platform/../mplab_ide/modules/../../dat/en_msgs.txt" dist/${CND_CONF}/${IMAGE_TYPE}/SmartModule.X.${IMAGE_TYPE}.hex -odist/${CND_CONF}/${IMAGE_TYPE}/SmartModule.X.${IMAGE_TYPE}.hex

endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/ICD3
	${RM} -r dist/ICD3

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
