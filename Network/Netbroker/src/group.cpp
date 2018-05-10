

// [ User Includes ] //
#include "INetBroker.h"

Group::Group( const std::string& __rName, const long& __rMaxBufferSize, const long& __rQueueSize)
: m_name( __rName )
, m_maxBufferSize( __rMaxBufferSize)
, m_queueSize( __rQueueSize )
{


}

Group::Group( const Group& __ref)
: m_name( __ref.m_name )
, m_maxBufferSize( __ref.m_maxBufferSize )
, m_queueSize( __ref.m_queueSize )
{
    
   

}


Group& Group::operator=(const Group& __right)
{
    m_name = __right.m_name;
    m_maxBufferSize = __right.m_maxBufferSize;
    m_queueSize = __right.m_queueSize;

    return *this;
}

bool Group::createBuffer
(const unsigned long&           __rSize
 , const char* const            __pData
 , const short&                 __rOpCode
 , Buffer&                      __rInto
 )
{

    if( __rSize > m_maxBufferSize )
        return false;

    __rInto.erase();

    __rInto.m_size      = __rSize;
    __rInto.m_data = (char*) calloc( __rSize, sizeof(char) );
    memcpy( __rInto.m_data, __pData, __rSize );
    __rInto.m_groupName = m_name;
    __rInto.m_opCode = __rOpCode;
    __rInto.m_type = Buffer::REGULAR;

    return true;


}


