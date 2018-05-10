/**
 * @file		tmutils.h
 * @brief		Contiene include e dichiarazioni per la libreria \b tmUtilspp
 * @author		Mykola Yanovych
 * @version		2.2
 * @date		31/03/2010
 *
 * @mainpage	La libreria tmUtilspp per la gestione di log file
 * @author		Mykola Yanovych
 * @version		2.2
 * \date		31/03/2010
 * @par			Descrizione:
 *				La libreria \b tmUtilspp permette di gestire funzionalita' di
 *				logging stampando messaggi di log sullo schermo e/o in
 *				file di log.\n\n
 *				Inoltre e' possibile avere per ogni funzione del programma un
 *				file di log separato. In tal caso tutti i messaggi di log
 *				prodotti dalla funzione verranno scritti solo nel file
 *				corrispondente.
 * @par			Interacce:
 *				Per semplificare l'uso dei metodi della classe \b CTrace sono
 *				stati definite alcune \b MACRO che permettono effettuare le chiamate
 *				della libreria in forma abbreviata. Le macro dell'interfaccia si
 *				trovano nel file tmtrace.h.
 * @par			Formato di output:
 *				Le tipiche stringe di output prodotte dalla chiamata CTrace::tmTrace:
 * @code 2010-03-31 12:37:26.828 TRACE    main() plog.cpp [00065] - Trace message
 * 2010-03-31 12:37:26.843 WARNING  main() plog.cpp [00066] - Warning message
 * 2010-03-31 12:37:26.890 ERROR    main() plog.cpp [00067] - Error message @endcode \n
 *				Le tipiche stringe di output prodotte dalle chiamate CTrace::tmStart
 *				e CTrace::tmEnd sono seguenti:
 * @code 2010-03-31 12:37:26.781 Function main() - STARTED
 * 2010-03-31 12:37:30.203 Function main() - ENDED @endcode
 */
#pragma once
/*
#ifndef TMUTILS_H
#define TMUTILS_H
*/
/** @cond */
#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#if defined(WIN32) || defined(_WIN32)
	#ifndef PATH_MAX
		#define PATH_MAX 1024
	#endif
#endif
/** @endcond */

#ifndef DllExport
	/** @brief	Definisce l'attributo per esportazione da .dll */
	#define DllExport __declspec( dllexport )
#endif

// Trace level
#define DBG_LOW         0	//!< Verranno stampati messaggi di errore
#define DBG_MEDIUM      1	//!< Verranno stampati messaggi di errore e di warning
#define DBG_HIGH        2	//!< Verranno stampati tutti i messaggi

#define DISPLAY_YES		1	//!< Stampa messaggi in log file e sullo schermo
#define DISPLAY_NO		0	//!< Stampa messaggi solo in log file

#define SINGLE_YES		1	//!< Stampa messaggi in log file e sullo schermo
#define SINGLE_NO		   0	//!< Stampa messaggi solo in log file

// Trace message type
#define TM_TRACE        0	//!< Messaggio di tracing
#define TM_WARNING     	1	//!< Messaggio di warning
#define TM_ERROR       	2	//!< Messaggio di errore
#define TM_PROC        	3	//!< Messaggio di inizio/fine funzione

#define FIRST_LOG		1	//!< Numero del primo log file nella sequenza

// Maximum trace file length
#define MAX_LOG_FILE	2000000000L		//!< Lunghezza massima del file in byte

#include "tmtrace.h"
#include "tmother.h"

//#endif
