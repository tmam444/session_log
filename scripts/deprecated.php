<?php

/**
 * Sends a query to the currently active database on the server that's associated with the specified link identifier.
 *
 * @param string $query The query, as a string.
 * @param resource $link_identifier The MySQL connection. If the link identifier is not specified, the last link opened by mysql_connect() is assumed. If no such link is found, it will try to create one as if mysql_connect() was called with no arguments. If no connection is found or established, an E_WARNING level error is generated.
 *
 * @return mixed Returns true on success or false on failure. For SELECT, SHOW, DESCRIBE, EXPLAIN and other statements returning resultset, mysql_query() returns a resource on success, or false on error.
 */
function mysql_query($query, $link_identifier = null) {
    // This is a dummy implementation, do not add any code here
}

/**
 * Open a connection to a MySQL Server
 *
 * @param string $server
 * @param string $username
 * @param string $password
 * @return resource
 */
function mysql_connect($server, $username, $password) {}

/**
 * Select a MySQL database
 *
 * @param string $database_name
 * @return bool
 */
function mysql_select_db($database_name) {}

/**
 * Get a result row as an enumerated array
 *
 * @param resource $result
 * @return array|false
 */
function mysql_fetch_row($result) {}

/**
 * Free result memory
 *
 * @param resource $result
 * @return bool
 */
function mysql_free_result($result) {}

/**
 * Get number of rows in result
 *
 * @param resource $result
 * @return int|false
 */
function mysql_num_rows($result) {}

// ... other deprecated mysql_* functions
