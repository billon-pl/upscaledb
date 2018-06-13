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

ups_db_t* create_env( ups_env_t **env );
void fill_db( ups_db_t* db, unsigned int item_count );
void erase_key( ups_db_t* db, unsigned int query );
void find_key( ups_db_t* db, unsigned int query );
void close_env( ups_env_t *env, ups_db_t* db );

//
//
//
int main()
{
    const unsigned int query = 0;


    const unsigned int item_count = 50;

    for( unsigned int n = 2; n < item_count; n++ )
    {
        // std::cout << n << "  " << std::flush << std::endl;

        ups_env_t* env = nullptr;
        ups_db_t* db = create_env( &env );
        fill_db( db, item_count );
        erase_key( db, query );
        find_key( db, query );
        close_env( env, db );
    }
    return 0;
}

//
//
//
ups_db_t* create_env( ups_env_t **env )
{
    const std::string db_name( "test.db" );
    const uint32_t mode = 0664;

    ups_status_t st = ups_env_create( env, db_name.c_str(), UPS_ENABLE_TRANSACTIONS, mode, 0 );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_env_create, UPS_ENABLE_TRANSACTIONS" );

    //ups_status_t st = ups_env_create( env, db_name.c_str(), 0, mode, 0 );
    //EXPECT_TRUE( st == UPS_SUCCESS, "ups_env_create" );

    ups_parameter_t params[] =
    {
        { UPS_PARAM_KEY_TYPE, UPS_TYPE_UINT32 },
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
void fill_db( ups_db_t* db, unsigned int item_count )
{
    for( unsigned int i = 0; i < item_count; i++ )
    {
        ups_key_t key = ups_make_key(&i, sizeof(i));
        ups_record_t record = {0};

        const ups_status_t st = ups_db_insert(db, 0, &key, &record, 0);
        EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_insert" );
    }
}

//
//
//
void erase_key( ups_db_t* db, unsigned int query )
{
    ups_key_t key = ups_make_key( &query, sizeof( query ) );

    ups_status_t st = ups_db_erase(db, 0, &key, 0);
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_erase, UPS_SUCCESS" );

    st = ups_db_erase(db, 0, &key, 0);
    EXPECT_TRUE( st == UPS_KEY_NOT_FOUND, "ups_db_erase, UPS_KEY_NOT_FOUND" );
}

//
//
//
void find_key( ups_db_t* db, unsigned int query )
{
    ups_key_t key = ups_make_key( &query, sizeof( query ) );

    ups_record_t record;
    bzero( &record, sizeof( record ) );

    ups_status_t st = ups_db_find( db, 0, &key, &record, UPS_FIND_GEQ_MATCH );
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_find" );


    // std::cout << *reinterpret_cast< unsigned int* >( key.data ) << std::endl;

    EXPECT_TRUE( query != *reinterpret_cast< unsigned int* >( key.data ), "Key not deleted" );
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
