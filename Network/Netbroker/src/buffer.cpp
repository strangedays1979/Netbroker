

// [ System Includes ] //
#include <stdlib.h>

// [ User Includes ] //
#include "INetBroker.h"

Buffer::Buffer
( const unsigned long&      __rSize
 , const char* const        __pData
 , const std::string&       __rGroupName
 , const short&             __rOpCode
 )
 : m_data((char*) calloc(m_size, sizeof(char) ))
 , m_size( __rSize )
 , m_groupName( __rGroupName )
 , m_opCode(__rOpCode)
 , m_type(DEFAULT)
{
    memcpy(m_data, __pData, m_size);
}




Buffer::Buffer()
: m_size(0)
, m_data(0)
, m_opCode(0)
, m_type(DEFAULT)
{

}


Buffer::~Buffer()
{
    erase();

}


Buffer::Buffer(const Buffer& __ref)
: m_size( __ref.m_size )
, m_data( (char*) calloc(m_size, sizeof(char) ))
, m_opCode( __ref.m_opCode )
, m_groupName( __ref.m_groupName)
, m_type( __ref.m_type)
{
    memcpy(m_data, __ref.m_data, m_size);
}

Buffer& Buffer::operator=(const Buffer& __right)
{
    erase();

    m_size = __right.m_size;
    m_data = new char[m_size];
    memcpy(m_data, __right.m_data, m_size);

    m_opCode = __right.m_opCode;
    m_groupName = __right.m_groupName;
    m_type = __right.m_type;

    return *this;

}



void Buffer::erase()
{

    if ( m_data != 0 )
    {
        free( m_data );
        m_data = 0;
    }

    m_size = 0;
    m_opCode = 0;
    m_type = DEFAULT;
    m_groupName.erase();
    m_sender.erase();

}

