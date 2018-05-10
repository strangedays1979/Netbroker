

/**
* @file         LogReport.cpp
* @brief        Log Report implementations.
* @author       Filippo Spina
* @version      1.00
* @date         16/09/2011
*/



#include "log_service.h"

#include <time.h>



bool LogReport::unserialize
( const unsigned char*  __pInBuffer
 , const unsigned&       __rInBufferSize
 , LogReport*&            __rpReturnedLog
 )
{

    if( __rpReturnedLog )
        return false;

    LogType     logType;
    __int64     date;
    __int32     procID;

    unsigned    bufferIterator = 0;
    unsigned    currentDataSize = 0;

    memcpy(&logType, __pInBuffer, sizeof(LogType) );
    bufferIterator += sizeof(LogType);

    memcpy(&date, (void*) &__pInBuffer[bufferIterator], sizeof(__int64) );
    bufferIterator+=sizeof(__int64);

    memcpy(&procID, (void*) &__pInBuffer[bufferIterator], sizeof(__int32) );
    bufferIterator+=sizeof(__int32);

    currentDataSize = *(unsigned*) (__pInBuffer + bufferIterator);
    std::string groupName( (char*) &__pInBuffer[bufferIterator+ sizeof(unsigned )]
    , currentDataSize );
    bufferIterator += currentDataSize + sizeof( unsigned );

    currentDataSize = *(unsigned*) (__pInBuffer + bufferIterator);
    std::string topic( (char*) &__pInBuffer[bufferIterator+ sizeof(unsigned )]
    , currentDataSize );
    bufferIterator += currentDataSize + sizeof( unsigned );

    __rpReturnedLog = new LogReport(logType, topic );

    __rpReturnedLog->getSourceRef() = groupName;
    __rpReturnedLog->getUTCtime() = date;
    __rpReturnedLog->getProcIDRef() = procID;


    currentDataSize = *(unsigned*) (__pInBuffer + bufferIterator);
    std::string detailsStr( (char*) &__pInBuffer[bufferIterator+ sizeof(unsigned )]
    , currentDataSize );

    __rpReturnedLog->getrDetailStream() << detailsStr;


    return true;
}


void LogReport::getFormattedReport( const std::string& __rSeverityMsg, std::ostringstream& __rStreamBuf ) const
{

    struct tm * pTimeInfo = 0;
    pTimeInfo = localtime  ( &getUTCtime() );

    __rStreamBuf.str("");

    __rStreamBuf << "\n********* Incoming " << __rSeverityMsg <<" Log Report *********"
        << "\n* Generated on:\t" << asctime(pTimeInfo)
        << "* From Source:\t" << getSourceRef()
        << "\n* Process ID:\t" << getProcIDRef()
        << "\n* Title:\t" << getTopicRef()
        << "\n* Body:\n" << getrDetailStream().str()
        << "\n*********************************************\n";

}

unsigned char*   LogReport::serialize(unsigned& __rReturnedSize)  const
{

    /** Start buffer ensemble **/

    unsigned bufSize(0);
    bufSize += sizeof(LogType );
    bufSize += sizeof(__int64);
    bufSize += sizeof(__int32);
    bufSize += m_source.size() + sizeof(unsigned );
    bufSize += m_topic.size() + sizeof(unsigned );
    bufSize += m_details.str().size() + sizeof(unsigned );

    unsigned char* pOutputBuffer = new unsigned char[bufSize];
    __rReturnedSize = bufSize;

    memset(pOutputBuffer, 0x00, bufSize);

    bufSize = 0;

    memcpy( (void*) &pOutputBuffer[bufSize], &m_logType, sizeof( LogType ) );
    bufSize += sizeof( LogType );

    memcpy( (void*) &pOutputBuffer[bufSize], &m_date, sizeof( __int64 ) );
    bufSize += sizeof( __int64 );

    memcpy( (void*) &pOutputBuffer[bufSize], &m_procID, sizeof( __int32 ) );
    bufSize += sizeof( __int32 );

    const unsigned& groupSize =     m_source.size();
    const unsigned& topicSize =     m_topic.size();
    const unsigned& detailsSize =   m_details.str().size();

    memcpy( (void*) &pOutputBuffer[bufSize], &groupSize, sizeof(unsigned ) );
    bufSize += sizeof(unsigned );
    memcpy( (void*) &pOutputBuffer[bufSize], m_source.c_str(), groupSize );
    bufSize += groupSize;

    memcpy( (void*) &pOutputBuffer[bufSize], &topicSize, sizeof(unsigned ) );
    bufSize += sizeof(unsigned );
    memcpy( (void*) &pOutputBuffer[bufSize], m_topic.c_str(), topicSize );
    bufSize += topicSize;

    memcpy( (void*) &pOutputBuffer[bufSize], &detailsSize, sizeof(unsigned ) );
    bufSize += sizeof(unsigned );
    memcpy( (void*) &pOutputBuffer[bufSize], m_details.str().c_str(), detailsSize );


    return pOutputBuffer;

}


LogReport::LogReport( const LogReport& __right )
: m_logType( __right.m_logType )
, m_topic( __right.m_topic )
{


}
