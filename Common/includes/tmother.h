/**
 * @file		tmother.h
 * @brief		Contiene le dichiarazioni delle funzioni communi
 * @author		Mykola Yanovych
 * @version		2.2
 * @date		31/03/2010
 */
#pragma once
/*
#ifndef TMOTHER_H
#define TMOTHER_H
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>

#if !defined (WIN32) && !defined (_WIN32)
	#include <limits.h>
#endif

/** @cond */
#define MAX_TS	50
/** @endcond */

#if defined(WIN32) || defined(_WIN32)
	DllExport char *trim( char *str );
	DllExport char *ltrim( char *str );
	DllExport char *rtrim( char *str );
	DllExport void tmSleep( const double wait );
	DllExport char *CommaPrint( const char *fmt, const double value );
#else
	char *trim( char *string );
	char *ltrim( char *string );
	char *rtrim( char *string );
	void tmSleep( const double wait );
	char *CommaPrint( const char *fmt, const double value );
#endif

//#endif
