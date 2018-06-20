//
// Test case for the issue #105 of original Christoph Rupp ubscale db
// This is the issue #8 in billon-pl/upscale repository
//


#include <iostream>
#include <ups/upscaledb.hpp>
#include <cstdlib>
#include <vector>
#include <random>
#include <algorithm>
#include <string>
#include <utility>
#include <limits>
#include <map>

//
//
//
#define EXPECT_TRUE( st, txt )     \
if( !( st ) )                      \
{                                  \
    std::cout << txt << std::endl; \
    exit( 1 );                     \
}


//
//
//
using DataType = std::map< std::string, std::string >;

//
//
//
ups_db_t* create_env( ups_env_t** env );
void fill_db( ups_db_t* db, ups_txn_t* txn, const DataType& data );
void erase_key( ups_db_t* db, ups_txn_t* txn, const DataType& query );
void find_key( ups_db_t* db, ups_txn_t* txn, const DataType& query );
void close_env( ups_env_t *env, ups_db_t* db );
ups_txn_t* create_txn( ups_env_t* env );
std::string get_random_string( std::size_t max_length );
DataType generate_data();
DataType generate_query( const DataType& data );

//
//
//
int main()
{
    const DataType data = generate_data();
    const DataType query = generate_query( data );

    ups_env_t* env = nullptr;
    ups_db_t* db = create_env( &env );

    ups_txn_t* txn = create_txn( env );

    fill_db( db, txn, data );
    erase_key( db, txn, query );
    find_key( db, txn, query );
    close_env( env, db );

    return 0;
}

//
//
//
ups_db_t* create_env( ups_env_t **env )
{
    const std::string db_name( "test.db" );
    const uint32_t mode = 0664;

    // const uint32_t flags = 0;
    // const uint32_t flags = UPS_ENABLE_TRANSACTIONS | UPS_ENABLE_FSYNC | UPS_DISABLE_MMAP | UPS_DISABLE_RECOVERY;
    const uint32_t flags = UPS_ENABLE_TRANSACTIONS;

    ups_status_t st = ups_env_create( env, db_name.c_str(), flags, mode, 0 );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_env_create, UPS_ENABLE_TRANSACTIONS" );

    //ups_status_t st = ups_env_create( env, db_name.c_str(), 0, mode, 0 );
    //EXPECT_TRUE( st == UPS_SUCCESS, "ups_env_create" );

    ups_parameter_t params[] =
    {
        { UPS_PARAM_KEY_TYPE, UPS_TYPE_BINARY },
        { UPS_PARAM_KEY_SIZE, UPS_KEY_SIZE_UNLIMITED },
        { UPS_PARAM_RECORD_TYPE, UPS_TYPE_BINARY},
        { UPS_PARAM_RECORD_SIZE, UPS_RECORD_SIZE_UNLIMITED },
        { 0, }
    };

    ups_db_t* db = nullptr;
    st = ups_env_create_db( *env, &db, 1, 0, &params[ 0 ] );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_env_create_db" );

    return db;
}

//
//
//
ups_txn_t* create_txn( ups_env_t* env )
{
    ups_txn_t* txn = nullptr;
    ups_status_t st = ups_txn_begin( &txn, env, nullptr, nullptr, 0 );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_txn_begin" );
    return txn;
}


//
//
//
void fill_db( ups_db_t* db, ups_txn_t* txn, const DataType& data )
{

    for( auto v : data )
    {
        const std::string sk = v.first;
        const std::string sr = v.second;

        // std::cout << "to insert: " << sk << std::endl;

        ups_key_t key = ups_make_key( (void*)sk.c_str(), (uint16_t)(sk.size() + 1) );
        ups_record_t record = ups_make_record( (void*)sr.c_str(), (uint16_t)(sr.size() + 1) );

        const ups_status_t st = ups_db_insert( db, txn, &key, &record, 0 );
        EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_insert" );
    }

    if( txn )
    {
        ups_txn_commit( txn, 0 );
    }
}

//
//
//
void erase_key( ups_db_t* db, ups_txn_t* txn, const DataType& query )
{

    for( auto q : query )
    {
        const std::string sk = q.first;
        // const std::string sr = q.second;

        // std::cout << "to erase: " << q << std::endl;

        ups_key_t key = ups_make_key( (void*)sk.c_str(), (uint16_t)(sk.size() + 1) );

        ups_status_t st = ups_db_erase( db, txn, &key, 0 );
        EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_erase, UPS_SUCCESS" );

        st = ups_db_erase( db, txn, &key, 0 );
        EXPECT_TRUE( st == UPS_KEY_NOT_FOUND, "ups_db_erase, UPS_KEY_NOT_FOUND" );
    }

    if( txn )
    {
        ups_txn_commit( txn, 0 );
    }
}

//
//
//
void find_key( ups_db_t* db, ups_txn_t* txn, const DataType& query )
{
    for( auto q : query )
    {
        const std::string sk = q.first;
        // std::cout << "to find: " << q << std::endl;

        ups_key_t key = ups_make_key( (void*)sk.c_str(), (uint16_t)(sk.size() + 1) );

        ups_record_t rec;
        bzero( &rec, sizeof( rec ) );

        ups_status_t st = ups_db_find( db, txn, &key, &rec, UPS_FIND_GEQ_MATCH );
        if ( st == UPS_SUCCESS )
        {
            const char* key_data = reinterpret_cast< char* >( key.data );
            const std::string ret( key_data );
            EXPECT_TRUE( sk != ret , "Key not deleted, B" );
        }
    }
}

//
//
//
void close_env( ups_env_t* env, ups_db_t* db )
{
    ups_status_t st = ups_db_close( db, 0 );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_close" );

    st = ups_env_close( env, UPS_AUTO_CLEANUP );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_env_close" );
}


//
//
//
DataType generate_data()
{
    const std::size_t max_length_key = 200;
    const std::size_t max_length_record = 15000;

    const std::size_t item_count = 10000;

    static_assert( max_length_key < std::numeric_limits< uint16_t >::max(), "Key too long" );
    static_assert( max_length_record < std::numeric_limits< uint16_t >::max(), "Record too long" );

    DataType data;

    while( data.size() < item_count )
    {
        const std::string sk = get_random_string( max_length_key );
        const std::string sr = get_random_string( max_length_record );

        // std::cout << sk << std::endl;
        // std::cout << sr << std::endl << std::endl;

        data[ sk ] = sr;
    }
    std::cout << "Item count: " << data.size() << std::endl;
    std::cout << "Max key length: " << max_length_key << std::endl;
    std::cout << "Max record length: " << max_length_record << std::endl;

    return data;
}

//
//
//
DataType generate_query( const DataType& data )
{
    DataType query;

    std::size_t i = 0;
    for( auto v : data )
    {
        // take every second item
        if( i % 2 == 0 )
        {
            query.insert( v );
        }
        i++;
    }
    return query;

}

//
//
//
std::string get_random_string( std::size_t max_length )
{
    const std::string alphabet( "0123456789"
                                "abcdefghijklmnopqrstuvwxyz"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ" );

    std::random_device rd;
    std::mt19937 g( rd() );
    // std::mt19937 g( rand() );
    std::uniform_int_distribution< std::size_t > pick( 0, alphabet.size() - 1 );


    std::uniform_int_distribution< std::size_t > pick_length( 1, max_length + 1 );
    std::string::size_type length = pick_length( g );
    assert( length > 0 );
    std::string s;
    s.reserve( length );

    while( length-- )
    {
        const char c = alphabet[ pick( g ) ];
        s += c;
    }
    return s;
}
