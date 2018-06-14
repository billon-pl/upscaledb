//
// Test case for the issue #105 of original Christoph Rupp ubscale db
// This is the issue #8 in billon-pl/upscale repository
//


#include <iostream>
#include <ups/upscaledb.hpp>
#include <cstdlib>

#define EXPECT_TRUE( st, txt )     \
if( !st )                          \
{                                  \
    std::cout << txt << std::endl; \
    exit( 1 );                     \
}

ups_db_t* create_env( ups_env_t** env );
void fill_db( ups_db_t* db, ups_txn_t* txn );
void erase_key( ups_db_t* db, ups_txn_t* txn, uint32_t query );
void find_key( ups_db_t* db, ups_txn_t* txn, uint32_t query );
void close_env( ups_env_t *env, ups_db_t* db );
ups_txn_t* create_txn( ups_env_t* env );

//
//
//
int main()
{
    const uint32_t query = 0;

    ups_env_t* env = nullptr;
    ups_db_t* db = create_env( &env );

    ups_txn_t* txn = create_txn( env );

    fill_db( db, txn );
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
    ups_status_t st = ups_txn_begin( &txn, env, NULL, NULL, 0 );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_txn_begin" );
    return txn;
}


//
//
//
void fill_db( ups_db_t* db, ups_txn_t* txn )
{
    const uint32_t item_count = 50;

    for( uint32_t n = 0; n < item_count; n++ )
    {
        ups_key_t key = ups_make_key( &n, sizeof( n ) );

        ups_record_t record;
        bzero( &record, sizeof( record ) );

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
void erase_key( ups_db_t* db, ups_txn_t* txn, uint32_t query )
{
    ups_key_t key = ups_make_key( &query, sizeof( query ) );

    ups_status_t st = ups_db_erase(db, txn, &key, 0);
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_erase, UPS_SUCCESS" );

    st = ups_db_erase(db, txn, &key, 0);
    EXPECT_TRUE( st == UPS_KEY_NOT_FOUND, "ups_db_erase, UPS_KEY_NOT_FOUND" );

    if( txn )
    {
        ups_txn_commit( txn, 0 );
    }
}

//
//
//
void find_key( ups_db_t* db, ups_txn_t* txn, uint32_t query )
{
    ups_key_t key = ups_make_key( &query, sizeof( query ) );

    ups_record_t record;
    bzero( &record, sizeof( record ) );

    ups_status_t st = ups_db_find( db, txn, &key, &record, UPS_FIND_GEQ_MATCH );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_find" );


    // std::cout << *reinterpret_cast< uint32_t* >( key.data ) << std::endl;

    EXPECT_TRUE( query != *reinterpret_cast< uint32_t* >( key.data ), "Key not deleted" );
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
