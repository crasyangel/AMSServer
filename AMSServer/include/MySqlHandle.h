#pragma once

#include "stdafx.h"

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//TODO: move mysql special method here
//use Mysql in muti-thread env
//better use 5.5 and later
//you should do it follow the page:
/*
//http://stackoverflow.com/questions/1455190/how-to-access-mysql-from-multiple-threads-concurrently
//http://dev.mysql.com/doc/refman/5.6/en/c-api-threaded-clients.html
*/

//and pay attention for this:
//Call mysql_library_init() once. you can place it in constructor
//Call mysql_library_end() once. you can place it in destructor
//and use a SingletonInstance Design Pattern member
//
//and for every thread, you should use mysql_init() at the beginning
//and  mysql_thread_end() at the end
//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!