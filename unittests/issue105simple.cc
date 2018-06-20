//
// Test case for the issue #105 of original Christoph Rupp ubscale db
// This is the issue #8 in billon-pl/upscale repository
//


#include <iostream>
#include <ups/upscaledb.hpp>
#include <cstdlib>
#include <vector>

#define EXPECT_TRUE( st, txt )     \
if( !( st ) )                      \
{                                  \
    std::cout << txt << std::endl; \
    exit( 1 );                     \
}

ups_db_t* create_env( ups_env_t** env );
void fill_db( ups_db_t* db, ups_txn_t* txn, const std::vector< uint32_t >& data );
void erase_key( ups_db_t* db, ups_txn_t* txn, const std::vector< uint32_t >& query );
void find_key( ups_db_t* db, ups_txn_t* txn, const std::vector< uint32_t >& query );
void close_env( ups_env_t *env, ups_db_t* db );
ups_txn_t* create_txn( ups_env_t* env );

std::vector< uint32_t > generate_data();
std::vector< uint32_t > generate_query( const std::vector< uint32_t >& data );

//
//
//
int main()
{
    const std::vector< uint32_t > data = generate_data();
    const std::vector< uint32_t > query = generate_query( data );

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
        { UPS_PARAM_KEY_TYPE, UPS_TYPE_UINT32 },
        { UPS_PARAM_RECORD_SIZE, 0 },
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
void fill_db( ups_db_t* db, ups_txn_t* txn, const std::vector< uint32_t >& data )
{
    for( uint32_t v : data )
    {
        // std::cout << "to insert: " << v << std::endl;

        ups_key_t key = ups_make_key( &v, sizeof( v ) );
        ups_record_t record = ups_make_record( nullptr, 0 );

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
void erase_key( ups_db_t* db, ups_txn_t* txn, const std::vector< uint32_t >& query )
{

    for( uint32_t q : query )
    {
        // std::cout << "to erase: " << q << std::endl;

        ups_key_t key = ups_make_key( &q, sizeof( q ) );

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
void find_key( ups_db_t* db, ups_txn_t* txn, const std::vector< uint32_t >& query )
{
    for( uint32_t q : query )
    {
        // std::cout << "to find: " << q << std::endl;

        ups_key_t key = ups_make_key( &q, sizeof( q ) );

        ups_record_t rec;
        bzero( &rec, sizeof( rec ) );

        ups_status_t st = ups_db_find( db, txn, &key, &rec, UPS_FIND_GEQ_MATCH );
        if ( st == UPS_SUCCESS )
        {
            const uint32_t key_data = *reinterpret_cast< uint32_t* >( key.data );
            EXPECT_TRUE( q != key_data , "Key not deleted, B" );
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
std::vector< uint32_t > generate_data()
{
    const std::size_t item_count = 100;

    std::vector< uint32_t > data;
    data.reserve( item_count );

    for( std::size_t i = 0; i < item_count; i++ )
    {
        data.push_back( i );
    }

    return data;
}

//
//
//
std::vector< uint32_t > generate_query( const std::vector< uint32_t >& data )
{
    std::vector< uint32_t > query;

    for( std::size_t i = 0; i < data.size(); i++ )
    {
        // take every second item
        if( i % 2 == 0 )
        {
            query.push_back( data[ i ] );
        }
    }
    return query;

}
