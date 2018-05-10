
//=====================================================================================================//
/**
* @file         boost_bidirectional_maps.h
* @brief        Boost based Multi Index containers.
* @author       Filippo Spina
* @version
* @copyright    2012 Selex S.I.
* @date         09/04/2012
*/
//=====================================================================================================//




#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>


using boost::multi_index_container;
using namespace boost::multi_index;


/* tags for accessing both sides of a bidirectional maps */

struct from_key_non_unique{};
struct from_key_second_non_unique{};
struct from_key_unique{};

struct from{};
struct to{};


template<typename FromType,typename ToType>
struct bidirectional_not_unique_values_unique_couple
{
	struct value_type
	{
		value_type(const FromType& first_,const ToType& second_):
	first(first_),second(second_)
	{}

    bool operator<(const value_type& _rRight) const
    {
        return (first < _rRight.first) || (second < _rRight.second);
    }

	FromType first;
	ToType   second;
	};

	typedef multi_index_container<
		value_type,
		indexed_by<
		ordered_non_unique<
		tag<from>,member<value_type,FromType,&value_type::first> >,
		ordered_non_unique<
		tag<to>,  member<value_type,ToType,&value_type::second> > ,
        ordered_unique<identity<value_type> > 
        >
	> type;

};

template<typename FromType,typename ToType>
struct bidirectional_unique_first
{
	struct value_type
	{
		value_type(const FromType& first_,const ToType& second_):
	first(first_),second(second_)
	{}
 

	FromType first;
	ToType   second;
	};

	typedef multi_index_container<
		value_type,
		indexed_by<
		ordered_unique<
		tag<from>,member<value_type,FromType,&value_type::first> >,
		ordered_non_unique<
		tag<to>,  member<value_type,ToType,&value_type::second> >
        >
	> type;

};

template<typename KeyType_non_unique, typename KeyType_unique, typename ToType>
struct bidirectional_double_key_second_unique_map
{
	struct value_type
	{
		value_type(const KeyType_non_unique& __rFirstKey, const KeyType_unique& __rSecondKey, const ToType& __val):
	key_non_unique(__rFirstKey), key_unique(__rSecondKey), value(__val)
	{}

	KeyType_non_unique key_non_unique;
    KeyType_unique     key_unique;
	ToType             value;
	};

	typedef multi_index_container<
		value_type,
		indexed_by<
		ordered_non_unique<
		tag<from_key_non_unique>,member<value_type,KeyType_non_unique,&value_type::key_non_unique> >,
        ordered_unique<
        tag<from_key_unique>,member<value_type,KeyType_unique, &value_type::key_unique > >,
		ordered_unique<
		tag<to>,  member<value_type,ToType,&value_type::value> >
		>
	> type;

};

template<typename KeyType_non_unique, typename KeyType_second_non_unique, typename ToType>
struct bidirectional_double_key_map
{
	struct value_type
	{
		value_type(const KeyType_non_unique& __rFirstKey, const KeyType_second_non_unique& __rSecondKey, const ToType& __val):
	key_non_unique(__rFirstKey), key_second_non_unique(__rSecondKey), value(__val)
	{}

	KeyType_non_unique              key_non_unique;
    KeyType_second_non_unique       key_second_non_unique;
	ToType                          value;
	};

	typedef multi_index_container<
		value_type,
		indexed_by<
		ordered_non_unique<
		tag<from_key_non_unique>,member<value_type,KeyType_non_unique,&value_type::key_non_unique> >,
        ordered_non_unique<
        tag<from_key_second_non_unique>,member<value_type,KeyType_second_non_unique, &value_type::key_second_non_unique > >,
		ordered_unique<
		tag<to>,  member<value_type,ToType,&value_type::value> >
		>
	> type;

};


template<typename FromType,typename ToType>
struct bidirectional_map
{
	struct value_type
	{
		value_type(const FromType& first_,const ToType& second_):
	first(first_),second(second_)
	{}

	FromType first;
	ToType   second;
	};

	typedef multi_index_container<
		value_type,
		indexed_by<
		ordered_unique<
		tag<from>,member<value_type,FromType,&value_type::first> >,
		ordered_unique<
		tag<to>,  member<value_type,ToType,&value_type::second> >
		>
	> type;

};

