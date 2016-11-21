//
// SQL related functions
//
mysql_connect(host,username,password := "");
mysql_query(connection,query,parameters := array{});
mysql_fetch_row(result);
mysql_affected_rows(result);
mysql_num_fields(result);
mysql_num_rows(result);
mysql_select_db(connection,database);
mysql_field_name(result,column_index);
mysql_close(connection);
