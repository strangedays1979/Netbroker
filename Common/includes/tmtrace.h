/**
* @file		tmtrace.h
* @brief		Contiene la dichiarazione della classe LTrace
* @author		Mykola Yanovych
* @version		2.2
* @date		31/03/2010
*/
#pragma once
/*
#ifndef TMTM_TRACE_H
#define TMTM_TRACE_H
*/
// C++ headers
#include <iostream>
#include <string>
#include <map>

#if defined (WIN32) || defined (_WIN32)
#include <windows.h>
#endif




// Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined (WIN32) || defined (_WIN32)
#include <process.h>
#include <direct.h>
#else
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include <sys/time.h>
#endif

using namespace std;

/** @cond */
#define M_FUNCTION	"FUNCTION"
#define M_ERROR		"ERROR   "
#define M_WARNING	   "WARNING "
#define M_TRACE		"TRACE   "

#define START		   "STARTED"
#define END			   "ENDED"

//namespace boost
//{
//    class recursive_mutex;
//}

/** @endcond */

/**
* @class	LTrace
* @brief	Implementa le funzionalita' di logging.
*
* La classe LTrace puo' essere usata per creare sia un
* singolo file di log che un log file per funzione.
*/
class LTrace {

    //boost::recursive_mutex *m_pSingletonMutex;
   CRITICAL_SECTION TraceMutex;

   /**
    * @struct	LogInfo
    * @brief	Contiene parametri di log file.
    */
   typedef struct {
      char		callerFunc[128];		   //!< Nome della funzione chiamante
      FILE	   *fd;						   //!< Struttura di log file
      int			Display;				      //!< Flag visualizzazione sullo schermo
      int			LogExist;				   //!< Flag esistenza di log file
      int			LogLevel;				   //!< Livello di debug
      int			DispLevel;				   //!< Livello di debug sullo schermo 
      long		logFileLength;			   //!< Lunghezza massima di log file
      char		PrevLogFile[PATH_MAX];	//!< Nome precedente di log file
      char		BaseLogFile[PATH_MAX];	//!< Nome corrente di log file
      char		LogFileDir[PATH_MAX];	//!< La cartella base di log file
      int			logFileNum;				   //!< Numero sequenziale di log file
      int			logId;					   //!< Identificativo univoco di log file
   } LogInfo;

   LogInfo	li;								//!< Struttura di un singolo log file
   int		SingleLog;						//!< Flag di un solo log file
   int		SingleLogFlag;					//!< Flag di un solo log file
   int		logIdMax;						//!< Valore massimo corrente di logId

   map <const char *, LogInfo> logPool;				//!< Pool dei log file aperti
   map <const char *, LogInfo>::iterator logIter;	//!< Iteratore del pool

   /**
    * @brief	Controlla la linghezza di log file.
    *
    * Se lunghezza supera quella di default, il file di log
    * verra' suddiviso in piu' parti.
    *
    * @param [in]	callerFunc	Nome della funzione chiamante
    */
   int CheckLogFile ( const int logId, const char *callerFunc );

   /**
    * @brief					Prepara messaggio per la stampa.
    * @param [in]	callerFunc	Nome della funzione chiamante
    * @param [in]	MsgType		Tipo messaggio da stampare
    * @param [out]	buf			Buffer di messaggio
    * @return  0	Successo
    * @return -1	Errore
    */
   int tmLogPrepare ( const int logId, const char *callerFunc, int MsgType, char *buf );

public:
   /**
    * @brief Metodo Singleton.
    *
    * Questo metodo implementa il Singleton pattern che permette
    * di avere una sola istanza della classe in questione.
    *
    * @return L'istanza della classe LTrace
    */
   static LTrace& Instance()
   {
      static LTrace instance;
      return instance;
   }

#if defined(WIN32) || defined(_WIN32)

public:
   /** 
    * @brief Costruttore di default.
    */
   DllExport LTrace();

   /**
    * @brief Distruttore di default.
    */
   DllExport ~LTrace();

public:
   /**
    * @brief					Inizializza i parametri di log file.
    * @param [in] callerFunc	Nome della funzione chiamante
    * @param [in] trcfile		Nome base di log file.\n
    *		Al nome base di log file verra' aggiunto il PID del processo,
    *		la data e numero del file in caso di parti separati.
    * @param [in] level		Livello di log.\n
    *		Livello puo' avere seguenti valori:\n
    *		@li DBG_HIGH   - Alto
    *		@li DBG_MEDIUM - Medio
    *		@li DBG_LOW    - Basso
    * @param [in] display		Flag della stampa sullo schermo.\n
    *		Flag della stampa puo' avere seguenti valori:\n
    *		@li DISPLAY_YES - Stampa messaggi in log file e sullo schermo
    *		@li DISPLAY_NO  - Stampa messaggi solo in log file
    * @param [in] logLength	Lunghezza massima di log file
    * @note
    *		@li Per creare un solo file di log e' neccessario chiamare
    *		tmLogInit() una sola volta all'inizio del programma.\n
    *		@li Per creare un log file per funzione e' neccessario chiamare
    *		tmLogInit() all'inizio della funzione desiderata.
    */
   DllExport int tmLogInit	(
      const char *callerFunc,
      const char *trcfile = NULL,
      const int   level = DBG_LOW,
      const int   display = DISPLAY_NO,
      const int   dispLevel = DBG_LOW,
      const int	  single = SINGLE_YES,
      const long  logLength = MAX_LOG_FILE
   );

   /**
    * @brief					Inizializza il livello di log.
    * @param [in] callerFunc	Nome della funzione chiamante
    * @param [in] level		Livello di log.\n
    *		Livello puo' avere seguenti valori:\n
    *		@li DBG_HIGH   - Alto
    *		@li DBG_MEDIUM - Medio
    *		@li DBG_LOW    - Basso
    */
   DllExport void tmLogLevel	( const int logId, const char *callerFunc, int level );

   /**
    * @brief					Inizializza il livello di log.
    * @param [in] callerFunc	Nome della funzione chiamante
    * @param [in] level		Livello di log.\n
    *		Livello puo' avere seguenti valori:\n
    *		@li DBG_HIGH   - Alto
    *		@li DBG_MEDIUM - Medio
    *		@li DBG_LOW    - Basso
    */

   DllExport void tmDispLevel	( const int logId, const char *callerFunc, int level );
   /**
    * @brief					Stampa messaggio dell'inizio di una funzione.
    * @param [in]	callerFunc	Nome della funzione chiamante
    */
   DllExport void tmStart		( const int logId, const char *callerFunc );

   /**
    * @brief					Stampa messaggio della fine di una funzione.
    * @param [in]	callerFunc	Nome della funzione chiamante
    */ 
   DllExport void tmEnd		( const int logId, const char *callerFunc );

   /**
    * @brief					Stampa messaggio di log.
    * @param [in] MsgType		Tipo messaggio da stampare.\n
    *		Tipo messaggio da stampare puo' avere gli seguenti valori:\n
    *		@li TM_TRACE	- Messaggio di trace
    *		@li TM_WARNING	- Messaggio di warning
    *		@li TM_ERROR	- Messaggio di errore
    *		@li TM_PROC		- Messaggio di inizio/fine funzione
    * @param [in] callerFunc	Nome della funzione chiamante
    * @param [in] File			Nome base di log file
    * @param [in] Line			Numero riga da dove stampato messaggio
    * @param [in] Message		Messaggio da stampare
    */
   DllExport void tmTrace	(
      const int	  logId,
      const int   MsgType,
      const char *callerFunc,
      const char *File,
      const int   Line,
      const char *Message,
      ...
   );
#else
   /** @cond */
protected:
   LTrace();
   ~LTrace();

public:
   int tmLogInit		(
      const char *callerFunc,
      const char *trcfile = NULL,
      const int   level = DBG_LOW,
      const int   display = DISPLAY_NO,
      const int   dispLevel = DBG_LOW,
      const int	single = SINGLE_YES,
      const long  logLength = MAX_LOG_FILE
   );
   void tmLogLevel		( const int logId, const char *callerFunc, int level );
   void tmDispLevel	( const int logId, const char *callerFunc, int level );
   void tmStart		( const int logId, const char *callerFunc );
   void tmEnd			( const int logId, const char *callerFunc );
   void tmTrace		(
      const int	logId,
      const int   MsgType,
      const char *callerFunc,
      const char *File,
      const int   Line,
      const char *Message,
      ...
   );
   /** @endcond */
#endif
};

/**
* @class	LStartStop
* @brief	Gestisce i messaggi STARTED/ENDED.
*
* La classe LStartStop crea messaggio STARTED nel costruttore
* e messaggio ENDED in distrittore.
* 
* In questo modo si puo' creare solo il messaggio STARTED,
* mentre il messaggio ENDED verra' stampato automaticamente
* uscendo dallo scopo di visibilita
*/
class LStartStop {
   int id;
   char *cf;

public:
   // Constructor
   LStartStop(const int logId, const char *callerFunc )
   {
      id = logId;
      cf = (char *)callerFunc;
      LTrace::Instance().tmStart ( id, cf );
   }

   // Destructor
   ~LStartStop()
   {
      LTrace::Instance().tmEnd ( id, cf );
   }
};


/**
* @def				LogInit(a,b, ...)
* @brief			Inizializza log file.
* @param [in]	a	Nome base di log file
* @param [in]	b	Livello di debug
* @param [in]	...	Flag stampa sullo screen
* @param [in]	...	Lunghezza massima di log file
*/
#ifdef __SUNPRO_CC
#define LogInit(a,b, ...)	LTrace::Instance().tmLogInit ( __func__, a, b, ##__VA_ARGS__ )
#else
#define LogInit(a,b, ...)	LTrace::Instance().tmLogInit ( __FUNCTION__, a, b, ##__VA_ARGS__ )
#endif

/**
* @def				LogLevel
* @brief			Inizializza livello di logging.
* @param [in]	a	Livello di debug
*/
#ifdef __SUNPRO_CC
#define LogLevel(a)			LTrace::Instance().tmLogLevel ( 0, __func__, a )
#define hLogLevel(a,b)		LTrace::Instance().tmLogLevel ( a, __func__, b )
#else
#define LogLevel(a)			LTrace::Instance().tmLogLevel ( 0, __FUNCTION__, a )
#define hLogLevel(a,b)		LTrace::Instance().tmLogLevel ( a, __FUNCTION__, b )
#endif

/**
* @def				LogLevel
* @brief			Inizializza livello di logging.
* @param [in]	a	Livello di debug
*/
#ifdef __SUNPRO_CC
#define DispLevel(a)		LTrace::Instance().tmDispLevel ( 0, __func__, a )
#define hDispLevel(a,b)		LTrace::Instance().tmDispLevel ( a, __func__, b )
#else
#define DispLevel(a)		LTrace::Instance().tmDispLevel ( 0, __FUNCTION__, a )
#define hDispLevel(a,b)		LTrace::Instance().tmDispLevel ( a, __FUNCTION__, b )
#endif

/**
* @def				TraceStart
* @brief			Stampa messaggio di inizio della funzione.
*/
#ifdef __SUNPRO_CC
#define TraceStart()		LTrace::Instance().tmStart (0, __func__ )
#define hTraceStart(a)		LTrace::Instance().tmStart ( a, __func__ )
#else
//#define TraceStart()		LStartStop lss( 0, __FUNCTION__ )
//#define hTraceStart(a)		LStartStop lss( a, __FUNCTION__ )

#define hTraceStart(a, b )	LTrace::Instance().tmTrace ( a, b, __FUNCTION__, __FILE__, __LINE__, "START", 0 )
#define hTraceEnd(a, b )	LTrace::Instance().tmTrace ( a, b, __FUNCTION__, __FILE__, __LINE__, "END", 0 )

#endif

/**
* @def				Trace
* @brief			Stampa messaggio di trace/log formattato/non formattato.
* @param [in]	a	Tipo messaggio da stampare
* @param [in]	b	Formato per argomenti da stampare
* @param [in]	...	Argomenti da stampare
*/
#ifdef __SUNPRO_CC
#define Trace(a, b, ...)		LTrace::Instance().tmTrace ( 0, a, __func__, __FILE__, __LINE__, b, ##__VA_ARGS__ )
#define hTrace(a, b, c, ...)	LTrace::Instance().tmTrace ( a, b, __func__, __FILE__, __LINE__, c, ##__VA_ARGS__ )
#else
#define Trace(a, b, ...)		LTrace::Instance().tmTrace ( 0, a, __FUNCTION__, __FILE__, __LINE__, b, ##__VA_ARGS__ )
#define hTrace(a, b, c, ...)	LTrace::Instance().tmTrace ( a, b, __FUNCTION__, __FILE__, __LINE__, c, ##__VA_ARGS__ )
#endif

//#endif // TMTM_TRACE_H
