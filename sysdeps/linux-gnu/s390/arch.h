/*
** S/390 version
** (C) Copyright 2001 IBM Poughkeepsie, IBM Corporation
*/

#define BREAKPOINT_VALUE { 0x00, 0x01 }
#define BREAKPOINT_LENGTH 2
#define DECR_PC_AFTER_BREAK 2

#define LT_ELFCLASS	ELFCLASS32
#define LT_ELF_MACHINE	EM_S390
