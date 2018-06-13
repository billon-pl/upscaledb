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

int main()
{
    ups_env_t* env = nullptr;
    ups_db_t* db = create_env( &env );


    const unsigned int item_count = 50;
    fill_db( db, item_count );


    unsigned int query = 0;

    ups_key_t key = ups_make_key( &query, sizeof( query ) );
    ups_status_t st = ups_db_erase(db, 0, &key, 0);
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_erase" );



    key = ups_make_key( &query, sizeof( query ) );
    ups_record_t record = {0};

    st = ups_db_find(db, 0, &key, &record, UPS_FIND_GEQ_MATCH);
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_find" );

    EXPECT_TRUE( 0 != *reinterpret_cast< unsigned int* >( key.data ), "Key not deleted" );


    st = ups_db_close(db, 0);
    EXPECT_TRUE( st == UPS_SUCCESS, "ups_db_close" );

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
