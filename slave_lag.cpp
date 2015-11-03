/**
 * @file server_lag.cpp  Create high INSERT load to create slave lag and check that Maxscale start routing queries to Master
 *
 * - in Maxscqale.cnf set max_slave_replication_lag=20
 * - in parallel thread execute as many INSERTs as possible
 * - using "select @@server_id;" check that queries go to one of the slave
 * - wait when slave lag > 20 (control lag using maxadmin interface)
 * - check that now queries go to Master
 */

#include <my_config.h>
#include "testconnections.h"
#include "sql_t1.h"
#include "maxadmin_operations.h"

char sql[1000000];

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
int exit_flag = 0;
void *query_thread( void *ptr );
void *checks_thread( void *ptr);

TestConnections * Test;

int main(int argc, char *argv[])
{
    char result[1024];
    char server_id[1024];
    char server1_id[1024];
    int res_d;
    int server1_id_d;
    int server_id_d;
    int rounds = 0;

    Test = new TestConnections(argc, argv);
    Test->set_timeout(2000);

    Test->repl->connect();
    Test->connect_rwsplit();

    // connect to the MaxScale server (rwsplit)

    if (Test->conn_rwsplit == NULL ) {
        printf("Can't connect to MaxScale\n");
        Test->copy_all_logs();
        exit(1);
    } else {

        create_t1(Test->conn_rwsplit);

        create_insert_string(sql, 50000, 1);
        Test->tprintf("sql_len=%lu\n", strlen(sql));
        Test->try_query(Test->conn_rwsplit, sql);

        pthread_t threads[1000];
        //pthread_t check_thread;
        int  iret[1000];
        //int check_iret;
        int j;
        exit_flag=0;
        /* Create independent threads each of them will execute function */
        for (j=0; j<32; j++) {
            iret[j] = pthread_create( &threads[j], NULL, query_thread, &sql);
        }

        execute_query(Test->conn_rwsplit, (char *) "select @@server_id; -- maxscale max_slave_replication_lag=20");

        find_field(Test->repl->nodes[0], (char *) "select @@server_id;", (char *) "@@server_id", &server1_id[0]);
        sscanf(server1_id, "%d", &server1_id_d);

        do {
            get_maxadmin_param(Test->maxscale_IP, (char *) "admin", Test->maxadmin_password, (char *) "show server server2", (char *) "Slave delay:", result);
            sscanf(result, "%d", &res_d);
            Test->tprintf("server2 lag: %d\n", res_d);
            find_field(Test->conn_rwsplit, (char *) "select @@server_id; -- maxscale max_slave_replication_lag=20", (char *) "@@server_id", &server_id[0]);
            sscanf(server_id, "%d", &server_id_d);
            Test->tprintf("%d\n", server_id_d);
            if ((rounds < 10) and (server1_id_d == server_id_d)) {
                Test->add_result(1, "Connected to the master!\n");
            } else {
                Test->tprintf("Connected to slave\n");
            }
            rounds++;
        } while (res_d < 21);

        exit_flag = 1;

        if (server1_id_d != server_id_d) {
            Test->tprintf("Master id is %d\n", server1_id_d);
            Test->add_result(1, "Lag is big, but connection is done to server with id %d\n", server_id_d);
        } else {
            Test->tprintf("Connected to master\n");
        }
        // close connections
        Test->close_rwsplit();
    }
    Test->repl->close_connections();

    Test->copy_all_logs(); return(Test->global_result);
}


void *query_thread( void *ptr )
{
    MYSQL * conn;
    conn = open_conn(Test->repl->port[0], Test->repl->IP[0], Test->repl->user_name, Test->repl->password, Test->repl->ssl);
    while (exit_flag == 0) {
        //execute_query(conn, (char *) "INSERT into t1 VALUES(1, 1)");
        execute_query(conn, (char *) ptr);
    }
    return NULL;
}

void *checks_thread( void *ptr )
{
    char result[1024];
    for (int i = 0; i < 1000; i++) {
        get_maxadmin_param(Test->maxscale_IP, (char *) "admin", Test->maxadmin_password, (char *) "show server server2", (char *) "Slave delay:", result);
        printf("server2: %s\n", result);
        get_maxadmin_param(Test->maxscale_IP, (char *) "admin", Test->maxadmin_password, (char *) "show server server3", (char *) "Slave delay:", result);
        printf("server3: %s\n", result);
        get_maxadmin_param(Test->maxscale_IP, (char *) "admin", Test->maxadmin_password, (char *) "show server server4", (char *) "Slave delay:", result);
        printf("server4: %s\n", result);
    }
    exit_flag = 1;
    return NULL;
}

